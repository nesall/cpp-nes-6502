#include "nesdefs.hpp"
#include "asmemitter.hpp"
#define FMT_HEADER_ONLY
#include "3rdparty/fmt/format.h"
#include "3rdparty/utils_log/logger.hpp"
#include <cassert>
#include <unordered_map>

namespace cppnes {
  namespace {
    struct OperandFormatter {
      OperandFormatter(const AsmEmitterOptions &o) : opts(o) {}
      const AsmEmitterOptions &opts;
      mutable std::unordered_map<std::string, int32_t> zpConstants_;
      mutable std::unordered_map<std::string, int32_t> absConstants_;

      std::string operator()(std::monostate) const { return ""; }
      std::string operator()(Accumulator) const { return "A"; }
      std::string operator()(Immediate i) const { return fmt::format("#${:02X}", i.value); }
      std::string operator()(ImmediateLabel il) const { 
        char prefix = (il.which == ByteOf::Low) ? '<' : '>';
        return "#" + std::string(1, prefix) + il.label.name();
      }

      std::string operator()(ZeroPage zp) const {
        auto s = fmt::format("${:02X}", zp.addr.value());
        if (opts.autoCreateConstants && zp.addr.isConstant()) {
          if (zp.addr.name().empty()) {
            LOG_MSG << fmt::format("Unable to auto-create constant for address ${:02X} because it has no name", zp.addr.value());
            return s;
          }
          auto it = zpConstants_.find(zp.addr.name());
          if (it != zpConstants_.cend() && it->second != zp.addr.value()) {
            LOG_MSG <<
              fmt::format("Duplicate constant name '{}' for address ${:02X} (previously ${:02X}). Skipped", zp.addr.name(), zp.addr.value(), it->second);
            if (opts.emitComments)
              s += fmt::format(" ; {}", zp.addr.name());
            return s;
          }
          zpConstants_[zp.addr.name()] = zp.addr.value();
          s = fmt::format("{}", zp.addr.name());
          if (opts.emitAddressHints)
            s += fmt::format(" ; ${:02X}", zp.addr.value());
        } else {
          if (opts.emitComments && !zp.addr.name().empty()) {
            s += fmt::format(" ; {}", zp.addr.name());
          }
        }
        return s;
      }

      std::string operator()(Absolute abs) const { 
        auto s = fmt::format("${:04X}", abs.addr.value());
        if (opts.autoCreateConstants && abs.addr.isConstant()) {
          if (abs.addr.name().empty()) {
            LOG_MSG << fmt::format("Unable to auto-create constant for address ${:04X} because it has no name", abs.addr.value());
            return s;
          }
          auto it = absConstants_.find(abs.addr.name());
          if (it != absConstants_.cend() && it->second != abs.addr.value()) {
            LOG_MSG <<
              fmt::format("Duplicate constant name '{}' for address ${:04X} (previously ${:04X}). Skipped", abs.addr.name(), abs.addr.value(), it->second);
            if (opts.emitComments)
              s += fmt::format(" ; {}", abs.addr.name());
            return s;
          }
          absConstants_[abs.addr.name()] = abs.addr.value();
          s = fmt::format("{}", abs.addr.name());
          if (opts.emitAddressHints)
            s += fmt::format(" ; ${:04X}", abs.addr.value());
        } else {
          if (opts.emitComments && !abs.addr.name().empty()) {
            s += fmt::format(" ; {}", abs.addr.name());
          }
        }
        return s;
      }

      std::string operator()(AbsoluteX absx) const {
        return std::visit([](const auto &base) {
          using T = std::decay_t<decltype(base)>;
          if constexpr (std::is_same_v<T, AbsAddress>)
            return fmt::format("${:04X},X", base.value());
          else // Label
            return fmt::format("{},X", base.name());
          }, absx.base);
      }

      std::string operator()(AbsoluteY absy) const {
        return std::visit([](const auto &base) {
          using T = std::decay_t<decltype(base)>;
          if constexpr (std::is_same_v<T, AbsAddress>)
            return fmt::format("${:04X},Y", base.value());
          else // Label
            return fmt::format("{},Y", base.name());
          }, absy.base);
      }

      std::string operator()(ZeroPageX zpx) const {
        return std::visit([](const auto &base) {
          using T = std::decay_t<decltype(base)>;
          if constexpr (std::is_same_v<T, ZpAddress>)
            return fmt::format("${:04X},X", base.value());
          else // Label
            return fmt::format("{},X", base.name());
          }, zpx.base);
      }
      
