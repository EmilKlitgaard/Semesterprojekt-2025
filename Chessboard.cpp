#include "Chessboard.h"

Chessboard::Chessboard() {
    initializeBoard();
    initializePhysicalCoordinates();
    initializeDeadPieceLocations();
    initializeReachabilityCheck();
    initializeMappings();
}

// Initialize board with standard chess starting positions
void Chessboard::initializeBoard() {
    startBoard = {
        {"2B", "3B", "4B", "5B", "6B", "4B", "3B", "2B"}, // Black major pieces
        {"1B", "1B", "1B", "1B", "1B", "1B", "1B", "1B"}, // Black pawns
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"0",   "0",  "0",  "0",  "0",  "0",  "0",  "0"},
        {"1W", "1W", "1W", "1W", "1W", "1W", "1W", "1W"}, // White pawns
        {"2W", "3W", "4W", "5W", "6W", "4W", "3W", "2W"}  // White major pieces
    };
    board = startBoard;
}

// Initialize physical coordinates for each board cell
void Chessboard::initializePhysicalCoordinates() {
    const double cellSize = 0.05;
    const double halfCell = cellSize / 2.0;

    for (int i = 0; i < 8; i++) {
        vector<pair<double, double>> row;
        for (int j = 0; j < 8; j++) {
            row.emplace_back((i * cellSize) + halfCell, (j * cellSize) + halfCell);
        }
        physicalCoordinates.push_back(row);
    }
}

// Generate coordinates for dead piece locations
void Chessboard::initializeDeadPieceLocations() {   
    deadRobotPieceNames.resize(16, "");
    deadPlayerPieceNames.resize(16, "");
    
    generateDeadPieceLocations(deadRobotPieceLocations, 0.025, 0.475, true);
    generateDeadPieceLocations(deadPlayerPieceLocations, 0.375, -0.075, false);
}

// Helper function to generate dead piece locations
void Chessboard::generateDeadPieceLocations(vector<Vector3d>& locations, double xStart, double yStart, bool isRobot) {
    const double cellSize = 0.05;
    const double z = 0.0;
    double y = yStart;

    for (int i = 0; i < 2; ++i) {
        if (i == 1) y += (isRobot ? cellSize : -cellSize);
        for (int j = 0; j < 8; ++j) {
            double x = isRobot ? (xStart + j * cellSize) : (xStart - j * cellSize);
            locations.push_back(Vector3d(x, y, z));
        }
    }
}

// Initialize all reachable positions for movement checks
void Chessboard::initializeReachabilityCheck() {
    const double hoverOffset = 0.10;
    for (const auto& row : physicalCoordinates) {
        for (const auto& pos : row) {
            addReachabilityPosition(pos.first, pos.second, hoverOffset);
        }
    }
    for (const auto& pos : deadRobotPieceLocations) {
        addReachabilityPosition(pos[0], pos[1], hoverOffset);
    }
    for (const auto& pos : deadPlayerPieceLocations) {
        addReachabilityPosition(pos[0], pos[1], hoverOffset);
    }
}

// Helper function to add a base and hover position
void Chessboard::addReachabilityPosition(double x, double y, double hoverOffset) {
    allPhysicalCoordinates.push_back(Vector3d(x, y, 0.0));          // Base position
    allPhysicalCoordinates.push_back(Vector3d(x, y, hoverOffset));  // Hover position
}

// Map chess notation (e.g. "a1") to matrix indices (row, col)
void Chessboard::initializeMappings() {
    const string columns = "abcdefgh";
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            string notation = columns[j] + to_string(8 - i);
            notationToIndex[notation] = {i, j};
            indexToNotation[{i, j}] = notation;
        }
    }
}

vector<Vector3d> Chessboard::getAllPhysicalCoordinates() {
    return allPhysicalCoordinates;
}

ChessboardPieces Chessboard::getStartBoard() {
    return startBoard;
}

