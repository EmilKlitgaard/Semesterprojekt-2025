#include "GUI.h"

using namespace std;

GUI gui;

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

// Wait for the start button to be clicked
void GUI::awaitStartGame() {
    while (!gameActive) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void GUI::setTurn(bool value) {
    turn = value; // True = player, False = robot
}

void GUI::setConnection(bool value) {
    connection = value; // True = connected, False = disconnected
}

// feed livefeed to GUI
void GUI::setVision(const cv::Mat& image) {
    lock_guard<mutex> lock(cvMutex);
    cvImage = image.clone();
}   

// Getter functions
int  GUI::getDifficulty() { return difficulty; }
bool GUI::getGameActive() { return gameActive; }
bool GUI::getTurn() { return turn; }
bool GUI::getConnection() { return connection; }    

cv::Mat GUI::getVision() const {
    lock_guard<mutex> lock(cvMutex);
    if (cvImage.empty()) return {};
    return cvImage.clone(); 
}