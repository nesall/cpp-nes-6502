#pragma once
// Copyright 2026, Arman Sahakyan
// A C++ framework for NES game development that generates readable, memory-safe 6502 assembly.

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <string>
#include <filesystem>
#include <vector>
#include <array>
#include <variant>

namespace cppnes {

  class Toolchain {
    std::string ca65path_;
    std::string ld65path_;
  public:
    [[nodiscard]] bool isValid();
    void setCa65(std::string_view path);
    void setLd65(std::string_view path);
    std::string ca65Path() const { return ca65path_; }
    std::string ld65Path() const { return ld65path_; }
    void compile(const std::filesystem::path &asmFile, const std::filesystem::path &objFile);
    void link(const std::filesystem::path &cfgFile, const std::filesystem::path &objFile, std::string_view outputPath);
  };

  // A label resolves to an address later.
  class Label {
    std::string name_;
  public:
    explicit Label(std::string_view name) : name_(name) {}
    std::string_view name() const { return name_; }
  };

  class ZpAddress {
    uint8_t value_;
    std::string name_;
  public:
    explicit ZpAddress(uint8_t value, std::string_view name = "") : value_(value), name_(name) {}
    [[nodiscard]]
    static ZpAddress fromValue(uint32_t value, std::string_view name = "") {
      if (0xFF < value) throw std::range_error("Zero page address must be between 0x0000 and 0x00FF");
      return ZpAddress{ static_cast<uint8_t>(value), name };
    }
    [[nodiscard]] uint8_t value() const { return value_; }
    [[nodiscard]] std::string_view name() const { return name_; }
  };

  class AbsAddress {
    uint16_t value_;
    std::string name_;
  public:
    explicit AbsAddress(uint16_t value, std::string_view name = "") : value_(value), name_(name) {
    }
    [[nodiscard]]
    static AbsAddress fromValue(uint32_t value, std::string_view name = "") {
      if (0xFFFF < value) throw std::runtime_error("Absolute address must be between 0x0000 and 0xFFFF");
      return AbsAddress{ static_cast<uint16_t>(value), name };
    }
    [[nodiscard]] uint16_t value() const { return value_; }
    [[nodiscard]] std::string_view name() const { return name_; }
  };

  class ZeroPageAllocator {
    uint16_t next_ = Min;
  public:
    [[nodiscard]] ZpAddress alloc(std::string_view name);
    static constexpr uint16_t Min = 0x0000;
    static constexpr uint16_t Max = 0x00FF;
  };

  class RamAllocator {
    uint16_t next_ = Min;
  public:
    [[nodiscard]] AbsAddress alloc(std::string_view name);
    [[nodiscard]] AbsAddress allocBlock(std::string_view name, uint16_t size, uint16_t baseAddress = 0);
    static constexpr uint16_t Min = 0x0200;
    static constexpr uint16_t Max = 0x07FF;
  };

  class MemoryMap {
  public:
    ZeroPageAllocator zeroPage;
    RamAllocator ram;
  };

  struct Immediate { uint8_t value; };
  struct Implied {}; // no operand
  struct Accumulator {}; // accumulator ops

  struct ZeroPage { ZpAddress addr; };
  struct ZeroPageX { ZpAddress addr; };
  struct ZeroPageY { ZpAddress addr; };

  struct Absolute { 
    AbsAddress addr;
    static Absolute val(uint32_t a) { return { AbsAddress::fromValue(a) }; }
  };
  struct AbsoluteX { AbsAddress addr; };
  struct AbsoluteY { AbsAddress addr; };

  struct Indirect { AbsAddress addr; };  // JMP only

  struct IndexedIndirectX { ZpAddress addr; }; // (zp,X)
  struct IndexedIndirectY { ZpAddress addr; }; // (zp),Y

  class Program;

  enum class Opcode { 
    LDA, STA, LDX, STX, LDY, STY, ADC, SBC, ASL, LSR,
    ROL, ROR, BIT, AND, ORA, EOR, CMP, CPX, CPY, JMP,
    JSR, RTS, BCC, BCS, INX, INY, DEX, DEY, INC, DEC,
    BEQ, BMI, BNE, BPL, BVC, BVS, BRK, PHP, PLP, PHA,
    PLA, CLC, SEC, CLI, SEI, CLV, CLD, SED, RTI, TAX,
    TXA, TAY, TYA, TSX, TXS, NOP
  };

