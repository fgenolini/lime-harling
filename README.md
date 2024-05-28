# lime-harling
Lime render of an external garden wall using a thrown mix of NHL lime
and granite dust

## Native C++ SDL game

This application is primarily a native (Microsoft Windows) 2D game
written in C++ for SDL 2.

### Building SDL code natively on Windows using C++

Install Visual Studio 2022 (with support for C++ 20)
or the Build Tools for VS 2022.

Download the SDL release archive for your platform,
such as `SDL2-devel-2.30.3-VC.zip` and extract it on your computer.

Use `cmake-gui`, point CMake to your SDL folder,
and then generate a VS 2022 solution, and use it to build and run the app.

If you use the command line tools, then type:

```bat
msbuild lime_harling.sln /t:Rebuild
```

Before you run the Windows executable just built,
ensure that `SDL2.dll` is in your `PATH`
or copy it to the same place as `lime_harling.exe`.

This will create a small window with no border and no title bar,
showing an animated colourful square.  To exit, press the `Esc` key.

## Web game using emscripten

Most of the C++ source code is the same for native and web version of the app,
with the web specific source code included at compile time by the preprocessor:

```c
#ifdef __EMSCRIPTEN__
  // emscripten only code
  emscripten_set_main_loop(game_loop, FRAME_RATE, SIMULATE_INFINITE_LOOP);
#endif
```

### Building for emscripten (C++ to web)
Use a command such as
```bash
emcc -std=c++20 -o index.html lime_harling.cpp --use-port=sdl2
```

### Testing on your computer

```bash
emrun index.html
```

### Suggested changes to emscripten
The emsripten console is quite verbose, and not useful for users
(only for developers during debugging),
so a lot of messages can simply be removed.

In `upstream\emscripten\src\library_sdl.js`,
comment out the line in `SDL_Quit()` :

```JavaScript
SDL_Quit: () => {
  _SDL_AudioQuit();

  // Not needed:
  // out('SDL_Quit called (and ignored)');
}
```

