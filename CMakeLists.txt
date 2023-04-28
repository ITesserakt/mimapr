cmake_minimum_required(VERSION 3.5)

project(mimapr CXX)

set(CXX_STANDARD 17)

add_executable(main src/main.cpp)

target_include_directories(main PRIVATE include)