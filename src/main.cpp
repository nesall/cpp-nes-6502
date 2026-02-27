#include "asmemitter.hpp"
#include "nesdefs_helper.hpp"
#include "3rdparty/CLI11.hpp"
#include "3rdparty/utils_log/logger.hpp"

int main(int argc, char *argv[]) {
  // Command line format: cppbuild-nes --out output/ --im output/temp --ca path/bin/ca65 --ld path/bin/ld65
  // CLI11

  CLI::App app{ "cppbuild-nes" };

  std::string outDir;
  std::string intermediateDir;
  std::string ca65Path;
  std::string ld65Path;

  app.add_option("--out", outDir, "Output directory")
    ->required()
    ->check(CLI::ExistingDirectory);

  app.add_option("--im", intermediateDir, "Intermediate directory")
    ->required();

  app.add_option("--ca", ca65Path, "Path to ca65 binary")
    ->required()
    ->check(CLI::ExistingFile);

  app.add_option("--ld", ld65Path, "Path to ld65 binary")
    ->required()
    ->check(CLI::ExistingFile);

  CLI11_PARSE(app, argc, argv);

  LOG_MSG << "Out" << outDir;
  LOG_MSG << "Intermediate" << intermediateDir;
  LOG_MSG << "ca65" << ca65Path;
  LOG_MSG << "ld65" << ld65Path;

  using namespace cppnes;
  Toolchain toolchain;
  toolchain.setCa65(ca65Path);
  toolchain.setLd65(ld65Path);


  MemoryMap mem;
  Program prg(mem);
  auto &reset = prg.initStandardReset();

  Subroutine &nmi = prg.addSubroutine("nmi_handler");
  nmi.jsr("readInput");
  nmi.jsr("updatePlayer1");
  nmi.rti();
  prg.setNMIVector(nmi);

  const Label titleNamLabel{ "TitleNam" };
  Resources rc;
  rc.loadCHR("D:/workspace/projects/cpp-nes-6502/rc/NewFile.chr");
  rc.addNametable(titleNamLabel.name(), "D:/workspace/projects/cpp-nes-6502/rc/title-scr.nam");
  rc.setChrUseFilename(true);

  Rom rom;
  rom.setProgram(prg);
  rom.setResources(rc);
  rom.setMirroring(Mirroring::None);
           
  const Label paletteLabel{ "PaletteData" };
  auto &block = prg.addDataBlock(paletteLabel);
  block.addBytes({ clr::Black, 0x2d, clr::PaleBlue, clr::White }, "Background palette 0");
  block.addBytes({ clr::Black, 0x0c, 0x21, 0x32 }, "Background palette 1");
  block.addBytes({ clr::Black, 0x05, 0x25, 0x25 }, "Background palette 2");
  block.addBytes({ clr::Black, 0x0b, 0x1a, 0x29 }, "Background palette 3");
  block.addBytes({ clr::Black, clr::DarkGray, clr::MediumGray, clr::White}, "Foreground palette 0");
  block.addBytes({ clr::Black, clr::BrightYellow, clr::Aqua, clr::DarkRed }, "Foreground palette 1");
  block.addBytes({ clr::Black, clr::BrightGreen, clr::DarkerBlue, clr::DarkRed }, "Foreground palette 2");
  block.addBytes({ clr::Black, clr::BlueViolet, clr::BrightPink, clr::DarkRed }, "Foreground palette 3");

  auto playerX = prg.allocZp("playerX", true);
  auto playerY = prg.allocZp("playerY", true);

  auto buttons = prg.allocZp("buttons", true);
  auto buttonsPrev = prg.allocZp("buttonsPrev", true);
  auto buttonsPressed = prg.allocZp("buttonsPressed", true); // newly pressed this frame
  auto buttonsReleased = prg.allocZp("buttonsReleased", true); // released this frame
  auto namPtr = prg.allocZp("namPtr", 2, true);

  reset
    .bblocks().setAddrByte(namPtr, 0x16).commentPrev("2 bytes")
    .bblocks().loadPalette(paletteLabel)
    .bblocks().loadNametable(titleNamLabel, namPtr)
    .bblocks().enableRendering(true)
    //.bblocks().setAddrByte(colorIndex, 0)
    .bblocks().setAddrByte(playerX, 120)
    .bblocks().setAddrByte(playerY, 100)
    .bblocks().enableNMI()
    .jmp("main");

  prg.addSubroutine("main")  
    .label("forever")
    .jmp("forever");

  prg.addSubroutine("readInput")
    .bblocks().readController(buttons, buttonsPrev, buttonsPressed, buttonsReleased)
    //.lda(zp(buttonsPressed))
    //.and_(imm(BTN_UP))
    //.beq(Label("no_up"))
    //.inc(zp(colorIndex))
    //.lda(zp(colorIndex))
    //.cmp(imm(16))
    //.bne(Label("update_pal"))
    //.bblocks().setAddrByte(colorIndex, 0)
    //.label(Label("update_pal"))
    //.bblocks().setPPUAddr(0x3f00)
    //.ldx(zp(colorIndex))
    //.lda(absx(paletteLabel))
    //.sta(abs(PPUDATA))
    //.label(Label("no_up"))
    //.bblocks().setAddrByte(buttonsPrev, buttons)
    .rts();

  prg.addSubroutine("updatePlayer1")
    .bblocks().initPadCallback(buttons, [&playerX, &playerY](Subroutine &sub, uint8_t btn)
      {
        switch (btn) {
        case BTN_UP: break;
          sub.dec(zp(playerY));
          break;
        case BTN_DOWN: break;
          sub.inc(zp(playerY));
          break;
        case BTN_LEFT: 
          sub.dec(zp(playerX));
          break;
        case BTN_RIGHT: 
          sub.inc(zp(playerX));
          break;
        case BTN_A: break;
        case BTN_B: break;
        case BTN_SELECT: break;
        case BTN_START: break;
        default: break;
        }
      }
    )
    .rts();

  rom.setToolchain(toolchain);
  rom.build(outDir, intermediateDir);
  return 0;
}