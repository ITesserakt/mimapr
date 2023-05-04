project(lodepng LANGUAGES CXX)

add_library(lodepng src/lodepng.cpp)

target_include_directories(lodepng PUBLIC include)