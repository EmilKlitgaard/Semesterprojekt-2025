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

void GUI::setGameRunning(bool value) {
    gameRunning = value;
}

void GUI::setGamePaused(bool value) {
    gamePaused = value;
}

void GUI::setCalibrating(bool value) {
    calibrating = value;
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
        while (gameRunning) this_thread::sleep_for(chrono::milliseconds(10));
        game.stopGame();
        game.resetChessboard();
    } else if (state == "Calibrate" && !gameRunning && gameInitialized) {
        game.calibrateGripper();
    } else if (state == "Calibrate" && gameRunning && gameInitialized) {
        game.calibrate();
    }
}

// Getter functions
int  GUI::getDifficulty() { return difficulty; }
bool GUI::getGameActive() { return gameActive; }
bool GUI::getGameResetting() { return resettingActive; }
bool GUI::getGameRunning() { return gameRunning; }
bool GUI::getGamePaused() { return gamePaused; }
bool GUI::getCalibrating() { return calibrating; }
string GUI::getTurn() { return turn; }
bool GUI::getConnection() { return connection; }    

cv::Mat GUI::getVision() const {
    lock_guard<mutex> lock(cvMutex);
    if (cvImage.empty()) return {};
    return cvImage.clone(); 
}