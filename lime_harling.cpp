/* SDL 2 game loop that shows a colourful square that changes colour over time
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

#if SDL_MAJOR_VERSION > 1
static SDL_Window *window{};
static SDL_Renderer *renderer{};
#else
static SDL_Surface *screen{};
#endif
static auto shift{0};
static auto flip{false};
static SDL_Event event{};

#ifdef __EMSCRIPTEN__
static auto display_size_changed{false};
static auto fullscreen_changed{false};
static EmscriptenFullscreenStrategy strategy{};
#endif
static auto is_fullscreen{false};
static auto want_out{false};
static auto screen_width{SCREEN_WIDTH};
static auto screen_height{SCREEN_HEIGHT};

static void end_sdl() noexcept
{
#if SDL_MAJOR_VERSION > 1
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
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
  SDL_SetRenderDrawColor(renderer, r, g, b, ALPHA);
  SDL_RenderDrawPoint(renderer, x, y);
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
  render_loop(sdl2_render_loop_fn);
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
static void update_screen_size(int w, int h) noexcept
{
  SDL_SetWindowSize(window, w, h);
  screen_width = w;
  screen_height = h;
}
#endif

static bool poll_event_once() noexcept
{
#if SDL_MAJOR_VERSION > 1
  if (screen_width < 1 || screen_height < 1) [[unlikely]]
  {
    auto w{0};
    auto h{0};
    if (SDL_GetRendererOutputSize(renderer, &w, &h) == 0)
    {
      screen_width = w;
      screen_height = h;
    }
  }
#endif

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

#ifndef __EMSCRIPTEN__
  case SDL_WINDOWEVENT:
    if (event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
      update_screen_size(event.window.data1, event.window.data2);
    }
#endif

  case SDL_KEYDOWN:
    switch (event.key.keysym.sym)
    {
    [[unlikely]] case SDLK_ESCAPE:
      want_out = true;
      break;

    default:
      if (event.key.keysym.scancode == SDL_SCANCODE_F) [[unlikely]]
      {
        screen_width = SCREEN_WIDTH;
        screen_height = SCREEN_HEIGHT;
        if (!is_fullscreen)
        {
#ifdef __EMSCRIPTEN__
          strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
          strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
          strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
          strategy.canvasResizedCallback = on_canvas_resize;
          strategy.canvasResizedCallbackUserData = nullptr;
          emscripten_request_fullscreen_strategy("#canvas", false, &strategy);
#else
          SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
          is_fullscreen = true;
#endif
        }
        else
        {
          // Exit fullscreen
#ifdef __EMSCRIPTEN__
          emscripten_exit_fullscreen();
#else
          SDL_SetWindowFullscreen(window, 0);
          is_fullscreen = false;
#endif
        }

#if SDL_MAJOR_VERSION > 1
        SDL_SetWindowSize(window, screen_width, screen_height);
        SDL_RenderSetLogicalSize(renderer, screen_width, screen_height);
#endif
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
  if (fullscreen_changed)
  {
    if (!is_fullscreen)
    {
      // Enter fullscreen
#if SDL_MAJOR_VERSION > 1
      SDL_SetWindowSize(window, screen_width, screen_height);
#endif
    }
    else
    {
      // Exit fullscreen
      screen_width = SCREEN_WIDTH;
      screen_height = SCREEN_HEIGHT;
#if SDL_MAJOR_VERSION > 1
      SDL_SetWindowFullscreen(window, 0);
      SDL_SetWindowSize(window, screen_width, screen_height);
#else
      screen = SDL_SetVideoMode(screen_width, screen_height, 32,
                                SDL_SWSURFACE | SDL_RESIZABLE);
#endif
    }

    fullscreen_changed = false;
  }

  if (display_size_changed)
  {
    display_size_changed = false;
    fullscreen_changed = false;
    if (!is_fullscreen && (screen_width < 1 || screen_height < 1)) [[unlikely]]
    {
      auto w{0.0};
      auto h{0.0};
      emscripten_get_element_css_size("#canvas", &w, &h);
#if SDL_MAJOR_VERSION > 1
      SDL_SetWindowSize(window, (int)w, (int)h);
#endif
      screen_width = (int)w;
      screen_height = (int)h;
    }

#if SDL_MAJOR_VERSION > 1
    if (screen_width < 1 || screen_height < 1) [[unlikely]]
    {
      auto w{0};
      auto h{0};
      SDL_GetRendererOutputSize(renderer, &w, &h);
      screen_width = w;
      screen_height = h;
    }
#endif
  }

  poll_event_once();
  if (want_out) [[unlikely]]
  {
    end_sdl();
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
    if (frame_time > 0 && frame_time < 42) [[likely]]
    {
      // 24 fps
      SDL_Delay((Uint32)(42 - frame_time));
    }
  }
#endif
}

#ifdef __EMSCRIPTEN__
static EM_BOOL on_web_display_size_changed(int event_type,
                                           const EmscriptenUiEvent *status, void *user_data) noexcept
{
  if (event_type != EMSCRIPTEN_EVENT_RESIZE) [[unlikely]]
  {
    return false;
  }

  display_size_changed = 1;
  if (is_fullscreen)
  {
    return true;
  }

  auto w{0.0};
  auto h{0.0};
  emscripten_get_element_css_size("#canvas", &w, &h);
  if (w == screen_width && h == screen_height)
  {
    return true;
  }

#if SDL_MAJOR_VERSION > 1
  SDL_SetWindowSize(window, (int)w, (int)h);
#endif
  screen_width = (int)w;
  screen_height = (int)h;
  return true;
}

static EM_BOOL on_web_fullscreen_changed(int event_type,
                                         const EmscriptenFullscreenChangeEvent *status,
                                         void *userData) noexcept
{
  if (event_type != EMSCRIPTEN_EVENT_FULLSCREENCHANGE) [[unlikely]]
  {
    return false;
  }

  fullscreen_changed = 1;
  is_fullscreen = status->isFullscreen;
  if (is_fullscreen)
  {
    screen_width = status->screenWidth;
    screen_height = status->screenHeight;
  }
  else
  {
    screen_width = SCREEN_WIDTH;
    screen_height = SCREEN_HEIGHT;
  }

  return true;
}
#endif

static bool init_sdl() noexcept
{
  printf("Press the Esc key to end the animation\n");
  printf("Press the F key for full screen\n");
#if SDL_MAJOR_VERSION > 1
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
#else
  SDL_Init(SDL_INIT_VIDEO);
#endif

#if SDL_MAJOR_VERSION > 1
#ifdef __EMSCRIPTEN__
  window = SDL_CreateWindow(WINDOW_TITLE,
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (window == NULL) [[unlikely]]
  {
    fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    return false;
  }

  // When testing on my computer the software renderer seems faster
  // particularly when switching to fullscreen
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) [[unlikely]]
  {
    fprintf(stderr, "renderer could not be created: %s\n",
            SDL_GetError());
    return false;
  }
#else
  SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_RESIZABLE, &window, &renderer);
  if (window == NULL) [[unlikely]]
  {
    fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    return false;
  }
#endif

  SDL_SetWindowTitle(window, WINDOW_TITLE);
  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
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
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                 0, 0, on_web_display_size_changed);
  emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT,
                                           nullptr, false, on_web_fullscreen_changed);
  constexpr auto FRAME_RATE = 24; // frames per second
  constexpr auto SIMULATE_INFINITE_LOOP = 1;
  emscripten_set_main_loop(game_loop, FRAME_RATE, SIMULATE_INFINITE_LOOP);
#else
  game_loop();
  end_sdl();
#endif
  return 0;
}
