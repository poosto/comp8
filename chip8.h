#pragma once

#include <array>
#include <cstdint>
#include <variant>

struct CHIP8 {
  std::array<uint8_t, 16> vx{};
  uint16_t pc{};
};

using Opcode = uint16_t;

enum class Instruction {
  NOP,
  ADD_IMM,
  JUMP,
  SE_IMM,
  SNE_IMM,
  SE,
  LD,
  OR,
  AND,
  XOR,
  ADD,
  SUB,
  SHR,
  SUBN,
  SHL,
  LD_IMM,
  SNE,
  INDIRECT_JUMP
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
