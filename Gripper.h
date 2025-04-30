#pragma once

#include <string>

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
