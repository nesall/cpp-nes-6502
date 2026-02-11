#pragma once

#include "nesdefs.hpp"

namespace cppnes {

  // Factory functions (optional ergonomic wrappers)
  inline Immediate imm(uint8_t v) { return { v }; }
  inline Implied impl() { return {}; }
  inline Accumulator acc() { return {}; }
  inline ZeroPage zp(ZpAddress a) { return { a }; }
  inline ZeroPage zpx(ZpAddress a) { return { a }; }
  inline ZeroPage zpy(ZpAddress a) { return { a }; }
  inline Absolute abs(AbsAddress a) { return { a }; }
  inline Absolute absx(AbsAddress a) { return { a }; }
  inline Absolute absy(AbsAddress a) { return { a }; }
  inline Indirect ind(AbsAddress a) { return { a }; }
  inline IndexedIndirectX idxIndX(ZpAddress a) { return { a }; }
  inline IndexedIndirectY idxIndY(ZpAddress a) { return { a }; }

  #define ImmZero Immediate{ 0x00 }
};


namespace cppnes {
  // PPU registers
  inline const AbsAddress PPUCTRL{ 0x2000, "PPUCTRL" };
  inline const AbsAddress PPUMASK{ 0x2001, "PPUMASK" };
  inline const AbsAddress PPUSTATUS{ 0x2002, "PPUSTATUS" };
  inline const AbsAddress OAMADDR{ 0x2003, "OAMADDR" };
  inline const AbsAddress OAMDATA{ 0x2004, "OAMDATA" };
  inline const AbsAddress PPUSCROLL{ 0x2005, "PPUSCROLL" };
  inline const AbsAddress PPUADDR{ 0x2006, "PPUADDR" };
  inline const AbsAddress PPUDATA{ 0x2007, "PPUDATA" };

  // APU/Controller registers
  inline const AbsAddress OAMDMA{ 0x4014, "OAMDMA" };
  inline const AbsAddress JOY1{ 0x4016, "JOY1" };
  inline const AbsAddress JOY2{ 0x4017, "JOY2" };

  inline const Absolute PPUCTRL_Abs{ Absolute{PPUCTRL} };
  inline const Absolute PPUMASK_Abs{ Absolute{PPUMASK} };
  inline const Absolute PPUSTATUS_Abs{ Absolute{PPUSTATUS} };
  inline const Absolute OAMADDR_Abs{ Absolute{OAMADDR} };
  inline const Absolute OAMDATA_Abs{ Absolute{OAMDATA} };
  inline const Absolute PPUSCROLL_Abs{ Absolute{PPUSCROLL} };
  inline const Absolute PPUADDR_Abs{ Absolute{PPUADDR} };
  inline const Absolute PPUDATA_Abs{ Absolute{PPUDATA} };

  // APU/Controller registers
  inline const Absolute OAMDMA_Abs{ Absolute{OAMDMA} };
  inline const Absolute JOY1_Abs{ Absolute{JOY1} };
  inline const Absolute JOY2_Abs{ Absolute{JOY2} };

  // Palette colors (subset — add full 64-color table as needed)
  namespace palette {
    // NES PPU palette color defines (0x00 to 0x3F)
    // Based on common community names and hue descriptions
    const uint8_t DarkGray = 0x00;  // almost black / dark gray
    const uint8_t DarkBlue = 0x01;
    const uint8_t DarkerBlue = 0x02;
    const uint8_t DarkViolet = 0x03;
    const uint8_t DarkMagenta = 0x04;
    const uint8_t DarkRed = 0x05;
    const uint8_t DarkOrange = 0x06;
    const uint8_t DarkBrown = 0x07;
    const uint8_t DarkOlive = 0x08;
    const uint8_t DarkGreen = 0x09;
    const uint8_t DarkTeal = 0x0A;
    const uint8_t DarkerTeal = 0x0B;
    const uint8_t VeryDarkBlue = 0x0C;
    const uint8_t AlmostBlack = 0x0D;  // often avoided (TV glitch risk on some sets)
    const uint8_t Black = 0x0F;  // commonly used true black

    const uint8_t MediumGray = 0x10;
    const uint8_t SkyBlue = 0x11;
    const uint8_t BrightBlue = 0x12;
    const uint8_t BlueViolet = 0x13;
    const uint8_t Purple = 0x14;
    const uint8_t MarioRed = 0x16;  // classic bright red
    const uint8_t Orange = 0x17;
    const uint8_t Gold = 0x18;
    const uint8_t Olive = 0x19;
    const uint8_t Green = 0x1A;
    const uint8_t LimeGreen = 0x1B;
    const uint8_t Aqua = 0x1C;

    const uint8_t LightGray = 0x20;
    const uint8_t VeryLightBlue = 0x21;
    const uint8_t LightBlue = 0x22;
    const uint8_t LightViolet = 0x23;
    const uint8_t LightMagenta = 0x24;
    const uint8_t HotPink = 0x25;
    const uint8_t BrightPink = 0x26;
    const uint8_t Salmon = 0x27;
    const uint8_t BrightOrange = 0x28;
    const uint8_t LuigiGreen = 0x29;  // classic bright green
    const uint8_t MintGreen = 0x2A;
    const uint8_t Cyan = 0x2B;
    const uint8_t Turquoise = 0x2C;

    const uint8_t White = 0x30;  // brightest white
    const uint8_t PaleBlue = 0x31;
    const uint8_t PaleLightBlue = 0x32;
    const uint8_t PaleViolet = 0x33;
    const uint8_t PaleMagenta = 0x34;
    const uint8_t PalePink = 0x35;
    const uint8_t PaleSalmon = 0x36;
    const uint8_t PaleOrange = 0x37;
    const uint8_t Yellow = 0x38;
    const uint8_t BrightYellow = 0x39;
    const uint8_t BrightLime = 0x3A;
    const uint8_t BrightGreen = 0x3B;
    const uint8_t BrightCyan = 0x3C;
  }
}