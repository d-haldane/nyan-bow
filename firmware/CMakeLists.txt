# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD "pico")

set(SOURCES
    synth.cpp
    oscillator.cpp
    effects.cpp
    ledstrip.cpp
    animations.cpp
    knobs.cpp
    ws2816.pio
)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(synth C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable( synth 
                synth.cpp 
                oscillator.cpp
                effects.cpp
                ledstrip.cpp
                animations.cpp
                knobs.cpp
                ws2816.pio)

pico_set_program_name(synth "synth")
pico_set_program_version(synth "0.2")

pico_enable_stdio_uart(synth 0)
pico_enable_stdio_usb(synth 1)

include(FetchContent)
FetchContent_Declare(
    libfixmath
    GIT_REPOSITORY https://github.com/PetteriAimonen/libfixmath.git
    GIT_TAG d308e466e1a09118d03f677c52e5fbf402f6fdd0
)
FetchContent_MakeAvailable(libfixmath)

target_compile_definitions(libfixmath PRIVATE
    # FIXMATH_FAST_SIN
    # FIXMATH_NO_64BIT
    # FIXMATH_NO_CACHE
    # FIXMATH_NO_HARD_DIVISION
    # FIXMATH_NO_OVERFLOW
    # FIXMATH_NO_ROUNDING
    # FIXMATH_OPTIMIZE_8BIT
)

# Add the standard library to the build
target_link_libraries(synth
        pico_stdlib
        pico_multicore
        libfixmath)

# Add the standard include files to the build
target_include_directories(synth PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# target_sources(pio_ws2816 PRIVATE ws2816.c)

# Add any user requested libraries
target_link_libraries(synth
        hardware_i2c
        hardware_dma
        hardware_timer
        hardware_pwm
        hardware_irq
        hardware_adc
        hardware_pio)

#target_link_libraries(pio_ws2812 PRIVATE pico_stdlib hardware_pio)

pico_generate_pio_header(synth ${CMAKE_CURRENT_LIST_DIR}/ws2816.pio)

pico_add_extra_outputs(synth)