      std::string operator()(ZeroPageY zpy) const {
        return std::visit([](const auto &base) {
          using T = std::decay_t<decltype(base)>;
          if constexpr (std::is_same_v<T, ZpAddress>)
            return fmt::format("${:04X},Y", base.value());
          else // Label
            return fmt::format("{},Y", base.name());
          }, zpy.base);
      }

      std::string operator()(Indirect ind) const { return fmt::format("(${:04X})", ind.addr.value()); }
      std::string operator()(IndexedIndirectX ind) const { return fmt::format("(${:02X},X)", ind.addr.value()); }
      std::string operator()(IndexedIndirectY ind) const { return fmt::format("(${:02X}),Y", ind.addr.value()); }
      std::string operator()(const Label &l) const { return std::string(l.name()); }
    };
  } // anonymous namespace
} // namespace cppnes

struct cppnes::AsmEmitter::Impl {
  Impl(const AsmEmitterOptions &options) : options_(options), formatter_(options) {}
  AsmEmitterOptions options_;
  OperandFormatter formatter_;
};

cppnes::AsmEmitter::AsmEmitter(const AsmEmitterOptions &options) : imp(new Impl{options})
{
}

cppnes::AsmEmitter::~AsmEmitter()
{
}

void cppnes::AsmEmitter::emitPrgAsm(const Program &program, std::ostream &out) const {

  std::ostringstream to;

  to << ".segment \"CODE\"\n\n";

  to << "_REPLACE_WITH_CONSTANTS_";

  for (const auto &sub : program.subroutines()) {
    to << ".proc " << sub->name() << "\n";
    std::string lastLine;
    enum class LastType { None, LabelDef, Instruction, Other };
    LastType lastType = LastType::None;
    for (const auto &entry : sub->instructions()) {
      // Determine the type of the current entry
      LastType currentType = LastType::Other;
      if (std::holds_alternative<Instruction>(entry)) {
        currentType = LastType::Instruction;
      } else if (std::holds_alternative<LabelDef>(entry)) {
        currentType = LastType::LabelDef;
      } else if (std::holds_alternative<InlineComment>(entry)) {
        currentType = LastType::Other; // We'll handle this below
      } else {
        currentType = LastType::Other;
      }
      if (std::holds_alternative<InlineComment>(entry) &&
        (lastType == LastType::Instruction || lastType == LastType::LabelDef)) {
        // Append inline comment to the last line
        lastLine += " " + formatEntry(entry);
      } else {
        // Output the previous line if it exists
        if (!lastLine.empty()) {
          to << lastLine << "\n";
        }
        lastLine = formatEntry(entry);
        lastType = currentType;
      }
    }
    // Output the last line if any
    if (!lastLine.empty()) {
      to << lastLine << "\n";
    }
    to << ".endproc ;" << sub->name() << "\n\n";
  }

  for (const auto &[name, db] : program.dataBlocks()) {
    to << db->label() << ":\n";
    for (const auto &entry : db->entries()) {
      std::visit([&](const auto &e) {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, DataBlock::ByteEntry>) {
          to << "  .byte ";
          for (size_t i = 0; i < e.data.size(); ++i) {
            to << fmt::format("${:02X}", e.data[i]);
            if (i < e.data.size() - 1) to << ",";
          }
          if (!e.comment.empty()) to << " ;" << e.comment;
          to << "\n";
        } else { // WordEntry
          to << "  .word ";
          for (size_t i = 0; i < e.data.size(); ++i) {
            to << fmt::format("${:04X}", e.data[i]);
            if (i < e.data.size() - 1) to << ",";
          }
          if (!e.comment.empty()) to << "  ;" << e.comment;
          to << "\n";
        }
        }, entry);
    }
    to << "\n";
  }
  to << formatOAM(program);
  to << formatVectors(program);
  to << "\n";

  std::string str{ to.str() };

  std::ostringstream constOut;
  if (!imp->formatter_.zpConstants_.empty()) {
    // sort by value
    std::vector<std::pair<std::string, int32_t>> sortedConstants(imp->formatter_.zpConstants_.begin(), imp->formatter_.zpConstants_.end());
    std::sort(sortedConstants.begin(), sortedConstants.end(), [](const auto &a, const auto &b) {
      return a.second < b.second;
      });
    for (const auto &[name, value] : sortedConstants) {
      constOut << fmt::format("{} = ${:02X}\n", name, value);
    }
  } 
  if (!imp->formatter_.absConstants_.empty()) {
    // sort by value
    std::vector<std::pair<std::string, int32_t>> sortedConstants(imp->formatter_.absConstants_.begin(), imp->formatter_.absConstants_.end());
    std::sort(sortedConstants.begin(), sortedConstants.end(), [](const auto &a, const auto &b) {
      return a.second < b.second;
      });
    for (const auto &[name, value] : sortedConstants) {
      constOut << fmt::format("{} = ${:04X}\n", name, value);
    }
  } 
  if (constOut.str().empty()) {
    str.erase(str.find("_REPLACE_WITH_CONSTANTS_"), std::string("_REPLACE_WITH_CONSTANTS_").length());
  } else {
    auto cs = constOut.str();
    cs = "; Auto-collected constants\n" + cs + "\n";
    str.replace(str.find("_REPLACE_WITH_CONSTANTS_"), std::string("_REPLACE_WITH_CONSTANTS_").length(), cs);
  }

  out << str;
}

