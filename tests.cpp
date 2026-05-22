#include <catch2/catch_test_macros.hpp>

#include "runner.h"

// ── Executor<ADD_IMM> ────────────────────────────────────────────────────────

TEST_CASE("Executor<ADD_IMM>") {
  SECTION("adds kk to vx[x]") {
    CHIP8 ctx{};
    ctx.vx[2] = 10;
    Executor<Instruction::ADD_IMM>{2, 5}(ctx);
    REQUIRE(ctx.vx[2] == 15);
  }
  SECTION("wraps on overflow") {
    CHIP8 ctx{};
    ctx.vx[0] = 255;
    Executor<Instruction::ADD_IMM>{0, 1}(ctx);
    REQUIRE(ctx.vx[0] == 0);
  }
  SECTION("does not touch other registers") {
    CHIP8 ctx{};
    Executor<Instruction::ADD_IMM>{3, 7}(ctx);
    for (int i = 0; i < 16; ++i)
      if (i != 3)
        REQUIRE(ctx.vx[i] == 0);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::ADD_IMM>{0, 0}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<SUB> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<SUB>") {
  SECTION("subtracts vx[y] from vx[x]") {
    CHIP8 ctx{};
    ctx.vx[1] = 10;
    ctx.vx[2] = 3;
    Executor<Instruction::SUB>{1, 2}(ctx);
    REQUIRE(ctx.vx[1] == 7);
  }
  SECTION("wraps on underflow") {
    CHIP8 ctx{};
    ctx.vx[0] = 0;
    ctx.vx[1] = 1;
    Executor<Instruction::SUB>{0, 1}(ctx);
    REQUIRE(ctx.vx[0] == 255);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::SUB>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<JUMP> ───────────────────────────────────────────────────────────

TEST_CASE("Executor<JUMP>") {
  SECTION("next_pc is Jump with correct target") {
    auto next = Executor<Instruction::JUMP>{0x300}.next_pc();
    REQUIRE(std::holds_alternative<Jump>(next));
    REQUIRE(std::get<Jump>(next).target == 0x300);
  }
  SECTION("does not modify ctx") {
    CHIP8 ctx{};
    ctx.vx[0] = 42;
    Executor<Instruction::JUMP>{0x200}(ctx);
    REQUIRE(ctx.vx[0] == 42);
  }
}

// ── Executor<INDIRECT_JUMP> ──────────────────────────────────────────────────

TEST_CASE("Executor<INDIRECT_JUMP>") {
  SECTION("target is V0 + nnn") {
    CHIP8 ctx{};
    ctx.vx[0] = 4;
    REQUIRE(Executor<Instruction::INDIRECT_JUMP>{0x100}.target(ctx) == 0x104);
  }
  SECTION("target with V0=0") {
    CHIP8 ctx{};
    REQUIRE(Executor<Instruction::INDIRECT_JUMP>{0x050}.target(ctx) == 0x050);
  }
  SECTION("next_pc is DynamicJump") {
    auto next = Executor<Instruction::INDIRECT_JUMP>{0}.next_pc();
    REQUIRE(std::holds_alternative<DynamicJump>(next));
  }
}

// ── Executor<SE> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<SE>") {
  SECTION("condition true when vx[x] == vx[y]") {
    CHIP8 ctx{};
    ctx.vx[0] = 3;
    ctx.vx[1] = 3;
    REQUIRE(Executor<Instruction::SE>{0, 1}.condition(ctx) == true);
  }
  SECTION("condition false when vx[x] != vx[y]") {
    CHIP8 ctx{};
    ctx.vx[0] = 5;
    ctx.vx[1] = 3;
    REQUIRE(Executor<Instruction::SE>{0, 1}.condition(ctx) == false);
  }
  SECTION("condition true when both zero") {
    CHIP8 ctx{};
    REQUIRE(Executor<Instruction::SE>{0, 1}.condition(ctx) == true);
  }
  SECTION("next_pc is Branch{skip=2, fall=1}") {
    auto next = Executor<Instruction::SNE>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Branch>(next));
    auto b = std::get<Branch>(next);
    REQUIRE(b.skip_by == 2);
    REQUIRE(b.fall_by == 1);
  }
}

// ── Executor<SNE> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<SNE>") {
  SECTION("condition true when vx[x] != vx[y]") {
    CHIP8 ctx{};
    ctx.vx[0] = 1;
    ctx.vx[1] = 2;
    REQUIRE(Executor<Instruction::SNE>{0, 1}.condition(ctx) == true);
  }
  SECTION("condition false when vx[x] == vx[y]") {
    CHIP8 ctx{};
    ctx.vx[0] = 5;
    ctx.vx[1] = 5;
    REQUIRE(Executor<Instruction::SNE>{0, 1}.condition(ctx) == false);
  }
  SECTION("condition false when both zero") {
    CHIP8 ctx{};
    REQUIRE(Executor<Instruction::SNE>{0, 1}.condition(ctx) == false);
  }
  SECTION("next_pc is Branch{skip=2, fall=1}") {
    auto next = Executor<Instruction::SNE>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Branch>(next));
    auto b = std::get<Branch>(next);
    REQUIRE(b.skip_by == 2);
    REQUIRE(b.fall_by == 1);
  }
}

