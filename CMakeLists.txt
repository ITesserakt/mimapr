cmake_minimum_required(VERSION 3.5)

project(mimapr CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(main src/main.cpp src/mesh.cpp src/config.cpp src/Solver.cpp)

target_include_directories(main PRIVATE lib/eigen/)
target_include_directories(main PRIVATE include/)
