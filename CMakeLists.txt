project("Tree")

cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "g++")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

find_package(PostgreSQL REQUIRED)

include_directories(include)
#file(GLOB SOURCES "./*.cpp")
add_executable(tree main.cpp pfm.cpp sm.cpp am.cpp)
target_link_libraries(tree PRIVATE PostgreSQL::PostgreSQL)

