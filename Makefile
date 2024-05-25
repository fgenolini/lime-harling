# Build the website using emscripten
#
# Before running this, setup the environment to point to emcc

.PHONY : all
all:
	emcc -std=c++20 -o index.html lime_harling.cpp -O3 --use-port=sdl2 --shell-file html_template/shell_minimal.html
