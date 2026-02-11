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

std::string cppnes::Program::initStandardReset()
{
  std::string name{ "reset_handler" };

  Label vblankwait0("vblankwait0");
  Label vblankwait1("vblankwait1");

  Subroutine &reset = addSubroutine(name);
  setResetVector(reset);
  reset
    .sei()
    .cld()
    .ldx(Immediate{ 0x40 })
    .stx(Absolute{ AbsAddress{0x4017} })
    .ldx(Immediate{ 0xFF })
    .txs()
    .inx()
    .stx(PPUCTRL_Abs)
    .stx(PPUMASK_Abs)
    .stx(Absolute{ AbsAddress{0x4010} })
    .bit(PPUSTATUS_Abs)
    .label(vblankwait0)
    .bit(PPUSTATUS_Abs)
    .bpl(vblankwait0)
    .label(vblankwait1)
    .bit(PPUSTATUS_Abs)
    .bpl(vblankwait1)
    .jmp(Label{ "main" });

  return name;
}

