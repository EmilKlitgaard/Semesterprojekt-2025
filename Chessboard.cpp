#include "Chessboard.h"

Chessboard::Chessboard() {
    initializeBoard();
    initializeMappings();
}

// Initialize board with standard chess starting positions
void Chessboard::initializeBoard() {
    board = {
        {"2B", "3B", "4B", "5B", "6B", "4B", "3B", "2B"}, // Black major pieces
        {"1B", "1B", "1B", "1B", "1B", "1B", "1B", "1B"}, // Black pawns
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"1W", "1W", "1W", "1W", "1W", "1W", "1W", "1W"}, // White pawns
        {"2W", "3W", "4W", "5W", "6W", "4W", "3W", "2W"}  // White major pieces
    };

    // Initialize physical coordinates (each cell is 0.05m x 0.05m)
    for (int i = 0; i < 8; i++) {
        vector<pair<double, double>> row;
        for (int j = 0; j < 8; j++) {
            row.emplace_back((i*0.05)+0.025, (j*0.05)+0.025);
        }
        physicalCoordinates.push_back(row);
    }

    // Generate deadPieceLocation coordinates
    deadPieceLocationIndex = 0;
    const double cellSize = 0.05; // Each cell is 0.05m x 0.05m
    const double xStart = 0.375;
    const double z = 0.0;
    double y = -0.075;
    for (int i = 0; i < 2; ++i) {
        if (i == 1) { y -= cellSize; }
        for (int j = 0; j < 8; ++j) {
            double x = xStart - j * cellSize;
            deadPieceLocations.push_back(Vector3d(x, y, z));
        }
    }
}

// Get the dead piece location at a specific index
Vector3d Chessboard::getDeadPieceLocation() {
    if (deadPieceLocationIndex < 0 || deadPieceLocationIndex >= deadPieceLocations.size()) {
        throw out_of_range("Index out of range for deadPieceLocation");
    }
    Vector3d location = deadPieceLocations[deadPieceLocationIndex];
    deadPieceLocationIndex++;
    cout << "Current deadPieceLocationIndex: " << deadPieceLocationIndex << endl;
    return location;
}

// Map chess notation (e.g. "a1") to matrix indices (row, col)
void Chessboard::initializeMappings() {
    string columns = "abcdefgh";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            string notation = columns[j] + to_string(8 - i);
            notationToIndex[notation] = {i, j};
            indexToNotation[{i, j}] = notation;
        }
    }
}

// Convert matrix index to chess notation (e.g., (0,0) -> "a8")
string Chessboard::getChessNotation(MatrixIndex position) {
    return indexToNotation[position];
}
// Convert matrix index to chess notation (e.g., ({0,0}, {0,4}) -> "a8a5")
string Chessboard::getChessNotation(MatrixIndex position1, MatrixIndex position2) {
    return indexToNotation[position1] + indexToNotation[position2];
}

// Convert chess notation to matrix index (e.g., "a1" -> (0,0))
MatrixIndex Chessboard::getMatrixIndex(const string& notation) {
    return notationToIndex[notation];
}

// Update the board state after a move
void Chessboard::updateChessboard(MatrixIndex from, MatrixIndex to) {
    string movedPiece = board[from.first][from.second];
    board[from.first][from.second] = "0"; // Empty original position
    board[to.first][to.second] = movedPiece; // Move piece to new position
}

void Chessboard::printBoard(const string &mode) {
    cout << "\n" << "Printing current chessboard:" << endl;
    if (mode == "ChessNotation") {
        for (int i = 0; i < board.size(); i++) {
            for (int j = 0; j < board[i].size(); j++) {
                cout << getChessNotation({i, j}) << "\t";
            }
            cout << endl;
        }
    } else if (mode == "CoordinateNotation") {
        for (int i = 0; i < physicalCoordinates.size(); i++) {
            for (int j = 0; j < physicalCoordinates[i].size(); j++) {
                auto coord = physicalCoordinates[i][j];
                cout << "(" << coord.first << "," << coord.second << ")\t";
            }
            cout << endl;
        }
    } else {
        for (const auto &row : board) {
            for (const auto &cell : row) {
                cout << cell << "\t";
            }
            cout << endl;
        }
    }
    cout << endl;
}

// Get the current board state
vector<vector<string>> Chessboard::getBoardState() {
    return board;
}

// Convert chess notation (e.g., "A2") to physical coordinates (XYZ).
Vector3d Chessboard::getPhysicalCoordinates(const string& notation) {
    auto indices = getMatrixIndex(notation); // returns {i, j} with i=0 for rank 8, i=7 for rank 1
    int i = indices.first;
    int j = indices.second;
    double x = physicalCoordinates[i][j].first;
    double y = physicalCoordinates[i][j].second;
    double z = 0.0;  // constant z height
    return Eigen::Vector3d(x, y, z);
}