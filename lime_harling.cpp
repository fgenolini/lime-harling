/* SDL 2 game loop that shows a colourful square that changes colour over time
 *
 * Tested on Windows 11 PCs with multiple monitors, and on Google Chrome
 */

#include <stdio.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "SDL.h"

constexpr auto ALPHA = (Uint8)255;
constexpr auto SCREEN_HEIGHT = 256;
constexpr auto SCREEN_WIDTH = 256;
constexpr auto WINDOW_TITLE = "Lime harling";
#ifdef __EMSCRIPTEN__
constexpr auto FRAME_RATE = 0; // automatically determined by browser
// constexpr auto FRAME_RATE = 24; // frames per second
constexpr auto SIMULATE_INFINITE_LOOP = 1;
#endif

#if SDL_MAJOR_VERSION > 1
static SDL_Window *window{};
static SDL_Renderer *renderer{};
static Uint32 *pixels{};
static int pitch{};
static SDL_Texture *texture{};
static SDL_PixelFormat *format{};
#else
static SDL_Surface *screen{};
#endif
static auto shift{0};
static auto flip{false};
static SDL_Event event{};

#ifdef __EMSCRIPTEN__
static EmscriptenFullscreenStrategy strategy{};
#endif
static auto is_fullscreen{false};
static auto want_out{false};
static auto screen_width{SCREEN_WIDTH};
static auto screen_height{SCREEN_HEIGHT};

