#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <string>

#include "Chessboard.h"

using namespace ur_rtde;
using namespace std;
using namespace Eigen;

using ChessboardMatrix = vector<vector<char>>;

#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0)

// Function to create a rotation matrix in degrees around Z-axis
Matrix3d getRotationMatrixZ(double angleDeg) {
    double angleRad = DEG_TO_RAD(angleDeg);
    Matrix3d Rotation;
    Rotation << 
        cos(angleRad), -sin(angleRad), 0,
        sin(angleRad),  cos(angleRad), 0,
        0,             0,            1;
    return Rotation;
}

// Convert from Chessboard Frame to Base Frame
Vector3d chessboardToBase(const Vector3d &chessPoint, const Vector3d &chessboardOrigin, const Matrix3d &R) {
    return R * chessPoint + chessboardOrigin;
}

// Move TCP to the awaiting position
void moveToAwaitPosition(RTDEControlInterface &rtde_control) {
    vector<double> awaitPosition = {-1.11701, -0.89012, -1.78024, -0.52360, 1.57080, 0.71558}; 
    rtde_control.moveJ(awaitPosition, 1.0, 0.3);
}

// Move TCP to a specific chessboard coordinate
void moveToChessboardPoint(RTDEControlInterface &rtde_control, RTDEReceiveInterface &rtde_receive, 
                           const Vector3d &chessboardTarget, const Vector3d &chessboardOrigin, 
                           const Matrix3d &R) {
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
    rtde_control.moveL(tcpPose, 1.0, 0.3);
}

// Placeholder: Simulate camera data retrieval.
ChessboardMatrix getCameraData() {
    ChessboardMatrix camBoard(8, vector<char>(8, 'e'));
    // Example simulation: Place a white (W) piece at A2 and a black (B) piece at A7.
    camBoard[1][0] = 'W'; // Row 1, Col 0 => A2
    camBoard[6][0] = 'B'; // Row 6, Col 0 => A7
    return camBoard;
}

// Compare last and new camera data to determine which cell changed.
// Returns a pair: {from (row, col), to (row, col)}.
pair<pair<int, int>, pair<int, int>> determinePlayerMove(const ChessboardMatrix &lastPositions, const ChessboardMatrix &newPositions) {
    pair<int, int> from = {-1, -1};
    pair<int, int> to = {-1, -1};
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (lastPositions[i][j] != 'e' && newPositions[i][j] == 'e') {
                from = {i, j}; // Piece moved away from here.
            }
            if (lastPositions[i][j] == 'e' && newPositions[i][j] != 'e') {
                to = {i, j}; // Piece moved into here.
            }
        }
    }
    return {from, to};
}

// Placeholder: Check if any king is dead (to end the game)
bool anyKingIsDead() {
    // Logic for checking kings (to be implemented)
    return false;
}

// Returns a move string in chess notation, e.g., "A2A4".
string stockfishMove() {
    cout << "Stockfish is calculating the best move..." << endl;
    string move = "A2A4"; // Dummy move
    cout << "Stockfish move: " << move << endl;
    return move;
}

void moveChessPiece(RTDEControlInterface &rtde_control, const vector<double> &fromCoordinate, 
                    const vector<double> &toCoordinate, bool toOccupied) {
    // If the destination cell is occupied, move the piece there first to a dead piece location.
    if (toOccupied) {
        vector<double> deadPieceLocation = {0.5, 0.5, 0.1, 0, M_PI, 0}; // Dummy dead-piece location
        cout << "Moving occupying piece to dead piece location..." << endl;
        rtde_control.moveL(deadPieceLocation, 0.5, 0.3);
    }
    // Sequence of movements for the moving piece:
    cout << "Opening gripper..." << endl;
    // rtde_control.openGripper(); // Placeholder for actual gripper command
    cout << "Moving down to piece at fromCoordinate..." << endl;
    rtde_control.moveL(fromCoordinate, 0.5, 0.3);
    cout << "Closing gripper..." << endl;
    // rtde_control.closeGripper(); // Placeholder for actual gripper command
    cout << "Lifting piece 0.1m..." << endl;
    vector<double> lift = fromCoordinate;
    lift[2] += 0.1;
    rtde_control.moveL(lift, 0.5, 0.3);
    cout << "Moving above destination coordinate..." << endl;
    vector<double> aboveDest = toCoordinate;
    aboveDest[2] += 0.1;
    rtde_control.moveL(aboveDest, 0.5, 0.3);
    cout << "Moving down to destination coordinate..." << endl;
    rtde_control.moveL(toCoordinate, 0.5, 0.3);
    cout << "Opening gripper to release piece..." << endl;
    // rtde_control.openGripper(); // Placeholder for actual gripper command
    cout << "Lifting piece 0.1m after release..." << endl;
    rtde_control.moveL(aboveDest, 0.5, 0.3);
}