  // The operand can be any addressing mode or label
  using Operand = std::variant<
    std::monostate,     // for implied
    Immediate,
    ZeroPage,
    ZeroPageX,
    ZeroPageY,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    IndexedIndirectX,
    IndexedIndirectY,
    Indirect,
    Label,
    Accumulator
  >;

  struct Instruction {
    Opcode opcode;
    Operand operand;
  };

  struct LabelDef { Label label; };  // label definition/placement
  using Entry = std::variant<Instruction, LabelDef>;


  class Subroutine {
  public:
    // LDA
    Subroutine &lda(Immediate i) { return emitInst(Opcode::LDA, i); }
    Subroutine &lda(ZeroPage i) { return emitInst(Opcode::LDA, i); }
    Subroutine &lda(ZeroPageX i) { return emitInst(Opcode::LDA, i); }
    Subroutine &lda(Absolute i) { return emitInst(Opcode::LDA, i); }
    Subroutine &lda(AbsoluteX i) { return emitInst(Opcode::LDA, i); }
    Subroutine &lda(AbsoluteY i) { return emitInst(Opcode::LDA, i); }
    Subroutine &lda(IndexedIndirectX i) { return emitInst(Opcode::LDA, i); }
    Subroutine &lda(IndexedIndirectY i) { return emitInst(Opcode::LDA, i); }

    // STA
    Subroutine &sta(ZeroPage i) { return emitInst(Opcode::STA, i); }
    Subroutine &sta(ZeroPageX i) { return emitInst(Opcode::STA, i); }
    Subroutine &sta(Absolute i) { return emitInst(Opcode::STA, i); }
    Subroutine &sta(AbsoluteX i) { return emitInst(Opcode::STA, i); }
    Subroutine &sta(AbsoluteY i) { return emitInst(Opcode::STA, i); }
    Subroutine &sta(IndexedIndirectX i) { return emitInst(Opcode::STA, i); }
    Subroutine &sta(IndexedIndirectY i) { return emitInst(Opcode::STA, i); }

    // LDX (load X)
    Subroutine &ldx(Immediate i) { return emitInst(Opcode::LDX, i); }
    Subroutine &ldx(ZeroPage i) { return emitInst(Opcode::LDX, i); }
    Subroutine &ldx(ZeroPageY i) { return emitInst(Opcode::LDX, i); }
    Subroutine &ldx(Absolute i) { return emitInst(Opcode::LDX, i); }
    Subroutine &ldx(AbsoluteY i) { return emitInst(Opcode::LDX, i); }

    // STX (store X)
    Subroutine &stx(ZeroPage i) { return emitInst(Opcode::STX, i); }
    Subroutine &stx(ZeroPageY i) { return emitInst(Opcode::STX, i); }
    Subroutine &stx(Absolute i) { return emitInst(Opcode::STX, i); }

    // LDY (load Y)
    Subroutine &ldy(Immediate i) { return emitInst(Opcode::LDY, i); }
    Subroutine &ldy(ZeroPage i) { return emitInst(Opcode::LDY, i); }
    Subroutine &ldy(ZeroPageX i) { return emitInst(Opcode::LDY, i); }
    Subroutine &ldy(Absolute i) { return emitInst(Opcode::LDY, i); }
    Subroutine &ldy(AbsoluteX i) { return emitInst(Opcode::LDY, i); }

    // STY (store Y)
    Subroutine &sty(ZeroPage i) { return emitInst(Opcode::STY, i); }
    Subroutine &sty(ZeroPageX i) { return emitInst(Opcode::STY, i); }
    Subroutine &sty(Absolute i) { return emitInst(Opcode::STY, i); }

    // Arithmetic
    Subroutine &adc(Immediate i) { return emitInst(Opcode::ADC, i); }
    Subroutine &adc(ZeroPage i) { return emitInst(Opcode::ADC, i); }
    Subroutine &adc(ZeroPageX i) { return emitInst(Opcode::ADC, i); }
    Subroutine &adc(Absolute i) { return emitInst(Opcode::ADC, i); }
    Subroutine &adc(AbsoluteX i) { return emitInst(Opcode::ADC, i); }
    Subroutine &adc(AbsoluteY i) { return emitInst(Opcode::ADC, i); }
    Subroutine &adc(IndexedIndirectX i) { return emitInst(Opcode::ADC, i); }
    Subroutine &adc(IndexedIndirectY i) { return emitInst(Opcode::ADC, i); }

