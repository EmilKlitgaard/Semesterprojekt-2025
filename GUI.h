#pragma once

#include <QtWidgets/QApplication>
#include <opencv2/opencv.hpp>
#include <mutex>

using namespace std;

class GUI {
public:
    //set
    void setDifficulty(int value);

    void setGameActive(bool value);

    void setTurn(bool value);

    void setConnection(bool value);

    
    void setVision(const cv::Mat& image); // livefeed to GUI

    //get
    int  getDifficulty();

    bool getGameActive();

    bool getTurn();

    bool getConnection();
    
    cv::Mat getVision() const;

private:

    mutable mutex cvMutex;

    // param
    cv::Mat cvImage;
    bool turn = true;
    bool connection = false;
    bool gameActive = false;
    int difficulty = 300;
};

extern GUI gui;