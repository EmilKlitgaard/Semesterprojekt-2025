#include "Chessboard.h"
#include <iostream>
#include <map>
#include <set>
#include <vector>

Chessboard board;

void resetChessboard() {
    cout << "CHESSBOARD RESET STARTED!" << endl;
    board.printBoard();

    struct Move {
        MatrixIndex from;
        MatrixIndex to;
        string piece;
        bool useBuffer;
    };

    ChessboardPieces desired = {
        {"2B","3B","4B","5B","6B","4B","3B","2B"},
        {"1B","1B","1B","1B","1B","1B","1B","1B"},
        {"0","0","0","0","0","0","0","0"},
        {"0","0","0","0","0","0","0","0"},
        {"0","0","0","0","0","0","0","0"},
        {"0","0","0","0","0","0","0","0"},
        {"1W","1W","1W","1W","1W","1W","1W","1W"},
        {"2W","3W","4W","5W","6W","4W","3W","2W"}
    };

    vector<Move> allMoves;

    while (true) {
        auto current = board.getBoardState();
        if (current == desired) break;

        // Track claimed destinations
        vector<vector<bool>> claimed(8, vector<bool>(8, false));
        map<MatrixIndex, MatrixIndex> perm;

        // Build permutation map with unique destinations
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                const string &p = current[r][c];
                if (p == "0" || p == desired[r][c]) continue;

                // Find first matching unclaimed destination
                for (int rr = 0; rr < 8; ++rr) {
                    for (int cc = 0; cc < 8; ++cc) {
                        if (!claimed[rr][cc] && 
                            desired[rr][cc] == p && 
                            current[rr][cc] != p) {
                            perm[{r, c}] = {rr, cc};
                            claimed[rr][cc] = true;
                            rr = cc = 8;  // break loops
                            break;
                        }
                    }
                }
            }
        }

        if (perm.empty()) break;

        // Decompose into chains + cycles
        const MatrixIndex bufferIdx{-1,-1};
        set<MatrixIndex> visited;
        vector<Move> schedule;

        auto processChain = [&](const vector<MatrixIndex>& chain, bool isCycle) {
            if (isCycle) {
                // Buffer first element
                schedule.push_back({chain[0], bufferIdx, current[chain[0].first][chain[0].second], true});
                
                // Rotate cycle
                for (int i = chain.size()-1; i > 0; --i) {
                    schedule.push_back({chain[i], chain[i-1], current[chain[i].first][chain[i].second], false});
                }
                
                // Return from buffer
                schedule.push_back({bufferIdx, chain.back(), current[chain[0].first][chain[0].second], false});
            } else {
                // Simple reverse chain
                for (int i = chain.size()-1; i >= 0; --i) {
                    if (perm.count(chain[i])) {
                        schedule.push_back({chain[i], perm[chain[i]], current[chain[i].first][chain[i].second], false});
                    }
                }
            }
        };

        for (auto &pr : perm) {
            if (visited.count(pr.first)) continue;

            vector<MatrixIndex> chain;
            MatrixIndex cur = pr.first;
            while (perm.count(cur) && !visited.count(cur)) {
                visited.insert(cur);
                chain.push_back(cur);
                cur = perm[cur];
            }

            bool isCycle = find(chain.begin(), chain.end(), cur) != chain.end();
            processChain(chain, isCycle);
        }

        // Execute moves
        for (auto &mv : schedule) {
            if (mv.useBuffer) {
                // Store in buffer
                board.getDeadPieceLocation(mv.piece, "Player");
                board.updateChessboard("0", mv.from);
                cout << "BUFFER: " << mv.piece << " from " << board.getChessNotation(mv.from) << endl;
                board.printBoard();
            } 
            else if (mv.from == bufferIdx) {
                // Retrieve from buffer
                Vector3d bufferLoc = board.searchDeadPieceLocation(mv.piece, "Player");
                board.updateChessboard(mv.piece, mv.to);
                cout << "RESTORE: " << mv.piece << " to " << board.getChessNotation(mv.to) << endl;
                board.printBoard();
            } 
            else {
                // Normal move
                board.updateChessboard(mv.from, mv.to);
                cout << "MOVE: " << mv.piece << " from " << board.getChessNotation(mv.from) << " to " << board.getChessNotation(mv.to) << endl;
                board.printBoard();
            }
            allMoves.push_back(mv);
        }
    }
    cout << "Reset complete in " << allMoves.size() << " moves" << endl;
}

void newResetChessboard() {
    cout << "CHESSBOARD RESET STARTED!" << endl;
    board.printBoard();

    ChessboardPieces desiredBoard = {
        {"2B","3B","4B","5B","6B","4B","3B","2B"},
        {"1B","1B","1B","1B","1B","1B","1B","1B"},
        {"0","0","0","0","0","0","0","0"},
        {"0","0","0","0","0","0","0","0"},
        {"0","0","0","0","0","0","0","0"},
        {"0","0","0","0","0","0","0","0"},
        {"1W","1W","1W","1W","1W","1W","1W","1W"},
        {"2W","3W","4W","5W","6W","4W","3W","2W"}
    };

    int allMoves = 0;

    ChessboardPieces currentBoard = board.getBoardState();
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
    //resetChessboard();
    newResetChessboard();
    return 0;
}
