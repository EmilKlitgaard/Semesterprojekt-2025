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
add_executable(ChessRobot main.cpp Chessboard.cpp Stockfish.cpp Vision.cpp PhoneticInput.cpp ChessGui.cpp)

# Link against the installed UR RTDE library
target_link_libraries(ChessRobot rtde pthread ${OpenCV_LIBS})

# For testing
add_executable(visionTestExecutable visionTest.cpp Chessboard.cpp Vision.cpp) 
target_link_libraries(visionTestExecutable ${OpenCV_LIBS})
add_executable(stockfishTestExecutable stockfishTest.cpp Chessboard.cpp Stockfish.cpp) 
target_link_libraries(stockfishTestExecutable ${OpenCV_LIBS})

# Add Qt components
find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

# Enable Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Link libraries
target_link_libraries(ChessRobot PRIVATE
    Qt6::Widgets
    Qt6::Core
    ${OpenCV_LIBS}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PORTAUDIO_INCLUDE_DIRS}
    ${VOSK_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
    /usr/local/include/ur_rtde
    ${EIGEN3_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
pkg_check_modules(VOSK REQUIRED vosk)

target_link_libraries(ChessRobot PRIVATE
    ${PORTAUDIO_LIBRARIES}
    ${VOSK_LIBRARIES}
    rtde
    pthread
    ${OpenCV_LIBS}
)