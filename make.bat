@ECHO OFF
REM Script to build the web page

REM Setup emscripten
REM cd source\repos\emsdk
REM emcmdprompt.bat

REM Build
REM SDL 1 is the default
REM emcc -std=c++20 -Wall -O3 -o index.html lime_harling.cpp --shell-file html_template\shell_minimal.html
REM SDL 2 requires --use-port=sdl2
emcc -std=c++20 -Wall -O3 -o index.html lime_harling.cpp --use-port=sdl2 --shell-file html_template\shell_minimal.html