    Subroutine &sbc(Immediate i) { return emitInst(Opcode::SBC, i); }
    Subroutine &sbc(ZeroPage i) { return emitInst(Opcode::SBC, i); }
    Subroutine &sbc(ZeroPageX i) { return emitInst(Opcode::SBC, i); }
    Subroutine &sbc(Absolute i) { return emitInst(Opcode::SBC, i); }
    Subroutine &sbc(AbsoluteX i) { return emitInst(Opcode::SBC, i); }
    Subroutine &sbc(AbsoluteY i) { return emitInst(Opcode::SBC, i); }
    Subroutine &sbc(IndexedIndirectX i) { return emitInst(Opcode::SBC, i); }
    Subroutine &sbc(IndexedIndirectY i) { return emitInst(Opcode::SBC, i); }

    // Increment / Decrement
    Subroutine &inx() { return emitInst(Opcode::INX); }
    Subroutine &iny() { return emitInst(Opcode::INY); }
    Subroutine &dex() { return emitInst(Opcode::DEX); }
    Subroutine &dey() { return emitInst(Opcode::DEY); }
    Subroutine &inc(ZeroPage i) { return emitInst(Opcode::INC, i); }
    Subroutine &inc(ZeroPageX i) { return emitInst(Opcode::INC, i); }
    Subroutine &inc(Absolute i) { return emitInst(Opcode::INC, i); }
    Subroutine &inc(AbsoluteX i) { return emitInst(Opcode::INC, i); }
    Subroutine &dec(ZeroPage i) { return emitInst(Opcode::DEC, i); }
    Subroutine &dec(ZeroPageX i) { return emitInst(Opcode::DEC, i); }
    Subroutine &dec(Absolute i) { return emitInst(Opcode::DEC, i); }
    Subroutine &dec(AbsoluteX i) { return emitInst(Opcode::DEC, i); }

    // Shift / rotate
    Subroutine &asl(Accumulator i) { return emitInst(Opcode::ASL, i); }
    Subroutine &asl(ZeroPage i) { return emitInst(Opcode::ASL, i); }
    Subroutine &asl(ZeroPageX i) { return emitInst(Opcode::ASL, i); }
    Subroutine &asl(Absolute i) { return emitInst(Opcode::ASL, i); }
    Subroutine &asl(AbsoluteX i) { return emitInst(Opcode::ASL, i); }
    Subroutine &lsr(Accumulator i) { return emitInst(Opcode::LSR, i); }
    Subroutine &lsr(ZeroPage i) { return emitInst(Opcode::LSR, i); }
    Subroutine &lsr(ZeroPageX i) { return emitInst(Opcode::LSR, i); }
    Subroutine &lsr(Absolute i) { return emitInst(Opcode::LSR, i); }
    Subroutine &lsr(AbsoluteX i) { return emitInst(Opcode::LSR, i); }
    Subroutine &rol(Accumulator i) { return emitInst(Opcode::ROL, i); }
    Subroutine &rol(ZeroPage i) { return emitInst(Opcode::ROL, i); }
    Subroutine &rol(ZeroPageX i) { return emitInst(Opcode::ROL, i); }
    Subroutine &rol(Absolute i) { return emitInst(Opcode::ROL, i); }
    Subroutine &rol(AbsoluteX i) { return emitInst(Opcode::ROL, i); }
    Subroutine &ror(Accumulator i) { return emitInst(Opcode::ROR, i); }
    Subroutine &ror(ZeroPage i) { return emitInst(Opcode::ROR, i); }
    Subroutine &ror(ZeroPageX i) { return emitInst(Opcode::ROR, i); }
    Subroutine &ror(Absolute i) { return emitInst(Opcode::ROR, i); }
    Subroutine &ror(AbsoluteX i) { return emitInst(Opcode::ROR, i); }

    // Bitwise/Logic
    Subroutine &and_(Immediate i) { return emitInst(Opcode::AND, i); }
    Subroutine &and_(ZeroPage i) { return emitInst(Opcode::AND, i); }
    Subroutine &and_(ZeroPageX i) { return emitInst(Opcode::AND, i); }
    Subroutine &and_(Absolute i) { return emitInst(Opcode::AND, i); }
    Subroutine &and_(AbsoluteX i) { return emitInst(Opcode::AND, i); }
    Subroutine &and_(AbsoluteY i) { return emitInst(Opcode::AND, i); }
    Subroutine &and_(IndexedIndirectX i) { return emitInst(Opcode::AND, i); }
    Subroutine &and_(IndexedIndirectY i) { return emitInst(Opcode::AND, i); }

