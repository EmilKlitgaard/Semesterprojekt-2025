#include "Stockfish.h"

Stockfish::Stockfish(const string& engine_path) : enginePath(engine_path) {
    startEngine();
}

Stockfish::~Stockfish() {
    stopEngine();
}

void Stockfish::startEngine() {
    int stdin_pipe[2];
    int stdout_pipe[2];

    if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0) {
        throw runtime_error("Pipe creation failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw runtime_error("Fork failed");
    }

    if (pid == 0) {
        close(stdin_pipe[1]);  // Close write end of stdin pipe
        close(stdout_pipe[0]); // Close read end of stdout pipe

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);

        execlp(enginePath.c_str(), enginePath.c_str(), (char*)NULL);
        exit(EXIT_FAILURE);
    } else {
        close(stdin_pipe[0]);  // Close read end of stdin pipe
        close(stdout_pipe[1]); // Close write end of stdout pipe

        // Store pipe file descriptors
        engine_stdin = stdin_pipe[1];
        engine_stdout = stdout_pipe[0];
        engine_pid = pid;

        // Wait for Stockfish to start and print its initial output
        string response = readResponse("Stockfish");
        cout << "Initial output: " << response << endl;
        
        sendCommand("uci");
        cout << "Waiting for uciok..." << endl;
        string uciResponse = readResponse("uciok");
        cout << "Received uciok." << endl;
        
        sendCommand("isready");
        cout << "Waiting for readyok..." << endl;
        string readyResponse = readResponse("readyok");
        cout << "Stockfish engine started successfully." << endl;
    }
}

void Stockfish::stopEngine() {
    if (engine_pid != -1) {
        sendCommand("quit");
        close(engine_stdin);
        close(engine_stdout);
        waitpid(engine_pid, NULL, 0);
        engine_pid = -1;
    }
}

void Stockfish::sendCommand(const string& command) {
    if (engine_pid == -1) {
        throw runtime_error("Engine not running");
    }
    cout << "Sending command: " << command << endl; // Debugging line
    write(engine_stdin, (command + "\n").c_str(), command.size() + 1);
}

string Stockfish::readResponse(const string& findString) {
    char buffer[4096];
    string response;
    while (true) {
        ssize_t bytes_read = read(engine_stdout, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) { break; }

        buffer[bytes_read] = '\0';
        response += buffer;
        //cout << "Received chunk: " << buffer << endl; // Debugging line

        // Stop reading if we have a complete response (ends with newline)
        if (response.find(findString) != string::npos) { break; }
    }
    return response;
}

string Stockfish::getBestMove(const string& latestMove) {
    // Send the position commands
    sendCommand("position startpos moves " + latestMove);
    sendCommand("go movetime 1000");

    string response = readResponse("bestmove");

    // Debugging: Print the raw response
    //cout << "Raw Stockfish response: " << response << endl;

    // Extract the best move from the response
    size_t bestmovePos = response.find("bestmove");
    if (bestmovePos == string::npos) {
        throw runtime_error("Stockfish did not return a valid move: " + response);
    }

    // Extract the move (e.g., "bestmove e7e5")
    string move = response.substr(bestmovePos + 9, 4);
    return move;
}
