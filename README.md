# lime-harling
Lime render of an external garden wall using a thrown mix of NHL lime and granite dust


## Suggested changes to emscripten
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


## Building SDL code natively on Windows using C++

Download the SDL release archive for your platform,
such as `SDL2-devel-2.30.3-VC.zip` and extract it on your computer.

Use a command such as
```bash
emcc -std=c++20 -o index.html lime_harling.cpp --use-port=sdl2
```

## Testing on your computer

```bash
emrun index.html
```
