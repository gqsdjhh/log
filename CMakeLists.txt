cmake_minimum_required(VERSION 3.11)

project(main)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_executable(main src/main.cpp ${CMAKE_SOURCE_DIR}/src/log.cpp)

