#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include "PhoneticInput.h"
#include "Chessboard.h"
#include "Stockfish.h"
#include "Vision.h"
#include "ChessGui.h"
#include <QtWidgets/QApplication>

using namespace ur_rtde;
using namespace std;
using namespace Eigen;

using ChessboardMatrix = vector<vector<char>>;
using MatrixIndex = pair<int, int>;

#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0)

string robotIp = "192.168.1.54";
int robotPort = 50002;
RTDEControlInterface rtde_control(robotIp, robotPort);
RTDEReceiveInterface rtde_receive(robotIp, robotPort);

Chessboard board;

/*============================================================
            		   FUNCTIONS
============================================================*/
// Function to create a rotation matrix in degrees around Z-axis
void printText(string text) {
    //cout << text << endl;
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

void showBoardState(const ChessboardMatrix& board) {
    Mat display(400, 400, CV_8UC3, Scalar(255, 255, 255));
    float squareSize = 400.0f / 8;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Point2f topLeft(j*squareSize, i*squareSize);
            Point2f bottomRight((j+1)*squareSize, (i+1)*squareSize);
            
            // Draw square
            rectangle(display, topLeft, bottomRight, Scalar(0, 0, 0), 1);
            
            // Draw piece
            if (board[i][j] == 'W') {
                circle(display, Point(topLeft.x + squareSize/2, topLeft.y + squareSize/2), 
                       15, Scalar(255, 0, 0), FILLED); // Blue for white
            }
            else if (board[i][j] == 'B') {
                circle(display, Point(topLeft.x + squareSize/2, topLeft.y + squareSize/2), 
                       15, Scalar(0, 0, 255), FILLED); // Red for black
            }
        }
    }
    
    imshow("Detected Board State", display);
    waitKey(1);
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