void cppnes::AsmEmitter::emitLinkerConfig(std::ostream &out) const {
  out <<
    R"(MEMORY {
  HEADER:   start = $0000,  size = $0010, fill = yes;
  PRG:      start = $8000,  size = $8000, fill = yes, fillval = $FF;
  CHR:      start = $0000,  size = $2000, fill = yes, fillval = $00;
  RAM:      start = $0300,  size = $0600, type = rw;
  OAMBUF:   start = $0200,  size = $0100, type = rw;
}

SEGMENTS {
  HEADER:   load = HEADER,  type = ro;
  CODE:     load = PRG,     type = ro,    start = $8000;
  RODATA:   load = PRG,     type = ro;
  VECTORS:  load = PRG,     type = ro,    start = $FFFA;
  CHARS:    load = CHR,     type = ro;
  OAM:      load = OAMBUF,  type = bss;
  BSS:      load = RAM,     type = bss;
}
)";
}

void cppnes::AsmEmitter::emitInesHeader(const Rom &rom, std::ostream &out) const {
  // iNES header — 16 bytes
  // ca65 emits this as part of the HEADER segment
  out << "; Generated by cpp-nes-6502\n"
    << "; --------------------------\n\n";
  out << ".segment \"HEADER\"\n"
    << fmt::format("  .byte $4E, $45, $53, $1A  ; 'NES' + MS-DOS EOF\n")
    << fmt::format("  .byte $02                  ; PRG-ROM size (2 x 16KB)\n")
    << fmt::format("  .byte $01                  ; CHR-ROM size (1 x 8KB)\n")
    << fmt::format("  .byte ${:02X}                  ; Mapper low / mirroring\n",
      rom.mirroringByte())
    << fmt::format("  .byte $00                  ; Mapper high\n")
    << "  .byte $00, $00, $00, $00, $00, $00, $00, $00  ; padding\n; Header is total 16 bytes.\n\n";
}

void cppnes::AsmEmitter::emitChars(const Resources &rc, std::ostream &out) const
{
  const auto &chr = rc.chrData();
  out << ".segment \"CHARS\"\n";

  if (chr.empty()) {
    out << "; WARNING: No CHR data loaded\n";
    out << ".res 8192 ; Reserving 8192 bytes of blank space\n";
    out << "\n";
    return;
  }
  if (rc.chrUseFilename()) {
    out << fmt::format("; CHR data loaded from file: {}\n", rc.chrPath());
    out << fmt::format(".incbin \"{}\"\n", rc.chrPath());
  } else {
    constexpr size_t bytesPerLine = 16;
    for (size_t i = 0; i < chr.size(); i += bytesPerLine) {
      out << "  .byte ";
      size_t lineEnd = (std::min)(i + bytesPerLine, chr.size());
      for (size_t j = i; j < lineEnd; ++j) {
        out << fmt::format("${:02X}", chr[j]);
        if (j < lineEnd - 1) out << ", ";
      }
      // Comment: byte offset, useful for debugging tile boundaries
      if (imp->options_.emitComments) {
        size_t tile = i / 16;
        out << fmt::format("  ; tile {:03d} offset ${:04X}", tile, i);
      }
      out << "\n";
    }
  }
  out << "\n";

  if (!rc.nametables().empty()) {
    out << ".segment \"RODATA\"\n";
    for (const auto &[label, filename] : rc.nametables()) {
      out << label << ":\n";
      out << fmt::format("  .incbin \"{}\"\n", filename);
    }
  }

  out << "\n";
}

void cppnes::AsmEmitter::emitStartup(std::ostream &out) const
{
  out << ".segment \"STARTUP\"\n";
}

