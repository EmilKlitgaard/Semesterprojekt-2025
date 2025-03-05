#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>
#include <unordered_map>

// Custom hash for std::pair<int, int>
struct PairHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
         auto h1 = std::hash<T1>{}(p.first);
         auto h2 = std::hash<T2>{}(p.second);
         return h1 ^ (h2 << 1);
    }
};

class Chessboard {
public:
    Chessboard();

    // Update board state using a move from one cell to another
    void updateChessboard(std::pair<int, int> from, std::pair<int, int> to);
    void printBoard();
    bool anyKingIsDead();
    std::string getChessNotation(std::pair<int, int> position);
    std::pair<int, int> getMatrixIndex(const std::string& notation);
    std::vector<std::vector<std::string>> getBoardState();

    // Retrieve physical coordinates for a given cell in chess notation.
    std::vector<double> getPhysicalCoordinates(const std::string &notation);

private:
    std::vector<std::vector<std::string>> board;
    std::vector<std::vector<std::pair<double, double>>> physicalCoordinates;
    std::unordered_map<std::string, std::pair<int, int>> notationToIndex;
    std::unordered_map<std::pair<int, int>, std::string, PairHash> indexToNotation;

    void initializeBoard();
    void initializeMappings();
};

