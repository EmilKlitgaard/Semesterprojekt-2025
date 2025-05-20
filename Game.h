#pragma once

#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <string>
#include <fstream>
#include <unordered_map>
#include <QtWidgets/QApplication>
#include <memory>
#include <thread>

using namespace std;

class Game {
    public:
        void initializeGame();
        void startGame();

        void resetChessboard();

    private:
        thread cameraThread;
        thread gameThread;
};

extern Game game;