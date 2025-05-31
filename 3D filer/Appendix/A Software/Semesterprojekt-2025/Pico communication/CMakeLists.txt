cmake_minimum_required(VERSION 3.13)

# === PROJECT STANDARDS ===
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# === YOUR LOCAL PICO SDK PATH ===
# (adjust to wherever you cloned pico-sdk)
set(PICO_SDK_PATH "/home/emilklitgaard/pico-sdk")

# === REQUIRED: pull in the pico_sdk_import helper ===
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PicoCommunication C CXX ASM)

# === INITIALIZE THE PICO SDK ===
pico_sdk_init()

# === YOUR MAIN EXECUTABLE ===
add_executable(PicoCommunication main.cpp)

# Choose stdio transport
pico_enable_stdio_uart(PicoCommunication 0)
pico_enable_stdio_usb(PicoCommunication 1)

# Link in the Pico libraries
target_link_libraries(PicoCommunication pico_stdlib hardware_i2c hardware_adc hardware_pwm)

# Make .uf2/.hex/.bin outputs
pico_add_extra_outputs(PicoCommunication)

