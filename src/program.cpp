#include "nesdefs.hpp"
#include "nesdefs_helper.hpp"

cppnes::Program::Program(MemoryMap &mmap) : mmap_(mmap)
{
}

void cppnes::Program::setResetVector(const Subroutine &handler)
{
  resetVector_ = &handler;
}

void cppnes::Program::setNMIVector(const Subroutine &handler)
{
  nmiVector_ = &handler;
}

void cppnes::Program::setIRQVector(const Subroutine &handler)
{
  irqVector_ = &handler;
}

cppnes::Subroutine &cppnes::Program::addSubroutine(std::string_view name)
{
  std::unique_ptr<Subroutine> ptr(new Subroutine(name));
  subroutines_.push_back(std::move(ptr));
  return *subroutines_.back();
}

cppnes::Subroutine &cppnes::Program::getSubroutine(std::string_view name)
{
  for (auto &sub : subroutines_) {
    if (sub->name_ == name) {
      return *sub;
    }
  }
  throw std::runtime_error("Subroutine not found: " + std::string(name));
}

cppnes::DataBlock &cppnes::Program::getDataBlock(const Label &label)
{
  auto it = dataBlocks_.find(label.name());
  if (it != dataBlocks_.end())
    return *it->second;
  throw std::runtime_error("Data block not found: " + label.name());
}

cppnes::DataBlock &cppnes::Program::addDataBlock(const Label &label)
{
  auto it = dataBlocks_.find(label.name());
  if (it != dataBlocks_.end())
    return *it->second;
  auto db = std::make_unique<DataBlock>(label.name());
  auto &ref = *db;
  dataBlocks_.emplace(label.name(), std::move(db));
  return ref;
}

cppnes::Subroutine &cppnes::Program::initStandardReset()
{
  std::string name{ "reset_handler" };

  Label vblankwait0("vblankwait0");
  Label vblankwait1("vblankwait1");

  Subroutine &reset = addSubroutine(name);
  setResetVector(reset);
  return reset
    .sei()
    .cld()
    //.bblocks().enableRendering(false)
    .ldx(imm(0x40))
    .stx(abs(0x4017))
    .ldx(imm(0xFF))
    .txs()
    .inx()
    .stx(abs(PPUCTRL))
    .stx(abs(PPUMASK))
    .stx(abs(0x4010))
    .bblocks().waitVBlank()
    .bblocks().waitVBlank()
    //.bblocks().enableRendering(true)
    ;
}

void cppnes::Program::addConstant(std::string_view name, int32_t value)
{
  constants_[std::string(name)] = value;
}

bool cppnes::Program::hasConstant(std::string_view name) const
{
  return constants_.count(std::string{ name });
}

int32_t cppnes::Program::getConstant(std::string_view name) const
{
  auto it = constants_.find(std::string(name));
  if (it == constants_.end())
    throw std::runtime_error("Constant not found: " + std::string(name));
  return it->second;
}
