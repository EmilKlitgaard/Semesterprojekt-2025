#include "Chessboard.h"
#include "Stockfish.h"
#include "Vision.h"
#include "Gripper.h"
#include "GUI.h"
#include "GUIWindow.h"
#include "Game.h"

Game game;

using namespace ur_rtde;
using namespace std;
using namespace Eigen;

using ChessboardPieces = vector<vector<string>>;
using ChessboardMatrix = vector<vector<char>>;
using MatrixIndex = pair<int, int>;

string robotIp = "192.168.1.54";
int robotPort = 50002;

unique_ptr<RTDEControlInterface> rtde_control;
unique_ptr<RTDEReceiveInterface> rtde_receive;
unique_ptr<Stockfish> engine;
unique_ptr<Gripper> gripper;
unique_ptr<ChessVision> camera;

Chessboard board;

Vector3d chessboardOrigin;
Matrix3d RotationMatrix;

Vector3d liftTransformVector = {0.0, 0.0, 0.1};
Vector3d calibrationTransformVector = {0.025, 0.025, 0.0};

MatrixIndex pieceIdx;
string deadPieceName;
bool deadPieceFound = false;

/*============================================================
            		   FUNCTIONS
============================================================*/
// Function to create a rotation matrix in degrees around Z-axis
void printText(string text) {
    cout << text << endl;
}

double degToRad(double angle) {
    return angle * M_PI / 180.0;
}

Matrix3d getRotationMatrixZ(double angleDeg) {
    double angleRad = degToRad(angleDeg);
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
    rtde_control->teachMode(); // Enable freemove mode
    printText("Freemove mode enabled. Move the robot pointer to the chessboard A1 corner and press ENTER.");
    cin.get(); // Wait for user input
    rtde_control->endTeachMode(); // Disable freemove mode
    printText("Freemove mode disabled. Saving chessboard frame origin.");

    vector<double> tcpPose = rtde_receive->getActualTCPPose();
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
    rtde_control->moveJ(awaitPosition, 2, 2);
}

// Move TCP to a specific chessboard coordinate
void moveToChessboardPoint(const Vector3d &chessboardTarget, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix, double speed = 3, double acceleration = 3) {
    // Convert chessboard target to base frame
    Vector3d baseTarget = chessboardToBase(chessboardTarget + calibrationTransformVector, chessboardOrigin, RotationMatrix);
    vector<double> tcpPose = {baseTarget[0], baseTarget[1], baseTarget[2], 0.0, M_PI, 0.0};
    // cout << "Moving TCP to Chessboard Point: [" << chessboardTarget.transpose() << "]" << endl;
    rtde_control->moveL(tcpPose, speed, acceleration);
}

void pickUpPiece(const Vector3d &position, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    moveToChessboardPoint(position + liftTransformVector, chessboardOrigin, RotationMatrix);
    moveToChessboardPoint(position, chessboardOrigin, RotationMatrix);
    gripper->closeGripper();
    gripper->stopGripper();
    moveToChessboardPoint(position + liftTransformVector, chessboardOrigin, RotationMatrix);
}

void placePiece(const Vector3d &position, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    moveToChessboardPoint(position + liftTransformVector, chessboardOrigin, RotationMatrix);
    moveToChessboardPoint(position, chessboardOrigin, RotationMatrix);
    gripper->openGripper();
    moveToChessboardPoint(position + liftTransformVector, chessboardOrigin, RotationMatrix);
}

void Game::calibrateGripper() {
    Vector3d calibrationTransformVector = {-0.025, -0.025, -0.005};
    moveToChessboardPoint(calibrationTransformVector + liftTransformVector, chessboardOrigin, RotationMatrix);
    gripper->openGripper();
    moveToChessboardPoint(calibrationTransformVector, chessboardOrigin, RotationMatrix);
    gripper->closeGripper();
    gripper->openGripper();
    moveToChessboardPoint(calibrationTransformVector + liftTransformVector, chessboardOrigin, RotationMatrix);
    moveToAwaitPosition();
    printText("Gripper calibrated.");
}

