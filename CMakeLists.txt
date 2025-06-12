cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# create project
project(chip8-cpp
        VERSION 0.1
        DESCRIPTION "A CHIP-8 interpreter written in C++ with SDL2. The beginning of my game emulator development journey."
        LANGUAGES CXX)

# options
option(CHIP8_CPP_CORE_ONLY "Only build the core" OFF)

# C++ version: C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add dependencies
include(cmake/common.cmake)

# add subdirectories
add_subdirectory(source)