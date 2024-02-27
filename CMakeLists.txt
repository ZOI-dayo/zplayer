cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
project(zplayer)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INC_DIR ${PROJECT_SOURCE_DIR}/include)
add_definitions( -std=c++17 -Wall )
include_directories(
         ${INC_DIR}
)
add_executable(
         zplayer
         ${SRC_DIR}/main.cpp
)
