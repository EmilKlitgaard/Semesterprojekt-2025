#pragma once

#include <string>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <boost/process.hpp>

using namespace std;

class Stockfish {
public:
    Stockfish(const string& enginePath);
    ~Stockfish();

    bool sendValidMove(const string& move);
    string getBestMove();

    bool isCheckmate();

    void addMoveToHistory(const string& move);
    vector<string> getMoveHistory() const;
    void printMoveHistory() const;
    void resetMoveHistory();

private:
    string enginePath;
    
    vector<string> moveHistory;

    void startEngine();
    void stopEngine();
    void sendCommand(const string& cmd);
    string readResponse();

    boost::process::opstream stockfishIn;
    boost::process::ipstream stockfishOut;
    unique_ptr<boost::process::child> process;
};
