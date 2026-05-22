#include <catch2/catch_test_macros.hpp>

#include "runner.h"

// ── Executor<ADD_IMM> ────────────────────────────────────────────────────────

TEST_CASE("Executor<ADD_IMM>") {
  SECTION("adds kk to vx[x]") {
    CHIP8 ctx{};
    ctx.regfile[2] = 10;
    Executor<Instruction::ADD_IMM>{2, 5}(ctx);
    REQUIRE(ctx.regfile[2] == 15);
  }
  SECTION("wraps on overflow") {
    CHIP8 ctx{};
    ctx.regfile[0] = 255;
    Executor<Instruction::ADD_IMM>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 0);
  }
  SECTION("does not touch other registers") {
    CHIP8 ctx{};
    Executor<Instruction::ADD_IMM>{3, 7}(ctx);
    for (int i = 0; i < 16; ++i)
      if (i != 3)
        REQUIRE(ctx.regfile[i] == 0);
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
    ctx.regfile[1] = 10;
    ctx.regfile[2] = 3;
    Executor<Instruction::SUB>{1, 2}(ctx);
    REQUIRE(ctx.regfile[1] == 7);
    REQUIRE(ctx.regfile[0xF] == 1); // Vx > Vy: no borrow, VF = 1
  }
  SECTION("wraps on underflow") {
    CHIP8 ctx{};
    ctx.regfile[0] = 0;
    ctx.regfile[1] = 1;
    Executor<Instruction::SUB>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 255);
    REQUIRE(ctx.regfile[0xF] == 0); // Vx < Vy: borrow, VF = 0
  }
  SECTION("sets VF=0 when Vx equals Vy") {
    CHIP8 ctx{};
    ctx.regfile[0] = 5;
    ctx.regfile[1] = 5;
    Executor<Instruction::SUB>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 0);
    REQUIRE(ctx.regfile[0xF] == 0); // Vx == Vy: not greater, VF = 0
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::SUB>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<LD> ─────────────────────────────────────────────────────────────

