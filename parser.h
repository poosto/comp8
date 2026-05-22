#pragma once

#include "executor.h"

template <Opcode Op> struct Parser {
  constexpr auto operator()() {
    enum class FrontTag {
      JUMP = 0x1,
      SE_IMM = 0x3,
      SNE_IMM = 0x4,
      SE = 0x5,
      ADD_IMM = 0x7,
      MATH = 0x8,
      SNE = 0x9,
      SET = 0xA,
      INDIRECT_JUMP = 0xB
    };

    constexpr uint8_t x = (Op >> 8) & 0xF;
    constexpr uint8_t y = (Op >> 4) & 0xF;
    constexpr uint8_t kk = Op & 0xFF;
    constexpr uint16_t nnn = Op & 0xFFF;

    constexpr FrontTag front_tag = FrontTag{(Op >> 12) & 0xF};

    if constexpr (front_tag == FrontTag::JUMP) {
      return Executor<Instruction::JUMP>{nnn};
    } else if constexpr (front_tag == FrontTag::SE_IMM) {
      return Executor<Instruction::SE_IMM>{x, kk};
    } else if constexpr (front_tag == FrontTag::SNE_IMM) {
      return Executor<Instruction::SNE_IMM>{x, kk};
    } else if constexpr (front_tag == FrontTag::SE) {
      return Executor<Instruction::SE>{x, y};
    } else if constexpr (front_tag == FrontTag::ADD_IMM) {
      return Executor<Instruction::ADD_IMM>{x, kk};
    } else if constexpr (front_tag == FrontTag::MATH) {
      // TODO: handle other cases based on back tag
      return Executor<Instruction::SUB>{x, y};
    } else if constexpr (front_tag == FrontTag::SNE) {
      return Executor<Instruction::SNE>{x, y};
    } else if constexpr (front_tag == FrontTag::INDIRECT_JUMP) {
      return Executor<Instruction::INDIRECT_JUMP>{nnn};
    } else {
      static_assert(false, "Unhandled opcode");
      std::terminate();
    }
  }
};
