#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include "Chessboard.h"

using namespace cv;
using namespace std;

class ChessVision {
    public:
        ChessVision(int cameraIndex = 0);
        vector<vector<char>> detectBoardState();
        void calibratePerspective(Mat& frame);
        Mat captureFrame() {
            Mat frame;
            cap >> frame;
            return frame;
        }
    
        // Move this to public
        vector<Point2f> detectChessboardCorners(Mat& frame);
    
    private:
        VideoCapture cap;
        Size boardSize = Size(8, 8);
        vector<Point2f> boardCorners;
        Mat cameraMatrix;
        Mat distCoeffs;
    
        vector<Point2f> detectColorDots(Mat& frame, Scalar lower, Scalar upper);
        char classifyPiece(Point2f position);
};