static void end_sdl() noexcept
{
#if SDL_MAJOR_VERSION > 1
  if (texture) [[likely]]
  {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }

  if (renderer) [[likely]]
  {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }

  if (window) [[likely]]
  {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
#endif

  SDL_Quit();
}

#ifdef __EMSCRIPTEN__
static EM_BOOL on_canvas_resize(int, const void *, void *) noexcept
{
  return false;
}
#endif

#if SDL_MAJOR_VERSION > 1
static void sdl2_render_loop_fn(int x, int y, Uint8 r, Uint8 g, Uint8 b) noexcept
{
  pixels[y * screen_width + x] = SDL_MapRGBA(format, r, g, b, ALPHA);
}
#else
static void sdl1_render_loop_fn(int x, int y, Uint8 r, Uint8 g, Uint8 b) noexcept
{
  *((Uint32 *)screen->pixels + y * screen_width + x) = SDL_MapRGBA(
      screen->format, r, g, b, ALPHA);
}
#endif

static void render_loop(void(render_loop_fn)(int x, int y, Uint8 r, Uint8 g, Uint8 b)) noexcept
{
  for (auto y = 0; y < screen_height; ++y)
  {
    for (auto x = 0; x < screen_width; ++x)
    {
      auto r = (Uint8)y;
      auto g = (Uint8)x;
      auto b = (Uint8)(255 - y);
      auto rot_r = (Uint8)(r + shift);
      auto rot_g = (Uint8)(g - shift * 3);
      auto rot_b = (Uint8)(b + shift * 2);
      render_loop_fn(x, y, rot_r, rot_g, rot_b);
    }
  }
}

// Render a single frame, to be called from the main game loop function
static void render_frame() noexcept
{
  if (screen_height < 1 || screen_width < 1) [[unlikely]]
  {
    return;
  }

#if SDL_MAJOR_VERSION > 1
  SDL_RenderClear(renderer);
  if (SDL_LockTexture(texture, nullptr, (void **)&pixels, &pitch) != 0) [[unlikely]]
  {
    fprintf(stderr, "Could not lock texture: %s\n", SDL_GetError());
    want_out = true;
    return;
  }

  if (pixels) [[likely]]
  {
    render_loop(sdl2_render_loop_fn);
  }

  SDL_UnlockTexture(texture);
  if (SDL_RenderCopy(renderer, texture, nullptr, nullptr) != 0) [[unlikely]]
  {
    fprintf(stderr, "Could not copy texture: %s\n", SDL_GetError());
    want_out = true;
    return;
  }

  SDL_RenderPresent(renderer);
#else
  if (SDL_MUSTLOCK(screen)) [[likely]]
  {
    SDL_LockSurface(screen);
  }

  render_loop(sdl1_render_loop_fn);
  if (SDL_MUSTLOCK(screen)) [[likely]]
  {
    SDL_UnlockSurface(screen);
  }

  SDL_Flip(screen);
#endif
  if (flip)
  {
    shift--;
  }
  else
  {
    shift++;
  }

  if (shift > 255)
  {
    flip = true;
  }
  else if (shift < 1)
  {
    flip = false;
  }
}

#ifndef __EMSCRIPTEN__
static void create_texture() noexcept
{
  // Scaled drawable dimensions, not screen size
  screen_width = SCREEN_WIDTH;
  screen_height = SCREEN_HEIGHT;
  if (texture) [[unlikely]]
  {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
  if (!texture) [[unlikely]]
  {
    fprintf(stderr, "Resized texture could not be created: %s\n", SDL_GetError());
    want_out = true;
  }
}
#endif

static bool poll_event_once() noexcept
{
  if (want_out) [[unlikely]]
  {
    return false;
  }

  if (!SDL_PollEvent(&event)) [[likely]]
  {
    return false;
  }

  switch (event.type)
  {
  [[unlikely]] case SDL_QUIT:
    want_out = true;
    break;

  case SDL_KEYDOWN:
    switch (event.key.keysym.sym)
    {
    [[unlikely]] case SDLK_ESCAPE:
      want_out = true;
      break;

    default:
      if (event.key.keysym.scancode == SDL_SCANCODE_Q) [[unlikely]]
      {
        want_out = true;
        break;
      }

      if (event.key.keysym.scancode == SDL_SCANCODE_F) [[unlikely]]
      {
        if (!is_fullscreen)
        {
          // Enter fullscreen
          is_fullscreen = true;
#ifdef __EMSCRIPTEN__
          strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
          strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
          strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
          strategy.canvasResizedCallback = on_canvas_resize;
          strategy.canvasResizedCallbackUserData = nullptr;
          emscripten_request_fullscreen_strategy("#canvas", false, &strategy);
#else
          if (texture) [[likely]]
          {
            SDL_DestroyTexture(texture);
            texture = nullptr;
          }

          if (renderer) [[likely]]
          {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
          }

          // When testing on my computer the software renderer is much faster
          // in fullscreen
          renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
          if (!renderer) [[unlikely]]
          {
            fprintf(stderr, "Software renderer could not be created: %s\n",
                    SDL_GetError());
            return false;
          }

          // Fake fullscreen works, real fullscreen struggles with multiple monitors
          if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
          {
            fprintf(stderr, "Could not go fullscreen: %s\n", SDL_GetError());
            want_out = true;
            return false;
          }

          create_texture();
#endif
        }
        else
        {
          // Exit fullscreen
          is_fullscreen = false;
#ifdef __EMSCRIPTEN__
          emscripten_exit_fullscreen();
#else
          if (texture) [[likely]]
          {
            SDL_DestroyTexture(texture);
            texture = nullptr;
          }

          if (renderer) [[likely]]
          {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
          }

          // Accelerated renderer is faster with windowed (not fullscreen)
          renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
          if (!renderer) [[unlikely]]
          {
            fprintf(stderr, "Accelerated renderer could not be created: %s\n",
                    SDL_GetError());
            return false;
          }

          SDL_SetWindowFullscreen(window, 0);
          create_texture();
          SDL_SetWindowPosition(window,
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#endif
        }
      }

      break;
    }

    break;
  }

  return true;
}

// Main game loop that renders frames until user quits the game
static void game_loop() noexcept
{
#ifdef __EMSCRIPTEN__
  poll_event_once();
  if (want_out) [[unlikely]]
  {
    emscripten_cancel_main_loop();
    return;
  }

  render_frame();
#else
  while (!want_out)
  {
    auto start_ticks = SDL_GetTicks64();
    while (poll_event_once())
    {
      if (want_out) [[unlikely]]
      {
        return;
      }
    }

    render_frame();
    auto end_ticks = SDL_GetTicks64();
    auto frame_time = end_ticks - start_ticks;
    if (frame_time > 0 && frame_time < 17) [[likely]]
    {
      // 60 fps, common monitor refresh rate
      SDL_Delay((Uint32)(17 - frame_time));
    }
  }
#endif
}

static bool init_sdl() noexcept
{
  printf("Press the Q or Esc key to end the animation\n");
  printf("Press the F key for full screen\n");
#if SDL_MAJOR_VERSION > 1
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) [[unlikely]]
  {
    fprintf(stderr, "SDL could not be initialised: %s\n", SDL_GetError());
    return false;
  }
#else
  SDL_Init(SDL_INIT_VIDEO);
#endif

#if SDL_MAJOR_VERSION > 1
  window = SDL_CreateWindow(WINDOW_TITLE,
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (!window) [[unlikely]]
  {
    fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    return false;
  }

#ifdef __EMSCRIPTEN__
  // When testing on my computer the software renderer seems faster
  // in my web browser, even more so in full screen
  auto flags = SDL_RENDERER_SOFTWARE;
#else
  auto flags = SDL_RENDERER_ACCELERATED;
#endif
  renderer = SDL_CreateRenderer(window, -1, flags);
  if (!renderer) [[unlikely]]
  {
    fprintf(stderr, "Renderer could not be created: %s\n",
            SDL_GetError());
    return false;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING,
                              SCREEN_WIDTH, SCREEN_HEIGHT);
  if (!texture) [[unlikely]]
  {
    fprintf(stderr, "Texture could not be created: %s\n", SDL_GetError());
    return false;
  }

  format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
  if (!format) [[unlikely]]
  {
    fprintf(stderr, "Pixel format could not be created: %s\n", SDL_GetError());
    return false;
  }

#else
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
  SDL_WM_SetCaption(WINDOW_TITLE, nullptr);
#endif
  return true;
}

int main(int, char **)
{
  if (!init_sdl())
  {
    return 1;
  }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(game_loop, FRAME_RATE, SIMULATE_INFINITE_LOOP);
#else
  game_loop();
#endif
  end_sdl();
  return 0;
}
