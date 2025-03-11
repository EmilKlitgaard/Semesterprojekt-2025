cmake_minimum_required(VERSION 3.5)
project(ChessRobot)

set(CMAKE_CXX_STANDARD 17)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(/usr/local/include/ur_rtde)
include_directories(${EIGEN3_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

link_directories(/usr/local/lib)

# Define the executable
add_executable(ChessRobot main.cpp Chessboard.cpp Stockfish.cpp Vision.cpp)

# Link against the installed UR RTDE library
target_link_libraries(ChessRobot rtde pthread ${OpenCV_LIBS})

# For testing
add_executable(visionTestExecutable visionTest.cpp Chessboard.cpp Vision.cpp) 
target_link_libraries(visionTestExecutable ${OpenCV_LIBS})
add_executable(stockfishTestExecutable stockfishTest.cpp Chessboard.cpp Stockfish.cpp) 
target_link_libraries(stockfishTestExecutable ${OpenCV_LIBS})