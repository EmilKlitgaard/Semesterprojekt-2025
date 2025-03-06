#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>
#include <unordered_map>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

// Custom hash for pair<int, int>
struct PairHash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const {
         auto h1 = hash<T1>{}(p.first);
         auto h2 = hash<T2>{}(p.second);
         return h1 ^ (h2 << 1);
    }
};

class Chessboard {
public:
    Chessboard();

    // Update board state using a move from one cell to another
    void updateChessboard(pair<int, int> from, pair<int, int> to);
    void printBoard(const string &mode = "Default");
    bool anyKingIsDead();
    string getChessNotation(pair<int, int> position);
    pair<int, int> getMatrixIndex(const string& notation);
    vector<vector<string>> getBoardState();

    // Retrieve physical coordinates for a given cell in chess notation.
    Vector3d getPhysicalCoordinates(const string &notation);

private:
    vector<vector<string>> board;
    vector<vector<pair<double, double>>> physicalCoordinates;
    unordered_map<string, pair<int, int>> notationToIndex;
    unordered_map<pair<int, int>, string, PairHash> indexToNotation;

    void initializeBoard();
    void initializeMappings();
};

