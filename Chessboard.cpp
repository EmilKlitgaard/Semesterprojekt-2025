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
        std::vector<std::pair<double, double>> row;
        for (int j = 0; j < 8; j++) {
            row.emplace_back(j * 0.05, i * 0.05);
        }
        physicalCoordinates.push_back(row);
    }
}

// Map chess notation (e.g., "A1") to matrix indices (row, col)
void Chessboard::initializeMappings() {
    std::string columns = "ABCDEFGH";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            std::string notation = columns[j] + std::to_string(8 - i);
            notationToIndex[notation] = {i, j};
            indexToNotation[{i, j}] = notation;
        }
    }
}

// Convert matrix index to chess notation (e.g., (0,0) -> "A8")
std::string Chessboard::getChessNotation(std::pair<int, int> position) {
    return indexToNotation[position];
}

// Convert chess notation to matrix index (e.g., "A8" -> (0,0))
std::pair<int, int> Chessboard::getMatrixIndex(const std::string& notation) {
    return notationToIndex[notation];
}

// Update the board state after a move
void Chessboard::updateChessboard(std::pair<int, int> from, std::pair<int, int> to) {
    std::string movedPiece = board[from.first][from.second];
    board[from.first][from.second] = "0"; // Empty original position
    board[to.first][to.second] = movedPiece; // Move piece to new position
}

// Print the chessboard in a readable format
void Chessboard::printBoard() {
    for (const auto& row : board) {
        for (const auto& cell : row) {
            std::cout << cell << "\t";
        }
        std::cout << std::endl;
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
std::vector<std::vector<std::string>> Chessboard::getBoardState() {
    return board;
}

// Retrieve physical coordinates (as a 6-DOF pose vector) for a given cell in chess notation.
std::vector<double> Chessboard::getPhysicalCoordinates(const std::string &notation) {
    auto indices = getMatrixIndex(notation);
    int i = indices.first;
    int j = indices.second;
    double x = physicalCoordinates[i][j].first;
    double y = physicalCoordinates[i][j].second;
    // For simplicity, use fixed z and orientation values.
    return {x, y, 0.1, 0, M_PI, 0};
}
