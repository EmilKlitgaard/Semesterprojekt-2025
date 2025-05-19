#include "Stockfish.h"
#include "GUI.h"

Stockfish::Stockfish(const string& enginePath) : enginePath(enginePath) {
    cout << "Starting Stockfish engine..." << endl;
    startEngine();
}

Stockfish::~Stockfish() {
    stopEngine();
}


void Stockfish::startEngine() {
    process = make_unique<boost::process::child> (enginePath, boost::process::std_in < stockfishIn, boost::process::std_out > stockfishOut);
    if (!process->running()) {
        cerr << "Kunne ikke starte Stockfish.\n";
        throw runtime_error("Start Failed");
    }
    sendCommand("uci");
    while (readResponse() != "uciok");
    sendCommand("isready");
    while (readResponse() != "readyok");
    sendCommand("setoption UCI_LimitStrength value true");
    cout << "Stockfish succesfully started." << endl;
}

void Stockfish::stopEngine() {
    // Afslutter Stockfish motoren pænt
    if (process && process->running()) {
        sendCommand("quit");
        process->wait();
    }
}

void Stockfish::sendCommand(const string& cmd) {
    stockfishIn << cmd << endl;
    stockfishIn.flush();
}

string Stockfish::readResponse() {
    string response;
    getline(stockfishOut, response);
    return response;
}

// Update the current move history
void Stockfish::addMoveToHistory(const string& move) {
    moveHistory.push_back(move);
}

// Get the current move history
vector<string> Stockfish::getMoveHistory() const {
    return moveHistory;
}

// Print the current move history
void Stockfish::printMoveHistory() const {
    cout << "Movehistory: ";
    for (const string& move : moveHistory) {
        cout << move << " ";
    }
    cout << endl;
}

// Method to check for checkmate
bool Stockfish::isCheckmate() { 
    ostringstream position;
    position << "position startpos moves";
    for (const string& move : moveHistory) {
        position << " " << move;
    }
    sendCommand(position.str());
    
    // Request quick analysis
    sendCommand("go depth 1");
    string response;

    while (true) {
        response = readResponse();
        if (response.find("bestmove") == 0) {
            // Check for mate indicator
            return response.find("(none)") != string::npos;
        }
    }    
}

// Check if the move is legal. If the move is legal, add it to the move history
bool Stockfish::sendValidMove(const string& move) {
    ostringstream position;
    position << "position startpos moves";
    for (const string& moves : moveHistory) {
        position << " " << moves;
    }

    cout << "Stockfish moves: " << position.str() << endl;

    sendCommand(position.str());

    sendCommand("go perft 1");

    vector<string> legalMoves;

    while (true) {
        string response = readResponse();
        if (response.empty()) continue;

        size_t colon = response.find(':');
        if (colon != string::npos) {
            string legalMove = response.substr(0, colon);
            legalMove.erase(0, legalMove.find_first_not_of(" \t\r\n"));
            legalMove.erase(legalMove.find_last_not_of(" \t\r\n") + 1);
            legalMoves.push_back(legalMove);
        }

        if (response.find("Nodes searched") != string::npos) {
            break;
        }
    }

    if (find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end()) {
        addMoveToHistory(move);
        return true;
    }

    return false;
}

// Returns the best move from Stockfish
string Stockfish::getBestMove() {
    sendCommand("setoption name UCI_Elo value " + to_string(gui.getDifficulty()));

    ostringstream position;
    position << "position startpos moves";
    for (const auto& moves : moveHistory) {
        position << " " << moves;
    }
    sendCommand(position.str());
    sendCommand("go depth 20");

    string bestMove;

    while (true) {
        string response = readResponse();
        if (response.find("bestmove") == 0) {
            istringstream iss(response);
            string tag;
            iss >> tag >> bestMove;

            addMoveToHistory(bestMove);
            break;
        }
    }
    return bestMove;
}