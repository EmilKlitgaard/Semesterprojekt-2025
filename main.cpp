#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <string>
#include <thread>
#include <fstream>

#include "Chessboard.h"
#include "Stockfish.h"
#include "Vision.h"

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

string inputPlayerMove() {
    string playerMove;
    printText("Input your move e.g.: 'e2e4': ");
    while (true) {
        cin >> playerMove;
        if (isValidMoveFormat(playerMove)) {
            return playerMove;
        } else {
            printText("Invalid move format. Please enter a move in the format 'e2e4': ");
        }
    }
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
    cout << "Moving TCP to Chessboard Point: [" << chessboardTarget.transpose() << "]" << endl;
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

// Compare camera data to determine which cell changed. Returns a pair: {from (row, col), to (row, col)}.
pair<MatrixIndex, MatrixIndex> determinePlayerMove(const ChessboardMatrix &lastPositions, const ChessboardMatrix &newPositions) {
    MatrixIndex from = {-1, -1};
    MatrixIndex to = {-1, -1};
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            printText("START TEST");
            cout << lastPositions[0][0] << endl;
            if (lastPositions[i][j] == 'W' && newPositions[i][j] == 'e') {
                if (from.first != -1 || from.second != -1) {
                    printText("ERROR DETECTED 1");
                    return {{-1, -1}, {-1, -1}}; // Error: Multiple moves detected
                }
                printText("FROM DETECTED");
                from = {i, j}; // Piece moved away from here.
            } else if (lastPositions[i][j] == 'B' && newPositions[i][j] == 'W') {
                if (to.first != -1 || to.second != -1) {
                    printText("ERROR DETECTED 2");
                    return {{-1, -1}, {-1, -1}}; // Error: Multiple moves detected
                }
                printText("TO 1 DETECTED");
                string pieceName = "Test";
                Vector3d deadPieceLocation = board.getDeadPieceLocation(pieceName, "Robot");
                to = {i, j}; // Piece moved into here.
            } else if (lastPositions[i][j] == 'e' && newPositions[i][j] == 'W') {
                if (to.first != -1 || to.second != -1) {
                    printText("ERROR DETECTED 3");
                    return {{-1, -1}, {-1, -1}}; // Error: Multiple moves detected
                }
                printText("TO 2 DETECTED");
                to = {i, j}; // Piece moved into here.
            }
            printText("END TEST");
        }
    }
    cout << "Player moved from: (" << from.first << "," << from.second << "), to: (" << to.first << "," << to.second << ")." << endl;
    return {from, to};
}

// Returns a move string in chess notation, e.g., "e2e4".
string stockfishMove(Stockfish &engine, const string &latestMove) {
    cout << "Stockfish is calculating the best move..." << endl;
    string bestMove = engine.getBestMove(latestMove);
    cout << "Stockfish best move: " << bestMove << endl;
    return bestMove;
}

bool isOccupied(string &toNotation) {
    ChessboardPieces boardState = board.getBoardState();
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board.getChessNotation({i, j}) == toNotation && boardState[i][j] != "0") { 
                cout << "Destination cell is occupied." << endl;
                return true; 
            }
        }
    }
    return false;
}

pair<string, string> getPieceName(string notation) {
    ChessboardPieces boardState = board.getBoardState();
    auto [fromIndex, toIndex] = board.getMatrixIndex(notation);    
    string fromPieceName = boardState[fromIndex.first][fromIndex.second];
    string toPieceName = boardState[toIndex.first][toIndex.second];
    return {fromPieceName, toPieceName};
}

void pickUpPiece(const Vector3d &position, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
    moveToChessboardPoint(position, chessboardOrigin, RotationMatrix);
    // closeGripper();
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
}

void placePiece(const Vector3d &position, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
    moveToChessboardPoint(position, chessboardOrigin, RotationMatrix);
    // openGripper();
    moveToChessboardPoint(position + transformVector, chessboardOrigin, RotationMatrix);
}

void moveChessPiece(string &robotMove, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    // Get the physical coordinates for the cells.
    auto [fromCoordinate, toCoordinate] = board.getPhysicalCoordinates(robotMove);

    // Print the robot moves
    cout << "Robot move piece from Coordinate: (" << fromCoordinate.transpose() << "), to Coordinate: (" << toCoordinate.transpose() << ")." << endl;
    
    // Get the piece names for later use
    auto [fromPieceName, toPieceName] = getPieceName(robotMove);

    // Check if the destination cell is already occupied.
    string toNotation = robotMove.substr(2, 2);
    if (isOccupied(toNotation)) {
        printText("Moving occupying piece to dead piece location...");
        Vector3d deadPieceLocation = board.getDeadPieceLocation(toPieceName, "Player");

        pickUpPiece(toCoordinate, chessboardOrigin, RotationMatrix);
        placePiece(deadPieceLocation, chessboardOrigin, RotationMatrix);
    }

    // Move the selected piece
    pickUpPiece(fromCoordinate, chessboardOrigin, RotationMatrix);
    placePiece(toCoordinate, chessboardOrigin, RotationMatrix);

    // Check for pawn promotion
    if (robotMove.length() == 5) {
        printText("Moving current piece to dead piece location...");
        Vector3d deadPieceLocation = board.getDeadPieceLocation(fromPieceName, "Robot");

        pickUpPiece(toCoordinate, chessboardOrigin, RotationMatrix);
        placePiece(deadPieceLocation, chessboardOrigin, RotationMatrix);

        printText("Retrieving pawnPromotedPiece from dead piece location...");
        string promotionType = robotMove.substr(4, 1); 
        string promotionPieceName;
        if (promotionType == "q") { 
            promotionPieceName = "5B";  // Queen
        } else if (promotionType == "r") { 
            promotionPieceName = "2B";  // Rook
        } else if (promotionType == "b") { 
            promotionPieceName = "4B";  // Bishop
        } else if (promotionType == "n") { 
            promotionPieceName = "3B";  // Knight
        } else {
            throw invalid_argument("Stockfish pawnPromotion output error. Stockfish output: " + robotMove);
        }
        Vector3d promotionPieceLocation = board.searchDeadPieceLocation(promotionPieceName);

        pickUpPiece(promotionPieceLocation, chessboardOrigin, RotationMatrix);
        placePiece(toCoordinate, chessboardOrigin, RotationMatrix);
    }
}

