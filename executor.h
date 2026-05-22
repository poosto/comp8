#pragma once

#include <algorithm>
#include <exception>
#include <limits>

#include "chip8.h"

template <Instruction> struct Executor {
  Executor() = delete;

  auto operator()(CHIP8 &) const noexcept -> void {
    static_assert(false, "Unhandled instruction");
    std::terminate();
  }
};

template <Instruction I, bool> struct ConditionalExecutor;

template <typename Op, typename T>
concept BinaryOp = std::regular_invocable<Op, T, T> &&
                   std::same_as<std::invoke_result_t<Op, T, T>, T>;

template <BinaryOp<uint8_t>> struct ArithmeticExecutor;

// Treated as a NOP (for now..?)
template <> struct Executor<Instruction::SYS> {
  const uint16_t nnn_{};

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <> struct Executor<Instruction::CLS> {
  constexpr Executor() {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    std::ranges::fill(ctx.vram_flat, 0);
  }
};

template <> struct Executor<Instruction::RET> {
  constexpr Executor() {}

  constexpr auto next_pc() const noexcept -> NextPC { return DynamicJump{}; }

  auto target(const CHIP8 &ctx) const noexcept -> uint16_t {
    return ctx.stack[ctx.sp];
  }

  auto operator()(CHIP8 &ctx) const noexcept -> void { --ctx.sp; }
};

template <> struct Executor<Instruction::ADD_IMM> {
  const uint8_t x_;
  const uint8_t kk_;

  constexpr Executor(uint8_t x, uint8_t kk) : x_{x}, kk_{kk} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &vx = ctx.regfile[x_];

    if (vx + kk_ > std::numeric_limits<uint8_t>::max()) {
      vx += kk_ - (std::numeric_limits<uint8_t>::max() + 1);
    } else {
      vx += kk_;
    }
  }
};

template <> struct Executor<Instruction::JP> {
  const uint16_t nnn_;

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC {
    return Jump{.target = nnn_};
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <> struct Executor<Instruction::CALL> {
  const uint16_t nnn_;

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC {
    return Jump{.target = nnn_};
  }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    ctx.stack[ctx.sp++] = ctx.pc;
  }
};

template <> struct Executor<Instruction::INDIRECT_JUMP> {
  const uint16_t nnn_;

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC { return DynamicJump{}; }

  auto target(const CHIP8 &ctx) const noexcept -> uint16_t {
    return ctx.regfile[0] + nnn_;
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <bool Cond> struct ConditionalExecutor<Instruction::SE_IMM, Cond> {
  const uint8_t x_;
  const uint8_t kk_;

  constexpr ConditionalExecutor(uint8_t x, uint8_t kk) : x_{x}, kk_{kk} {}

  constexpr auto next_pc() const noexcept -> NextPC {
    return Branch{.skip_by = 2, .fall_by = 1};
  }

  auto condition(CHIP8 &ctx) const noexcept -> bool {
    if constexpr (Cond) {
      return ctx.regfile[x_] == kk_;
    } else {
      return ctx.regfile[x_] != kk_;
    }
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <>
struct Executor<Instruction::SE_IMM>
    : ConditionalExecutor<Instruction::SE_IMM, true> {
  using ConditionalExecutor<Instruction::SE_IMM, true>::ConditionalExecutor;
};

template <>
struct Executor<Instruction::SNE_IMM>
    : ConditionalExecutor<Instruction::SE_IMM, false> {
  using ConditionalExecutor<Instruction::SE_IMM, false>::ConditionalExecutor;
};

template <bool Cond> struct ConditionalExecutor<Instruction::SE, Cond> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr ConditionalExecutor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC {
    return Branch{.skip_by = 2, .fall_by = 1};
  }

  auto condition(CHIP8 &ctx) const noexcept -> bool {
    if constexpr (Cond) {
      return ctx.regfile[x_] == ctx.regfile[y_];
    } else {
      return ctx.regfile[x_] != ctx.regfile[y_];
    }
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <>
struct Executor<Instruction::SE> : ConditionalExecutor<Instruction::SE, true> {
  using ConditionalExecutor<Instruction::SE, true>::ConditionalExecutor;
};

template <>
struct Executor<Instruction::SNE>
    : ConditionalExecutor<Instruction::SE, false> {
  using ConditionalExecutor<Instruction::SE, false>::ConditionalExecutor;
};

template <> struct Executor<Instruction::LD> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    ctx.regfile[x_] = ctx.regfile[y_];
  }
};

template <> struct Executor<Instruction::LD_IMM> {
  const uint8_t x_;
  const uint8_t kk_;

  constexpr Executor(uint8_t x, uint8_t kk) : x_{x}, kk_{kk} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void { ctx.regfile[x_] = kk_; }
};

template <BinaryOp<uint8_t> Op> struct ArithmeticExecutor {
  const uint8_t x_;
  const uint8_t y_;

  constexpr ArithmeticExecutor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &vx = ctx.regfile[x_], &vy = ctx.regfile[y_];
    vx = Op{}(vx, vy);
  }
};

template <>
struct Executor<Instruction::OR> : ArithmeticExecutor<std::bit_or<uint8_t>> {
  using ArithmeticExecutor<std::bit_or<uint8_t>>::ArithmeticExecutor;
};

template <>
struct Executor<Instruction::AND> : ArithmeticExecutor<std::bit_and<uint8_t>> {
  using ArithmeticExecutor<std::bit_and<uint8_t>>::ArithmeticExecutor;
};

template <>
struct Executor<Instruction::XOR> : ArithmeticExecutor<std::bit_xor<uint8_t>> {
  using ArithmeticExecutor<std::bit_xor<uint8_t>>::ArithmeticExecutor;
};

template <> struct Executor<Instruction::ADD> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &vx = ctx.regfile[x_], &vy = ctx.regfile[y_], &vf = ctx.regfile[0xF];

    if (vx + vy > std::numeric_limits<uint8_t>::max()) {
      vx += vy - (std::numeric_limits<uint8_t>::max() + 1);
      vf = 1;
    } else {
      vx += vy;
      vf = 0;
    }
  }
};

template <> struct Executor<Instruction::SUB> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &vx = ctx.regfile[x_], &vy = ctx.regfile[y_], &vf = ctx.regfile[0xF];

    vf = (vx > vy) ? 1 : 0;

    if (vx < vy) {
      vx = (std::numeric_limits<uint8_t>::max() + 1) - (vy - vx);
    } else {
      vx -= vy;
    }
  }
};

template <> struct Executor<Instruction::SHR> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &vx = ctx.regfile[x_], &vf = ctx.regfile[0xF];

    vf = (vx & 0x1) ? 1 : 0;

    vx >>= 1;
  }
};

template <> struct Executor<Instruction::SUBN> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &vx = ctx.regfile[x_], &vy = ctx.regfile[y_], &vf = ctx.regfile[0xF];

    vf = (vy > vx) ? 1 : 0;

    if (vy < vx) {
      vx = (std::numeric_limits<uint8_t>::max() + 1) - (vx - vy);
    } else {
      vx = vy - vx;
    }
  }
};

template <> struct Executor<Instruction::SHL> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &vx = ctx.regfile[x_], &vf = ctx.regfile[0xF];

    vf = ((vx >> 7) & 0x1) ? 1 : 0;

    vx <<= 1;
  }
};