// ── Parser ───────────────────────────────────────────────────────────────────

TEST_CASE("Parser") {
  SECTION("0x7XKK decodes to ADD_IMM and executes correctly") {
    // 0x7201: ADD V2, 1
    constexpr auto exec = Parser<0x7201>{}();
    CHIP8 ctx{};
    ctx.vx[2] = 5;
    exec(ctx);
    REQUIRE(ctx.vx[2] == 6);
  }
  SECTION("0x9XY0 decodes to SNE with correct registers") {
    // 0x9010: SNE V0, V1
    constexpr auto exec = Parser<0x9010>{}();
    CHIP8 ctx{};
    ctx.vx[0] = 3;
    ctx.vx[1] = 0;
    REQUIRE(exec.condition(ctx) == true);
  }
  SECTION("0x1NNN decodes to JUMP with correct target") {
    // 0x1005: JP 5
    constexpr auto exec = Parser<0x1005>{}();
    auto next = exec.next_pc();
    REQUIRE(std::holds_alternative<Jump>(next));
    REQUIRE(std::get<Jump>(next).target == 5);
  }
  SECTION("0x70FF decodes to ADD_IMM V0, 255") {
    constexpr auto exec = Parser<0x70FF>{}();
    CHIP8 ctx{};
    ctx.vx[0] = 1;
    exec(ctx);
    REQUIRE(ctx.vx[0] == 0); // 1 + 255 = 256 = 0 (mod 256)
  }
  SECTION("0x7307 decodes to ADD_IMM V3, 7") {
    constexpr auto exec = Parser<0x7307>{}();
    CHIP8 ctx{};
    exec(ctx);
    REQUIRE(ctx.vx[3] == 7);
  }
}

// ── execute_program ──────────────────────────────────────────────────────────

TEST_CASE("execute_program") {
  SECTION("V0=0: skips loop, post-loop runs once") {
    CHIP8 ctx{};
    execute_program(ctx);
    REQUIRE(ctx.vx[0] == 0);
    REQUIRE(ctx.vx[2] == 0);
    REQUIRE(ctx.vx[3] == 7);
  }
  SECTION("V0=1: one iteration") {
    CHIP8 ctx{};
    ctx.vx[0] = 1;
    execute_program(ctx);
    REQUIRE(ctx.vx[0] == 0);
    REQUIRE(ctx.vx[2] == 1);
    REQUIRE(ctx.vx[3] == 7);
  }
  SECTION("V0=3: three iterations") {
    CHIP8 ctx{};
    ctx.vx[0] = 3;
    execute_program(ctx);
    REQUIRE(ctx.vx[0] == 0);
    REQUIRE(ctx.vx[2] == 3);
    REQUIRE(ctx.vx[3] == 7);
  }
  SECTION("unrelated registers are not modified") {
    CHIP8 ctx{};
    ctx.vx[0] = 2;
    execute_program(ctx);
    for (int i = 4; i < 16; ++i)
      REQUIRE(ctx.vx[i] == 0);
  }
}