// pair<MatrixIndex, MatrixIndex> getCameraData(ChessVision chessVision) {
//     while (true) {
//         // Retrieve camera data before and after player's move.
//         printText("Place camera in frame and press ENTER...");
//         cin.get();
//         ChessboardMatrix lastPositions = chessVision.processCurrentFrame();
//         printText("Make your move and press ENTER...");
//         cin.get();
//         ChessboardMatrix newPositions = chessVision.processCurrentFrame();
        
//         // Determine the player's move. Converts camera output matrix(8x8) to move index e.g.: ({2.1}, {2.3}).
//         auto [playerFromIndex, playerToIndex] = determinePlayerMove(lastPositions, newPositions);
        
//         if (playerFromIndex.first != -1 && playerFromIndex.second != -1) {
//             return {playerFromIndex, playerToIndex};­
//         } else {
//             printText("Invalid camera data, try again...");
//         }
//     }
// }

pair<MatrixIndex, MatrixIndex> getCameraData(ChessVision &chessVision) {
    // Capture the initial reference board state.
    printText("Getting initial reference board state...");
    ChessboardMatrix refState = chessVision.getRefVisionBoard();

    while (true) {
        printText("Make your move and press ENTER...");
        cin.get();
        ChessboardMatrix newState1 = chessVision.processCurrentFrame();
        
        if (newState1.empty()) continue; // Handle empty vision output
        
        // Determine the player's move comparing the reference to the new state.
        auto [moveFrom, moveTo] = determinePlayerMove(refState, newState1);
        
        // If a valid move is detected...
        if (moveFrom.first != -1 && moveFrom.second != -1) {
            printText("Change detected. Verifying redundancy...");

            // Redundancy check 1:
            printText("Press ENTER for redundancy check 1...");
            cin.get();
            ChessboardMatrix newState2 = chessVision.processCurrentFrame();

            // Redundancy check 2:
            printText("Press ENTER for redundancy check 2...");
            cin.get();
            ChessboardMatrix newState3 = chessVision.processCurrentFrame();

            // Check if all three moves are identical.
            if (newState1 == newState2 && newState2 == newState3) {
                chessVision.setRefVisionBoard(newState1);
                return {moveFrom, moveTo};
            }
            else {
                printText("Redundancy check failed. Restarting detection...");
            }
        }
        else {
            printText("No valid move detected. Waiting for change...");
        }
    }
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
    
    //   ==========   INITIALIZE COMPUTER VISION   ==========   //
    ChessVision chessVision(1);

    // Start kamera-feed i separat tråd
    std::thread cameraThread([&]() {
        chessVision.showLiveFeed();
    });

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

    //   ==========   BEGIN CHESS GAME   ==========   //
    printText("Press ENTER to start chess game...");
    cin.get();
    Stockfish engine("/home/ubuntu/Stockfish/src/stockfish");  
    printText("\n----- CHESS GAME STARTED -----");
    
    while (true) {
        auto [playerFromIndex, playerToIndex] = getCameraData(chessVision);

        // Convert into chess notation e.g "e2e4"
        //string playerMove = inputPlayerMove(); // Manually input playermove in chess notation e.g.: "e2e4"
        string playerMove = board.getChessNotation(playerFromIndex, playerToIndex);
        cout << "Player move: " << playerMove << endl;
        auto [playerFromIdx, playerToIdx] = board.getMatrixIndex(playerMove);

        // Update internal chessboard with the player's move.
        board.updateChessboard(playerFromIdx, playerToIdx);
        board.printBoard();
        
        // Get the move from Stockfish (in chess notation, e.g., "a2a4")
	    string robotMove = stockfishMove(engine, playerMove);
        
        // Move the chess piece using the robot.
        moveChessPiece(robotMove, chessboardOrigin, RotationChess);
        
        // After robot move, update the board accordingly.
        auto [robotFromIdx, robotToIdx] = board.getMatrixIndex(playerMove);
	    board.updateChessboard(robotFromIdx, robotToIdx);
        board.printBoard();
        
        moveToAwaitPosition();

        if (engine.isCheckmate()) {
            printText("Game has ended");
            return 0;
        }
    }
    return 0;
}
