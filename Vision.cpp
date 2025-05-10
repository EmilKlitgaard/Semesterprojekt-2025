#include "Vision.h"

// Constructor: Opens the camera
ChessVision::ChessVision(int cameraIndex) {
    cap.open(cameraIndex);
    if (!cap.isOpened()) {
        cerr << "Error: Camera not opened!" << endl;
        exit(1);
    }
    initializeVisionBoard();
}

// Destructor: Releases the camera
ChessVision::~ChessVision() {
    cap.release();
}

void ChessVision::initializeVisionBoard() {
    refVisionBoard = ChessboardMatrix(8, vector<char>(8, 'e')); // 8x8 matrix filled with 'e'
    // Fill the first two rows with 'B' (Black pieces) and last two rows with 'W' (White pieces)
    for (int row = 0; row < 8; ++row) {
        if (row < 2) {
            fill(refVisionBoard[row].begin(), refVisionBoard[row].end(), 'B');
        } else if (row >= 6) {
            fill(refVisionBoard[row].begin(), refVisionBoard[row].end(), 'W');
        }
    }
}


ChessboardMatrix ChessVision::getRefVisionBoard() {
    return refVisionBoard;
}

void ChessVision::setRefVisionBoard(ChessboardMatrix &newRefVisionBoard) {
    refVisionBoard = newRefVisionBoard;
}

// Detects chessboard corners
bool ChessVision::detectChessboard(Mat& frame) {
    bool found = findChessboardCornersSB(frame, boardSize, corners);
    // if (found) drawChessboardCorners(frame, boardSize, corners, found);
    return found;
}

// Transforms the chessboard to a top-down view
bool ChessVision::transformChessboard(Mat& frame) {
    if (corners.size() != boardSize.area()) return false; // Checks if chessboard is detected.

    // Calculate the four outermost corners. Adding or subtracting the difference from the diagonal square.
    Point2f topLeft = corners[0] + (corners[0] - corners[boardSize.width + 1]);
    Point2f topRight = corners[boardSize.width - 1] + (corners[boardSize.width - 1] - corners[2 * boardSize.width - 2]);
    Point2f bottomLeft = corners[(boardSize.height - 1) * boardSize.width] + 
                         (corners[(boardSize.height - 1) * boardSize.width] - corners[(boardSize.height - 2) * boardSize.width + 1]);
    Point2f bottomRight = corners.back() + (corners.back() - corners[corners.size() - boardSize.width - 2]);

    // Define new source points using the corrected outer corners
    vector<Point2f> srcCorners = {topLeft, topRight, bottomRight, bottomLeft};

    // Define target points for the perspective transform (8x8 board)
    vector<Point2f> dstCorners = { 
        Point2f(0, 0), 
        Point2f(targetSize, 0), 
        Point2f(targetSize, targetSize), 
        Point2f(0, targetSize) 
    };

    // Compute the transformation matrix
    Mat warpMatrix = getPerspectiveTransform(srcCorners, dstCorners);

    // Apply the perspective warp
    warpPerspective(frame, frame, warpMatrix, Size(targetSize, targetSize));

    return true;
}


// Detects red and blue dots
/*
void ChessVision::detectDots(Mat& frame) {
    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV); // HSV (Hue-Saturation-Value) is optimal for isolating specific colors.

    // Define HSV ranges for red and blue. Two masks for red because of different shades that can be seen as red.
    Mat maskRed1, maskRed2, maskGreen;
    inRange(hsv, Scalar(0, 100, 100), Scalar(10, 255, 255), maskRed1); // Light red
    inRange(hsv, Scalar(170, 100, 100), Scalar(180, 255, 255), maskRed2); // Dark red
    inRange(hsv, Scalar(100, 150, 50), Scalar(140, 255, 255), maskGreen); // Blue

    Mat maskRed = maskRed1 | maskRed2;  // Combine red masks 

    // Finding contours/shapes
    vector<vector<Point>> contoursRed, contoursBlue;
    findContours(maskRed, contoursRed, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    findContours(maskGreen, contoursBlue, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Draw red dots
    for (const auto& contour : contoursRed) { // Looping through every shape found.
        double area = contourArea(contour);
        if (area > 500 && area < 5000) { // Accepter "realistisk størrelse"
            Rect boundingBox = boundingRect(contour); // Creates a rectangle around the shape.
            Point center = (boundingBox.tl() + boundingBox.br()) * 0.5; // Takes top left and bottom right and muliiplies by 0.5 to find the center.
            circle(frame, center, 5, Scalar(0, 0, 255), -1); // Draws the circle.
        }
    }

    // Draw blue dots
    for (const auto& contour : contoursBlue) { 
        double area = contourArea(contour);
        if (area > 500 && area < 5000) { 
            Rect boundingBox = boundingRect(contour); 
            Point center = (boundingBox.tl() + boundingBox.br()) * 0.5; 
            circle(frame, center, 5, Scalar(255, 0, 0), -1); 
        }
    }
}
*/

