cmake_minimum_required(VERSION 3.5)

project(mimapr CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(lib/yaml-cpp)

add_executable(main src/main.cpp src/config.cpp src/mesh.cpp src/Solver.cpp)

find_package(OpenMP)
if (${OpenMP_FOUND})
    target_link_libraries(main PRIVATE ${OpenMP_LIBRARIES})
    target_include_directories(main PRIVATE ${OpenMP_INCLUDE_DIRS})
    target_compile_definitions(main PRIVATE USE_OPEN_MP)
endif ()

target_include_directories(main PRIVATE lib/eigen/)
target_include_directories(main PRIVATE lib/yaml-cpp/include)
target_include_directories(main PRIVATE include/)

target_link_libraries(main PRIVATE yaml-cpp)

target_compile_definitions(main PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)

configure_file(resources/config.yml resources/config.yml COPYONLY)
