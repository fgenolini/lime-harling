/* SDL 2 game loop that shows a colourful square that changes colour over time
*/

#include <stdio.h>
#ifdef _WIN32
// This may not be a Win32 (Windows / DOS) thing, but this is the case on my PC
#include "SDL.h"
#else
// If the wrong include is used, then the emscripten display will be black

// For SDL 1
// #include <SDL/SDL.h>

// For SDL 2
#include <SDL2/SDL.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Renderer size needs to be small to be usable in a web browser (emscripten)
constexpr Uint16 SCREEN_WIDTH = 256;
constexpr Uint16 SCREEN_HEIGHT = 256;

#if SDL_MAJOR_VERSION > 1
static SDL_Window* window{};
static SDL_Renderer* renderer{};
#else
static SDL_Surface* screen{};
#endif
static Uint8 shift{};
static SDL_Event event{};

static void end_sdl() noexcept
{
#if SDL_MAJOR_VERSION > 1
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
#endif
  SDL_Quit();
  printf("SDL ended\n");
}

// Render a single frame, to be called from the main game loop function
static void render_frame() noexcept
{
#if SDL_MAJOR_VERSION > 1
  SDL_RenderClear(renderer);
  for (auto vert = 0; vert < SCREEN_HEIGHT; vert++)
  {
    for (auto horiz = 0; horiz < SCREEN_WIDTH; horiz++)
    {
#ifdef TEST_SDL_LOCK_OPTS
      // Alpha behaves like in the browser, so write proper opaque pixels.
      Uint8 alpha = 255;
#else
      // To emulate native behavior with blitting to screen, alpha component is ignored. Test that it is so by outputting
      // data (and testing that it does get discarded)
      Uint8 alpha = (Uint8)((vert + horiz) % 255);
#endif

      Uint8 r = (Uint8)vert;
      Uint8 g = (Uint8)horiz;
      Uint8 b = (Uint8)(255 - vert);
      Uint8 rot_r = (Uint8)(r + shift);
      Uint8 rot_g = (Uint8)(g - shift * 3);
      Uint8 rot_b = (Uint8)(b + shift * 2);
      SDL_SetRenderDrawColor(renderer, rot_r, rot_g, rot_b, alpha);
      SDL_RenderDrawPoint(renderer, horiz, vert);
    }
  }

  SDL_RenderPresent(renderer);
#else
  if (SDL_MUSTLOCK(screen))
  {
    SDL_LockSurface(screen);
  }

  for (int i = 0; i < SCREEN_HEIGHT; i++)
  {
    for (int j = 0; j < SCREEN_WIDTH; j++)
    {
#ifdef TEST_SDL_LOCK_OPTS
      // Alpha behaves like in the browser, so write proper opaque pixels.
      int alpha = 255;
#else
      // To emulate native behavior with blitting to screen, alpha component is ignored. Test that it is so by outputting
      // data (and testing that it does get discarded)
      int alpha = (i + j) % 255;
#endif
      Uint8 r = i;
      Uint8 g = j;
      Uint8 b = 255 - i;
      auto rot_r = r + shift;
      auto rot_g = g - shift * 3;
      auto rot_b = b + shift * 2;
      *((Uint32*)screen->pixels + i * 256 + j) = SDL_MapRGBA(screen->format,
        rot_r, rot_g, rot_b, alpha);
    }
  }

  if (SDL_MUSTLOCK(screen))
  {
    SDL_UnlockSurface(screen);
  }

  SDL_Flip(screen);
#endif
  shift++;
  if (shift > 255) {
    shift = 0;
  }
}

// Main game loop that renders frames until user quits the game
static void game_loop() noexcept
{
#ifdef __EMSCRIPTEN__
  render_frame();
#if SDL_MAJOR_VERSION > 1
  int w{};
  int h{};
  SDL_GetRendererOutputSize(renderer, &w, &h);
  if (w != SCREEN_WIDTH || h != SCREEN_HEIGHT) {
    // Frame rendering assumes a fixed dimension rendering surface
    printf("resolution change, w %d, h %d\n", w, h);
    end_sdl();
    emscripten_cancel_main_loop();
    return;
  }
#endif

  if (SDL_PollEvent(&event)) {
    switch (event.type)
    {
    case SDL_QUIT:
      printf("quit\n");
      end_sdl();
      emscripten_cancel_main_loop();
      break;

    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        printf("escape\n");
        end_sdl();
        emscripten_cancel_main_loop();
        break;
      }

      break;
    }
  }
#else
  auto want_out{ false };
  while (!want_out) {
    auto start_ticks = SDL_GetTicks64();

    int w{};
    int h{};
    SDL_GetRendererOutputSize(renderer, &w, &h);
    if (w != SCREEN_WIDTH || h != SCREEN_HEIGHT) {
      // Frame rendering assumes a fixed dimension rendering surface
      printf("resolution change, w %d, h %d\n", w, h);
      want_out = true;
      continue;
    }

    while (SDL_PollEvent(&event) && !want_out) {
      switch (event.type)
      {
      case SDL_QUIT:
        want_out = true;
        break;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          want_out = true;
          break;
        }

        break;
      }
    }

    render_frame();
    auto end_ticks = SDL_GetTicks64();
    auto frame_time = end_ticks - start_ticks;
    if (frame_time > 0 && frame_time < 42)
    {
      // 24 fps
      SDL_Delay((Uint32)(42 - frame_time));
    }
  }
#endif
}

int main(int, char**)
{
#ifdef __EMSCRIPTEN__
  printf("Press the Esc key to end the animation\n");
#else
  printf("Lime render of an external garden wall using a thrown mix of NHL lime and granite dust\n");
#endif

#if SDL_MAJOR_VERSION > 1
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
#else
  SDL_Init(SDL_INIT_VIDEO);
#endif

#if SDL_MAJOR_VERSION > 1
  SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
    SDL_WINDOW_BORDERLESS, &window, &renderer);
  if (window == NULL) {
    fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    return 1;
  }

  SDL_SetRenderDrawColor(renderer, 12, 123, 50, 255);
  SDL_RenderClear(renderer);
#else
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
#endif

#ifdef __EMSCRIPTEN__
  constexpr auto FRAME_RATE = 0;
  constexpr auto SIMULATE_INFINITE_LOOP = 1;
  emscripten_set_main_loop(game_loop, FRAME_RATE, SIMULATE_INFINITE_LOOP);
#else
  game_loop();
  end_sdl();
#endif
  return 0;
}

