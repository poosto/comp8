#pragma once

#include "parser.h"

#include <utility>

// TOOD: convert Seen pack to pre-computed back-edge table
// This avoids templated functions exploding with different Seen parameters
template <uint16_t PC, uint16_t... Seen>
static inline auto run(CHIP8 &ctx) -> void {
  if constexpr (PC >= GAME_ROM.size() || ((Seen == PC) || ...)) {
    ctx.pc = PC;
    return;
  } else {
    constexpr Opcode op = GAME_ROM[PC];
    constexpr auto exec = Parser<op>{}();

    exec(ctx);

    constexpr auto next = exec.next_pc();

    // Can't use std::visit because we need to use the constexpr next object
    // in the template parameter. The std::visit pattern gives us a lambda
    // argument, and arguments are never constexpr ofc
    if constexpr (std::holds_alternative<Increment>(next)) {
      run<PC + std::get<Increment>(next).by, Seen..., PC>(ctx);
    } else if constexpr (std::holds_alternative<Jump>(next)) {
      run<std::get<Jump>(next).target, Seen..., PC>(ctx);
    } else if constexpr (std::holds_alternative<Branch>(next)) {
      constexpr auto branch = std::get<Branch>(next);

      if (exec.condition(ctx)) {
        run<PC + branch.skip_by, Seen..., PC>(ctx);
      } else {
        run<PC + branch.fall_by, Seen..., PC>(ctx);
      }
    } else if constexpr (std::holds_alternative<DynamicJump>(next)) {
      ctx.pc = exec.target(ctx);
      return;
    }
  }
}

// Makes a runtime call to a templated (compile-time) function
// This is the bridge between compile-time chains of known instructions and
// runtime jumps
template <size_t... PCs>
static auto dispatch(CHIP8 &ctx, std::index_sequence<PCs...>) -> void {
  (
      [&] {
        if (ctx.pc == PCs) {
          run<PCs>(ctx);
        }
      }(),
      ...);
}

static inline auto execute_program(CHIP8 &ctx) -> void {
  ctx.pc = 0;
  while (ctx.pc < GAME_ROM.size()) {
    dispatch(ctx, std::make_index_sequence<GAME_ROM.size()>{});
  }
}
