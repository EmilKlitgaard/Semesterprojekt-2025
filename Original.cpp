#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <unistd.h>

int main()
{
    // Define the robot's IP address and custom port
    std::string robot_ip = "192.168.1.54";
    int port = 50002;

    // Create RTDE control and receive interfaces
    ur_rtde::RTDEControlInterface rtde_control(robot_ip, port);
    ur_rtde::RTDEReceiveInterface rtde_receive(robot_ip, port);

    // Check if the connection is successful
    if (!rtde_control.isConnected() || !rtde_receive.isConnected())
    {
        std::cerr << "Failed to connect to the robot at " << robot_ip << ":" << port << std::endl;
        return -1;
    }
    
    const double pi = 3.141592653589793238462643383;

    // Move to a safe joint configuration first
    double speed = 3;        // Joint speed (rad/s)
    double acceleration = 3.0; // Joint acceleration (rad/s^2)
    
    std::vector<double> start = {-(3*pi)/8+0.05, -pi/4+0.05, 0.0, -pi/2, 0.0, 0.0};
    std::vector<double> first = {0.3, -0.3, 0.4, 0.0, 0.0, 0.0};
    
    rtde_control.moveJ(start, speed, acceleration);
    std::cout << "Moved to: ";
    for (const auto& val : start) std::cout << val << " ";
    std::cout << std::endl;
    sleep(1);

    rtde_control.moveJ(first, speed, acceleration);
    std::cout << "Moved to: ";
    for (const auto& val : first) std::cout << val << " ";
    std::cout << std::endl;
    sleep(1);

    // Stop the RTDE control script
    rtde_control.stopScript();

    return 0;
}
