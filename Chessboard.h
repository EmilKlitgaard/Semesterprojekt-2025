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

using MatrixIndex = pair<int, int>;

// Custom hash for MatrixIndex
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
    void updateChessboard(MatrixIndex from, MatrixIndex to);
    void printBoard(const string &mode = "Default");

    string getChessNotation(MatrixIndex position);
    string getChessNotation(MatrixIndex position1, MatrixIndex position2);
    MatrixIndex getMatrixIndex(const string& notation);
    Vector3d getPhysicalCoordinates(const string &notation);

    vector<vector<string>> getBoardState();
    
    Vector3d getDeadPieceLocation();
    Vector3d setDeadPieceLocation(string player = "robot");

    void updateDeadPieces(MatrixIndex from, MatrixIndex to);

private:
    vector<vector<string>> board;
    vector<vector<pair<double, double>>> physicalCoordinates;
    unordered_map<string, MatrixIndex> notationToIndex;
    unordered_map<MatrixIndex, string, PairHash> indexToNotation;

    vector<string> deadRobotPieces;
    vector<Vector3d> deadPiecePlayerLocations;
    vector<Vector3d> deadPieceRobotLocations;
    int deadPiecePlayerLocationIndex;
    int deadPieceRobotLocationIndex;

    void initializeBoard();
    void initializeMappings();
};