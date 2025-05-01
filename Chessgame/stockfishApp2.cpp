#include "stockfishApp2.h"
#include <sstream>
#include <algorithm>

using namespace std;

StockfishApp::StockfishApp(const string& path)
    : pathToEngine(path) {}

bool StockfishApp::start() {
    // Starter Stockfish motoren
    process = make_unique<boost::process::child>(
        pathToEngine,
        boost::process::std_in < stockfishIn,
        boost::process::std_out > stockfishOut
    );

    if (!process->running()) {
        cerr << "Kunne ikke starte Stockfish.\n";
        return false;
    }

    // Initialiserer UCI protokol og venter på klar signal
    sendCommand("uci");
    while (readLine() != "uciok");

    sendCommand("isready");
    while (readLine() != "readyok");

    return true;
}

void StockfishApp::sendCommand(const string& cmd) {
    // Sender kommando til Stockfish
    stockfishIn << cmd << endl;
    stockfishIn.flush();
}

string StockfishApp::readLine() {
    // Læser en linje fra Stockfish
    string line;
    getline(stockfishOut, line);
    return line;
}

bool StockfishApp::sendMove(const string& move) {
    // Sender nuværende position baseret på trækkene
    ostringstream position;
    position << "position startpos moves";
    for (const string& m : moveHistory) {
        position << " " << m;
    }
    sendCommand(position.str());

    // Beder Stockfish vise lovlige træk
    sendCommand("go perft 1");

    vector<string> legalMoves;

    // Læser træk indtil og vi ser "Nodes searched" hvsi vist
    while (true) {
        string line = readLine();
        if (line.empty()) continue;

        size_t colon = line.find(':');
        if (colon != string::npos) {
            string legalMove = line.substr(0, colon);
            legalMove.erase(0, legalMove.find_first_not_of(" \t\r\n"));
            legalMove.erase(legalMove.find_last_not_of(" \t\r\n") + 1);
            legalMoves.push_back(legalMove);
        }

        if (line.find("Nodes searched") != string::npos) {
            break;
        }
    }

    // Tjekker om spillerens træk er lovligt
    if (find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end()) {
        moveHistory.push_back(move);
        return true;
    }

    return false;
}

std::string StockfishApp::getBestMove() {
    // Sender nuværende position til Stockfish
    std::ostringstream oss;
    oss << "position startpos moves";
    for (const auto& m : moveHistory) {
        oss << " " << m;
    }

    sendCommand(oss.str());
    sendCommand("go depth 20");

    std::string bestMove;

    // Læser bedste træk fra Stockfish
    while (true) {
        std::string line = readLine();
        if (line.find("bestmove") == 0) {
            std::istringstream iss(line);
            std::string tag;
            iss >> tag >> bestMove;

            if (bestMove != "(none)") {
                moveHistory.push_back(bestMove);
            }

            break;
        }
    }

    // Tjekker om modstanderen er sat mat
    std::ostringstream nextPosition;
    nextPosition << "position startpos moves";
    for (const auto& m : moveHistory) {
        nextPosition << " " << m;
    }

    sendCommand(nextPosition.str());
    sendCommand("go depth 20");

    while (true) {
        std::string line = readLine();
        if (line.find("bestmove") == 0) {
            std::istringstream iss(line);
            std::string tag, nextMove;
            iss >> tag >> nextMove;

            if (nextMove == "(none)") {
                std::cout << "Stockfish spiller: " << bestMove << "\n";
                printHistory();
                std::cout << "Gameover!\n";
                quit();
                std::exit(0);
            }

            break;
        }
    }

    return bestMove;
}

void StockfishApp::quit() {
    // Afslutter Stockfish motoren pænt
    if (process && process->running()) {
        sendCommand("quit");
        process->wait();
    }
}

void StockfishApp::printHistory() const {
    // Printer alle træk foretaget i spillet
    cout << "Trækhistorik: ";
    for (const string& move : moveHistory) {
        cout << move << " ";
    }
    cout << endl;
}