// Function to move the robot to the corners of the chessboard
void moveToBoardCorners(const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    moveToAwaitPosition();
    Vector3d chessboardTarget1(0.0, 0.0, 0.0);
    moveToChessboardPoint(chessboardTarget1 + liftTransformVector, chessboardOrigin, RotationMatrix);
    Vector3d chessboardTarget2(0.4, 0.0, 0.0);
    moveToChessboardPoint(chessboardTarget2 + liftTransformVector, chessboardOrigin, RotationMatrix);
    Vector3d chessboardTarget3(0.4, 0.4, 0.0);
    moveToChessboardPoint(chessboardTarget3 + liftTransformVector, chessboardOrigin, RotationMatrix);
    Vector3d chessboardTarget4(0.0, 0.4, 0.0);
    moveToChessboardPoint(chessboardTarget4 + liftTransformVector, chessboardOrigin, RotationMatrix);
    moveToAwaitPosition();
}

// Check if all positions within the calibrated chessboard are reachable
bool AllPositionsReachable(const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    printText("Checking for reachability...");
    vector<Vector3d> allPositions = board.getAllPhysicalCoordinates();
    for (const auto& position : allPositions) {
        Vector3d baseTarget = chessboardToBase(position, chessboardOrigin, RotationMatrix);
        vector<double> tcpPose = {baseTarget[0], baseTarget[1], baseTarget[2]};
        // cout << "Checking reachability for coordinate: (" << tcpPose[0] << ", " << tcpPose[1] << ", " << tcpPose[2] << ")" << endl;
        vector<double> JntPos = {1,1,1}; // rtde_control->getInverseKinematics(tcpPose);
        if (JntPos.empty()) {
            cout << "Target pose" << baseTarget.transpose() << " is not reachable by the robot" << endl;
            return false;
        }
    }
    cout << "All target poses are reachable" << endl;
    return true;
}

// Compare camera data to determine which cell changed. Returns a pair: {from (row, col), to (row, col)}.
pair<MatrixIndex, MatrixIndex> determinePlayerMove(const ChessboardMatrix lastPositions, const ChessboardMatrix newPositions) {
    MatrixIndex from = {-1, -1};
    MatrixIndex to = {-1, -1};

    // Check for castling
    if (lastPositions[0][0] == 'W' && newPositions[0][0] == 'e' && lastPositions[0][1] == 'e' && newPositions[0][1] == 'W' && lastPositions[0][2] == 'e' && newPositions[0][2] == 'W' && lastPositions[0][3] == 'W' && newPositions[0][3] == 'e') {
        return {{0, 3}, {0, 1}};
    } else if (lastPositions[0][3] == 'W' && newPositions[0][3] == 'e' && lastPositions[0][4] == 'e' && newPositions[0][4] == 'W' && lastPositions[0][5] == 'e' && newPositions[0][5] == 'W' && lastPositions[0][6] == 'e' && newPositions[0][6] == 'e' && lastPositions[0][7] == 'W' && newPositions[0][7] == 'e') {
        return {{0, 3}, {0, 5}};
    }

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (lastPositions[i][j] == 'W' && newPositions[i][j] == 'e') {
                if (from.first != -1 || from.second != -1) {
                    return {{-1, -1}, {-1, -1}}; // Error: Multiple moves detected
                }
                from = {i, j}; // Piece moved away from here.
            } else if (lastPositions[i][j] == 'B' && newPositions[i][j] == 'W') {
                if (to.first != -1 || to.second != -1) {
                    return {{-1, -1}, {-1, -1}}; // Error: Multiple moves detected
                }
                to = {i, j}; // Piece moved into here.
                pieceIdx = to;
                deadPieceName = board.getPieceName(to);
                deadPieceFound = true;
            } else if (lastPositions[i][j] == 'e' && newPositions[i][j] == 'W') {
                if (to.first != -1 || to.second != -1) {
                    return {{-1, -1}, {-1, -1}}; // Error: Multiple moves detected
                }
                to = {i, j}; // Piece moved into here.
                pieceIdx = to;
            }
        }
    }
    // cout << "Player moved from: (" << from.first << "," << from.second << "), to: (" << to.first << "," << to.second << ")." << endl;
    return {from, to};
}

