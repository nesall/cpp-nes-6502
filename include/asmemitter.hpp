#pragma once

#include "nesdefs.hpp"
#include <ostream>

namespace cppnes {

  class Program;
  class Rom;

  struct AsmEmitterOptions {
    bool emitComments = true;   // e.g. ; playerX after $0010
    bool emitAddressHints = true;  // e.g. ; $2000 after PPU_CTRL
  };

  class AsmEmitter {
  public:

    explicit AsmEmitter(const AsmEmitterOptions &options = {});

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
    std::string formatVectors(const Program &prg) const;
    std::string opcodeToString(Opcode op) const;

    AsmEmitterOptions options_;
  };

} // namespace cppnes
