cmake_minimum_required(VERSION 3.7)
project(chip8)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(chip8 SHARED chip8.c)
add_executable(test_sdl main.c)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
#set ( CMAKE_C_FLAGS "-Weverything")
target_link_libraries(test_sdl ${SDL2_LIBRARIES} chip8)
