cmake_minimum_required(VERSION 3.20)
project(lobster)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lobster_core src/Orderbook.cpp)

target_include_directories(lobster_core PUBLIC include)
target_compile_features(lobster_core PUBLIC cxx_std_20)

add_executable(lobster src/main.cpp)
target_link_libraries(lobster PRIVATE lobster_core)
