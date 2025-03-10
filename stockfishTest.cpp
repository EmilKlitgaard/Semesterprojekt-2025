#include "Stockfish.h"
#include <iostream>

using namespace std;

int main() {
    // Initialize Stockfish engine
    Stockfish engine("/home/ubuntu/Stockfish/src/stockfish");

    // Simulate a losing battle (Fool's Mate scenario)
    vector<string> moves = {
        "f2f3",  // Player move
        "e7e5",  // Stockfish move
        "g2g4",  // Player move
        "d8h4"   // Stockfish move (checkmate)
    };

    // Play out the moves
    for (const string& move : moves) {
        cout << "Playing move: " << move << endl;
        engine.addMoveToHistory(move);
        
        // Check for checkmate
        if (engine.isCheckmate()) {
            cout << "Checkmate detected! Game over." << endl;
        } else {
            cout << "No checkmate detected." << endl;
        }
    }
    return 0;
}