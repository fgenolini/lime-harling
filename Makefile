# Build the website using emscripten
#
# Before running this, setup the environment to point to emcc

.PHONY : all
all:
	emcc -o index.html hello_world_sdl.cpp -O3 --shell-file html_template/shell_minimal.html
