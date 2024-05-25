cmake_minimum_required(VERSION 3.29)

project(lime_harling)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(lime_harling lime_harling.cpp)
set_property(TARGET lime_harling PROPERTY CXX_STANDARD 11)
target_link_libraries(lime_harling ${SDL2_LIBRARIES})
