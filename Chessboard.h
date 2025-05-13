#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <unordered_map>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

using ChessboardPieces = vector<vector<string>>;
using MatrixIndex = pair<int, int>;

// Custom hash for MatrixIndex
struct PairHash {
    template <class T1, class T2>
    size_t operator() (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

class Chessboard {
public:
    Chessboard();

    // Update board state using a move from one cell to another
    void updateChessboard(MatrixIndex from, MatrixIndex to);
    void updateChessboard(string, MatrixIndex);
    void printBoard(const string &mode = "Default");

    string getChessNotation(MatrixIndex position);
    string getChessNotation(MatrixIndex position1, MatrixIndex position2);
    pair<MatrixIndex, MatrixIndex> getMatrixIndex(string& notation);
    pair<Vector3d, Vector3d> getPhysicalCoordinates(string &notation);
    Vector3d getPhysicalCoordinate(MatrixIndex idx) const;

    ChessboardPieces getStartBoard();
    ChessboardPieces getBoardState();
    
    string getPieceName(MatrixIndex Idx);
    string getPieceName(string piece);

    Vector3d getDeadPieceLocation(const string &pieceName, const string &origin);
    Vector3d searchDeadPieceLocation(const string &pieceName, const string &origin);

    vector<Vector3d> getAllPhysicalCoordinates();

private:
    void initializeBoard();
    void initializePhysicalCoordinates();
    void initializeDeadPieceLocations();
    void initializeReachabilityCheck();
    void initializeMappings();

    void addReachabilityPosition(double x, double y, double hoverOffset);
    void generateDeadPieceLocations(vector<Vector3d>& locations, double xStart, double yStart, bool isRobot);

    ChessboardPieces startBoard;
    ChessboardPieces board;

    unordered_map<string, MatrixIndex> notationToIndex;
    unordered_map<MatrixIndex, string, PairHash> indexToNotation;

    vector<string> deadRobotPieceNames;
    vector<string> deadPlayerPieceNames;
    vector<Vector3d> deadRobotPieceLocations;
    vector<Vector3d> deadPlayerPieceLocations;

    vector<vector<pair<double, double>>> physicalCoordinates;
    vector<Vector3d> allPhysicalCoordinates;
};