/*============================================================
            MAIN START
============================================================*/
int main() {
    /*   ==========   SETUP UR5 CONNECTION   ==========   */
    string robotIp = "192.168.1.54";
    int robotPort = 50002;
    RTDEControlInterface rtde_control(robotIp, robotPort);
    RTDEReceiveInterface rtde_receive(robotIp, robotPort);
    
    // Check if the connection is successful
    if (!rtde_control.isConnected() || !rtde_receive.isConnected()) {
        cerr << "Failed to connect to the robot at " << robotIp << ":" << robotPort << endl;
        return -1;
    }
    
    /*   ==========   SET TCP OFFSET   ==========   */
    vector<double> tcpOffset = {0.0, 0.0, 0.2, 0.0, 0.0, 0.0};
    rtde_control.setTcp(tcpOffset);
    
    /*   ==========   SET CHESSBOARD ORIGIN   ==========   */
    rtde_control.teachMode(); // Enable freemove mode
    cout << "Freemove mode enabled. Move the robot pointer to the chessboard A1 corner and press ENTER." << endl;
    cin.get(); // Wait for user input
    rtde_control.endTeachMode(); // Disable freemove mode
    cout << "Freemove mode disabled. Saving chessboard frame origin." << endl;
    vector<double> tcpPose = rtde_receive.getActualTCPPose();
    Vector3d chessboardOrigin(tcpPose[0], tcpPose[1], tcpPose[2]);
    
    // Define rotation matrix for chessboard frame (22.5° base offset and -90° alignment)
    Matrix3d RotationChess = getRotationMatrixZ(22.5 - 90);
    cout << "Chessboard Frame Origin (Base Frame): [" << chessboardOrigin.transpose() << "]" << endl;
    
    /*   ==========   BEGIN PRE-GAME MOVEMENTS   ==========   */
    moveToAwaitPosition(rtde_control);
    
    Vector3d chessboardTarget(0.0, 0.0, 0.0);
    moveToChessboardPoint(rtde_control, rtde_receive, chessboardTarget, chessboardOrigin, RotationChess);
    
    Vector3d chessboardTarget1(0.1, 0.0, 0.0);
    moveToChessboardPoint(rtde_control, rtde_receive, chessboardTarget1, chessboardOrigin, RotationChess);
    
    Vector3d chessboardTarget2(0.1, 0.0, 0.1);
    moveToChessboardPoint(rtde_control, rtde_receive, chessboardTarget2, chessboardOrigin, RotationChess);
    
    /*   ==========   BEGIN CHESS GAME   ==========   */
    Chessboard board;
    cout << "Press ENTER to start chess game..." << endl;
    cin.get();    
    cout << "CHESS GAME STARTED" << endl;
    
    while (true) {
        // Retrieve camera data before and after player's move.
        ChessboardMatrix lastPositions = getCameraData();
        cout << "Make your move and press ENTER..." << endl;
        cin.get();
        ChessboardMatrix newPositions = getCameraData();
        
        // Determine the player's move.
        auto moveIndices = determinePlayerMove(lastPositions, newPositions); 
        int fromRow = moveIndices.first.first;
        int fromCol = moveIndices.first.second;
        int toRow   = moveIndices.second.first;
        int toCol   = moveIndices.second.second;
        
        // Update our internal chessboard with the player's move.
        board.updateChessboard({fromRow, fromCol}, {toRow, toCol});
        board.printBoard(); 
        
        if (anyKingIsDead()) {
            moveToAwaitPosition(rtde_control);
            cout << "Game has ended" << endl;
            return 0;
        }
        
        // Get the move from Stockfish (in chess notation, e.g., "A2A4")
        string stockfishMoveStr = stockfishMove();
        
        // Convert Stockfish move to from- and to- notations.
        string fromNotation = stockfishMoveStr.substr(0, 2);
        string toNotation   = stockfishMoveStr.substr(2, 2);
        
        // Get the physical coordinates for these cells.
        vector<double> fromCoordinate = board.getPhysicalCoordinates(fromNotation);
        vector<double> toCoordinate   = board.getPhysicalCoordinates(toNotation);
        
        // Check if the destination cell is already occupied.
        bool toOccupied = false;
        auto boardState = board.getBoardState();
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (board.getChessNotation({i, j}) == toNotation && boardState[i][j] != "0") {
                    toOccupied = true;
                }
            }
        }
        
        // Move the chess piece using the robot.
        moveChessPiece(rtde_control, fromCoordinate, toCoordinate, toOccupied);
        
        if (anyKingIsDead()) {
            moveToAwaitPosition(rtde_control);
            cout << "Game has ended" << endl;
            return 0;
        }
        
        // After robot move, update the board accordingly.
        int fromRowIdx = fromNotation[1] - '1';
        int fromColIdx = fromNotation[0] - 'A';
        int toRowIdx   = toNotation[1] - '1';
        int toColIdx   = toNotation[0] - 'A';
        board.updateChessboard({fromRowIdx, fromColIdx}, {toRowIdx, toColIdx});
        board.printBoard();
        
        moveToAwaitPosition(rtde_control);
    }
    
    return 0;
}
