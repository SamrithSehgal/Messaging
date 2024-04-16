project("Tree")

cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "g++")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")


include_directories(include)
file(GLOB SOURCES "./*.cpp")
add_executable(tree ${SOURCES})