Vector3d Chessboard::getDeadPieceLocation(const string &pieceName, const string &origin) {
    if (origin != "Robot" && origin != "Player") throw invalid_argument("Invalid origin");

    vector<Vector3d> &locations = (origin == "Robot" ? deadRobotPieceLocations : deadPlayerPieceLocations);
    vector<string> &names = (origin == "Robot" ? deadRobotPieceNames : deadPlayerPieceNames);
    
    int slot = -1;
    for (int i = 0; i < names.size(); ++i) {
        if (names[i].empty()) {
            slot = i;
            break;
        }
    }
    if (slot < 0) throw out_of_range("No free deadPiece slots available");

    names[slot] = pieceName;
    Vector3d location = locations[slot];
    cout << "Added: '" << pieceName << "' into deadPieceSlot[" << slot << "] = (" << location.transpose() << ")" << endl;
    return location;
}


// Search for a specific dead piece location
Vector3d Chessboard::searchDeadPieceLocation(const string &pieceName, const string &origin) {
    if (origin != "Robot" && origin != "Player") throw invalid_argument("Invalid origin");
    
    vector<Vector3d> &locations = (origin == "Robot" ? deadRobotPieceLocations : deadPlayerPieceLocations);
    vector<string> &names = (origin == "Robot" ? deadRobotPieceNames : deadPlayerPieceNames);

    for (int i = 0; i < names.size(); ++i) {
        if (names[i] == pieceName) {
            cout << "Found '" << pieceName << "' at index: " << i << ", with coordinate (" << locations[i].x() << ", " << locations[i].y() << ", " << locations[i].z() << ")" << std::endl;
            names[i].clear();
            return locations[i];
        }
    }
    throw invalid_argument("Piece name not found in dead pieces");
}

// Convert matrix index to chess notation (e.g., (0,0) -> "a8")
string Chessboard::getChessNotation(MatrixIndex position) {
    return indexToNotation[position];
}
// Convert matrix index to chess notation (e.g., ({0,0}, {0,4}) -> "a8a5")
string Chessboard::getChessNotation(MatrixIndex position1, MatrixIndex position2) {
    return indexToNotation[position1] + indexToNotation[position2];
}

// Convert chess notation to matrix index (e.g., "a8a5" -> ({0,0}, {0,4}))
pair<MatrixIndex, MatrixIndex> Chessboard::getMatrixIndex(string& notation) {
    string fromNotation = notation.substr(0, 2);
    string toNotation   = notation.substr(2, 2);
    return {notationToIndex[fromNotation], notationToIndex[toNotation]};
}

// Update the board state after a move
void Chessboard::updateChessboard(MatrixIndex from, MatrixIndex to) {
    string movedPiece = board[from.first][from.second];
    board[from.first][from.second] = "0"; // Empty original position
    board[to.first][to.second] = movedPiece; // Move piece to new position
}

// Update the board with a single piece. E.g. after pawn promotion
void Chessboard::updateChessboard(string newPiece, MatrixIndex Idx) {
    board[Idx.first][Idx.second] = newPiece; // Move piece to new position
}

string Chessboard::getPieceName(MatrixIndex Idx) {
    string piece = board[Idx.first][Idx.second];
    return getPieceName(piece);
}

string Chessboard::getPieceName(string piece) {
    string pieceType = piece.substr(0,1);
    if (pieceType == "1") return "Pawn";
    else if (pieceType == "2") return "Rook";
    else if (pieceType == "3") return "Knight";
    else if (pieceType == "4") return "Bishop";
    else if (pieceType == "5") return "Queen";
    else if (pieceType == "6") return "King";
    else if (pieceType == "0") {
        cerr << "Error: The location is empty!" << endl;
        return "EMPTY";
    } else {
        cerr << "Error: Number does not exist!" << endl;
        return "ERROR";
    }
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
ChessboardPieces Chessboard::getBoardState() {
    return board;
}

// Convert chess notation (e.g., "a2a4") to physical coordinates {(XYZ), (XYZ)}.
pair<Vector3d, Vector3d> Chessboard::getPhysicalCoordinates(string& notation) {
    auto [from, to] = getMatrixIndex(notation);
    auto& [x1, y1] = physicalCoordinates[from.first][from.second];
    auto& [x2, y2] = physicalCoordinates[to.first][to.second];
    return { Vector3d(x1, y1, 0.0), Vector3d(x2, y2, 0.0) };
}

Vector3d Chessboard::getPhysicalCoordinate(MatrixIndex idx) const {
    auto &cell = physicalCoordinates[idx.first][idx.second];
    return Eigen::Vector3d(cell.first, cell.second, 0.0);
}