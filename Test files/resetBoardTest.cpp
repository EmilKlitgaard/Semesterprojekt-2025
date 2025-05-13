#include "../Chessboard.h"

#include <iostream>
#include <map>
#include <set>
#include <vector>

Chessboard board;

void resetChessboard() {
    cout << "CHESSBOARD RESET STARTED!" << endl;
    board.printBoard();

    int allMoves = 0;

    ChessboardPieces currentBoard = board.getBoardState();
    ChessboardPieces desiredBoard = board.getStartBoard();

    while (currentBoard != desiredBoard) {
        while (true) {
            bool moveFound = false;
            for (int fromRow = 0; fromRow < 8; ++fromRow) {
                for (int fromCol = 0; fromCol < 8; ++fromCol) {
                    const string &currentPiece = currentBoard[fromRow][fromCol];
                    if (currentPiece == "0" || currentPiece == desiredBoard[fromRow][fromCol]) continue; // Skip empty or already correct pieces
                    
                    // Find the destination for the current piece
                    for (int toRow = 0; toRow < 8; ++toRow) {
                        for (int toCol = 0; toCol < 8; ++toCol) {
                            if (currentPiece == desiredBoard[toRow][toCol] && currentBoard[toRow][toCol] == "0") {
                                // Move the piece to the destination
                                MatrixIndex from = {fromRow, fromCol};
                                MatrixIndex to = {toRow, toCol};
                                board.updateChessboard(from, to);
                                cout << "Moved " << currentPiece << " from " << board.getChessNotation({fromRow, fromCol}) << " to " << board.getChessNotation({toRow, toCol}) << endl;
                                board.printBoard();
                                currentBoard = board.getBoardState();
                                moveFound = true;
                                toRow = toCol = 8; // Break loops
                                allMoves++;
                            }
                        }
                    }
                }
            }
            if (!moveFound) break;
        }

        bool moveFound = false;
        // Look for misplaced pieces and move one to dead piece location
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                const string &currentPiece = currentBoard[row][col];
                if (currentPiece != desiredBoard[row][col] && currentPiece != "0") {
                    const string &currentPieceName = board.getPieceName(currentPiece);
                    string origin = currentPiece.substr(1,1) = "B" ? "Robot" : "Player";
                    Vector3d deadPieceLocation = board.getDeadPieceLocation(currentPieceName, origin);
                    MatrixIndex from = {row, col};
                    board.updateChessboard("0", from);
                    cout << "Moved " << currentPiece << " from " << board.getChessNotation({row, col}) << " to dead piece location" << endl;
                    board.printBoard();
                    currentBoard = board.getBoardState();
                    allMoves++;
                    row = col = 8; // Break loops
                    moveFound = true;
                }
            }
        }
        // Recover from dead piece location
        if (!moveFound) {
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    const string &currentPiece = currentBoard[row][col];
                    if (currentPiece == "0" && desiredBoard[row][col] != "0") {
                        const string &desiredPiece = desiredBoard[row][col];
                        const string &desiredPieceName = board.getPieceName(desiredPiece);
                        string origin = currentPiece.substr(1,1) = "B" ? "Robot" : "Player";
                        Vector3d location = board.searchDeadPieceLocation(desiredPieceName, origin);
                        MatrixIndex to = {row, col};
                        board.updateChessboard(desiredPiece, to);
                        cout << "Retrived " << desiredPiece << " from dead piece location, and moved to " << board.getChessNotation({row, col}) << endl;
                        board.printBoard();
                        currentBoard = board.getBoardState();
                        allMoves++;
                        row = col = 8; // Break loops
                    }
                }
            }
        }
    }
    cout << "Reset complete in " << allMoves << " moves" << endl;
}

int main() {
    resetChessboard();
    return 0;
}
