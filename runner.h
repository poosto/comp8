#pragma once

#include "parser.h"

#include <optional>
#include <utility>

// TODO: create a concrete type for PC, like using [name] = uint16_t

struct Node {
  enum class State {
    UNVISITED,
    PARTIALLY_VISITED,
    COMPLETED
  } state = State::UNVISITED;
};

using Successors = std::pair<std::optional<uint16_t>, std::optional<uint16_t>>;

// TODO: have these constants match the existing code, like skip_by and fall_by
consteval auto get_successors(uint16_t pc) -> Successors {
  uint16_t op = GAME_ROM[pc];
  uint16_t next = static_cast<uint16_t>(pc + 1);
  uint16_t skip = static_cast<uint16_t>(pc + 2);
  uint16_t addr = static_cast<uint16_t>(op & 0x0FFF);

  switch (op & 0xF000) {
  case 0x1000:
    return {addr, std::nullopt};
  case 0x2000:
    return {addr, std::nullopt};
  case 0xB000:
    return {std::nullopt, std::nullopt};
  case 0x0000:
    if (op == 0x00EE)
      return {std::nullopt, std::nullopt};
    return {next, std::nullopt};
  case 0x3000:
  case 0x4000:
  case 0x5000:
  case 0x9000:
    return {next, skip};
  default:
    return {next, std::nullopt};
  }
}

consteval auto dfs(uint16_t pc, std::array<Node, GAME_ROM.size()> &nodes,
                   std::array<bool, GAME_ROM.size()> &back_edges) noexcept
    -> void {

  nodes[pc].state = Node::State::PARTIALLY_VISITED;

  const auto [first, second] = get_successors(pc);

  for (const auto successor : {first, second}) {
    if (!successor.has_value()) {
      continue;
    }

    uint16_t target = successor.value();

    if (target >= GAME_ROM.size()) {
      continue;
    }

    if (nodes[target].state == Node::State::PARTIALLY_VISITED) {
      back_edges[target] = true;
    } else if (nodes[target].state == Node::State::UNVISITED) {
      dfs(target, nodes, back_edges);
    }
  }

  nodes[pc].state = Node::State::COMPLETED;
}

consteval auto compute_back_edges() -> std::array<bool, GAME_ROM.size()> {
  std::array<Node, GAME_ROM.size()> nodes{};
  std::array<bool, GAME_ROM.size()> back_edges{};

  // TODO: update to proper PC value (0x200)
  dfs(0x0, nodes, back_edges);

  return back_edges;
}

constexpr auto BACK_EDGES = compute_back_edges();
static_assert(BACK_EDGES[1] == true);

template <uint16_t PC> inline auto run(CHIP8 &ctx) -> void {
  if constexpr (PC >= GAME_ROM.size()) {
    ctx.pc = PC;
    return;
  } else {
    constexpr Opcode op = GAME_ROM[PC];
    constexpr auto exec = Parser<op>{}();

    exec(ctx);

    constexpr auto next = exec.next_pc();

    // Can't use std::visit because we need to use the constexpr next object
    // in the template parameter. The std::visit pattern gives us a lambda
    // argument, and arguments are never constexpr ofc.
    // Also, for back edges, we return the next PC to the dynamic dispatcher,
    // and for non-back edges we call the next function recursively to get
    // compiler inlining and other optimizations.
    if constexpr (std::holds_alternative<Increment>(next)) {
      constexpr auto target = PC + std::get<Increment>(next).by;
      if constexpr (BACK_EDGES[PC]) {
        ctx.pc = target;
        return;
      } else {
        run<target>(ctx);
      }
    } else if constexpr (std::holds_alternative<Jump>(next)) {
      constexpr auto target = std::get<Jump>(next).target;
      if constexpr (BACK_EDGES[PC]) {
        ctx.pc = target;
        return;
      } else {
        run<target>(ctx);
      }
    } else if constexpr (std::holds_alternative<Branch>(next)) {
      constexpr auto branch = std::get<Branch>(next);
      if (exec.condition(ctx)) {
        constexpr auto target = PC + branch.skip_by;
        if constexpr (BACK_EDGES[PC]) {
          ctx.pc = target;
          return;
        } else {
          run<target>(ctx);
        }
      } else {
        constexpr auto target = PC + branch.fall_by;
        if constexpr (BACK_EDGES[PC]) {
          ctx.pc = target;
          return;
        } else {
          run<target>(ctx);
        }
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
constexpr auto DISPATCH_TABLE =
    ([]<uint16_t... PCs>(std::integer_sequence<uint16_t, PCs...>) constexpr {
      return std::array{&run<PCs>...};
    })(std::make_integer_sequence<uint16_t, GAME_ROM.size()>{});

inline auto execute_program(CHIP8 &ctx) -> void {
  ctx.pc = 0;
  while (ctx.pc < GAME_ROM.size()) {
    DISPATCH_TABLE[ctx.pc](ctx);
  }
}
