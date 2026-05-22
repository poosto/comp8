#pragma once

#include "chip8.h"
#include <SDL.h>

#include <array>
#include <stdexcept>

// TODO: change these into perameters as the display should have no knowledge of
// the CHIP8 system
static constexpr int DISPLAY_W = CHIP8::VIDEO_WIDTH;
static constexpr int DISPLAY_H = CHIP8::VIDEO_HEIGHT;
static constexpr int DISPLAY_SCALE = 10;

struct SDLContext {
  SDLContext() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
      throw std::runtime_error(SDL_GetError());
  }
  ~SDLContext() { SDL_Quit(); }

  SDLContext(const SDLContext &) = delete;
  SDLContext &operator=(const SDLContext &) = delete;
};

struct SDLWindow {
  SDL_Window *ptr;

  SDLWindow(const char *title, int w, int h)
      : ptr{SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, w, h, 0)} {
    if (!ptr)
      throw std::runtime_error(SDL_GetError());
  }
  ~SDLWindow() { SDL_DestroyWindow(ptr); }

  SDLWindow(const SDLWindow &) = delete;
  SDLWindow &operator=(const SDLWindow &) = delete;
};

struct SDLRenderer {
  SDL_Renderer *ptr;

  explicit SDLRenderer(SDL_Window *window)
      : ptr{SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)} {
    if (!ptr)
      throw std::runtime_error(SDL_GetError());
  }
  ~SDLRenderer() { SDL_DestroyRenderer(ptr); }

  SDLRenderer(const SDLRenderer &) = delete;
  SDLRenderer &operator=(const SDLRenderer &) = delete;
};

struct SDLTexture {
  SDL_Texture *ptr;

  SDLTexture(SDL_Renderer *renderer, int w, int h)
      : ptr{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING, w, h)} {
    if (!ptr)
      throw std::runtime_error(SDL_GetError());
  }
  ~SDLTexture() { SDL_DestroyTexture(ptr); }

  SDLTexture(const SDLTexture &) = delete;
  SDLTexture &operator=(const SDLTexture &) = delete;
};

struct Display {
  std::array<uint8_t, DISPLAY_W * DISPLAY_H> pixels{};

  Display()
      : ctx_{},
        window_{"CHIP-8", DISPLAY_W * DISPLAY_SCALE, DISPLAY_H * DISPLAY_SCALE},
        renderer_{window_.ptr}, texture_{renderer_.ptr, DISPLAY_W, DISPLAY_H} {}

  Display(const Display &) = delete;
  Display &operator=(const Display &) = delete;

  auto clear() -> void { pixels.fill(0); }

  auto present() -> void {
    void *raw;
    int pitch;
    SDL_LockTexture(texture_.ptr, nullptr, &raw, &pitch);

    auto *buf = static_cast<uint32_t *>(raw);
    for (int i = 0; i < DISPLAY_W * DISPLAY_H; ++i)
      buf[i] = pixels[i] ? 0xFFFFFFFF : 0x000000FF; // RGBA: white or black

    SDL_UnlockTexture(texture_.ptr);
    SDL_RenderClear(renderer_.ptr);
    SDL_RenderCopy(renderer_.ptr, texture_.ptr, nullptr, nullptr);
    SDL_RenderPresent(renderer_.ptr);
  }

  // Returns true if the window close button was pressed.
  auto poll_quit() -> bool {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_QUIT)
        return true;
    return false;
  }

private:
  SDLContext ctx_;
  SDLWindow window_;
  SDLRenderer renderer_;
  SDLTexture texture_;
};
