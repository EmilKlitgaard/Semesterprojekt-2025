#ifndef STOCKFISH_APP2_H
#define STOCKFISH_APP2_H

#include <string>
#include <vector>
#include <iostream>
#include <boost/process.hpp>

class StockfishApp {
public:
    StockfishApp(const std::string& path);
    bool start();
    bool sendMove(const std::string& move);
    std::string getBestMove();
    void quit();
    void printHistory() const;

private:
    std::string pathToEngine;
    std::vector<std::string> moveHistory;

    boost::process::opstream stockfishIn;
    boost::process::ipstream stockfishOut;
    std::unique_ptr<boost::process::child> process;

    void sendCommand(const std::string& cmd);
    std::string readLine();
};

#endif
