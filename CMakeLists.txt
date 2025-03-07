cmake_minimum_required(VERSION 3.5)
project(ChessRobot)

set(CMAKE_CXX_STANDARD 17)

# Find the UR RTDE package
include_directories(/usr/local/include/ur_rtde)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

link_directories(/usr/local/lib)

# Define the executable
add_executable(ChessRobot main.cpp Chessboard.cpp Stockfish.cpp Vision.cpp)

# Link against the installed UR RTDE library
target_link_libraries(ChessRobot rtde pthread ${OpenCV_LIBS})

# For testing
add_executable(TestExecutable test.cpp Vision.cpp Chessboard.cpp) 
target_link_libraries(TestExecutable ${OpenCV_LIBS})