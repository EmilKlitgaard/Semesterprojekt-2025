#include <ur_rtde/rtde_control_interface.h> 
#include <ur_rtde/rtde_receive_interface.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <string>
#include <thread>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <unistd.h>
#include "Chessboard.h"
#include "Gripper.h"

using namespace ur_rtde;
using namespace std;
using namespace Eigen;

using ChessboardPieces = vector<vector<string>>;
using ChessboardMatrix = vector<vector<char>>;
using MatrixIndex = pair<int, int>;

#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0)

string robotIp = "192.168.1.54";
int robotPort = 50002;
RTDEControlInterface rtde_control(robotIp, robotPort);
RTDEReceiveInterface rtde_receive(robotIp, robotPort);

Chessboard board;
Gripper gripper("/dev/ttyACM0");

Vector3d transformVector = {0.0, 0.0, 0.1};

/*============================================================
            		   FUNCTIONS
============================================================*/
// Function to create a rotation matrix in degrees around Z-axis
void printText(string text) {
    cout << text << endl;
}

Matrix3d getRotationMatrixZ(double angleDeg) {
    double angleRad = DEG_TO_RAD(angleDeg);
    Matrix3d Rotation;
    Rotation << 
        cos(angleRad),  -sin(angleRad), 0,
        sin(angleRad),  cos(angleRad),  0,
        0,              0,              1;
    return Rotation;
}

// Convert from Chessboard Frame to Base Frame
Vector3d chessboardToBase(const Vector3d &chessboardTarget, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    return RotationMatrix * chessboardTarget + chessboardOrigin;
}

bool isValidMoveFormat(const string& move) {
    // Check length
    if (move.length() != 4) return false;

    // Check repeated input
    if (move[0]+move[1] == move[2]+move[3]) return false;
    
    // Check characters
    bool validFirst = islower(move[0]) && move[0] >= 'a' && move[0] <= 'h';
    bool validSecond = isdigit(move[1]) && move[1] >= '1' && move[1] <= '8';
    bool validThird = islower(move[2]) && move[2] >= 'a' && move[2] <= 'h';
    bool validFourth = isdigit(move[3]) && move[3] >= '1' && move[3] <= '8';
    
    return validFirst && validSecond && validThird && validFourth;
}

Vector3d setChessboardOrigin(bool calibrate = false){
    string filename = "calibration_data.txt";
    Vector3d chessboardOrigin;

    // Load from file if calibration is not needed
    if (!calibrate) {
        ifstream file(filename);
        if (file.is_open()) {
            double x, y, z;
            file >> x >> y >> z;
            file.close();
            chessboardOrigin = Vector3d(x, y, z);
            cout << "Loaded calibration data: [" << chessboardOrigin.transpose() << "]" << endl;
            return chessboardOrigin;
        } else {
            printText("Calibration file not found! Running calibration...");
        }
    }

    // Perform calibration
    rtde_control.teachMode(); // Enable freemove mode
    printText("Freemove mode enabled. Move the robot pointer to the chessboard A1 corner and press ENTER.");
    cin.get(); // Wait for user input
    rtde_control.endTeachMode(); // Disable freemove mode
    printText("Freemove mode disabled. Saving chessboard frame origin.");

    vector<double> tcpPose = rtde_receive.getActualTCPPose();
    cout << "Captured TCP pose coordinates: (" << tcpPose[0] << ", " << tcpPose[1] << ", " << tcpPose[2] << ")" << endl;
    chessboardOrigin = Vector3d(tcpPose[0], tcpPose[1], tcpPose[2]);

    // Save calibration data
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << chessboardOrigin.x() << " " << chessboardOrigin.y() << " " << chessboardOrigin.z() << endl;
        outFile.close();
        printText("Calibration data saved.");
    } else {
        printText("Error saving calibration data!");
    }
    return chessboardOrigin;
}

// Move TCP to the awaiting position
void moveToAwaitPosition() {
    vector<double> awaitPosition = {-1.11701, -0.89012, -1.78024, -0.52360, 1.57080, 0.71558}; 
    rtde_control.moveJ(awaitPosition, 1.0, 0.3);
}

// Move TCP to a specific chessboard coordinate
void moveToChessboardPoint(const Vector3d &chessboardTarget, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix, double speed = 0.5, double acceleration = 0.5) {
    // Convert chessboard target to base frame
    Vector3d baseTarget = chessboardToBase(chessboardTarget, chessboardOrigin, RotationMatrix);
    vector<double> tcpPose = {baseTarget[0], baseTarget[1], baseTarget[2], 0.0, M_PI, 0.0};
    //cout << "Moving TCP to Chessboard Point: [" << chessboardTarget.transpose() << "]" << endl;
    rtde_control.moveL(tcpPose, speed, acceleration);
}

