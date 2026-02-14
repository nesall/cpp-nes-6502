#pragma once

#include "nesdefs.hpp"
#include <ostream>
#include <memory>

namespace cppnes {

  class Program;
  class Rom;

  struct AsmEmitterOptions {
    bool emitComments = true; // e.g. ; playerX after $0010
    bool emitAddressHints = true; // e.g. ; $2000 after PPU_CTRL
    bool autoCreateConstants = true;
  };

  class AsmEmitter {
  public:
    explicit AsmEmitter(const AsmEmitterOptions &options = {});
    ~AsmEmitter();
    AsmEmitter(const AsmEmitter &) = delete;
    AsmEmitter &operator=(const AsmEmitter &) = delete;

    // Primary entry points — called by Rom::emitAsm()
    void emitPrgAsm(const Program &prg, std::ostream &out) const;
    void emitLinkerConfig(std::ostream &out) const;
    void emitInesHeader(const Rom &rom, std::ostream &out) const;
    void emitChars(const Resources &rc, std::ostream &out) const;
    void emitStartup(std::ostream &out) const;

  private:
    std::string formatEntry(const Entry &entry) const;
    std::string formatInstruction(const Instruction &inst) const;
    std::string formatLabelDef(const LabelDef &ldef) const;
    std::string formatLineComment(const LineComment &c) const;
    std::string formatInlineComment(const InlineComment &c) const;
    std::string formatVectors(const Program &prg) const;
    std::string formatOAM(const Program &prg) const;
    std::string opcodeToString(Opcode op) const;

    struct Impl;
    std::unique_ptr<Impl> imp;
  };

} // namespace cppnes