string inputPlayerMove() {
    string playerMove;
    cout << "Input your move e.g.: 'e2e4': " << endl;
    while (true) {
        cin >> playerMove;
        if (isValidMoveFormat(playerMove)) {
            return playerMove;
        } else {
            cout << "Invalid move format. Please enter a move in the format 'e2e4': " << endl;
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

// Placeholder: Simulate camera data retrieval.
ChessboardMatrix getCameraData(int i) {
    ChessboardMatrix cameraBoard(8, vector<char>(8, 'e'));
    // Example simulation:
    if (i == 1) {
        cameraBoard[6][2] = 'W';
    } else {
        cameraBoard[4][2] = 'W';
    }
    return cameraBoard;
}
ChessboardMatrix getCameraData() {
    static ChessVision vision(0);
    auto detectedBoard = vision.detectBoardState();
    showBoardState(detectedBoard);
    return detectedBoard;
}

// Compare camera data to determine which cell changed. Returns a pair: {from (row, col), to (row, col)}.
pair<MatrixIndex, MatrixIndex> determinePlayerMove(const ChessboardMatrix &lastPositions, const ChessboardMatrix &newPositions) {
    MatrixIndex from = {-1, -1};
    MatrixIndex to = {-1, -1};
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (lastPositions[i][j] != 'e' && newPositions[i][j] == 'e') {
                from = {i, j}; // Piece moved away from here.
            }
            if (lastPositions[i][j] == 'e' && newPositions[i][j] != 'e' || lastPositions[i][j] == 'W' && newPositions[i][j] == 'B' || lastPositions[i][j] == 'B' && newPositions[i][j] == 'W') {
                to = {i, j}; // Piece moved into here.
            }
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
    auto boardState = board.getBoardState();
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

void moveChessPiece(string &fromNotation, string &toNotation, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    // Get the physical coordinates for these cells.
    Vector3d fromCoordinate = board.getPhysicalCoordinates(fromNotation);
    Vector3d toCoordinate = board.getPhysicalCoordinates(toNotation);

    Vector3d transformVector = {0.0, 0.0, 0.1};

    // Print the robot moves
    cout << "Robot move piece from Coordinate: (" << fromCoordinate.transpose() << "), to Coordinate: (" << toCoordinate.transpose() << ")." << endl;
    
    // Check if the destination cell is already occupied.
    bool toOccupied = isOccupied(toNotation);

    // If the destination cell is occupied, move the piece there to a dead piece location.
    if (toOccupied) {
        printText("Moving occupying piece to dead piece location...");
        Vector3d deadPieceLocation = board.getDeadPieceLocation();
    
        printText("Moving above toCoordinate...");
        moveToChessboardPoint(toCoordinate + transformVector, chessboardOrigin, RotationMatrix);
        printText("Moving down...");
        moveToChessboardPoint(toCoordinate, chessboardOrigin, RotationMatrix);
        printText("Closing gripper to pick up piece...");
        // closeGripper();
        printText("Moving up...");
        moveToChessboardPoint(toCoordinate + transformVector, chessboardOrigin, RotationMatrix);
        printText("Moving above deadPieceLocation...");
        moveToChessboardPoint(deadPieceLocation + transformVector, chessboardOrigin, RotationMatrix);
        printText("Moving down to place dead piece...");
        moveToChessboardPoint(deadPieceLocation, chessboardOrigin, RotationMatrix);
        printText("Opening gripper to let go of piece...");
        // openGripper();
        printText("Moving up...");
        moveToChessboardPoint(deadPieceLocation + transformVector, chessboardOrigin, RotationMatrix);
    }
    
    // Sequence of movements for picking up the piece:
    printText("Moving above fromCoordinate...");
    moveToChessboardPoint(fromCoordinate + transformVector, chessboardOrigin, RotationMatrix);
    printText("Moving down...");
    moveToChessboardPoint(fromCoordinate, chessboardOrigin, RotationMatrix);
    printText("Closing gripper to pick up piece...");
    // closeGripper();
    printText("Moving up...");
    moveToChessboardPoint(fromCoordinate + transformVector, chessboardOrigin, RotationMatrix);
    
    // Sequence of movements for moving and placing the piece:
    printText("Moving above toCoordinate...");
    moveToChessboardPoint(toCoordinate + transformVector, chessboardOrigin, RotationMatrix);
    printText("Moving down...");
    moveToChessboardPoint(toCoordinate, chessboardOrigin, RotationMatrix);
    printText("Opening gripper to release piece...");
    // openGripper();
    printText("Moving up...");
    moveToChessboardPoint(toCoordinate + transformVector, chessboardOrigin, RotationMatrix);
}

/*============================================================
            		    MAIN START
============================================================*/
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    ChessGui gui;
    gui.show();

    //   ==========   VALIDATE UR5 CONNECTION   ==========   //
    if (!rtde_control.isConnected() || !rtde_receive.isConnected()) {
        cerr << "Failed to connect to the robot at " << robotIp << ":" << robotPort << endl;
        return -1;
    }
    
    //   ==========   SET TCP OFFSET   ==========   //
    vector<double> tcpOffset = {0.0, 0.0, 0.1, 0.0, 0.0, 0.0};
    rtde_control.setTcp(tcpOffset);
    
    //   ==========   SET CHESSBOARD ORIGIN   ==========   //
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

    //   ==========   INITIALIZE COMPUTER VISION   ==========   //
    /*ChessVision vision(0);
    cout << "Calibrating camera... Point camera at chessboard and press any key" << endl;
    Mat frame;
    while (true) {
        frame = vision.captureFrame();
        if (frame.empty()) {
            cerr << "Failed to capture frame!" << endl;
            return -1;
        }
        
        try {
            vision.calibratePerspective(frame);
            break;
        } catch (const exception& e) {
            putText(frame, "Adjust camera to see full chessboard", Point(10, 30),
                    FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
            imshow("Calibration", frame);
            if (waitKey(100) == 27) break; // ESC to exit
        }
    }
    destroyAllWindows();*/
    
    //   ==========   BEGIN PRE-GAME MOVEMENTS   ==========   //
    moveToAwaitPosition();

    Vector3d chessboardTarget1(0.0, 0.0, 0.0);
    moveToChessboardPoint(chessboardTarget1, chessboardOrigin, RotationChess);
    
    Vector3d chessboardTarget2(0.4, 0.4, 0.0);
    moveToChessboardPoint(chessboardTarget2, chessboardOrigin, RotationChess);

    moveToAwaitPosition();

    //   ==========   BEGIN CHESS GAME   ==========   //
    cout << "Press ENTER to start chess game..." << endl;
    cin.get();
    Stockfish engine("/home/ubuntu/Stockfish/src/stockfish");  
    cout << "\n" << "----- CHESS GAME STARTED -----" << endl;
    
    while (true) {
        // Retrieve camera data before and after player's move.
        /*ChessboardMatrix lastPositions = getCameraData(1);
        cout << "Make your move and press ENTER..." << endl;
        cin.get();
        ChessboardMatrix newPositions = getCameraData(2);
        
        // Determine the player's move.
        auto moveIndices = determinePlayerMove(lastPositions, newPositions);
        MatrixIndex playerFromIndex = moveIndices.first;
        MatrixIndex playerToIndex = moveIndices.second;

        // Convert into chess notation e.g "e2e4"
        string playerMove = board.getChessNotation(playerFromIndex, playerToIndex);*/
        string playerMove = inputPlayerMove(); // Manually input playermove in chess notation e.g.: "e2e4"
        cout << "Player move: " << playerMove << endl;
        string playerFromNotation = playerMove.substr(0, 2);
        string playerToNotation   = playerMove.substr(2, 2);
        MatrixIndex playerFromIdx = board.getMatrixIndex(playerFromNotation);
	    MatrixIndex playerToIdx   = board.getMatrixIndex(playerToNotation);

        // Update our internal chessboard with the player's move.
        board.updateChessboard(playerFromIdx, playerToIdx);
        board.printBoard();
        
        // Get the move from Stockfish (in chess notation, e.g., "A2A4")
	    string stockfisBesthMove = stockfishMove(engine, playerMove);

        // Convert Stockfish move to from- and to- notations.
        string fromNotation = stockfisBesthMove.substr(0, 2);
        string toNotation   = stockfisBesthMove.substr(2, 2);
        
        // Move the chess piece using the robot.
        moveChessPiece(fromNotation, toNotation, chessboardOrigin, RotationChess);
        
        // After robot move, update the board accordingly.
        MatrixIndex fromIdx = board.getMatrixIndex(fromNotation);
	    MatrixIndex toIdx   = board.getMatrixIndex(toNotation);
	    board.updateChessboard(fromIdx, toIdx);
        board.printBoard();
        
        moveToAwaitPosition();

        if (engine.isCheckmate()) {
            //moveToAwaitPosition(rtde_control);
            cout << "Game has ended" << endl;
            return 0;
        }
    }

    QObject::connect(&gui, &ChessGui::gameStarted, [&]() {
        // Start game logic
    });

    QObject::connect(&gui, &ChessGui::boardReset, [&]() {
        // Reset board logic
    });

    QObject::connect(&gui, &ChessGui::voiceCommandToggled, [&](bool enabled) {
        // Toggle voice commands
    });

    QObject::connect(&gui, &ChessGui::playerFirstToggled, [&](bool playerFirst) {
        // Set who goes first
    });

    QObject::connect(&gui, &ChessGui::speedChessToggled, [&](bool enabled) {
        // Enable/disable speed chess
    });

    QObject::connect(&gui, &ChessGui::speedChessTimeChanged, [&](int minutes) {
        // Update speed chess time
    });

    QObject::connect(&gui, &ChessGui::difficultyChanged, [&](int value) {
        // Update Stockfish difficulty
    });

    return app.exec();
}