TEST_CASE("Executor<LD>") {
  SECTION("copies vx[y] into vx[x]") {
    CHIP8 ctx{};
    ctx.regfile[2] = 42;
    Executor<Instruction::LD>{1, 2}(ctx);
    REQUIRE(ctx.regfile[1] == 42);
  }
  SECTION("does not modify vx[y]") {
    CHIP8 ctx{};
    ctx.regfile[3] = 99;
    Executor<Instruction::LD>{0, 3}(ctx);
    REQUIRE(ctx.regfile[3] == 99);
  }
  SECTION("overwrites existing value in vx[x]") {
    CHIP8 ctx{};
    ctx.regfile[0] = 100;
    ctx.regfile[1] = 7;
    Executor<Instruction::LD>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 7);
  }
  SECTION("does not touch other registers") {
    CHIP8 ctx{};
    ctx.regfile[5] = 7;
    Executor<Instruction::LD>{1, 5}(ctx);
    for (int i = 0; i < 16; ++i)
      if (i != 1 && i != 5)
        REQUIRE(ctx.regfile[i] == 0);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::LD>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<LD_IMM> ─────────────────────────────────────────────────────────

TEST_CASE("Executor<LD_IMM>") {
  SECTION("sets vx[x] to kk") {
    CHIP8 ctx{};
    Executor<Instruction::LD_IMM>{4, 0xAB}(ctx);
    REQUIRE(ctx.regfile[4] == 0xAB);
  }
  SECTION("overwrites existing value") {
    CHIP8 ctx{};
    ctx.regfile[0] = 100;
    Executor<Instruction::LD_IMM>{0, 55}(ctx);
    REQUIRE(ctx.regfile[0] == 55);
  }
  SECTION("does not touch other registers") {
    CHIP8 ctx{};
    Executor<Instruction::LD_IMM>{2, 1}(ctx);
    for (int i = 0; i < 16; ++i)
      if (i != 2)
        REQUIRE(ctx.regfile[i] == 0);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::LD_IMM>{0, 0}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<OR> ─────────────────────────────────────────────────────────────

TEST_CASE("Executor<OR>") {
  SECTION("Vx = Vx OR Vy") {
    CHIP8 ctx{};
    ctx.regfile[0] = 0b10101010;
    ctx.regfile[1] = 0b01010101;
    Executor<Instruction::OR>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 0xFF);
  }
  SECTION("OR with zero leaves Vx unchanged") {
    CHIP8 ctx{};
    ctx.regfile[2] = 0xAB;
    Executor<Instruction::OR>{2, 3}(ctx);
    REQUIRE(ctx.regfile[2] == 0xAB);
  }
  SECTION("does not modify Vy") {
    CHIP8 ctx{};
    ctx.regfile[1] = 0x0F;
    Executor<Instruction::OR>{0, 1}(ctx);
    REQUIRE(ctx.regfile[1] == 0x0F);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::OR>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<AND> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<AND>") {
  SECTION("Vx = Vx AND Vy") {
    CHIP8 ctx{};
    ctx.regfile[0] = 0b11001100;
    ctx.regfile[1] = 0b10101010;
    Executor<Instruction::AND>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 0b10001000);
  }
  SECTION("AND with 0xFF leaves Vx unchanged") {
    CHIP8 ctx{};
    ctx.regfile[2] = 0x5A;
    ctx.regfile[3] = 0xFF;
    Executor<Instruction::AND>{2, 3}(ctx);
    REQUIRE(ctx.regfile[2] == 0x5A);
  }
  SECTION("AND with zero clears Vx") {
    CHIP8 ctx{};
    ctx.regfile[0] = 0xFF;
    Executor<Instruction::AND>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 0);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::AND>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<XOR> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<XOR>") {
  SECTION("Vx = Vx XOR Vy") {
    CHIP8 ctx{};
    ctx.regfile[0] = 0b11001100;
    ctx.regfile[1] = 0b10101010;
    Executor<Instruction::XOR>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 0b01100110);
  }
  SECTION("XOR with same value clears Vx") {
    CHIP8 ctx{};
    ctx.regfile[2] = 0xAB;
    ctx.regfile[3] = 0xAB;
    Executor<Instruction::XOR>{2, 3}(ctx);
    REQUIRE(ctx.regfile[2] == 0);
  }
  SECTION("XOR with zero leaves Vx unchanged") {
    CHIP8 ctx{};
    ctx.regfile[3] = 0x3C;
    Executor<Instruction::XOR>{3, 4}(ctx);
    REQUIRE(ctx.regfile[3] == 0x3C);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::XOR>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<ADD> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<ADD>") {
  SECTION("adds vx[y] to vx[x] without carry") {
    CHIP8 ctx{};
    ctx.regfile[1] = 10;
    ctx.regfile[2] = 20;
    Executor<Instruction::ADD>{1, 2}(ctx);
    REQUIRE(ctx.regfile[1] == 30);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("wraps on overflow and sets VF=1") {
    CHIP8 ctx{};
    ctx.regfile[1] = 200;
    ctx.regfile[2] = 100;
    Executor<Instruction::ADD>{1, 2}(ctx);
    REQUIRE(ctx.regfile[1] == 44); // 300 - 256
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("sum exactly 255 does not carry") {
    CHIP8 ctx{};
    ctx.regfile[0] = 200;
    ctx.regfile[1] = 55;
    Executor<Instruction::ADD>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 255);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("sum exactly 256 carries") {
    CHIP8 ctx{};
    ctx.regfile[0] = 200;
    ctx.regfile[1] = 56;
    Executor<Instruction::ADD>{0, 1}(ctx);
    REQUIRE(ctx.regfile[0] == 0);
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::ADD>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<SHR> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<SHR>") {
  SECTION("shifts vx[x] right by one") {
    CHIP8 ctx{};
    ctx.regfile[2] = 0b00001110;
    Executor<Instruction::SHR>{2, 0}(ctx);
    REQUIRE(ctx.regfile[2] == 0b00000111);
  }
  SECTION("sets VF=1 when LSB is 1") {
    CHIP8 ctx{};
    ctx.regfile[1] = 0b00000011;
    Executor<Instruction::SHR>{1, 0}(ctx);
    REQUIRE(ctx.regfile[1] == 0b00000001);
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("sets VF=0 when LSB is 0") {
    CHIP8 ctx{};
    ctx.regfile[1] = 0b00001100;
    Executor<Instruction::SHR>{1, 0}(ctx);
    REQUIRE(ctx.regfile[1] == 0b00000110);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::SHR>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<SUBN> ───────────────────────────────────────────────────────────

TEST_CASE("Executor<SUBN>") {
  SECTION("stores Vy - Vx in Vx when Vy > Vx") {
    CHIP8 ctx{};
    ctx.regfile[1] = 3;
    ctx.regfile[2] = 10;
    Executor<Instruction::SUBN>{1, 2}(ctx);
    REQUIRE(ctx.regfile[1] == 7); // Vx = Vy - Vx = 10 - 3
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("sets VF=0 when Vy < Vx") {
    CHIP8 ctx{};
    ctx.regfile[1] = 10;
    ctx.regfile[2] = 3;
    Executor<Instruction::SUBN>{1, 2}(ctx);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("wraps on underflow") {
    CHIP8 ctx{};
    ctx.regfile[1] = 10; // Vx
    ctx.regfile[2] = 3;  // Vy: Vy - Vx = 3 - 10 wraps to 249
    Executor<Instruction::SUBN>{1, 2}(ctx);
    REQUIRE(ctx.regfile[1] == 249);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("sets VF=0 when Vy equals Vx") {
    CHIP8 ctx{};
    ctx.regfile[1] = 5;
    ctx.regfile[2] = 5;
    Executor<Instruction::SUBN>{1, 2}(ctx);
    REQUIRE(ctx.regfile[1] == 0);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::SUBN>{0, 1}.next_pc();
    REQUIRE(std::holds_alternative<Increment>(next));
    REQUIRE(std::get<Increment>(next).by == 1);
  }
}

// ── Executor<SHL> ────────────────────────────────────────────────────────────

TEST_CASE("Executor<SHL>") {
  SECTION("shifts vx[x] left by one") {
    CHIP8 ctx{};
    ctx.regfile[1] = 0b00000010;
    Executor<Instruction::SHL>{1, 0}(ctx);
    REQUIRE(ctx.regfile[1] == 0b00000100);
  }
  SECTION("sets VF=1 when MSB is 1") {
    CHIP8 ctx{};
    ctx.regfile[1] = 0b10000000;
    Executor<Instruction::SHL>{1, 0}(ctx);
    REQUIRE(ctx.regfile[1] == 0);
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("sets VF=0 when MSB is 0") {
    CHIP8 ctx{};
    ctx.regfile[1] = 0b00000001;
    Executor<Instruction::SHL>{1, 0}(ctx);
    REQUIRE(ctx.regfile[1] == 0b00000010);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("VF captures MSB before shift") {
    CHIP8 ctx{};
    ctx.regfile[2] = 0b11000000;
    Executor<Instruction::SHL>{2, 0}(ctx);
    REQUIRE(ctx.regfile[2] == 0b10000000); // low 8 bits of 0b110000000
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("next_pc is Increment{1}") {
    auto next = Executor<Instruction::SHL>{0, 1}.next_pc();
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
    ctx.regfile[0] = 42;
    Executor<Instruction::JUMP>{0x200}(ctx);
    REQUIRE(ctx.regfile[0] == 42);
  }
}

// ── Executor<INDIRECT_JUMP> ──────────────────────────────────────────────────

TEST_CASE("Executor<INDIRECT_JUMP>") {
  SECTION("target is V0 + nnn") {
    CHIP8 ctx{};
    ctx.regfile[0] = 4;
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
    ctx.regfile[0] = 3;
    ctx.regfile[1] = 3;
    REQUIRE(Executor<Instruction::SE>{0, 1}.condition(ctx) == true);
  }
  SECTION("condition false when vx[x] != vx[y]") {
    CHIP8 ctx{};
    ctx.regfile[0] = 5;
    ctx.regfile[1] = 3;
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
    ctx.regfile[0] = 1;
    ctx.regfile[1] = 2;
    REQUIRE(Executor<Instruction::SNE>{0, 1}.condition(ctx) == true);
  }
  SECTION("condition false when vx[x] == vx[y]") {
    CHIP8 ctx{};
    ctx.regfile[0] = 5;
    ctx.regfile[1] = 5;
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

// ── Executor<SE_IMM> ────────────────────────────────────────────────────────

TEST_CASE("Executor<SE_IMM>") {
  SECTION("condition true when vx[x] == kk") {
    CHIP8 ctx{};
    ctx.regfile[0] = 3;
    REQUIRE(Executor<Instruction::SE_IMM>{0, 3}.condition(ctx) == true);
  }
  SECTION("condition false when vx[x] != kk") {
    CHIP8 ctx{};
    ctx.regfile[0] = 5;
    REQUIRE(Executor<Instruction::SE_IMM>{0, 3}.condition(ctx) == false);
  }
  SECTION("condition true when both zero") {
    CHIP8 ctx{};
    REQUIRE(Executor<Instruction::SE_IMM>{0, 0}.condition(ctx) == true);
  }
  SECTION("next_pc is Branch{skip=2, fall=1}") {
    auto next = Executor<Instruction::SE_IMM>{0, 0}.next_pc();
    REQUIRE(std::holds_alternative<Branch>(next));
    auto b = std::get<Branch>(next);
    REQUIRE(b.skip_by == 2);
    REQUIRE(b.fall_by == 1);
  }
}

// ── Executor<SNE_IMM> ────────────────────────────────────────────────────────

TEST_CASE("Executor<SNE_IMM>") {
  SECTION("condition true when vx[x] != kk") {
    CHIP8 ctx{};
    ctx.regfile[0] = 1;
    REQUIRE(Executor<Instruction::SNE_IMM>{0, 2}.condition(ctx) == true);
  }
  SECTION("condition false when vx[x] == kk") {
    CHIP8 ctx{};
    ctx.regfile[0] = 5;
    REQUIRE(Executor<Instruction::SNE_IMM>{0, 5}.condition(ctx) == false);
  }
  SECTION("condition false when both zero") {
    CHIP8 ctx{};
    REQUIRE(Executor<Instruction::SNE_IMM>{0, 0}.condition(ctx) == false);
  }
  SECTION("next_pc is Branch{skip=2, fall=1}") {
    auto next = Executor<Instruction::SNE_IMM>{0, 0}.next_pc();
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
    ctx.regfile[2] = 5;
    exec(ctx);
    REQUIRE(ctx.regfile[2] == 6);
  }
  SECTION("0x9XY0 decodes to SNE with correct registers") {
    // 0x9010: SNE V0, V1
    constexpr auto exec = Parser<0x9010>{}();
    CHIP8 ctx{};
    ctx.regfile[0] = 3;
    ctx.regfile[1] = 0;
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
    ctx.regfile[0] = 1;
    exec(ctx);
    REQUIRE(ctx.regfile[0] == 0); // 1 + 255 = 256 = 0 (mod 256)
  }
  SECTION("0x7307 decodes to ADD_IMM V3, 7") {
    constexpr auto exec = Parser<0x7307>{}();
    CHIP8 ctx{};
    exec(ctx);
    REQUIRE(ctx.regfile[3] == 7);
  }
  SECTION("0x8120 decodes to LD V1, V2") {
    constexpr auto exec = Parser<0x8120>{}();
    CHIP8 ctx{};
    ctx.regfile[2] = 55;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 55);
  }
  SECTION("0x6145 decodes to LD_IMM V1, 0x45") {
    constexpr auto exec = Parser<0x6145>{}();
    CHIP8 ctx{};
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 0x45);
  }
  SECTION("0x8121 decodes to OR V1, V2") {
    constexpr auto exec = Parser<0x8121>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 0x0F;
    ctx.regfile[2] = 0xF0;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 0xFF);
  }
  SECTION("0x8122 decodes to AND V1, V2") {
    constexpr auto exec = Parser<0x8122>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 0xFF;
    ctx.regfile[2] = 0xAA;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 0xAA);
  }
  SECTION("0x8123 decodes to XOR V1, V2") {
    constexpr auto exec = Parser<0x8123>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 0xFF;
    ctx.regfile[2] = 0xFF;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 0);
  }
  SECTION("0x8124 decodes to ADD V1, V2 with carry") {
    constexpr auto exec = Parser<0x8124>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 200;
    ctx.regfile[2] = 100;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 44);
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("0x8125 decodes to SUB V1, V2 with VF") {
    constexpr auto exec = Parser<0x8125>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 10;
    ctx.regfile[2] = 3;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 7);
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("0x8126 decodes to SHR V1 with LSB in VF") {
    constexpr auto exec = Parser<0x8126>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 0b00000110;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 0b00000011);
    REQUIRE(ctx.regfile[0xF] == 0);
  }
  SECTION("0x8127 decodes to SUBN V1, V2") {
    constexpr auto exec = Parser<0x8127>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 3;
    ctx.regfile[2] = 10;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 7); // Vx = Vy - Vx = 10 - 3
    REQUIRE(ctx.regfile[0xF] == 1);
  }
  SECTION("0x812E decodes to SHL V1 with MSB in VF") {
    constexpr auto exec = Parser<0x812E>{}();
    CHIP8 ctx{};
    ctx.regfile[1] = 0b10000000;
    exec(ctx);
    REQUIRE(ctx.regfile[1] == 0);
    REQUIRE(ctx.regfile[0xF] == 1);
  }
}

// ── execute_program ──────────────────────────────────────────────────────────

TEST_CASE("execute_program") {
  SECTION("loop always runs exactly 5 times") {
    CHIP8 ctx{};
    ctx.regfile[0] = 1;
    execute_program(ctx);
    REQUIRE(ctx.regfile[0] == 1); // V0 (argc) unchanged
    REQUIRE(ctx.regfile[1] == 0); // loop counter exhausted
    REQUIRE(ctx.regfile[2] == 5); // 5 * 1
    REQUIRE(ctx.regfile[3] == 7);
  }
  SECTION("accumulates V0 five times") {
    CHIP8 ctx{};
    ctx.regfile[0] = 3;
    execute_program(ctx);
    REQUIRE(ctx.regfile[2] == 15); // 5 * 3
    REQUIRE(ctx.regfile[3] == 7);
  }
  SECTION("with V0=0 accumulator stays zero") {
    CHIP8 ctx{};
    execute_program(ctx);
    REQUIRE(ctx.regfile[1] == 0);
    REQUIRE(ctx.regfile[2] == 0);
    REQUIRE(ctx.regfile[3] == 7);
  }
  SECTION("accumulator wraps on overflow") {
    CHIP8 ctx{};
    ctx.regfile[0] = 52; // 5 * 52 = 260, wraps to 4
    execute_program(ctx);
    REQUIRE(ctx.regfile[2] == 4);
    REQUIRE(ctx.regfile[3] == 7);
  }
  SECTION("unrelated registers are not modified") {
    CHIP8 ctx{};
    execute_program(ctx); // V0=0: no ADD carry, VF stays 0
    for (int i = 4; i < 16; ++i)
      REQUIRE(ctx.regfile[i] == 0);
  }
}
