#include "nesdefs.hpp"
#include "asmemitter.hpp"
#include "3rdparty/utils_log/logger.hpp"
#include <fstream>
#include <cassert>

struct cppnes::Rom::Impl {
  Toolchain *tools_ = nullptr;
  Program *prg_ = nullptr;
  Resources *resources_ = nullptr;
  AsmEmitterOptions emitterOptions_;
  Mapper mapper_ = Mapper::CNROM;
  Mirroring mirroring_ = Mirroring::Horizontal;
};

cppnes::Rom::Rom() : imp(new Impl)
{
}

cppnes::Rom::~Rom()
{
}

void cppnes::Rom::setToolchain(Toolchain &tc)
{
  imp->tools_ = &tc;
}

void cppnes::Rom::setProgram(Program &prg)
{
  imp->prg_ = &prg;
}

void cppnes::Rom::setMapper(Mapper mapper)
{
  imp->mapper_ = mapper;
}

void cppnes::Rom::setMirroring(Mirroring mirroring)
{
  imp->mirroring_ = mirroring;
}

void cppnes::Rom::setResources(Resources &rc)
{
  imp->resources_ = &rc;
}

void cppnes::Rom::setEmitterOptions(const AsmEmitterOptions &options)
{
  imp->emitterOptions_ = options;
}

uint8_t cppnes::Rom::mirroringByte() const
{
  return imp->mirroring_ == Mirroring::Vertical ? 0x01 : 0x00;
}

void cppnes::Rom::emitAsm(std::string_view dirPath)
{
  assert(imp->prg_);
  assert(imp->resources_);
  std::filesystem::path dir{ dirPath };
  LOG_MSG << "Rom::emitAsm" << std::filesystem::absolute(dir).generic_string();
  if (!std::filesystem::exists(dir)) {
    std::filesystem::create_directories(dir);
  }
  AsmEmitter emitter(imp->emitterOptions_);
  std::ofstream prg{ dir / "prg.asm" };
  std::ofstream cfg{ dir / "lnk.cfg" };
  emitter.emitInesHeader(*this, prg);
  emitter.emitPrgAsm(*imp->prg_, prg);
  emitter.emitChars(*imp->resources_, prg);
  emitter.emitStartup(prg);
  emitter.emitLinkerConfig(cfg);
}

void cppnes::Rom::build(std::string_view outputPath, std::string_view workingDir)
{
  assert(imp->tools_);
  // 1. Emit asm/cfg to a temp dir
  auto workDir = !workingDir.empty() ? std::filesystem::path{ workingDir } : std::filesystem::temp_directory_path() / "cpp-nes-6502";
  std::filesystem::create_directories(workDir);
  emitAsm(workDir.string());

  // 2. Invoke ca65 + ld65 via Toolchain
  imp->tools_->compile(workDir / "prg.asm", workDir / "prg.o");
  imp->tools_->link(workDir / "lnk.cfg", workDir / "prg.o", std::filesystem::path(outputPath) / "prg.nes");
}