// Returns a move string in chess notation, e.g., "e2e4".
string stockfishMove() {
    cout << "Stockfish is calculating the best move..." << endl;
    string bestMove = engine->getBestMove();
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

void moveChessPiece(string &robotMove, const Vector3d &chessboardOrigin, const Matrix3d &RotationMatrix) {
    // Get the physical coordinates for the cells.
    auto [fromCoordinate, toCoordinate] = board.getPhysicalCoordinates(robotMove);

    // Print the robot moves
    cout << "Robot move piece from Coordinate: (" << fromCoordinate.transpose() << "), to Coordinate: (" << toCoordinate.transpose() << ")." << endl;
    
    // Get the piece names for later use
    auto [fromPieceName, toPieceName] = getPieceName(robotMove);
    fromPieceName = board.getPieceName(fromPieceName);

    // Check if the destination cell is already occupied.
    string toNotation = robotMove.substr(2, 2);
    if (isOccupied(toNotation)) {
        printText("Moving occupying piece to dead piece location...");
        
        toPieceName = board.getPieceName(toPieceName);
        Vector3d deadPieceLocation = board.getDeadPieceLocation(toPieceName, "Player");

        pickUpPiece(toCoordinate, chessboardOrigin, RotationMatrix);
        placePiece(deadPieceLocation, chessboardOrigin, RotationMatrix);
    }

    // Check for castling
    if ((robotMove == "dec8" || robotMove == "e8g8") && fromPieceName == "King") {
        printText("Castling detected. Moving rook...");
        Vector3d rookFromCoordinate = (robotMove == "dec8" ? board.getPhysicalCoordinate({0, 7}) : board.getPhysicalCoordinate({0, 0}));
        Vector3d rookToCoordinate = (robotMove == "dec8" ? board.getPhysicalCoordinate({0, 5}) : board.getPhysicalCoordinate({0, 3}));
        pickUpPiece(rookFromCoordinate, chessboardOrigin, RotationMatrix);
        placePiece(rookToCoordinate, chessboardOrigin, RotationMatrix);
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
            promotionPieceName = "Queen";
        } else if (promotionType == "r") { 
            promotionPieceName = "Rook";
        } else if (promotionType == "b") { 
            promotionPieceName = "Bishop"; 
        } else if (promotionType == "n") { 
            promotionPieceName = "Knight";
        } else {
            throw invalid_argument("Stockfish pawnPromotion output error. Stockfish output: " + robotMove);
        }
        Vector3d promotionPieceLocation = board.searchDeadPieceLocation(promotionPieceName, "Robot");

        pickUpPiece(promotionPieceLocation, chessboardOrigin, RotationMatrix);
        placePiece(toCoordinate, chessboardOrigin, RotationMatrix);
    }
}

