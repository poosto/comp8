#pragma once

#include <exception>
#include <limits>

#include "chip8.h"

template <Instruction I, bool Cond = true> struct Executor {
  auto operator()(CHIP8 &) const noexcept -> void {
    static_assert(false, "Unhandled instruction");
    std::terminate();
  }
};

template <> struct Executor<Instruction::ADD_IMM, true> {
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

template <> struct Executor<Instruction::SUB, true> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

  constexpr auto next_pc() const noexcept -> NextPC { return Increment{}; }

  auto operator()(CHIP8 &ctx) const noexcept -> void {
    auto &x = ctx.vx[x_], &y = ctx.vx[y_];

    if (x < y) {
      x = (std::numeric_limits<uint8_t>::max() + 1) - (y - x);
    } else {
      x -= y;
    }
  }
};

template <> struct Executor<Instruction::JUMP, true> {
  const uint16_t nnn_;

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC {
    return Jump{.target = nnn_};
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <> struct Executor<Instruction::INDIRECT_JUMP, true> {
  const uint16_t nnn_;

  constexpr Executor(uint16_t nnn) : nnn_{nnn} {}

  constexpr auto next_pc() const noexcept -> NextPC { return DynamicJump{}; }

  auto target(const CHIP8 &ctx) const noexcept -> uint16_t {
    return ctx.vx[0] + nnn_;
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <bool Cond> struct Executor<Instruction::SE_IMM, Cond> {
  const uint8_t x_;
  const uint8_t kk_;

  constexpr Executor(uint8_t x, uint8_t kk) : x_{x}, kk_{kk} {}

  constexpr auto next_pc() const noexcept -> NextPC {
    return Branch{.skip_by = 2, .fall_by = 1};
  }

  auto condition(CHIP8 &ctx) const noexcept -> bool {
    if constexpr (Cond) {
      return ctx.vx[x_] == ctx.vx[kk_];
    } else {
      return ctx.vx[x_] != ctx.vx[kk_];
    }
  }

  auto operator()(CHIP8 &) const noexcept -> void {}
};

template <>
struct Executor<Instruction::SNE_IMM> : Executor<Instruction::SE_IMM, false> {
  using Executor<Instruction::SE_IMM, false>::Executor;
};

template <bool Cond> struct Executor<Instruction::SE, Cond> {
  const uint8_t x_;
  const uint8_t y_;

  constexpr Executor(uint8_t x, uint8_t y) : x_{x}, y_{y} {}

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
struct Executor<Instruction::SNE> : Executor<Instruction::SE, false> {
  using Executor<Instruction::SE, false>::Executor;
};
