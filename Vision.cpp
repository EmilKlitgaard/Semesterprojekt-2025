#include "Vision.h"

ChessVision::ChessVision(int cameraIndex) {
    cap.open(cameraIndex);
    if (!cap.isOpened()) {
        throw runtime_error("Failed to open camera!");
    }
}

vector<Point2f> ChessVision::detectChessboardCorners(Mat& frame) {
    vector<Point2f> corners;
    bool found = findChessboardCorners(frame, boardSize, corners,
        CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);
    
    if (found) {
        // Refine corner locations
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1),
            TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
    }
    return corners;
}

vector<Point2f> ChessVision::detectColorDots(Mat& frame, Scalar lower, Scalar upper) {
    Mat hsv, mask;
    vector<Point2f> dots;
    
    cvtColor(frame, hsv, COLOR_BGR2HSV);
    inRange(hsv, lower, upper, mask);
    
    // Remove noise
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    morphologyEx(mask, mask, MORPH_OPEN, kernel);
    
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        if (contourArea(contour) > 100) { // Filter small noise
            Moments m = moments(contour);
            dots.emplace_back(m.m10/m.m00, m.m01/m.m00);
        }
    }
    return dots;
}

void ChessVision::calibratePerspective(Mat& frame) {
    boardCorners = detectChessboardCorners(frame);
    if (boardCorners.size() != 4) {
        throw runtime_error("Need exactly 4 chessboard corners for calibration!");
    }
}

vector<vector<char>> ChessVision::detectBoardState() {
    Mat frame;
    cap >> frame;
    
    // Detect chessboard corners
    auto corners = detectChessboardCorners(frame);
    if (corners.empty()) return vector<vector<char>>(8, vector<char>(8, 'e'));
    
    // Warp perspective to get top-down view
    Mat warped;
    vector<Point2f> targetCorners = {
        Point2f(0, 0),
        Point2f(400, 0),
        Point2f(400, 400),
        Point2f(0, 400)
    };
    Mat H = findHomography(corners, targetCorners);
    warpPerspective(frame, warped, H, Size(400, 400));
    
    // Detect color dots
    vector<Point2f> redDots = detectColorDots(warped, 
        Scalar(0, 120, 70), Scalar(10, 255, 255)); // Red range
    vector<Point2f> blueDots = detectColorDots(warped, 
        Scalar(100, 150, 50), Scalar(140, 255, 255)); // Blue range
    
    // Map positions to chessboard squares
    vector<vector<char>> boardState(8, vector<char>(8, 'e'));
    float squareSize = 400.0f / 8;
    
    auto mapPosition = [&](Point2f pt, char color) {
        int col = min(7, static_cast<int>(pt.x / squareSize));
        int row = min(7, static_cast<int>(pt.y / squareSize));
        boardState[row][col] = (color == 'B') ? 'W' : 'B';
    };
    
    for (const auto& pt : redDots) mapPosition(pt, 'R');
    for (const auto& pt : blueDots) mapPosition(pt, 'B');
    
    return boardState;
}