pair<MatrixIndex, MatrixIndex> getCameraData(ChessVision &camera) {
    // Capture the initial reference board state.
    printText("Getting initial reference board state...");
    ChessboardMatrix refState = board.getRefVisionBoard();

    while (true) {
        if (!gui.getGameActive()) return {{-1, -1}, {-1, -1}}; // Exit if game is not running

        ChessboardMatrix newState1 = camera.processCurrentFrame();
        
        if (newState1.empty()) continue; // Handle empty vision output
        
        // Determine the player's move comparing the reference to the new state.
        auto [moveFrom, moveTo] = determinePlayerMove(refState, newState1);
        
        // If a valid move is detected...
        if (moveFrom.first != -1 && moveFrom.second != -1 && moveTo.first != -1 && moveTo.second != -1) {
            printText("Change detected. Verifying redundancy...");

            // Redundancy check 1:
            //printText("Redundancy check 1...");
            ChessboardMatrix newState2 = camera.processCurrentFrame();
            // Redundancy check 2:
            //printText("Redundancy check 2...");
            ChessboardMatrix newState3 = camera.processCurrentFrame();
            // Redundancy check 3:
            //printText("Redundancy check 3...");
            ChessboardMatrix newState4 = camera.processCurrentFrame();
            // Redundancy check 4:
            //printText("Redundancy check 4...");
            ChessboardMatrix newState5 = camera.processCurrentFrame();
            // Redundancy check 5:
            //printText("Redundancy check 5...");
            ChessboardMatrix newState6 = camera.processCurrentFrame();

            // Check if all three moves are identical.
            if (newState1 == newState2 && newState2 == newState3 && newState3 == newState4 && newState4 == newState5 && newState5 == newState6) {
                if (deadPieceFound) {
                    board.getDeadPieceLocation(deadPieceName, "Robot");
                }
                cout << "Move detected: " << board.getChessNotation(moveFrom, moveTo) << endl;
                return {moveFrom, moveTo};
            }
            else {
                printText("Redundancy check failed. Restarting detection...");
            }
        }
        else {
            //printText("No valid move detected. Waiting for change...");
        }
        deadPieceFound = false;
    }
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

string checkPawnPromotion(string &playerMove) {
    auto [fromPieceIdx, toPieceIdx] = board.getMatrixIndex(playerMove);
    if (fromPieceIdx.first == 0 && board.getPieceName(fromPieceIdx) == "Pawn") {
        cout << "Pawn promotion has been detected. Enter promotion piece name: [Queen, Knight, Rook, Bishop] " << endl;
        string promotionType;
        while (true) {
            string promotionType = window->selectPawnPromotion();
            //cin >> promotionType;
            if (promotionType == "Queen") {
                promotionType = "q";
                board.updateChessboard("5W", pieceIdx);
                break;
            } else if (promotionType == "Knight") {
                promotionType = "n";
                board.updateChessboard("3W", pieceIdx);
                break;
            } else if (promotionType == "Rook") {
                promotionType = "r";
                board.updateChessboard("2W", pieceIdx);
                break;
            } else if (promotionType == "Bishop") {
                promotionType = "b";
                board.updateChessboard("4W", pieceIdx);
                break;
            }
            cout << "Invalid input. Please enter a valid piece name: " << endl;
        }
        playerMove += promotionType;
        cout << "Updated player move: " << playerMove << endl;
    }
    return playerMove;
}

// Get a valid player move from the camera
string getValidPlayerMove(ChessVision &camera) {
    string playerMove;
    bool validMove = false;
    do {
        auto [playerFromIndex, playerToIndex] = getCameraData(camera);
        if (playerFromIndex.first == -1 || playerToIndex.first == -1) return "Cancelled";

        playerMove = board.getChessNotation(playerFromIndex, playerToIndex);

        validMove = engine->sendValidMove(playerMove);

        if (!validMove) {
            printText("Invalid move. Make a valid move.");
            sleep(1);
        }
    } while (!validMove);

    playerMove = checkPawnPromotion(playerMove);
    cout << "Player move: " << playerMove << endl;
    return playerMove;
}

void pauseGame() {
    gui.setGamePaused(true);
    while (gui.getCalibrating()) this_thread::sleep_for(chrono::milliseconds(100));
    gui.setGamePaused(false);
}

void Game::resetChessboard() {
    thread ([] {
        while (gui.getGameActive()) this_thread::sleep_for(chrono::milliseconds(100));

        cout << "CHESSBOARD RESET STARTED!" << endl;
        board.printBoard();

        int allMoves = 0;

        ChessboardPieces currentBoard = board.getBoardState();
        ChessboardPieces desiredBoard = board.getStartBoard();

        while (currentBoard != desiredBoard) {
            while (true) {
                bool moveFound = false;
                for (int fromRow = 0; fromRow < 8; ++fromRow) {
                    for (int fromCol = 0; fromCol < 8; ++fromCol) {
                        const string &currentPiece = currentBoard[fromRow][fromCol];
                        if (currentPiece == "0" || currentPiece == desiredBoard[fromRow][fromCol]) continue; // Skip empty or already correct pieces
                        
                        // Find the destination for the current piece
                        for (int toRow = 0; toRow < 8; ++toRow) {
                            for (int toCol = 0; toCol < 8; ++toCol) {
                                if (currentPiece == desiredBoard[toRow][toCol] && currentBoard[toRow][toCol] == "0") {
                                    // Move the piece to the destination
                                    MatrixIndex from = {fromRow, fromCol};
                                    MatrixIndex to = {toRow, toCol};
                                    board.updateChessboard(from, to);
                                    Vector3d fromCoordinate = board.getPhysicalCoordinate(from);
                                    Vector3d toCoordinate = board.getPhysicalCoordinate(to);
                                    pickUpPiece(fromCoordinate, chessboardOrigin, RotationMatrix);
                                    placePiece(toCoordinate, chessboardOrigin, RotationMatrix);
                                    cout << "Moved " << currentPiece << " from " << board.getChessNotation({fromRow, fromCol}) << " to " << board.getChessNotation({toRow, toCol}) << endl;
                                    board.printBoard();
                                    currentBoard = board.getBoardState();
                                    moveFound = true;
                                    toRow = toCol = 8; // Break loops
                                    allMoves++;
                                }
                            }
                        }
                    }
                }
                if (!moveFound) break;
            }

            bool moveFound = false;
            // Look for misplaced pieces and move one to dead piece location
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    const string &currentPiece = currentBoard[row][col];
                    if (currentPiece != desiredBoard[row][col] && currentPiece != "0") {
                        const string &currentPieceName = board.getPieceName(currentPiece);
                        string origin = currentPiece.substr(1,1) == "B" ? "Robot" : "Player";
                        Vector3d deadPieceLocation = board.getDeadPieceLocation(currentPieceName, origin);
                        MatrixIndex from = {row, col};
                        board.updateChessboard("0", from);
                        cout << "Moved " << currentPiece << " from " << board.getChessNotation({row, col}) << " to dead piece location" << endl;
                        board.printBoard();
                        currentBoard = board.getBoardState();
                        allMoves++;
                        row = col = 8; // Break loops
                        moveFound = true;
                    }
                }
            }
            // Recover from dead piece location
            if (!moveFound) {
                for (int row = 0; row < 8; ++row) {
                    for (int col = 0; col < 8; ++col) {
                        const string &currentPiece = currentBoard[row][col];
                        if (currentPiece == "0" && desiredBoard[row][col] != "0") {
                            const string &desiredPiece = desiredBoard[row][col];
                            const string &desiredPieceName = board.getPieceName(desiredPiece);
                            string origin = currentPiece.substr(1,1) == "B" ? "Robot" : "Player";
                            MatrixIndex to = {row, col};
                            Vector3d fromCoordinate = board.searchDeadPieceLocation(desiredPieceName, origin);
                            Vector3d toCoordinate = board.getPhysicalCoordinate(to);
                            pickUpPiece(fromCoordinate, chessboardOrigin, RotationMatrix);
                            placePiece(toCoordinate, chessboardOrigin, RotationMatrix);
                            board.updateChessboard(desiredPiece, to);
                            cout << "Retrived " << desiredPiece << " from dead piece location, and moved to " << board.getChessNotation({row, col}) << endl;
                            board.printBoard();
                            currentBoard = board.getBoardState();
                            allMoves++;
                            row = col = 8; // Break loops
                        }
                    }
                }
            }
        }
        moveToAwaitPosition();
        engine->resetMoveHistory();
        gui.setGameResetting(false);
        cout << "Reset complete in " << allMoves << " moves" << endl;
    }).detach();
}

