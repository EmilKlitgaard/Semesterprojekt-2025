#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>

using namespace ur_rtde;
using namespace std;
using namespace Eigen;

#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0)

// Function to create a rotation matrix in degrees around Z-axis
Matrix3d getRotationMatrixZ(double angleDeg) {
    double angleRad = DEG_TO_RAD(angleDeg);
    Matrix3d Rotation;
    Rotation << 
    	cos(angleRad), 	-sin(angleRad),	0,
   	sin(angleRad), 	cos(angleRad), 	0,
        0,              0,             	1;
    return Rotation;
}

// Function to create a rotation matrix in degrees around Y-axis
Matrix3d getRotationMatrixY(double angleDeg) {
    double angleRad = DEG_TO_RAD(angleDeg);
    Matrix3d Rotation;
    Rotation << 
    	cos(angleRad), 	0,	sin(angleRad),
   	0, 		1, 	0,
        -sin(angleRad), 0,     	cos(angleRad);
    return Rotation;
}

// Convert from Base Frame to Chessboard Frame
Vector3d baseToChessboard(const Vector3d &basePoint, const Vector3d &chessboardOrigin, const Matrix3d &R) {
    return R.transpose() * (basePoint - chessboardOrigin);
}

// Convert from Chessboard Frame to Base Frame
Vector3d chessboardToBase(const Vector3d &chessPoint, const Vector3d &chessboardOrigin, const Matrix3d &R) {
    return R * chessPoint + chessboardOrigin;
}

// Move TCP to a specific chessboard coordinate
void moveToChessboardPoint(RTDEControlInterface &rtde_control, const Vector3d &chessboardTarget, const Vector3d &chessboardOrigin, const Matrix3d &R) {
    // Convert chessboard target to base frame
    Vector3d baseTarget = chessboardToBase(chessboardTarget, chessboardOrigin, R);

    // Get current TCP pose
    vector<double> tcpPose = rtde_control.getForwardKinematics();

    // Set new XYZ but keep current orientation
    tcpPose[0] = baseTarget[0];
    tcpPose[1] = baseTarget[1];
    tcpPose[2] = baseTarget[2];
    tcpPose[3] = 0;
    tcpPose[4] = M_PI;
    tcpPose[5] = 0;
    
    cout << "Moving TCP to Chessboard Point: [" << chessboardTarget.transpose() << "]" << endl;
    rtde_control.moveL(tcpPose, 0.5, 0.3); // Move with 0.5 m/s speed and 0.3 acceleration
}

// Move TCP by an offset in the chessboard frame
void moveRelativeInChessboard(RTDEControlInterface &rtde_control, RTDEReceiveInterface &rtde_receive, 
                              const Vector3d &chessboardOrigin, const Matrix3d &R, const Vector3d &offset) {
    // Get current TCP pose
    vector<double> tcpPose = rtde_receive.getActualTCPPose();
    Vector3d basePosition(tcpPose[0], tcpPose[1], tcpPose[2]);

    // Convert base position to chessboard frame
    Vector3d chessboardPosition = baseToChessboard(basePosition, chessboardOrigin, R);

    // Apply movement in chessboard frame
    Vector3d newChessboardPosition = chessboardPosition + offset;

    // Convert back to base frame
    Vector3d newBasePosition = chessboardToBase(newChessboardPosition, chessboardOrigin, R);

    // Keep current orientation
    tcpPose[0] = newBasePosition[0];
    tcpPose[1] = newBasePosition[1];
    tcpPose[2] = newBasePosition[2];

    cout << "Moving TCP by offset [" << offset.transpose() << "] in Chessboard Frame" << endl;
    rtde_control.moveL(tcpPose, 0.5, 0.3);
}

int main() {
    string robotIp = "192.168.1.54";
    int robotPort = 50002;
    RTDEControlInterface rtde_control(robotIp, robotPort);
    RTDEReceiveInterface rtde_receive(robotIp, robotPort);

    rtde_control.teachMode(); // Enable freemove mode
    cout << "Freemove mode enabled. Move the robot to the chessboard A1 corner and press Enter." << endl;
    cin.get(); // Wait for user input
    rtde_control.endTeachMode(); // Disable freemove mode
    cout << "Freemove mode disabled. Saving chessboard frame origin." << endl;

    // Get TCP position in base frame
    vector<double> tcpPose = rtde_receive.getActualTCPPose();
    Vector3d chessboardOrigin(tcpPose[0], tcpPose[1], tcpPose[2]);

    // Define rotation matrix for chessboard frame (22.5 degree base offset and -90 degree alignment)
    Matrix3d RotationChess = getRotationMatrixZ(22.5 - 90);

    // Print chessboard frame information
    cout << "Chessboard Frame Origin (Base Frame): [" << chessboardOrigin.transpose() << "]" << endl;

    // Test move 1: Move to a specific point in chessboard frame
    Vector3d chessboardTarget(0.1, 0.0, 0.0);
    moveToChessboardPoint(rtde_control, chessboardTarget, chessboardOrigin, RotationChess);
    
    Vector3d chessboardTarget1(0.1, 0.1, 0.0);
    moveToChessboardPoint(rtde_control, chessboardTarget1, chessboardOrigin, RotationChess);
    
    Vector3d chessboardTarget2(0.1, 0.1, 0.1);
    moveToChessboardPoint(rtde_control, chessboardTarget2, chessboardOrigin, RotationChess);

    // Wait for user input before next move
    cout << "Press Enter for relative movement in chessboard frame..." << endl;
    cin.get();

    // Test move 2: Move relatively in chessboard frame
    Vector3d chessboardOffset(0.05, -0.05, 0.0); // Move 5cm right, 5cm back in chessboard frame
    moveRelativeInChessboard(rtde_control, rtde_receive, chessboardOrigin, RotationChess, chessboardOffset);
    
    Vector3d chessboardOffset2(0.0, 0.0, 0.1); // Move 5cm right, 5cm back in chessboard frame
    moveRelativeInChessboard(rtde_control, rtde_receive, chessboardOrigin, RotationChess, chessboardOffset2);

    return 0;
}
