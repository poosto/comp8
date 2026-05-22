#pragma once

#include <array>
#include <cstdint>
#include <mdspan>
#include <variant>

struct CHIP8 {
  static constexpr int VIDEO_WIDTH = 64;
  static constexpr int VIDEO_HEIGHT = 32;

  // TODO: use std::byte instead?
  std::array<uint8_t, VIDEO_WIDTH * VIDEO_HEIGHT> vram_flat{};
  std::mdspan<uint8_t, std::extents<size_t, VIDEO_WIDTH, VIDEO_HEIGHT>> vram{
      vram_flat.data()};

  std::array<uint8_t, 16> regfile{};
  uint16_t I{};

  uint16_t pc{};

  std::array<uint16_t, 16> stack{};
  uint8_t sp{};
};

using Opcode = uint16_t;

enum class Instruction {
  SYS,          // 0nnn
  CLS,          // 00E0
  RET,          // 00EE
  JP,           // 1nnn
  CALL,         // 2nnn
  SE_IMM,       // 3xkk
  SNE_IMM,      // 4xkk
  SE,           // 5xy0
  LD_IMM,       // 6xkk
  ADD_IMM,      // 7xkk
  LD,           // 8xy0
  OR,           // 8xy1
  AND,          // 8xy2
  XOR,          // 8xy3
  ADD,          // 8xy4
  SUB,          // 8xy5
  SHR,          // 8xy6
  SUBN,         // 8xy7
  SHL,          // 8xyE
  SNE,          // 9xy0
  LD_I,         // Annn
  INDIRECT_JUMP // Bnnn TODO: rename this enum
};

struct Increment {
  const uint16_t by{1};
};

struct Jump {
  const uint16_t target{};
};

struct Branch {
  const uint16_t skip_by{};
  const uint16_t fall_by{1};
};

// No data as the target is fully at runtime
struct DynamicJump {};

using NextPC = std::variant<Increment, Jump, Branch, DynamicJump>;

constexpr std::array<Opcode, 7> GAME_ROM{
    0x6105, // LD_IMM V1, 5      — V1 = 5 (loop counter)
    0x4100, // SNE_IMM V1, 0     — if V1 != 0 skip; else fall through
    0x1006, // JP 6              — exit loop
    0x8204, // ADD V2, V0        — V2 += V0 (accumulate argc)
    0x71FF, // ADD_IMM V1, 0xFF  — V1--
    0x1001, // JP 1              — loop back to condition
    0x7307, // ADD_IMM V3, 7     — post-loop
};
