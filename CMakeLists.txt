cmake_minimum_required(VERSION 3.5)
project(ChessRobot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

# Required packages
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(
    /home/sammy/ur_rtde/include
    ${EIGEN3_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

link_directories(/home/sammy/ur_rtde/build)

set(CMAKE_AUTOMOC ON)

# Define the main executable
add_executable(chessRobotExecutable
    main.cpp
    Chessboard.cpp
    Stockfish.cpp
    Vision.cpp
    Gripper.cpp
    ChessBotGUI.cpp
    GUI/GUI.cpp
    GUI/GUIWindow.cpp
)

target_link_libraries(chessRobotExecutable
    rtde
    pthread
    ${OpenCV_LIBS}
    Qt6::Widgets
)

# Tests
add_executable(visionTestExecutable visionTest.cpp Chessboard.cpp Vision.cpp)
target_link_libraries(visionTestExecutable ${OpenCV_LIBS})

add_executable(stockfishTestExecutable stockfishTest.cpp Chessboard.cpp Stockfish.cpp)
target_link_libraries(stockfishTestExecutable)

add_executable(chessTestExecutable mainTest.cpp Chessboard.cpp Stockfish.cpp)
target_link_libraries(chessTestExecutable)

add_executable(picoTestExecutable picoTest.cpp)
target_link_libraries(picoTestExecutable)
