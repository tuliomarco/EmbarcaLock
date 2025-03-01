# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(embarca_lock C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(embarca_lock 
    embarca_lock.c 
    ui/display_manager.c 
    ui/user_choose.c
    ui/matrix_animations.c
    core/password_manager.c
    core/sound_manager.c
    )

pico_set_program_name(embarca_lock "embarca_lock")
pico_set_program_version(embarca_lock "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(embarca_lock 1)
pico_enable_stdio_usb(embarca_lock 1)

pico_generate_pio_header(embarca_lock ${CMAKE_CURRENT_LIST_DIR}/core/ws2812b.pio)

# Add the standard library to the build
target_link_libraries(embarca_lock
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(embarca_lock PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/ui
        ${CMAKE_CURRENT_LIST_DIR}/core
)

# Add any user requested libraries
target_link_libraries(embarca_lock 
        hardware_i2c
        hardware_adc
        hardware_flash
        hardware_sync
        hardware_clocks
        hardware_pio
        hardware_pwm
        )

pico_add_extra_outputs(embarca_lock)

