cmake_minimum_required(VERSION 3.7)
project(chip8)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8 chip8.c)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
target_link_libraries(chip8 ${SDL2_LIBRARIES})

