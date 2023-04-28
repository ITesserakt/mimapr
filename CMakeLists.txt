cmake_minimum_required(VERSION 3.5)

project(mimapr CXX)

set(CXX_STANDARD 20)

add_executable(main src/main.cpp src/mesh.cpp src/config.cpp)

target_include_directories(main PRIVATE lib/eigen/)
target_include_directories(main PRIVATE include/)
