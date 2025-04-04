#include "Vision.h"
#include <thread>

int main() {
    ChessVision chessVision(3);

    // Start kamera-feed i separat tråd
    std::thread cameraThread([&]() {
        chessVision.showLiveFeed();
    });

    string input;
    while (true) {
        cout << "Tryk Enter for at processere billede ('stop' for afslut): ";
        getline(cin, input);
        if (input == "stop" || input == "Stop") break;

        chessVision.processCurrentFrame();
    }

    cameraThread.join();  // Vent på kamera-tråden
    return 0;
}