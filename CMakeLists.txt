cmake_minimum_required(VERSION 3.29)

project(lime_harling)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(lime_harling lime_harling.cpp)

target_link_libraries(lime_harling ${SDL2_LIBRARIES})
