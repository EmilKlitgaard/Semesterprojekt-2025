#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace cv;
using namespace std;

class ChessVision {
public:
    ChessVision(int cameraIndex = 0);
    ~ChessVision();
    
    bool processFrame(Mat& output);
    void releaseCamera();

private:
    VideoCapture cap;
    Size boardSize = Size(7, 7); // Inner corners for 8x8 board
    vector<Point2f> corners;
    const int targetSize = 600; // Warped output size

    bool findChessboard(Mat& frame);
    bool transformChessboard(Mat& frame);
    bool checkCornersValid(const vector<Point2f>& corners);
    
    void drawOverlay(Mat& frame);
    void detectDots(Mat& frame); 
    
    vector<Point2f> getSquareCenters() const;
    vector<Point2f> getOuterCorners() const;
    
    Mat preprocessImage(Mat& input);
};
