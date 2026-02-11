#include "asmemitter.hpp"
#include "nesdefs_helper.hpp"
#include <iostream>
#include <sstream>

int main() {
  //std::cout << "main start...\n";

  using namespace cppnes;
  MemoryMap mem;

  Program prg(mem);

  prg.initStandardReset();

  Subroutine &nmi = prg.addSubroutine("nmi_handler");
  nmi.rti();
  prg.setNMIVector(nmi);

  Subroutine &irq = prg.addSubroutine("irq_handler");
  irq.rti();
  prg.setIRQVector(irq);

  Resources rcs;
  Rom rom;
  rom.setProgram(prg);
  rom.setResources(rcs);

  prg.addSubroutine("main")
    .ldx(PPUSTATUS_Abs)
    .ldx(Immediate{ 0x3f })
    .stx(PPUADDR_Abs)
    .ldx(ImmZero)
    .stx(PPUADDR_Abs)
    .lda(Immediate{ palette::Salmon })
    .sta(PPUDATA_Abs)
    .lda(Immediate{ 0b00011110 })
    .sta(PPUMASK_Abs)
    .label(Label("forever"))
    .jmp("forever");

  rom.emitAsm("./output");

  //std::cout << "main end.\n";
  return 0;
}