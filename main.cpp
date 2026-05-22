#include "display.h"
#include "runner.h"

#include <cstdio>
#include <string_view>

// TODO: read thru the article about chip8 static recomp

template <typename T>
static inline auto print_arr(std::string_view prefix, T arr) -> void {
  printf("%.*s", (int)prefix.size(), prefix.data());

  for (size_t i = 0; i < arr.size(); ++i) {
    printf("%u ", arr[i]);
  }

  printf("\n");
}

auto main(int argc, char **) -> int {
  CHIP8 chip8{};

  chip8.regfile[0] = (uint8_t)argc;

  print_arr("Context before: ", chip8.regfile);
  execute_program(chip8);
  print_arr("Context after:  ", chip8.regfile);

  Display display{};
  while (!display.poll_quit()) {
    auto frame_start = SDL_GetTicks64();

    display.present();

    auto elapsed = SDL_GetTicks64() - frame_start;
    if (elapsed < 16)
      SDL_Delay(16 - elapsed);
  }

  return 0;
}
