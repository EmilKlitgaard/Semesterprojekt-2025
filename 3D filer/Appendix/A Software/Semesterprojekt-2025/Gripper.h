#pragma once

#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <stdexcept>

class Gripper {
public:
    Gripper(const std::string& portName);
    ~Gripper();

    void openGripper();
    void closeGripper();
    void stopGripper();

private:
    int serial_fd;

    bool configurePort();
    bool sendRaw(const std::string& cmd);
    bool waitFor(const std::string& token, double timeoutSeconds);
};