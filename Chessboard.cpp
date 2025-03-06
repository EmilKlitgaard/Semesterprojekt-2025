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
            row.emplace_back((j*0.05)-0.025, (i*0.05)-0.025);
        }
        physicalCoordinates.push_back(row);
    }
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
string Chessboard::getChessNotation(pair<int, int> position) {
    return indexToNotation[position];
}

// Convert chess notation to matrix index (e.g., "a1" -> (0,0))
pair<int, int> Chessboard::getMatrixIndex(const string& notation) {
    return notationToIndex[notation];
}

// Update the board state after a move
void Chessboard::updateChessboard(pair<int, int> from, pair<int, int> to) {
    string movedPiece = board[from.first][from.second];
    board[from.first][from.second] = "0"; // Empty original position
    board[to.first][to.second] = movedPiece; // Move piece to new position
}

// Print the chessboard in a readable format
void Chessboard::printBoard() {
    for (const auto& row : board) {
        for (const auto& cell : row) {
            cout << cell << "\t";
        }
        cout << endl;
    }
}

// Check if any king is missing (dead), to end the game.
bool Chessboard::anyKingIsDead() {
    int kingCount = 0;
    for (const auto& row : board) {
        for (const auto& cell : row) {
            if (cell == "6B" || cell == "6W") {
                kingCount++;
            }
        }
    }
    if (kingCount < 2) {
        return true;
    } else {
        return false;
    }
}

// Get the current board state
vector<vector<string>> Chessboard::getBoardState() {
    return board;
}

// Convert chess notation (e.g., "A2") to physical coordinates (XYZ).
Eigen::Vector3d Chessboard::getPhysicalCoordinates(const std::string& notation) {
    auto indices = getMatrixIndex(notation);
    int i = indices.first;
    int j = indices.second;
    double x = physicalCoordinates[i][j].first;
    double y = physicalCoordinates[i][j].second;
    double z = 0.1;
    return Eigen::Vector3d(x, y, z);
}
