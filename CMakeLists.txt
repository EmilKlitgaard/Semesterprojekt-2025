# Set minimum required version of cmake
cmake_minimum_required(VERSION 3.12)

#include build functions from pico sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

#set name of project (as PROJECT_NAME) and c++ standards
project(blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#creates a pico_sdk subdirectory in our project for the libraries
pico_sdk_init()

#tell cmake where to find the executable source file
add_executable(${PROJECT_NAME}
    main.c
)

#create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

#LINK TO pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
)