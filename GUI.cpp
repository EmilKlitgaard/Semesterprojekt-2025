#include "GUI.h"
#include "Game.h"

GUI gui;

using namespace std;

void GUI::setDifficulty(int value){
    if (value < 300) {
        difficulty = 300;
    } else if (value > 3000) {
        difficulty = 3000;
    } else {
        difficulty = value;
    }
}

void GUI::setGameActive(bool value) {
    gameActive = value;
}

void GUI::setGameInitialized(bool value) {
    gameInitialized = value;
}

void GUI::setGameResetting(bool value) {
    resettingActive = value;
}

// Wait for the start button to be clicked
void GUI::awaitStartGame() {
    while (!gameActive) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void GUI::setTurn(string value) {
    turn = value;
}

void GUI::setConnection(bool value) {
    connection = value; // True = connected, False = disconnected
}

// feed livefeed to GUI
void GUI::setVision(const cv::Mat& image) {
    lock_guard<mutex> lock(cvMutex);
    cvImage = image.clone();
}   

void GUI::changeState(string state) {
    cout << "State: " << state << endl;
    if (state == "Start" && !gameActive && !gameInitialized) {
        gameActive = true;
        game.initializeGame();
    } else if (state == "Start" && !gameActive && gameInitialized) {
        gameActive = true;
        game.startGame();
    } else if (state == "Reset" && !resettingActive && gameInitialized) {
        gameActive = false;
        resettingActive = true;
        game.resetChessboard();
    }
}

// Getter functions
int  GUI::getDifficulty() { return difficulty; }
bool GUI::getGameActive() { return gameActive; }
bool GUI::getGameResetting() { return resettingActive; }
string GUI::getTurn() { return turn; }
bool GUI::getConnection() { return connection; }    

cv::Mat GUI::getVision() const {
    lock_guard<mutex> lock(cvMutex);
    if (cvImage.empty()) return {};
    return cvImage.clone(); 
}