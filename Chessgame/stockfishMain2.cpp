#include "stockfishApp2.h"
#include <iostream>

int main() {
    StockfishApp app("/usr/games/stockfish");

    if (!app.start()) return 1;

    std::string move;
    while (true) {
        std::cout << "Your move (or 'quit'): ";
        std::cin >> move;

        if (move == "quit") break;

        if (!app.sendMove(move)) {
            std::cout << "Invalid move!\n";
            continue;
        }

        std::string bestMove = app.getBestMove();

        if (bestMove == "none") {
            std::cout << "Stockfish has no legal moves. Game over.\n";
            break;
        }
        
        std::cout << "Stockfish plays: " << bestMove << "\n";
        app.printHistory();
    }

    app.quit();
    return 0;
}