void Game::initializeGame() {
    printText("--- INITIALIZING GAME ---");

    //   ==========   INITIALIZE STOCKFISH ENGINE   ==========   //
    do {
        try {
            engine = make_unique<Stockfish>("/usr/local/bin/stockfish");
        } catch (const exception& e) {
            cerr << "Failed to initialize Stockfish. Error: " << e.what() << endl;
        }
    } while (!engine);

    //   ==========   INITIALIZE UR5 CONNECTION   ==========   //
    do {
        try {
            rtde_control = make_unique<RTDEControlInterface>(robotIp, robotPort);
            rtde_receive = make_unique<RTDEReceiveInterface>(robotIp, robotPort);
            gui.setConnection(true);
            cout << "Successfully connected to the robot." << endl;
        } catch (const exception& e) {
            cerr << "Failed to connect to the robot at ip: " << robotIp << ", port: " << robotPort << ". Error: " << e.what() << endl;
            gui.setConnection(false);
            sleep(1);
        }
    } while (!rtde_control || !rtde_receive);

    //   ==========   INITIALIZE GRIPPER COMMUNICATION   ==========   //
    do {
        try {
            gripper = make_unique<Gripper>("/dev/ttyACM0");
            cout << "Successfully connected to the gripper." << endl;
        } catch (const exception& e) {
            cerr << "Failed to connect to the gripper. Error: " << e.what() << endl;
            sleep(1);
        }
    } while (!gripper);

    //   ==========   INITIALIZE COMPUTER VISION   ==========   //
    do {     
        try {
            camera = make_unique<ChessVision>(1); // Initialize ChessVision with camera index 1
            cout << "Successfully connected to the camera." << endl;
        } catch (const exception& e) {
            cerr << "Failed to connect to the camera. Error: " << e.what() << endl;
            sleep(1); // Retry after 1 second
        }
    } while (!camera);

    // Start kamera-feed i separat tråd
    thread cameraThread([&]() {
        try {
            camera->showLiveFeed();
        } catch (const exception& e) {
            cerr << "Error in camera feed thread: " << e.what() << endl;
        }
    });
    cameraThread.detach();
    sleep(2); // Give the camera some time to initialize
    
    //   ==========   SET CALIBRATION TOOL TCP OFFSET   ==========   //
    vector<double> tcpCalibrationOffset = {0.0, 0.0, 0.1, 0.0, 0.0, 0.0};
    rtde_control->setTcp(tcpCalibrationOffset);

    //   ==========   SET CHESSBOARD ORIGIN   ==========   //
    chessboardOrigin = setChessboardOrigin(false);
    
    // Define rotation matrix for chessboard frame (22.5° base offset and -90° alignment)
    RotationMatrix = getRotationMatrixZ(22.5 - 90);
    //cout << "Chessboard Frame Origin (Base Frame): [" << chessboardOrigin.transpose() << "]" << endl;

    // Check if all positions within the calibrated chessboard are reachable
    while (!AllPositionsReachable(chessboardOrigin, RotationMatrix)) {
        chessboardOrigin = setChessboardOrigin(true);
    }

    //   ==========   UPDATE TCP OFFSET   ==========   //
    const vector<double> tcpOffset = {0.0, 0.0, 0.224, 0.0, 0.0, 0.0}; // Should be 0.224
    rtde_control->setTcp(tcpOffset);
    
    //   ==========   BEGIN PRE-GAME MOVEMENTS   ==========   //
    moveToAwaitPosition();
    // moveToBoardCorners(chessboardOrigin, RotationMatrix);
    gui.setGameInitialized(true);
    startGame();
}

