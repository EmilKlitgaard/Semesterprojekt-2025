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
    void setTurn(string value);
    void setConnection(bool value);
    void setVision(const cv::Mat& image);

    void awaitStartGame();

    int  getDifficulty();
    bool getGameActive();
    string getTurn();
    bool getConnection();
    cv::Mat getVision() const;

private:
    mutable mutex cvMutex;

    cv::Mat cvImage;
    
    string turn;
    bool connection = false;
    bool gameActive = false;
    int difficulty = 300;
};

extern GUI gui;