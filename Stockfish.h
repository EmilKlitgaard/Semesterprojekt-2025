#pragma once
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>

using namespace std;

class Stockfish {
public:
    Stockfish(const string& engine_path);
    ~Stockfish();

    // Returns the best move given the latest move (in UCI notation)
    string getBestMove(const string& latestMove);

    // Check for Checkmate. 
    bool isCheckmate();

    // Update and get move history
    void addMoveToHistory(const string& move);
    vector<string> getMoveHistory() const;

    void sendCommand(const string& command);

private:
    string enginePath;
    int engine_stdin = -1;
    int engine_stdout = -1;
    int engine_pid = -1;
    
    vector<string> moveHistory;

    void startEngine();
    void stopEngine();
    
    string readResponse(const string& findString);
};

extern Stockfish engine;