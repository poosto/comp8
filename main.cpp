#include "runner.h"

#include <cstdio>
#include <string_view>

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

  chip8.vx[0] = (uint8_t)argc;

  print_arr("Context before: ", chip8.vx);
  execute_program(chip8);
  print_arr("Context after:  ", chip8.vx);

  return 0;
}