// Computes the centers of chessboard squares
vector<Point2f> ChessVision::getSquareCenters() const {
    vector<Point2f> centers;
    if (corners.size() != boardSize.area()) return centers;

    // Loop through the 8x8 grid
    for (int row = 0; row < boardSize.height; row++) {
        for (int col = 0; col < boardSize.width; col++) {
            if (row < boardSize.height - 1 && col < boardSize.width - 1) {
                // Get four corner points for each square
                Point2f tl = corners[row * boardSize.width + col];
                Point2f tr = corners[row * boardSize.width + (col + 1)];
                Point2f bl = corners[(row + 1) * boardSize.width + col];
                Point2f br = corners[(row + 1) * boardSize.width + (col + 1)];

                // Compute the center of the square
                centers.emplace_back((tl + tr + bl + br) * 0.25f);
            }
        }
    }
    return centers;
}

// Captures and processes video frames
/*void ChessVision::showFrame() {
    Mat frame;
    while (true) {
        cap >> frame;  // Capture frame
        if (frame.empty()) break;

        Mat original = frame.clone();
        bool found = detectChessboard(frame);

        if (found) {
            if (transformChessboard(frame)) {
                detectDots(frame);
            } else {
                frame = original.clone();
            }
        }

        imshow("ChessVision", frame);
        if (waitKey(30) == 27) break;  // Exit on 'Esc' key
    }
}
*/

Mat currentFrame;  // Tilføj som private variabel i header-filen

void ChessVision::showLiveFeed() {
    Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        currentFrame = frame.clone();  // Gem til senere brug

        imshow("Live Kamera", frame);
        int key = waitKey(30);
        if (key == 27) break;  // ESC for at afslutte
    }
}

void ChessVision::printBoard(Mat &frame, ChessboardMatrix &boardMatrix) {
    cout << "Skakbræt (W = Rød, B = Blå, e = Tom):" << endl;
    for (const auto& row : boardMatrix) {
        for (char c : row) {
            cout << c << ' ';
        }
        cout << endl;
    }

    // Tegn prikker visuelt på billedet
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (boardMatrix[row][col] == 'W') {
                Point center((col + 0.5) * (targetSize / 8.0), (row + 0.5) * (targetSize / 8.0));
                circle(frame, center, 12, Scalar(0, 0, 255), 2); // rød cirkel
            }
            else if (boardMatrix[row][col] == 'B') {
                Point center((col + 0.5) * (targetSize / 8.0), (row + 0.5) * (targetSize / 8.0));
                circle(frame, center, 12, Scalar(0, 255, 0), 2); // Grøn cirkel
            }
        }
    }
}

ChessboardMatrix ChessVision::processCurrentFrame() {
    if (currentFrame.empty()) {
        throw runtime_error("Ingen frame tilgængelig!");
    }
    
    Mat frame;
    bool found = false;

    ChessboardMatrix boardMatrix;

    // Prøv op til 10 gange at finde skakbrættet
    for (int i = 0; i < 10 && !found; ++i) {
        frame = currentFrame.clone();
        Mat original = frame.clone();

        found = detectChessboard(frame);

        if (found) {
            if (transformChessboard(frame)) {
                // Detect farvede prikker og vis resultat i matrix
                boardMatrix = getBoardMatrix(frame);

                //printBoard(frame, boardMatrix);
            }

            break;  // Succes
        }

        // waitKey(100); // Giv tid mellem forsøg
    }

    if (!found) {
        cerr << "Kunne ikke finde skakbrættet." << endl;
    }

    imshow("Processeret Billede", frame);
    // waitKey(1);

    return boardMatrix;
}

ChessboardMatrix ChessVision::getBoardMatrix(const Mat& frame) {
    // Lav en 8x8 matrix fyldt med 'E' for "Empty"
    ChessboardMatrix board(8, vector<char>(8, 'e'));

    // Konverter billedet til HSV for bedre farvegenkendelse
    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    // Find røde og blå farver i billedet
    Mat maskRed1, maskRed2, maskGreen;
    inRange(hsv, Scalar(0, 100, 100), Scalar(10, 255, 255), maskRed1);   // Lys rød
    inRange(hsv, Scalar(170, 100, 100), Scalar(180, 255, 255), maskRed2); // Mørk rød
    inRange(hsv, Scalar(30, 76, 51), Scalar(90, 255, 255), maskGreen);  // Grøn

    // Kombiner de to røde masker
    Mat maskRed = maskRed1 | maskRed2;

    // Find konturer (områder) for røde og blå prikker
    vector<vector<Point>> contoursRed, contoursBlue;
    findContours(maskRed, contoursRed, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    findContours(maskGreen, contoursBlue, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    auto markBoard = [&](const vector<vector<Point>>& contours, char symbol) {
        for (const auto& contour : contours) {
            double area = contourArea(contour);
            if (area < 20 || area > 5000) continue; // Ignorer små konturer (<5 pixels)
    
            Rect box = boundingRect(contour);
            Point center = (box.tl() + box.br()) * 0.5;
    
            int col = static_cast<int>(center.x / (targetSize / 8.0));
            int row = static_cast<int>(center.y / (targetSize / 8.0));
    
            if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                board[row][col] = symbol;
            }
        }
    };
    
    // Marker felterne med røde og blå prikker
    markBoard(contoursRed, 'W');
    markBoard(contoursBlue, 'B');

    //imshow("Rød maske", maskRed);
    //imshow("Grøn maske", maskGreen);

    return board;
}