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
