#pragma once

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

template <> struct Executor<Instruction::ADD_IMM> {
  const uint8_t x_;
  const uint8_t kk_;

  constexpr Executor(uint8_t x, uint8_t kk) : x_{x}, kk_{kk} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &x = ctx.vx[x_];

    if (x + kk_ > std::numeric_limits<uint8_t>::max()) {
      x += kk_ - (std::numeric_limits<uint8_t>::max() + 1);
    } else {
      x += kk_;
    }
  }
};

template <> struct Executor<Instruction::JUMP> {
  const uint16_t nnn_;

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC {
    return Jump{.target = nnn_};
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <> struct Executor<Instruction::INDIRECT_JUMP> {
  const uint16_t nnn_;

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC { return DynamicJump{}; }

  auto target(const CHIP8 &ctx) const noexcept -> uint16_t {
    return ctx.vx[0] + nnn_;
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
      return ctx.vx[x_] == kk_;
    } else {
      return ctx.vx[x_] != kk_;
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
      return ctx.vx[x_] == ctx.vx[y_];
    } else {
      return ctx.vx[x_] != ctx.vx[y_];
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
    ctx.vx[x_] = ctx.vx[y_];
  }
};

template <> struct Executor<Instruction::LD_IMM> {
  const uint8_t x_;
  const uint8_t kk_;

  constexpr Executor(uint8_t x, uint8_t kk) : x_{x}, kk_{kk} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void { ctx.vx[x_] = kk_; }
};

template <BinaryOp<uint8_t> Op> struct ArithmeticExecutor {
  const uint8_t x_;
  const uint8_t y_;

  constexpr ArithmeticExecutor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &x = ctx.vx[x_], &y = ctx.vx[y_];
    x = Op{}(x, y);
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
    auto &x = ctx.vx[x_], &y = ctx.vx[y_], &vf = ctx.vx[0xF];

    if (x + y > std::numeric_limits<uint8_t>::max()) {
      x += y - (std::numeric_limits<uint8_t>::max() + 1);
      vf = 1;
    } else {
      x += y;
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
    auto &x = ctx.vx[x_], &y = ctx.vx[y_], &vf = ctx.vx[0xF];

    vf = (x > y) ? 1 : 0;

    if (x < y) {
      x = (std::numeric_limits<uint8_t>::max() + 1) - (y - x);
    } else {
      x -= y;
    }
  }
};
