#pragma once

#include <QtWidgets/QApplication>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <thread>
#include <iostream>
#include <chrono>

using namespace std;

class GUI {
public:
    void setDifficulty(int value);
    void setGameActive(bool value);
    void setGameInitialized(bool value);
    void setGameResetting(bool value);
    void setGameRunning(bool value);
    void setGamePaused(bool value);
    void setCalibrating(bool value);
    void setTurn(string value);
    void setConnection(bool value);
    void setVision(const cv::Mat& image);
    void changeState(string state); 

    int  getDifficulty();
    bool getGameActive();
    bool getGameResetting();
    bool getGameRunning();
    bool getGamePaused();
    bool getCalibrating();
    string getTurn();
    bool getConnection();
    cv::Mat getVision() const;

private:
    mutable mutex cvMutex;

    cv::Mat cvImage;
    
    string turn;
    bool connection = false;
    bool gameActive = false;
    bool resettingActive = false;
    bool gameRunning = false;
    bool gamePaused = false;
    bool calibrating = false;
    bool gameInitialized = false;
    int difficulty = 300;
};

extern GUI gui;