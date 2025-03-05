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

// Move TCP to a the awaiting position
void moveToAwaitPosition(RTDEControlInterface &rtde_control) {
    vector<double> awaitPosition = {-1.11701, -0.89012, -1.78024, -0.52360, 1.57080, 0.71558}; 
    rtde_control.moveJ(awaitPosition, 1.0, 0.3);
}

// Move TCP to a specific chessboard coordinate
void moveToChessboardPoint(RTDEControlInterface &rtde_control, RTDEReceiveInterface &rtde_receive, const Vector3d &chessboardTarget, const Vector3d &chessboardOrigin, const Matrix3d &R) {
    // Convert chessboard target to base frame
    Vector3d baseTarget = chessboardToBase(chessboardTarget, chessboardOrigin, R);

    // Get current TCP pose
    vector<double> tcpPose = rtde_receive.getActualTCPPose();

    // Set new XYZ and orientation
    tcpPose[0] = baseTarget[0];
    tcpPose[1] = baseTarget[1];
    tcpPose[2] = baseTarget[2];
    tcpPose[3] = 0;
    tcpPose[4] = M_PI;
    tcpPose[5] = 0;
    
    cout << "Moving TCP to Chessboard Point: [" << chessboardTarget.transpose() << "]" << endl;
    rtde_control.moveL(tcpPose, 1.0, 0.3); // Move with 0.5 m/s speed and 0.3 acceleration
}

int main() {
    /*   -----   SETUP UR5 CONNECTION   -----   */
    string robotIp = "192.168.1.54";
    int robotPort = 50002;
    RTDEControlInterface rtde_control(robotIp, robotPort);
    RTDEReceiveInterface rtde_receive(robotIp, robotPort);
    
    // Check if the connection is successful
    if (!rtde_control.isConnected() || !rtde_receive.isConnected()) {
        cerr << "Failed to connect to the robot at " << robotIp << ":" << robotPort << endl;
        return -1;
    }
    
    
    /*   -----   SET TCP OFFSET   -----   */
    vector<double> tcpOffset = {0.0, 0.0, 0.2, 0.0, 0.0, 0.0};
    rtde_control.setTcp(tcpOffset);
    
    
    /*   -----   SET CHESSBOARD ORIGIN   -----   */
    rtde_control.teachMode(); // Enable freemove mode
    
    cout << "Freemove mode enabled. Move the robot pointer to the chessboard A1 corner and press ENTER." << endl;
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


    /*   -----   BEGIN MAIN CODE   -----   */
    // Test move 1: Move to a specific point in chessboard frame
    moveToAwaitPosition(rtde_control);
    
    Vector3d chessboardTarget(0.0, 0.0, 0.0);
    moveToChessboardPoint(rtde_control, rtde_receive, chessboardTarget, chessboardOrigin, RotationChess);
    
    Vector3d chessboardTarget1(0.1, 0.0, 0.0);
    moveToChessboardPoint(rtde_control, rtde_receive, chessboardTarget1, chessboardOrigin, RotationChess);
    
    Vector3d chessboardTarget2(0.1, 0.0, 0.1);
    moveToChessboardPoint(rtde_control, rtde_receive, chessboardTarget2, chessboardOrigin, RotationChess);


    /*   -----   BEGIN CHESS GAME   -----   */
    // Wait for user input
    cout << "Press ENTER to start chess game..." << endl;
    cin.get();
    
    // Setup chess matrix
    chessboard
    
    cout << "CHESS GAME STARTED" << endl;
    
    while (true) {
        lastPositions = getCameraData(); // Get the image data from the camera. The data include the positions of all of the pieces. 
        cout << "Make your move and press ENTER..." << endl;
        cin.get();
        newPositions = getCameraData(); // Get the image data from the camera. The data include the positions of all of the pieces. 
        determinePlayerMove(); // Algorithm to determine the diffrence in the two images. Hereby the moved piece. 
        if (anyKingDied()) {
            moveToAwaitPosition(rtde_control);
            cout << "Game has ended" << endl;
            return 0;
        }
        updateChessboard(); // Update the chessboard matrix.
        
        stockfishMove(); // Send the player move to Stockfish and recieve a counter move. 
        updateChessooard(); // Update the chessboard matrix.
        
        fromCoordinate, toCoordinate = getChessPieceCoordinates(); // Get the coresponding XYZ coordinates from the chessboard matrix.
        moveChessPiece(); // Move 0.1m above the <fromCoordinate>, Open gripper, Move down to the piece, Close the gripper, Move up 0.1m, Move 0.1m above the <toCoordinate>, Move down to the location, Open the gripper, Move up 0.1m. (If the <toCoordinate> is already occupied, then first move that piece to the <deadPieceLocation>).
        if (anyKingDied()) {
            moveToAwaitPosition(rtde_control);
            cout << "Game has ended" << endl;
            return 0;
        }
        moveToAwaitPosition(rtde_control);
    }

    return 0;
}
