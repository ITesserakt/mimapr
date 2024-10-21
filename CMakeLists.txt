cmake_minimum_required(VERSION 3.5)

project(mimapr CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

add_subdirectory(lib/yaml-cpp)
add_subdirectory(lib/heatmap)
add_subdirectory(lib/lodepng)

add_executable(main
        src/main.cpp
        src/config.cpp
        src/mesh.cpp
        src/Solver.cpp
        src/drawer.cpp
        src/ProgressBar.cpp
        src/ffmpeg/mod.cpp
)

find_package(OpenMP)
if (${OpenMP_FOUND})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    target_include_directories(main PRIVATE ${OpenMP_INCLUDE_DIRS})
    target_compile_definitions(main PRIVATE USE_OPEN_MP)
endif ()

target_include_directories(main PRIVATE lib/eigen/)
target_include_directories(main PRIVATE lib/yaml-cpp/include)
target_include_directories(main PRIVATE include/)

target_link_libraries(main PRIVATE yaml-cpp)
target_link_libraries(main PRIVATE heatmap)
target_link_libraries(main PRIVATE lodepng)

configure_file(resources/config.yml ${CMAKE_BINARY_DIR}/res/config.yml COPYONLY)
if (NOT (MSVC_IDE))
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
endif ()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/lib")