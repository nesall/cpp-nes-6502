#pragma once

#include "nesdefs.hpp"
#include <functional>

namespace cppnes {

  // Immediate
  inline Immediate imm(uint8_t v) { return { v }; }

  // Zero page — accept wider type for overflow checking
  inline ZeroPage zp(ZpAddress a) { return { a }; }
  inline ZeroPage zp(uint32_t v) { return { ZpAddress::fromValue(v) }; }
  inline ZeroPageX zpx(ZpAddress a) { return { a }; }
  inline ZeroPageX zpx(uint32_t v) { return { ZpAddress::fromValue(v) }; }
  inline ZeroPageY zpy(ZpAddress a) { return { a }; }
  inline ZeroPageY zpy(uint32_t v) { return { ZpAddress::fromValue(v) }; }

  // Absolute — accept wider type for overflow checking
  inline Absolute abs(AbsAddress a) { return { a }; }
  inline Absolute abs(uint32_t v) { return { AbsAddress::fromValue(v) }; }
  inline AbsoluteX absx(AbsAddress a) { return { a }; }
  inline AbsoluteX absx(uint32_t v) { return { AbsAddress::fromValue(v) }; }
  inline AbsoluteX absx(const Label &l) { return { l }; }
  inline AbsoluteY absy(AbsAddress a) { return { a }; }
  inline AbsoluteY absy(const Label &l) { return { l }; }
  inline AbsoluteY absy(uint32_t v) { return { AbsAddress::fromValue(v) }; }

  // Indirect
  inline Indirect ind(AbsAddress a) { return { a }; }
  inline Indirect ind(uint32_t v) { return { AbsAddress::fromValue(v) }; }

  // Indexed indirect
  inline IndexedIndirectX indx(ZpAddress a) { return { a }; }
  inline IndexedIndirectX indx(uint32_t v) { return { ZpAddress::fromValue(v) }; }
  inline IndexedIndirectY indy(ZpAddress a) { return { a }; }
  inline IndexedIndirectY indy(uint32_t v) { return { ZpAddress::fromValue(v) }; }

  // Helpers
  inline Implied impl() { return {}; }
  inline Accumulator acc() { return {}; }

  // Common constants
  inline const Immediate immZero{ 0x00 };
};


namespace cppnes {
  // PPU registers

  // 0x2000
  inline const AbsAddress PPUCTRL{ 0x2000, "PPUCTRL", true };
  // 0x2001
  inline const AbsAddress PPUMASK{ 0x2001, "PPUMASK", true };
  // 0x2002
  inline const AbsAddress PPUSTATUS{ 0x2002, "PPUSTATUS", true };
  // 0x2003
  inline const AbsAddress OAMADDR{ 0x2003, "OAMADDR", true };
  // 0x2004
  inline const AbsAddress OAMDATA{ 0x2004, "OAMDATA", true };
  // 0x2005
  inline const AbsAddress PPUSCROLL{ 0x2005, "PPUSCROLL", true };
  // 0x2006
  inline const AbsAddress PPUADDR{ 0x2006, "PPUADDR", true };
  // 0x2007
  inline const AbsAddress PPUDATA{ 0x2007, "PPUDATA", true };

  // APU/Controller registers

  // 0x4014
  inline const AbsAddress OAMDMA{ 0x4014, "OAMDMA", true };
  // 0x4016
  inline const AbsAddress JOY1{ 0x4016, "JOY1", true };
  // 0x4017
  inline const AbsAddress JOY2{ 0x4017, "JOY2", true };

  const uint8_t BTN_RIGHT = 0b00000001;
  const uint8_t BTN_LEFT = 0b00000010;
  const uint8_t BTN_DOWN = 0b00000100;
  const uint8_t BTN_UP = 0b00001000;
  const uint8_t BTN_START = 0b00010000;
  const uint8_t BTN_SELECT = 0b00100000;
  const uint8_t BTN_B = 0b01000000;
  const uint8_t BTN_A = 0b10000000;

