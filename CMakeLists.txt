# Build project specification for CMake

cmake_minimum_required(VERSION 3.29)

include(CheckIPOSupported)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(lime_harling)

if(MSVC)
  # Only Windows is tested
  add_executable(lime_harling WIN32 lime_harling.cpp)
  target_compile_options(lime_harling PUBLIC /W4 /WX /EHs-c- /D_HAS_EXCEPTIONS=0)
else()
  add_executable(lime_harling lime_harling.cpp)
  target_compile_options(lime_harling PUBLIC -Wall -Wextra -Wpedantic -flto -fno-exceptions)
endif()

set_property(TARGET lime_harling PROPERTY CXX_STANDARD 23)
set_property(TARGET lime_harling PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(lime_harling ${SDL2_LIBRARIES})

