@ECHO OFF
REM Script to build the web page

REM Setup emscripten
REM cd source\repos\emsdk
REM emcmdprompt.bat

REM Build
emcc -o index.html hello_world_sdl.cpp -O3 --shell-file html_template\shell_minimal.html