  // Palette colors (subset — add full 64-color table as needed)
  namespace clr {
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
    const uint8_t AlmostBlack_AVOID = 0x0D;  // often avoided (TV glitch risk on some sets)
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

    const std::array<uint8_t, 16> fgPalSuperMarioBros = {
      SkyBlue, MarioRed, Orange, Olive,
      SkyBlue, MarioRed, White, BrightOrange,
      SkyBlue, DarkGreen, White, Orange,
      SkyBlue, Black, PaleSalmon, DarkBrown
    };
    const std::array<uint8_t, 16> bgPalSuperMarioBros = {
      SkyBlue, LimeGreen, DarkGreen, Black,
      SkyBlue, White, BrightBlue, Black,
      SkyBlue, PaleSalmon, DarkBrown, Black,
      SkyBlue, Black, DarkOrange, Black
    };
  } // namespace clr


  namespace bblocks {
    Subroutine &waitVBlank(Subroutine &sub);
    Subroutine &clearMemory(Subroutine &sub, AbsAddress start, uint8_t length);
    Subroutine &clearMemory(Subroutine &sub, AbsAddress start, uint16_t length, ZpAddress ptr, ZpAddress count);
    Subroutine &clearPage(Subroutine &sub, AbsAddress start);
    Subroutine &loadPalette(Subroutine &sub, const Label &dataLabel);
    Subroutine &loadNametable(Subroutine &sub, const Label &dataLabel, ZpAddress counter);
    Subroutine &loopX(Subroutine &sub, uint8_t count, std::function<void(Subroutine &)> body);
    Subroutine &uploadSprites(Subroutine &sub, AbsAddress oamBuffer = AbsAddress{ 0x0200 });
    Subroutine &setPPUAddr(Subroutine &sub, uint16_t addr);
    Subroutine &readController(Subroutine &sub, ZpAddress buttons, ZpAddress buttonsPrev, ZpAddress buttonsPressed, ZpAddress buttonsReleased);
    Subroutine &setAddrByte(Subroutine &sub, ZpAddress addr, uint8_t b);
    Subroutine &setAddrByte(Subroutine &sub, ZpAddress addr, ZpAddress b);
    Subroutine &setAddrByte(Subroutine &sub, AbsAddress addr, uint8_t b);
    Subroutine &setAddrWord(Subroutine &sub, AbsAddress addr, uint16_t w);
    Subroutine &ppuWriteBytes(Subroutine &sub, const Label &src, uint8_t count);
    Subroutine &ppuWriteBytesZpPtr(Subroutine &sub, ZpAddress ptr, uint8_t count);
    Subroutine &ppuFill(Subroutine &sub, uint8_t value, uint8_t count);
    Subroutine &memcpy(Subroutine &sub, ZpAddress src, ZpAddress dst, uint16_t count, ZpAddress counter);
    Subroutine &memset16(Subroutine &sub, AbsAddress start, uint16_t value, uint16_t count, ZpAddress ptr, ZpAddress cnt, ZpAddress val);
    Subroutine &memset8(Subroutine &sub, AbsAddress start, uint8_t value, uint16_t count, ZpAddress ptr, ZpAddress cnt);
    Subroutine &clearOAMBuffer(Subroutine &sub, AbsAddress buffer = AbsAddress{ 0x0200 });
    Subroutine &enableRendering(Subroutine &sub, bool enable);
    Subroutine &setPPUMaskBits(Subroutine &sub, uint8_t bitsToSet, uint8_t bitsToClear);
    Subroutine &enableNMI(Subroutine &sub);
    Subroutine &initPadCallback(Subroutine &sub, ZpAddress buttons, std::function<void(Subroutine &, uint8_t)> callback);

    //Subroutine &switchBank(Subroutine &sub, uint8_t bank); // for future implementation
  } // namespace blocks


  class SubroutineBblocksProxy {
    Subroutine &sub_;

  public:
    explicit SubroutineBblocksProxy(Subroutine &s) : sub_(s) {}

