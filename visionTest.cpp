#include "Vision.h"
#include <iostream>

int main() {
    try {
        ChessVision vision(0);
        namedWindow("Chessboard Detection", WINDOW_NORMAL);
        
        while (true) {
            Mat frame;
            bool success = vision.processFrame(frame);
            
            if (!frame.empty()) {
                imshow("Chessboard Detection", frame);
            }
            
            if (waitKey(1) == 27) break; // ESC to exit
        }
        
        vision.releaseCamera();
    } catch (const exception& e) {
        cerr << "Fatal Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}