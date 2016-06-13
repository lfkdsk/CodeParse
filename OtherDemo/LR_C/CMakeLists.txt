cmake_minimum_required(VERSION 3.5)
project(LR_C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(LR_C ${SOURCE_FILES})