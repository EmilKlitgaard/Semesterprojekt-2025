cmake_minimum_required(VERSION 3.5)
project(ChessRobot)

set(CMAKE_CXX_STANDARD 11)

# Find the UR RTDE package
include_directories(/usr/local/include/ur_rtde)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

link_directories(/usr/local/lib)

# Define the executable
add_executable(ChessRobot main.cpp Chessboard.cpp)

# Link against the installed UR RTDE library
target_link_libraries(ChessRobot rtde pthread)

