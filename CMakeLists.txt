cmake_minimum_required(VERSION 3.20)
project(lobster)
set(CMAKE_CXX_STANDARD 20)

set(SRC_FILES src/main.cpp)

add_executable(lobster ${SRC_FILES})
