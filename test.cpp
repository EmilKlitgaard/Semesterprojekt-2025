#include "Vision.h"
#include <opencv2/highgui/highgui.hpp>

int main() {
    try {
        ChessVision vision(0);
        namedWindow("Camera Feed", WINDOW_NORMAL);
        
        while (true) {
            Mat frame = vision.captureFrame();
            if (frame.empty()) break;
            
            // Detect and draw chessboard
            auto corners = vision.detectChessboardCorners(frame);
            if (corners.size() == 4) {
                for (auto& pt : corners) {
                    circle(frame, pt, 10, Scalar(0, 255, 0), 2);
                }
            }
            
            // Show raw camera feed
            imshow("Camera Feed", frame);
            
            // Show detected board state
            auto board = vision.detectBoardState();
            Mat display(400, 400, CV_8UC3, Scalar(255, 255, 255));
            // ... (copy the showBoardState code here) ...
            
            if (waitKey(30) >= 0) break;
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}