// Check if all positions within the calibrated chessboard are reachable
bool AllPositionsReachable(const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    printText("Checking for reachability...");
    vector<Vector3d> allPositions = board.getAllPhysicalCoordinates();
    for (const auto& position : allPositions) {
        Vector3d baseTarget = chessboardToBase(position, chessboardOrigin, RotationMatrix);
        vector<double> tcpPose = {baseTarget[0], baseTarget[1], baseTarget[2]};
        // cout << "Checking reachability for coordinate: (" << tcpPose[0] << ", " << tcpPose[1] << ", " << tcpPose[2] << ")" << endl;
        vector<double> JntPos = {1,1,1}; // rtde_control.getInverseKinematics(tcpPose);
        if (JntPos.empty()) {
            cout << "Target pose" << baseTarget.transpose() << " is not reachable by the robot" << endl;
            return false;
        }
    }
    cout << "All target poses are reachable" << endl;
    return true;
}

void pickUpPiece(const Vector3d &position, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
    moveToChessboardPoint(position, chessboardOrigin, RotationMatrix);
    gripper.closeGripper();
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
}

void placePiece(const Vector3d &position, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
    moveToChessboardPoint(position, chessboardOrigin, RotationMatrix);
    gripper.openGripper();
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
}

std::atomic<bool> endLoop(false);
std::atomic<int> pickUpCnt(0);

void pickUpCycle(const Vector3d &fromCoordinates, const Vector3d &toCoordinates, const Vector3d &chessboardOrigin, const Matrix3d &RotationChess) {
    while(pickUpCnt <= 100) {
        pickUpPiece(fromCoordinates, chessboardOrigin, RotationChess);
        placePiece(toCoordinates, chessboardOrigin, RotationChess);
        pickUpCnt++;
        cout << "Pickup count: " << pickUpCnt << endl;
        pickUpPiece(toCoordinates, chessboardOrigin, RotationChess);
        placePiece(fromCoordinates, chessboardOrigin, RotationChess);
        pickUpCnt++;
        cout << "Pickup count: " << pickUpCnt << endl;
    }
}

void waitForEnter() {
    cin.get();
    endLoop = true;
    moveToAwaitPosition();
    cout << "\nLoop interrupted. Total pick-ups: " << pickUpCnt << endl;
    exit(0);
}

    

/*============================================================
            		    MAIN START
============================================================*/
int main() {
    //   ==========   VALIDATE UR5 CONNECTION   ==========   //
    if (!rtde_control.isConnected() || !rtde_receive.isConnected()) {
        cerr << "Failed to connect to the robot at " << robotIp << ":" << robotPort << endl;
        return -1;
    }
    
    //   ==========   SET CALIBRATION TOOL TCP OFFSET   ==========   //
    vector<double> tcpCalibrationOffset = {0.0, 0.0, 0.1, 0.0, 0.0, 0.0};
    rtde_control.setTcp(tcpCalibrationOffset);

    //   ==========   SET CHESSBOARD ORIGIN   ==========   //
    Vector3d chessboardOrigin = setChessboardOrigin(false);
    
    // Define rotation matrix for chessboard frame (22.5° base offset and -90° alignment)
    Matrix3d RotationChess = getRotationMatrixZ(22.5 - 90);
    //cout << "Chessboard Frame Origin (Base Frame): [" << chessboardOrigin.transpose() << "]" << endl;

    // Check if all positions within the calibrated chessboard are reachable
    while (!AllPositionsReachable(chessboardOrigin, RotationChess)) {
        chessboardOrigin = setChessboardOrigin(true);
    }

    //   ==========   UPDATE TCP OFFSET   ==========   //
    vector<double> tcpOffset = {0.0, 0.0, 0.2, 0.0, 0.0, 0.0};
    rtde_control.setTcp(tcpOffset);
    
    //   ==========   BEGIN PRE-GAME MOVEMENTS   ==========   //
    moveToAwaitPosition();
    Vector3d chessboardTarget1(0.0, 0.0, 0.0);
    moveToChessboardPoint(chessboardTarget1, chessboardOrigin, RotationChess);
    Vector3d chessboardTarget2(0.4, 0.4, 0.0);
    moveToChessboardPoint(chessboardTarget2, chessboardOrigin, RotationChess);
    moveToAwaitPosition();

    string move = "e3e5";
    auto [fromCoordinate, toCoordinate] = board.getPhysicalCoordinates(move);
   
    //   ==========   BEGIN MOVEMENTS   ==========   //
    thread inputThread(waitForEnter);
    pickUpCycle(fromCoordinate, toCoordinate, chessboardOrigin, RotationChess);  // runs in main thread

    inputThread.join();  // wait for user input to finish

    return 0;   
}