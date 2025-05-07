#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

using ChessboardMatrix = vector<vector<char>>;

class ChessVision {
public:
    ChessVision(int cameraIndex);  // Constructor
    ~ChessVision();  // Destructor

    bool detectChessboard(Mat& frame);  // Detects chessboard
    bool transformChessboard(Mat& frame);  // Warps the board
    void detectDots(Mat& frame);  // Detects red/blue dots
    vector<Point2f> getSquareCenters() const;  // Calculates square centers
    void showFrame();  // Captures and displays frames
    void showLiveFeed();              // Live kamera
    ChessboardMatrix processCurrentFrame();      // Én billed-behandling
    ChessboardMatrix getBoardMatrix(const Mat& frame);  // Returnerer 8x8 matrix
    void printBoard(Mat &frame, ChessboardMatrix &boardMatrix);
    
    ChessboardMatrix getRefVisionBoard();
    void setRefVisionBoard(ChessboardMatrix &newRefVisionBoard);

private:
    void initializeVisionBoard();

    VideoCapture cap;  // Camera object
    Size boardSize = Size(7, 7);  // Chessboard size (7x7 corners)
    vector<Point2f> corners;  // Stores detected corners
    const int targetSize = 1000;  // Target size for transformed board
    Mat currentFrame;  // Gem seneste live-frame
    ChessboardMatrix refVisionBoard;
};

