/* SDL 2 game loop that shows a colourful square that changes colour over time
*/

#include <stdio.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "SDL.h"

constexpr auto SCREEN_WIDTH = 256;
constexpr auto SCREEN_HEIGHT = 256;

#if SDL_MAJOR_VERSION > 1
static SDL_Window* window{};
static SDL_Renderer* renderer{};
#else
static SDL_Surface* screen{};
#endif
static auto shift{ 0 };
static auto flip{ false };
static SDL_Event event{};

#ifdef __EMSCRIPTEN__
static auto display_size_changed{ false };
static auto fullscreen_changed{ false };
static EmscriptenFullscreenStrategy strategy{};
#endif
static auto is_fullscreen{ false };
static auto screen_width{ SCREEN_WIDTH };
static auto screen_height{ SCREEN_HEIGHT };

static void end_sdl() noexcept
{
#if SDL_MAJOR_VERSION > 1
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
#endif
  SDL_Quit();
  printf("SDL ended\n");
}

#ifdef __EMSCRIPTEN__
static EM_BOOL on_canvas_resize(int, const void*, void*)
{
  return false;
}
#endif

// Render a single frame, to be called from the main game loop function
static void render_frame() noexcept
{
  if (screen_height < 1 || screen_width < 1)
  {
    return;
  }

#if SDL_MAJOR_VERSION > 1
  SDL_RenderClear(renderer);
  for (auto vert = 0; vert < screen_height; vert++)
  {
    for (auto horiz = 0; horiz < screen_width; horiz++)
    {
      auto alpha = (Uint8)255;
      auto r = (Uint8)vert;
      auto g = (Uint8)horiz;
      auto b = (Uint8)(255 - vert);
      auto rot_r = (Uint8)(r + shift);
      auto rot_g = (Uint8)(g - shift * 3);
      auto rot_b = (Uint8)(b + shift * 2);
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

  for (auto i = 0; i < screen_height; i++)
  {
    for (auto j = 0; j < screen_width; j++)
    {
      auto alpha = (Uint8)255;
      auto r = (Uint8)i;
      auto g = (Uint8)j;
      auto b = (Uint8)(255 - i);
      auto rot_r = (Uint8)(r + shift);
      auto rot_g = (Uint8)(g - shift * 3);
      auto rot_b = (Uint8)(b + shift * 2);
      *((Uint32*)screen->pixels + i * screen_width + j) = SDL_MapRGBA(
        screen->format, rot_r, rot_g, rot_b, alpha);
    }
  }

  if (SDL_MUSTLOCK(screen))
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
    else {
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
    if (!is_fullscreen && (screen_width < 1 || screen_height < 1))
    {
      auto w{ 0.0 };
      auto h{ 0.0 };
      emscripten_get_element_css_size("#canvas", &w, &h);
      printf("GL canvas size change, w %f, h %f\n", w, h);
#if SDL_MAJOR_VERSION > 1
      SDL_SetWindowSize(window, (int)w, (int)h);
#endif
      screen_width = (int)w;
      screen_height = (int)h;
    }

#if SDL_MAJOR_VERSION > 1
    if (screen_width < 1 || screen_height < 1)
    {
      auto w{ 0 };
      auto h{ 0 };
      SDL_GetRendererOutputSize(renderer, &w, &h);
      screen_width = w;
      screen_height = h;
      printf("GL renderer size, w %d, h %d\n", w, h);
    }
#endif
  }

  render_frame();
  if (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT:
      printf("quit\n");
      end_sdl();
      emscripten_cancel_main_loop();
      break;

    case SDL_KEYDOWN:
      auto key_name = SDL_GetKeyName(event.key.keysym.sym);
      if (key_name && strcmp(key_name, "unknown key") != 0)
      {
        printf("%s\n", key_name);
      }

      switch (event.key.keysym.sym)
      {
      case SDLK_ESCAPE:
        end_sdl();
        emscripten_cancel_main_loop();
        break;

      default:
        if (event.key.keysym.scancode == SDL_SCANCODE_F)
        {
          if (!is_fullscreen)
          {
            strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
            strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
            strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            strategy.canvasResizedCallback = on_canvas_resize;
            strategy.canvasResizedCallbackUserData = nullptr;
            emscripten_request_fullscreen_strategy("#canvas", false, &strategy);
          }
          else
          {
            emscripten_exit_fullscreen();
          }
        }

        break;
      }

      break;
    }
  }
#else
  auto want_out{ false };
  while (!want_out)
  {
    auto start_ticks = SDL_GetTicks64();
    if (screen_width < 1 || screen_height < 1)
    {
      auto w{ 0 };
      auto h{ 0 };
      SDL_GetRendererOutputSize(renderer, &w, &h);
      screen_width = w;
      screen_height = h;
    }

    while (SDL_PollEvent(&event) && !want_out)
    {
      switch (event.type)
      {
      case SDL_QUIT:
        want_out = true;
        break;

      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          update_screen_size(event.window.data1, event.window.data2);
        }

        break;

      case SDL_KEYDOWN:
        auto key_name = SDL_GetKeyName(event.key.keysym.sym);
        if (key_name && strcmp(key_name, "unknown key") != 0)
        {
          printf("%s\n", key_name);
        }

        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
          want_out = true;
          break;

        default:
          if (event.key.keysym.scancode == SDL_SCANCODE_F)
          {
            screen_width = SCREEN_WIDTH;
            screen_height = SCREEN_HEIGHT;
            if (is_fullscreen)
            {
              // Exit fullscreen
              SDL_SetWindowFullscreen(window, 0);
              is_fullscreen = false;
            }
            else
            {
              SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
              is_fullscreen = true;
            }

            SDL_SetWindowSize(window, screen_width, screen_height);
            SDL_RenderSetLogicalSize(renderer, screen_width, screen_height);
          }

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

#ifdef __EMSCRIPTEN__
static EM_BOOL on_web_display_size_changed(int event_type,
  const EmscriptenUiEvent* status, void* user_data) noexcept
{
  if (event_type != EMSCRIPTEN_EVENT_RESIZE)
  {
    return false;
  }

  display_size_changed = 1;
  if (is_fullscreen)
  {
    return true;
  }

  auto w{ 0.0 };
  auto h{ 0.0 };
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
  const EmscriptenFullscreenChangeEvent* status,
  void* userData) noexcept
{
  if (event_type != EMSCRIPTEN_EVENT_FULLSCREENCHANGE)
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

int main(int, char**)
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
  window = SDL_CreateWindow("Lime harling",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (window == NULL)
  {
    fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer)
  {
    fprintf(stderr, "renderer could not be created: %s\n",
      SDL_GetError());
    return 1;
  }
#else
  SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
    SDL_WINDOW_RESIZABLE, &window, &renderer);
  if (window == NULL)
  {
    fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    return 1;
  }
#endif

  SDL_SetWindowTitle(window, "Lime harling");
  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
#else
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
#endif
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

