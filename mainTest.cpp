#include "Chessboard.h"
#include <iostream>

int main() {
    cout << "Starting" << endl;
    try {
        // Create a Chessboard object
        Chessboard board;

        // Set 3 dead pieces for the Robot side
        Vector3d robotLoc1 = board.getDeadPieceLocation("Queen", "Robot");
        Vector3d robotLoc2 = board.getDeadPieceLocation("Rook", "Robot");
        Vector3d robotLoc3 = board.getDeadPieceLocation("Bishop", "Robot");

        // Set 3 dead pieces for the Player side
        Vector3d playerLoc1 = board.getDeadPieceLocation("Queen", "Player");
        Vector3d playerLoc2 = board.getDeadPieceLocation("Knight", "Player");
        Vector3d playerLoc3 = board.getDeadPieceLocation("Pawn", "Player");

        // Search for the dead piece location of the Queen.
        // (Note: searchDeadPieceLocation currently searches through deadPieceNames without differentiating origin.)
        Vector3d queenLocation = board.searchDeadPieceLocation("Queen");
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}