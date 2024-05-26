cmake_minimum_required(VERSION 3.29)

include(CheckIPOSupported)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(lime_harling)

add_executable(lime_harling lime_harling.cpp)
set_property(TARGET lime_harling PROPERTY CXX_STANDARD 23)
if(MSVC)
  target_compile_options(lime_harling PRIVATE /W4 /WX)
else()
  target_compile_options(lime_harling PRIVATE -Wall -Wextra -Wpedantic -flto -fno-exceptions)
endif()
set_property(TARGET lime_harling PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(lime_harling ${SDL2_LIBRARIES})
