@ECHO OFF
REM Script to build the web page

REM Setup emscripten
REM cd source\repos\emsdk
REM emcmdprompt.bat

REM Build
REM SDL 1 is the default
REM emcc -v -std=c++23 -Wall -g0 -O3 -flto -fno-exceptions -DSDL1 -o index.html lime_harling.cpp --shell-file html_template\shell_minimal.html
REM SDL 2 requires --use-port=sdl2
emcc -std=c++23 -Wall -g0 -O3 -flto -fno-exceptions -o index.html lime_harling.cpp --use-port=sdl2 --shell-file html_template\shell_minimal.html