void Game::calibrate() {
    gui.setCalibrating(true);
    while (!gui.getGamePaused()) this_thread::sleep_for(chrono::milliseconds(100));
    calibrateGripper();
    gui.setCalibrating(false);
}

//   ==========   RUN MAIN CODE IN SEPERATE THREAD   ==========   //
void Game::startGame() {
    printText("--- STARTING GAME ---");
    
    gameThread = thread ([this]() {
        gui.setGameRunning(true);

        while (gui.getGameActive()) {
            // Set player turn on GUI
            gui.setTurn("Player");

            // Get the player's move from the camera.
            string playerMove = getValidPlayerMove(*camera);
            if (playerMove == "Cancelled") break;
            
            //string playerMove = inputPlayerMove(); // Manually input playermove in chess notation e.g.: "e2e4"
            
            // Get the index of the piece that was moved. Update internal chessboard with the player's move.
            auto [playerFromIdx, playerToIdx] = board.getMatrixIndex(playerMove);
            board.updateChessboard(playerFromIdx, playerToIdx);
            board.printBoard();
            
            // Set robot turn on GUI
            gui.setTurn("Robot");

            // Get the move from Stockfish (in chess notation, e.g., "a2a4")
            string robotMove = stockfishMove();
            
            // Move the chess piece using the robot.
            moveChessPiece(robotMove, chessboardOrigin, RotationMatrix);
            
            // After robot move, update the board accordingly.
            auto [robotFromIdx, robotToIdx] = board.getMatrixIndex(robotMove);
            board.updateChessboard(robotFromIdx, robotToIdx);
            board.printBoard();
            
            moveToAwaitPosition();
        
            if (engine->isCheckmate()) {
                printText("Game has ended");
                return 0;
            }

            if (gui.getCalibrating()) pauseGame();
        }
        gui.setGameRunning(false);
        printText("--- GAME STOPPED ---");
        return 0;
    });
}

void Game::stopGame() {
    if (gameThread.joinable()) {
        gameThread.join();
    }
}