std::string cppnes::AsmEmitter::formatEntry(const Entry &entry) const {
  return std::visit([this](const auto &e) -> std::string {
    using T = std::decay_t<decltype(e)>;
    if constexpr (std::is_same_v<T, Instruction>)
      return formatInstruction(e);
    else if constexpr (std::is_same_v<T, LabelDef>)
      return formatLabelDef(e);
    else if constexpr (std::is_same_v<T, LineComment>)
      return formatLineComment(e);
    else if constexpr (std::is_same_v<T, InlineComment>)
      return formatInlineComment(e);
    else
      static_assert(false, "Unhandled Entry type");
    }, entry);
}

std::string cppnes::AsmEmitter::formatInstruction(const Instruction &inst) const
{
  std::string line = "  " + opcodeToString(inst.opcode);
  std::string operand = std::visit(imp->formatter_, inst.operand);
  if (!operand.empty()) line += " " + operand;
  return line;
}

std::string cppnes::AsmEmitter::formatLabelDef(const LabelDef &ldef) const
{
  return fmt::format("{}:", ldef.label.name());
}

std::string cppnes::AsmEmitter::formatLineComment(const LineComment &c) const
{
  return fmt::format("; {}", c.comment);
}

std::string cppnes::AsmEmitter::formatInlineComment(const InlineComment &c) const
{
  return fmt::format("; {}", c.comment);
}

std::string cppnes::AsmEmitter::formatVectors(const Program &prg) const
{
  assert(prg.nmiVector() && prg.resetVector());
  auto vec = [](const Subroutine *s) -> std::string {
    return s ? std::string(s->name()) : "0";
    };
  return fmt::format(
    ".segment \"VECTORS\"\n"
    "  .word {} ; NMI\n"
    "  .word {} ; RESET\n"
    "  .word {} ; IRQ\n",
    vec(prg.nmiVector()),
    vec(prg.resetVector()),
    (prg.irqVector() ? vec(prg.irqVector()) : std::string{"0"}));
}

std::string cppnes::AsmEmitter::formatOAM(const Program &) const
{
  return fmt::format(
    ".segment \"OAM\"\n"
    "OAMBuffer:\n"
    ".res 256\n\n");
}

std::string cppnes::AsmEmitter::opcodeToString(Opcode op) const
{
  switch (op) {
  case Opcode::LDA: return "LDA";
  case Opcode::STA: return "STA";
  case Opcode::LDX: return "LDX";
  case Opcode::STX: return "STX";
  case Opcode::LDY: return "LDY";
  case Opcode::STY: return "STY";
  case Opcode::ADC: return "ADC";
  case Opcode::SBC: return "SBC";
  case Opcode::ASL: return "ASL";
  case Opcode::LSR: return "LSR";
  case Opcode::ROL: return "ROL";
  case Opcode::ROR: return "ROR";
  case Opcode::BIT: return "BIT";
  case Opcode::AND: return "AND";
  case Opcode::ORA: return "ORA";
  case Opcode::EOR: return "EOR";
  case Opcode::CMP: return "CMP";
  case Opcode::CPX: return "CPX";
  case Opcode::CPY: return "CPY";
  case Opcode::JMP: return "JMP";
  case Opcode::JSR: return "JSR";
  case Opcode::RTS: return "RTS";
  case Opcode::BCC: return "BCC";
  case Opcode::BCS: return "BCS";
  case Opcode::INX: return "INX";
  case Opcode::INY: return "INY";
  case Opcode::DEX: return "DEX";
  case Opcode::DEY: return "DEY";
  case Opcode::INC: return "INC";
  case Opcode::DEC: return "DEC";
  case Opcode::BEQ: return "BEQ";
  case Opcode::BMI: return "BMI";
  case Opcode::BNE: return "BNE";
  case Opcode::BPL: return "BPL";
  case Opcode::BVC: return "BVC";
  case Opcode::BVS: return "BVS";
  case Opcode::BRK: return "BRK";
  case Opcode::PHP: return "PHP";
  case Opcode::PLP: return "PLP";
  case Opcode::PHA: return "PHA";
  case Opcode::PLA: return "PLA";
  case Opcode::CLC: return "CLC";
  case Opcode::SEC: return "SEC";
  case Opcode::CLI: return "CLI";
  case Opcode::SEI: return "SEI";
  case Opcode::CLV: return "CLV";
  case Opcode::CLD: return "CLD";
  case Opcode::SED: return "SED";
  case Opcode::RTI: return "RTI";
  case Opcode::TAX: return "TAX";
  case Opcode::TXA: return "TXA";
  case Opcode::TAY: return "TAY";
  case Opcode::TYA: return "TYA";
  case Opcode::TSX: return "TSX";
  case Opcode::TXS: return "TXS";
  case Opcode::NOP: return "NOP";
  default:
    throw std::runtime_error(fmt::format("Unknown opcode: {}", static_cast<int>(op)));
  }
}
