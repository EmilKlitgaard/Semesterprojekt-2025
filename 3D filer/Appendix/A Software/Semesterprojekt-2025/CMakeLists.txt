cmake_minimum_required(VERSION 3.5)
project(ChessRobot)

set(CMAKE_CXX_STANDARD 17)

# Meta object compiler
set(CMAKE_AUTOMOC ON)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(/usr/local/include/ur_rtde)
include_directories(${EIGEN3_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

link_directories(/usr/local/lib)

# Define the executablepickupTest
add_executable(chessRobotExecutable main.cpp Game.cpp Chessboard.cpp Stockfish.cpp Vision.cpp Gripper.cpp GUI.cpp GUIWindow.cpp)
# Link against the installed UR RTDE library
target_link_libraries(chessRobotExecutable rtde pthread ${OpenCV_LIBS} Qt6::Widgets)

# Testing executables
#add_executable(visionTestExecutable Test\ files/visionTest.cpp Vision.cpp) 
#target_link_libraries(visionTestExecutable ${OpenCV_LIBS})

#add_executable(stockfishTestExecutable Test\ files/stockfishTest.cpp Chessboard.cpp Stockfish.cpp) 
#target_link_libraries(stockfishTestExecutable)

#add_executable(chessTestExecutable Test\ files/mainTest.cpp Chessboard.cpp Stockfish.cpp) 
#target_link_libraries(chessTestExecutable)

#add_executable(pickupTestExecutable Test\ files/pickupTest.cpp Chessboard.cpp Gripper.cpp) 
#target_link_libraries(pickupTestExecutable rtde)

#add_executable(picoTestExecutable Test\ files/picoTest.cpp)
#target_link_libraries(picoTestExecutable)

#add_executable(resetBoardTestExecutable Test\ files/resetBoardTest.cpp Chessboard.cpp)
#target_link_libraries(resetBoardTestExecutable)
