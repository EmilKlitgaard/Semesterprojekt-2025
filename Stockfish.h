#pragma once

#include <string>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

class Stockfish {
public:
    Stockfish(const string& engine_path);
    ~Stockfish();

    string getBestMove(const string& latestMove);

private:
    string enginePath;
    int engine_stdin = -1;
    int engine_stdout = -1;
    int engine_pid = -1;

    void startEngine();
    void stopEngine();
    void sendCommand(const string& command);
    string readResponse(const string& findString);
};
