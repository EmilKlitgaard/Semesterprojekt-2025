#pragma once
#include<QApplication>
#include <opencv2/opencv.hpp>


class GUI {
public:
    //set
    void setDifficulty(int value){
        if(!gameActive) {
            if (value < 300) {
                difficulty = 300;
            }
            if (value > 3000) {
                difficulty = 3000;
            }
            difficulty = value;
        }

        /*
        setoption name UCI_LimitStrength value true
        setoption name UCI_Elo value 1200 
        */

    }

    void setGameActive(bool value) {
        gameActive = value;
    }

    void setTurn(bool value) {
        turn = value; // 1 = player, 0 = robot
    }

    void setConnection(bool value) {
        connection = value; // 1 = connected, 0 = disconnected
    }

    // feed livefeed to GUI
    void GUI::setVision(const cv::Mat& image) {
        visionImage = image.clone();
    }   

    //get
    int  getDifficulty() { return difficulty; }
    bool getGameActive() { return gameActive; }
    bool getTurn() { return turn; }
    bool getConnection() { return connection; }    
    cv::Mat GUI::getVision() const {
        return visionImage.clone();
    }

private:

    cv::Mat visionImage; // contain image

    // param
    bool turn = true;
    bool connection = false;
    bool gameActive = false;
    int difficulty = 300;
};

extern GUI gui;