#pragma once
#include<QApplication>
#include <opencv2/opencv.hpp>


class GUI {
public:
    //set
    void setDifficulty(int value);

    void setGameActive(bool value);

    void setTurn(bool value);

    void setConnection(bool value);

    // feed livefeed to GUI
    void setVision(const cv::Mat& image);

    //get
    int  getDifficulty() { return difficulty; }
    bool getGameActive() { return gameActive; }
    bool getTurn() { return turn; }
    bool getConnection() { return connection; }    
    cv::Mat getVision() const;

private:
    // param
    cv::Mat cvImage;
    bool turn = true;
    bool connection = false;
    bool gameActive = false;
    int difficulty = 300;
};

extern GUI gui;