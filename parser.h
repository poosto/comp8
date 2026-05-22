#pragma once

#include "executor.h"

template <Opcode Op> struct Parser {
  constexpr auto operator()() {
    enum class FrontTag : uint8_t {
      CHIP8_COMMANDS = 0x0,
      JP = 0x1,
      CALL = 0x2,
      SE_IMM = 0x3,
      SNE_IMM = 0x4,
      SE = 0x5,
      LD_IMM = 0x6,
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

    if constexpr (front_tag == FrontTag::CHIP8_COMMANDS) {
      enum class BackTag : uint16_t { CLS = 0xE0, RET = 0xEE, SYS };

      constexpr BackTag back_tag = BackTag{Op & 0xFF};

      if constexpr (back_tag == BackTag::CLS) {
        return Executor<Instruction::CLS>{};
      } else if constexpr (back_tag == BackTag::RET) {
        return Executor<Instruction::RET>{};
      } else {
        // SYS instruction does not have a back tag as its format is 0nnn
        return Executor<Instruction::SYS>{nnn};
      }
    } else if constexpr (front_tag == FrontTag::JP) {
      return Executor<Instruction::JP>{nnn};
    } else if constexpr (front_tag == FrontTag::CALL) {
      return Executor<Instruction::CALL>{nnn};
    } else if constexpr (front_tag == FrontTag::SE_IMM) {
      return Executor<Instruction::SE_IMM>{x, kk};
    } else if constexpr (front_tag == FrontTag::SNE_IMM) {
      return Executor<Instruction::SNE_IMM>{x, kk};
    } else if constexpr (front_tag == FrontTag::SE) {
      return Executor<Instruction::SE>{x, y};
    } else if constexpr (front_tag == FrontTag::LD_IMM) {
      return Executor<Instruction::LD_IMM>{x, kk};
    } else if constexpr (front_tag == FrontTag::ADD_IMM) {
      return Executor<Instruction::ADD_IMM>{x, kk};
    } else if constexpr (front_tag == FrontTag::MATH) {
      enum class BackTag : uint8_t {
        LD = 0x0,
        OR = 0x1,
        AND = 0x2,
        XOR = 0x3,
        ADD = 0x4,
        SUB = 0x5,
        SHR = 0x6,
        SUBN = 0x7,
        SHL = 0xE
      };

      constexpr BackTag back_tag = BackTag{Op & 0xF};

      if constexpr (back_tag == BackTag::LD) {
        return Executor<Instruction::LD>{x, y};
      } else if constexpr (back_tag == BackTag::OR) {
        return Executor<Instruction::OR>{x, y};
      } else if constexpr (back_tag == BackTag::AND) {
        return Executor<Instruction::AND>{x, y};
      } else if constexpr (back_tag == BackTag::XOR) {
        return Executor<Instruction::XOR>{x, y};
      } else if constexpr (back_tag == BackTag::ADD) {
        return Executor<Instruction::ADD>{x, y};
      } else if constexpr (back_tag == BackTag::SUB) {
        return Executor<Instruction::SUB>{x, y};
      } else if constexpr (back_tag == BackTag::SHR) {
        return Executor<Instruction::SHR>{x, y};
      } else if constexpr (back_tag == BackTag::SUBN) {
        return Executor<Instruction::SUBN>{x, y};
      } else if constexpr (back_tag == BackTag::SHL) {
        return Executor<Instruction::SHL>{x, y};
      } else {
        static_assert(false, "Unhandled opcode");
        std::terminate();
      }
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