    Subroutine &waitVBlank() { return bblocks::waitVBlank(sub_); }
    Subroutine &clearMemory(AbsAddress start, uint8_t length) { return bblocks::clearMemory(sub_, start, length); }
    Subroutine &clearMemory(AbsAddress start, uint16_t length, ZpAddress ptr, ZpAddress count) { return bblocks::clearMemory(sub_, start, length, ptr, count); }
    Subroutine &clearPage(AbsAddress start) { return bblocks::clearPage(sub_, start); }
    Subroutine &loadPalette(const Label &dataLabel) { return bblocks::loadPalette(sub_, dataLabel); }
    Subroutine &loadNametable(const Label &namLabel, ZpAddress counter) { return bblocks::loadNametable(sub_, namLabel, counter); }
    Subroutine &loopX(uint8_t count, std::function<void(Subroutine &)> body) { return bblocks::loopX(sub_, count, std::move(body)); }
    Subroutine &uploadSprites(AbsAddress oamBuffer = AbsAddress{ 0x0200 }) { return bblocks::uploadSprites(sub_, oamBuffer); }
    Subroutine &setPPUAddr(uint16_t addr) { return bblocks::setPPUAddr(sub_, addr); }
    Subroutine &readController(ZpAddress buttons, ZpAddress buttonsPrev, ZpAddress buttonsPressed, ZpAddress buttonsReleased) { 
      return bblocks::readController(sub_, buttons, buttonsPrev, buttonsPressed, buttonsReleased);
    }
    Subroutine &setAddrByte(AbsAddress addr, uint8_t b) { return bblocks::setAddrByte(sub_, addr, b); }
    Subroutine &setAddrByte(ZpAddress addr, uint8_t b) { return bblocks::setAddrByte(sub_, addr, b); }
    Subroutine &setAddrByte(ZpAddress addr, ZpAddress b) { return bblocks::setAddrByte(sub_, addr, b); }
    Subroutine &setAddrWord(AbsAddress addr, uint16_t w) { return bblocks::setAddrWord(sub_, addr, w); }
    Subroutine &ppuWriteBytes(const Label &src, uint8_t count) { return bblocks::ppuWriteBytes(sub_, src, count); }
    Subroutine &ppuWriteBytesZpPtr(ZpAddress ptr, uint8_t count) { return bblocks::ppuWriteBytesZpPtr(sub_, ptr, count); }
    Subroutine &ppuFill(uint8_t value, uint8_t count) { return bblocks::ppuFill(sub_, value, count); }
    Subroutine &memcpy(ZpAddress src, ZpAddress dst, uint16_t count, ZpAddress counter) { return bblocks::memcpy(sub_, src, dst, count, counter); }
    Subroutine &memset16(AbsAddress start, uint16_t value, uint16_t count, ZpAddress ptr, ZpAddress cnt, ZpAddress val) {
      return bblocks::memset16(sub_, start, value, count, ptr, cnt, val);
    }
    Subroutine &memset8(AbsAddress start, uint8_t value, uint16_t count, ZpAddress ptr, ZpAddress cnt) { return bblocks::memset8(sub_, start, value, count, ptr, cnt); }
    Subroutine &clearOAMBuffer(AbsAddress buffer = AbsAddress{ 0x0200 }) { return bblocks::clearOAMBuffer(sub_, buffer); }
    Subroutine &enableRendering(bool enable) { return bblocks::enableRendering(sub_, enable); }
    Subroutine &setPPUMaskBits(uint8_t bitsToSet, uint8_t bitsToClear) { return bblocks::setPPUMaskBits(sub_, bitsToSet, bitsToClear); }
    Subroutine &enableNMI() { return bblocks::enableNMI(sub_); }
    Subroutine &initPadCallback(ZpAddress buttons, std::function<void(Subroutine &, uint8_t)> callback) {
      return bblocks::initPadCallback(sub_, buttons, std::move(callback));
    }
  };

} // namespace cppnes