# Build the website using emscripten
#
# Before running this, setup the environment to point to emcc

.PHONY : all
all:
	emcc -std=c++23 -Wall -g0 -O3 -flto -fno-exceptions -o index.html lime_harling.cpp --use-port=sdl2 -sALLOW_MEMORY_GROWTH=1 --shell-file html_template/shell_minimal.html
