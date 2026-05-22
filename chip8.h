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

static constexpr std::array<Opcode, 6> GAME_ROM{
    0x9010, // SNE V0, V1  — if V0 != 0 skip; else fall through (exit)
    0x1005, // JP 5        — exit to post-loop instruction
    0x7201, // ADD V2, 1   — V2++ (loop work)
    0x70FF, // ADD V0, 255 — V0-- (255 = -1 mod 256)
    0x1000, // JP 0        — back-edge
    0x7307, // ADD V3, 7   — post-loop, always runs once
};
