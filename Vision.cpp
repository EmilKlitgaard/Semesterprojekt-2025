#include "Vision.h"

ChessVision::ChessVision(int cameraIndex) {
    cap.open(cameraIndex, CAP_V4L2);
    if (!cap.isOpened()) {
        cerr << "Failed to open camera!" << endl;
        throw runtime_error("Camera error");
    }
    
    cap.set(CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(CAP_PROP_AUTO_EXPOSURE, 1);
    cap.set(CAP_PROP_EXPOSURE, 150);
    cap.set(CAP_PROP_FPS, 30);
}

ChessVision::~ChessVision() {
    releaseCamera();
}

Mat ChessVision::preprocessImage(Mat& input) {
    Mat gray;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    return gray;
}

bool ChessVision::findChessboard(Mat& frame) {
    Mat processed = preprocessImage(frame);
    
    bool found = findChessboardCorners(processed, boardSize, corners,
        CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
    
    if (found) {
        if (!checkCornersValid(corners)) return false;
        
        TermCriteria criteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.001);
        cornerSubPix(processed, corners, Size(11, 11), Size(-1, -1), criteria);
        
        if (corners.size() != boardSize.area()) {
            cerr << "Invalid corner count: " << corners.size() << endl;
            return false;
        }
    }
    return found;
}

bool ChessVision::checkCornersValid(const vector<Point2f>& corners) {
    return all_of(corners.begin(), corners.end(), [](const Point2f& p) {
        return !cvIsNaN(p.x) && !cvIsNaN(p.y) && 
               p.x >= 0 && p.y >= 0 &&
               p.x < 10000 && p.y < 10000;
    });
}

vector<Point2f> ChessVision::getOuterCorners() const {
    vector<Point2f> outerCorners;
    if (corners.size() != 49) return outerCorners;

    // Calculate outer corners by extrapolating from inner corners
    for (int row = -1; row <= boardSize.height; row++) {
        for (int col = -1; col <= boardSize.width; col++) {
            if (row >= 0 && row < boardSize.height && col >= 0 && col < boardSize.width) {
                // Use existing inner corners
                outerCorners.push_back(corners[row * boardSize.width + col]);
            } else {
                Point2f p;
                if (row == -1 && col == -1) {
                    // Top-left outer corner
                    p = 2 * corners[0] - corners[boardSize.width + 1];
                } else if (row == -1 && col == boardSize.width) {
                    // Top-right outer corner: use vertical extrapolation
                    p = 2 * corners[boardSize.width - 1] - corners[boardSize.width + (boardSize.width - 2)];
                } else if (row == boardSize.height && col == -1) {
                    // Bottom-left outer corner
                    p = 2 * corners[boardSize.width * (boardSize.height - 1)] - 
                        corners[boardSize.width * (boardSize.height - 2) + 1];
                } else if (row == boardSize.height && col == boardSize.width) {
                    // Bottom-right outer corner: use horizontal extrapolation
                    p = 2 * corners.back() - corners[boardSize.width * (boardSize.height - 2) + (boardSize.width - 2)];
                } else if (row == -1) {
                    // Top edge
                    p = 2 * corners[col] - corners[boardSize.width + col];
                } else if (row == boardSize.height) {
                    // Bottom edge
                    p = 2 * corners[boardSize.width * (boardSize.height - 1) + col] - 
                        corners[boardSize.width * (boardSize.height - 2) + col];
                } else if (col == -1) {
                    // Left edge
                    p = 2 * corners[row * boardSize.width] - 
                        corners[row * boardSize.width + 1];
                } else if (col == boardSize.width) {
                    // Right edge
                    p = 2 * corners[row * boardSize.width + (boardSize.width - 1)] - 
                        corners[row * boardSize.width + (boardSize.width - 2)];
                }
                outerCorners.push_back(p);
            }
        }
    }
    return outerCorners;
}

vector<Point2f> ChessVision::getSquareCenters() const {
    vector<Point2f> centers;
    vector<Point2f> outerCorners = getOuterCorners();
    if (outerCorners.size() != 81) return centers;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Point2f tl = outerCorners[row * 9 + col];
            Point2f tr = outerCorners[row * 9 + (col + 1)];
            Point2f bl = outerCorners[(row + 1) * 9 + col];
            Point2f br = outerCorners[(row + 1) * 9 + (col + 1)];

            centers.emplace_back((tl + tr + bl + br) * 0.25f);
        }
    }
    return centers;
}

bool ChessVision::transformChessboard(Mat& frame) {
    vector<Point2f> outerCorners = getOuterCorners();
    if (outerCorners.size() != 81) return false;

    // Get outermost corners: TL, TR, BR, BL
    vector<Point2f> srcCorners = {
        outerCorners.front(),                // Top-left
        outerCorners[8],                     // Top-right
        outerCorners.back(),                 // Bottom-right
        outerCorners[72]                     // Bottom-left
    };

    vector<Point2f> dstCorners = {
        Point2f(0, 0),
        Point2f(targetSize, 0),
        Point2f(targetSize, targetSize),
        Point2f(0, targetSize)
    };

    Mat warpMatrix = getPerspectiveTransform(srcCorners, dstCorners);
    warpPerspective(frame, frame, warpMatrix, Size(targetSize, targetSize));
    return true;
}

void ChessVision::drawOverlay(Mat& frame) {
    vector<Point2f> centers = getSquareCenters();
    if (centers.size() != 64) return;

    for (size_t i = 0; i < centers.size(); i++) {
        int row = i / 8;
        int col = i % 8;
        
        // Chess notation (a1 to h8)
        string label = string(1, 'a' + col) + to_string(8 - row);
        putText(frame, label, centers[i], FONT_HERSHEY_DUPLEX, 0.6, 
               Scalar(0, 255, 0), 1);
    }
}

bool ChessVision::processFrame(Mat& output) {
    Mat frame;
    if (!cap.read(frame)) {
        cerr << "Frame read error!" << endl;
        return false;
    }
    
    if (frame.empty()) {
        cerr << "Empty frame!" << endl;
        return false;
    }

    Mat original = frame.clone();
    bool boardFound = findChessboard(frame);
    
    if (boardFound) {
        drawOverlay(frame);
        if (!transformChessboard(frame)) {
            frame = original.clone();
        }
    } else {
        frame = original.clone();
    }

    output = frame.clone();
    return boardFound;
}

void ChessVision::releaseCamera() {
    if (cap.isOpened()) cap.release();
}