    Subroutine &ora(Immediate i) { return emitInst(Opcode::ORA, i); }
    Subroutine &ora(ZeroPage i) { return emitInst(Opcode::ORA, i); }
    Subroutine &ora(ZeroPageX i) { return emitInst(Opcode::ORA, i); }
    Subroutine &ora(Absolute i) { return emitInst(Opcode::ORA, i); }
    Subroutine &ora(AbsoluteX i) { return emitInst(Opcode::ORA, i); }
    Subroutine &ora(AbsoluteY i) { return emitInst(Opcode::ORA, i); }
    Subroutine &ora(IndexedIndirectX i) { return emitInst(Opcode::ORA, i); }
    Subroutine &ora(IndexedIndirectY i) { return emitInst(Opcode::ORA, i); }

    Subroutine &eor(Immediate i) { return emitInst(Opcode::EOR, i); }
    Subroutine &eor(ZeroPage i) { return emitInst(Opcode::EOR, i); }
    Subroutine &eor(ZeroPageX i) { return emitInst(Opcode::EOR, i); }
    Subroutine &eor(Absolute i) { return emitInst(Opcode::EOR, i); }
    Subroutine &eor(AbsoluteX i) { return emitInst(Opcode::EOR, i); }
    Subroutine &eor(AbsoluteY i) { return emitInst(Opcode::EOR, i); }
    Subroutine &eor(IndexedIndirectX i) { return emitInst(Opcode::EOR, i); }
    Subroutine &eor(IndexedIndirectY i) { return emitInst(Opcode::EOR, i); }

    Subroutine &bit(ZeroPage i) { return emitInst(Opcode::BIT, i); }
    Subroutine &bit(Absolute i) { return emitInst(Opcode::BIT, i); }

    // Compare
    Subroutine &cmp(Immediate i) { return emitInst(Opcode::CMP, i); }
    Subroutine &cmp(ZeroPage i) { return emitInst(Opcode::CMP, i); }
    Subroutine &cmp(Absolute i) { return emitInst(Opcode::CMP, i); }
    Subroutine &cpx(Immediate i) { return emitInst(Opcode::CPX, i); }
    Subroutine &cpx(ZeroPage i) { return emitInst(Opcode::CPX, i); }
    Subroutine &cpx(Absolute i) { return emitInst(Opcode::CPX, i); }
    Subroutine &cpy(Immediate i) { return emitInst(Opcode::CPY, i); }
    Subroutine &cpy(ZeroPage i) { return emitInst(Opcode::CPY, i); }
    Subroutine &cpy(Absolute i) { return emitInst(Opcode::CPY, i); }

    // Transfers
    Subroutine &tax() { return emitInst(Opcode::TAX); }
    Subroutine &tay() { return emitInst(Opcode::TAY); }
    Subroutine &txa() { return emitInst(Opcode::TXA); }
    Subroutine &tya() { return emitInst(Opcode::TYA); }

    // Stack
    Subroutine &pha() { return emitInst(Opcode::PHA); }
    Subroutine &pla() { return emitInst(Opcode::PLA); }
    Subroutine &php() { return emitInst(Opcode::PHP); }
    Subroutine &plp() { return emitInst(Opcode::PLP); }
    Subroutine &txs() { return emitInst(Opcode::TXS); }
    Subroutine &tsx() { return emitInst(Opcode::TSX); }

    // Flags
    Subroutine &sei() { return emitInst(Opcode::SEI); }
    Subroutine &cli() { return emitInst(Opcode::CLI); }
    Subroutine &clc() { return emitInst(Opcode::CLC); }
    Subroutine &sec() { return emitInst(Opcode::SEC); }
    Subroutine &clv() { return emitInst(Opcode::CLV); }
    Subroutine &sed() { return emitInst(Opcode::SED); }
    Subroutine &cld() { return emitInst(Opcode::CLD); }

    // Branches — take a Label
    Subroutine &bne(const Label &label) { return emitInst(Opcode::BNE, label); }
    Subroutine &beq(const Label &label) { return emitInst(Opcode::BEQ, label); }
    Subroutine &bcc(const Label &label) { return emitInst(Opcode::BCC, label); }
    Subroutine &bcs(const Label &label) { return emitInst(Opcode::BCS, label); }
    Subroutine &bmi(const Label &label) { return emitInst(Opcode::BMI, label); }
    Subroutine &bpl(const Label &label) { return emitInst(Opcode::BPL, label); }
    Subroutine &bvc(const Label &label) { return emitInst(Opcode::BVC, label); }
    Subroutine &bvs(const Label &label) { return emitInst(Opcode::BVS, label); }

