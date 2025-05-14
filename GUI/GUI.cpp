#include "GUI.h"

//set
void GUI::setDifficulty(int value){
    if(!gameActive) {
        if (value < 300) {
            difficulty = 300;
        }
        if (value > 3000) {
            difficulty = 3000;
        }
        difficulty = value;
    }
}

void GUI::setGameActive(bool value) {
    gameActive = value;
}

void GUI::setTurn(bool value) {
    turn = value; // 1 = player, 0 = robot
}

void GUI::setConnection(bool value) {
    connection = value; // 1 = connected, 0 = disconnected
}

// feed livefeed to GUI
void GUI::setVision(const cv::Mat& image) {
    std::lock_guard<std::mutex> lock(cvMutex);
    cvImage = image.clone();
}   

//get
int  GUI::getDifficulty() { return difficulty; }

bool GUI::getGameActive() { return gameActive; }

bool GUI::getTurn() { return turn; }

bool GUI::getConnection() { return connection; }    

cv::Mat GUI::getVision() const {
    std::lock_guard<std::mutex> lock(cvMutex);
    if (cvImage.empty()) return {};
    return cvImage.clone(); 
}