    // Jump
    Subroutine &jmp(Absolute i) { return emitInst(Opcode::JMP, i); }
    Subroutine &jmp(Indirect i) { return emitInst(Opcode::JMP, i); }
    Subroutine &jmp(const Label &label) { return emitInst(Opcode::JMP, label); }
    Subroutine &jmp(std::string_view labelName) { return emitInst(Opcode::JMP, Label(labelName)); }
    Subroutine &jsr(Absolute i) { return emitInst(Opcode::JSR, i); }
    Subroutine &jsr(const Label &label) { return emitInst(Opcode::JSR, label); }
    Subroutine &jsr(std::string_view labelName) { return emitInst(Opcode::JSR, Label(labelName)); }
    Subroutine &rts() { return emitInst(Opcode::RTS); }
    Subroutine &rti() { return emitInst(Opcode::RTI); }

    // Other
    Subroutine &brk() { return emitInst(Opcode::BRK); } // triggers an interrupt request (IRQ)
    Subroutine &nop() { return emitInst(Opcode::NOP); }
    Subroutine &label(const Label &l) { instructions_.push_back(LabelDef{ l }); return *this; }

    std::string name() const { return name_; }
    const std::vector<Entry> &instructions() const { return instructions_; }
  private:
    friend class Program;
    Subroutine(std::string_view name);

    template<typename AddrMode>
    Subroutine &emitInst(Opcode opcode, AddrMode operand) {
      instructions_.push_back(Instruction{ opcode, operand });
      return *this;
    }

    Subroutine &emitInst(Opcode opcode) {
      instructions_.push_back(Instruction{ opcode, std::monostate{} });
      return *this;
    }

    std::vector<Entry> instructions_;
    std::string name_;
  };

  // Program is the 6502 code. Subroutines, labels, memory map, interrupt vectors. Pure logic.
  class Program {
  public:
    explicit Program(MemoryMap &mmap);
    // Reset vector occurs when the system is first turned on, or when the user presses the Reset button on the front of the console.
    void setResetVector(const Subroutine &handler);
    // NMI vector (“Non-Maskable Interrupt”) occurs when the PPU starts preparing the next frame of graphics, 60 times per second.
    void setNMIVector(const Subroutine &handler);
    // IRQ vector (“Interrupt Request”) can be triggered by the NES’ sound processor or from certain types of cartridge hardware.
    void setIRQVector(const Subroutine &handler);
    const Subroutine *resetVector() const { return resetVector_; }
    const Subroutine *nmiVector() const { return nmiVector_; }
    const Subroutine *irqVector() const { return irqVector_; }
    Subroutine &addSubroutine(std::string_view name);
    Subroutine &getSubroutine(std::string_view name);
    std::string initStandardReset();
    const std::vector<std::unique_ptr<Subroutine>> &subroutines() const { return subroutines_; }
  private:
    MemoryMap &mmap_;
    std::vector<std::unique_ptr<Subroutine>> subroutines_;
    const Subroutine *resetVector_ = nullptr;
    const Subroutine *nmiVector_ = nullptr;
    const Subroutine *irqVector_ = nullptr;
  };

  class Resources {
    std::vector<uint8_t> chrData_;
    std::array<uint8_t, 16> bgPal_;
    std::array<uint8_t, 16> spPal_;
  public:
    void loadCHR(std::string_view path);
    void loadPalettes(std::string_view path);
    void setPalettes(std::array<uint8_t, 16> bg, std::array<uint8_t, 16> spr);

    const std::vector<uint8_t> &chrData() const { return chrData_; }
  };

  enum class Mapper { NROM, MMC1, UNROM, CNROM, MMC3 };
  enum class Mirroring { Horizontal, Vertical };

  struct AsmEmitterOptions;

  // Rom is the final cartridge artifact. Pure packaging.
  class Rom {
    struct Impl;
    std::unique_ptr<Impl> imp;
  public:
    Rom();
    ~Rom();
    void setToolchain(Toolchain &tc);
    void setProgram(Program &prg);
    void setResources(Resources &rc);
    void setMapper(Mapper mapper);
    void setMirroring(Mirroring mirroring);
    void setEmitterOptions(const AsmEmitterOptions &options);
    uint8_t mirroringByte() const;
    void emitAsm(std::string_view dirPath);
    void build(std::string_view filePath);
  };

} // namespace cppnes
