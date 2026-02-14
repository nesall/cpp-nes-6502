#include "nesdefs_helper.hpp"
#include "3rdparty/utils_log/logger.hpp"

cppnes::Subroutine &cppnes::bblocks::waitVBlank(Subroutine &sub)
{
  static int vblankCount = 0;
  Label wait("@vblank" + std::to_string(vblankCount++));
  sub.label(wait)
    .lda(abs(PPUSTATUS))
    .bpl(wait);
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::clearMemory(Subroutine &sub, AbsAddress start, uint8_t length)
{
  static int clearCount = 0;
  Label loop("@clearLoop" + std::to_string(clearCount++));
  sub.lda(immZero)
    .tax()
    .label(loop)
    .sta(absx(start))
    .inx()
    .cpx(imm(length))
    .bne(loop);
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::clearMemory(Subroutine &sub,
  AbsAddress start,
  uint16_t length,
  ZpAddress ptr,
  ZpAddress count)
{
  static int clearCount = 0;
  int n = clearCount++;
  Label loop("@clearLoop" + std::to_string(n));

  sub
    // ptr = start
    .lda(imm(start.value() & 0xFF))
    .sta(zp(ptr))
    .lda(imm((start.value() >> 8) & 0xFF))
    .sta(zp(ptr + 1))
    // count = length
    .lda(imm(length & 0xFF))
    .sta(zp(count))
    .lda(imm((length >> 8) & 0xFF))
    .sta(zp(count + 1))
    .lda(immZero)
    .ldy(immZero)
    .label(loop)
    .sta(indy(ptr))        // STA (ptr),Y
    // Advance ptr
    .inc(zp(ptr))
    .bne(Label(loop.name() + "_skip"))
    .inc(zp(ptr + 1))
    .label(Label(loop.name() + "_skip"))
    // 16-bit Decrement count
    .lda(zp(count))
    .bne(Label(loop.name() + "_dec_hi"))
    .dec(zp(count + 1))
    .label(Label(loop.name() + "_dec_hi"))
    .dec(zp(count))
    // Check if count != 0
    .lda(zp(count))
    .ora(zp(count + 1))
    .bne(loop);

  return sub;
}

/*
Used for:
  - Clearing OAM shadow ($0200)
  - Clearing a RAM page
  - Stack page if needed
  - Nametable buffers in CPU RAM
*/
cppnes::Subroutine &cppnes::bblocks::clearPage(Subroutine &sub, AbsAddress start)
{
  assert((start.value() & 0x00FF) == 0 && "clearPage requires page-aligned address");
  if ((start.value() & 0xFF) != 0)
    throw std::invalid_argument("clearPage requires page-aligned address");
  static int clearCount = 0;
  Label loop("@clearPage" + std::to_string(clearCount++));
  sub
    .lda(immZero)          // A = 0
    .ldx(immZero)          // X = 0
    .label(loop)
    .sta(absx(start))      // STA start,X
    .inx()
    .bne(loop);            // loop until X wraps to 0
  return sub;
}

/*
memset16: Write 16-bit value repeatedly to memory
Uses Y as per-page counter, increments high bytes only on page wrap.

ZpAddress ptr(0x00);   // uses $00,$01
ZpAddress cnt(0x02);   // uses $02,$03
ZpAddress val(0x04);   // uses $04,$05
bblocks::memset16(
    sub,
    AbsAddress(0x0300),  // start
    0x3F3F,              // value
    64,                  // word count
    ptr,
    cnt, val);
*/
cppnes::Subroutine &cppnes::bblocks::memset16(Subroutine &sub,
  AbsAddress start,
  uint16_t value,
  uint16_t count,
  ZpAddress ptr,
  ZpAddress cnt,
  ZpAddress val)
{
  static int id = 0;
  int n = id++;

  Label loop("@memset16_" + std::to_string(n));

  sub
    // ptr = start
    .lda(imm(start.value() & 0xFF))
    .sta(zp(ptr))
    .lda(imm((start.value() >> 8) & 0xFF))
    .sta(zp(ptr + 1))

    // cnt = count
    .lda(imm(count & 0xFF))
    .sta(zp(cnt))
    .lda(imm((count >> 8) & 0xFF))
    .sta(zp(cnt + 1))

    // val = value
    .lda(imm(value & 0xFF))
    .sta(zp(val))
    .lda(imm((value >> 8) & 0xFF))
    .sta(zp(val + 1))

    .ldy(immZero)

    .label(loop)
    // Write low byte
    .lda(zp(val))
    .sta(indy(ptr))
    .iny()

    // Write high byte
    .lda(zp(val + 1))
    .sta(indy(ptr))
    .iny()
    .bne(loop)  // loop while Y < 256

    // Next page
    .inc(zp(ptr + 1))

    // Decrement count (16-bit)
    .dec(zp(cnt))
    .bne(loop)
    .dec(zp(cnt + 1))
    .bne(loop);

  return sub;
}

cppnes::Subroutine &cppnes::bblocks::memset8(Subroutine &sub, AbsAddress start, uint8_t value, uint16_t count, ZpAddress ptr, ZpAddress cnt)
{
  static int id = 0;
  int n = id++;
  Label loop("@memset8_" + std::to_string(n));
  Label incptr("@memset8_incptr_" + std::to_string(n));

  sub
    // ptr = start
    .lda(imm(start.value() & 0xFF))
    .sta(zp(ptr))
    .lda(imm((start.value() >> 8) & 0xFF))
    .sta(zp(ptr + 1))

    // cnt = count
    .lda(imm(count & 0xFF))
    .sta(zp(cnt))
    .lda(imm((count >> 8) & 0xFF))
    .sta(zp(cnt + 1))

    // A = value
    .lda(imm(value))
    .ldy(immZero)

    .label(loop)
    .sta(indy(ptr))        // store value at (ptr),Y

    // advance ptr
    .inc(zp(ptr))
    .bne(incptr)
    .inc(zp(ptr + 1))
    .label(incptr)

    // decrement count
    .dec(zp(cnt))
    .bne(loop)
    .dec(zp(cnt + 1))
    .bne(loop);

  return sub;
}

cppnes::Subroutine &cppnes::bblocks::clearOAMBuffer(Subroutine &sub, AbsAddress buffer)
{
  // OAM buffer is 256 bytes, must be page-aligned
  assert((buffer.value() & 0xFF) == 0 && "OAM buffer must be page-aligned");
  if ((buffer.value() & 0xFF) != 0)
    throw std::invalid_argument("OAM buffer must be page-aligned");
  static int clearCount = 0;
  Label loop("@clearOAM_" + std::to_string(clearCount++));
  sub
    .lda(immZero)
    .ldx(immZero)
    .label(loop)
    .sta(absx(buffer))
    .inx()
    .bne(loop); // loops until X wraps to 0 (256 bytes)
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::loadPalette(Subroutine &sub, const Label &dataLabel, uint8_t mask)
{
  static int palCount = 0;
  Label loop("@loadPalLoop" + std::to_string(palCount++));
  setPPUAddr(sub, 0x3f00)
    .ldx(immZero)
    .label(loop)
    .lda(absx(dataLabel))
    .sta(abs(PPUDATA))
    .inx()
    .cpx(imm(0x20))  // 32 bytes total (16 bg + 16 sprite)
    .bne(loop)
    .lda(imm(mask))
    .sta(abs(PPUMASK));
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::loopX(Subroutine &sub, uint8_t count, std::function<void(Subroutine &)> body)
{
  if (count == 0) return sub;
  static int loopCount = 0;
  Label loop("@loop" + std::to_string(loopCount++));
  sub.ldx(imm(count))
    .label(loop);
  body(sub);  // user code inside loop
  sub.dex()
    .bne(loop);
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::uploadSprites(Subroutine &sub, AbsAddress oamBuffer)
{
  return sub
    .bblocks().setAddrByte(OAMADDR, 0)
    .bblocks().setAddrByte(OAMDMA, oamBuffer.value() >> 8); // high byte of $0200 = $02
  ;
}

cppnes::Subroutine &cppnes::bblocks::setPPUAddr(Subroutine &sub, uint16_t addr)
{
  sub
    .lda(abs(PPUSTATUS)) // reset latch
    .bblocks().setAddrByte(PPUADDR, addr >> 8)
    .bblocks().setAddrByte(PPUADDR, addr & 0xFF);
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::readController(Subroutine &sub, ZpAddress buttons, ZpAddress buttonsPrev, ZpAddress buttonsPressed, ZpAddress buttonsReleased)
{
  static int readCount = 0;
  Label loop("@readButtonStates" + std::to_string(readCount++));
  sub
    .comment("save previous")
    .lda(zp(buttons))
    .sta(zp(buttonsPrev))
    .comment("strobe")
    //.lda(imm(1))
    //.sta(abs(JOY1))
    .bblocks().setAddrByte(JOY1, 1)
    //.lda(imm(0))
    //.sta(abs(JOY1))
    .bblocks().setAddrByte(JOY1, 0)
    .comment("read 8 buttons")
    .ldx(imm(0x08))
    .lda(imm(0))
    .label(loop)
    .lda(abs(JOY1))
    .lsr()
    .rol(zp(buttons))
    .dex()
    .bne(loop)
    .comment("buttonsPressed = current & (~previous)")
    .lda(zp(buttonsPrev))
    .eor(imm(0xff))
    .and_(zp(buttons))
    .sta(zp(buttonsPressed))
    .comment("buttonsReleased = previous & (~current)")
    .lda(zp(buttons))
    .eor(imm(0xff))
    .and_(zp(buttonsPrev))
    .sta(zp(buttonsReleased))
    ;
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::setAddrByte(Subroutine &sub, ZpAddress addr, uint8_t b)
{
  return sub
    .lda(imm(b))
    .sta(zp(addr));
}

cppnes::Subroutine &cppnes::bblocks::setAddrByte(Subroutine &sub, AbsAddress addr, uint8_t b)
{
  return sub
    .lda(imm(b))
    .sta(abs(addr));
}

cppnes::Subroutine &cppnes::bblocks::setAddrByte(Subroutine &sub, ZpAddress addr, ZpAddress b)
{
  return sub
    .lda(zp(b))
    .sta(zp(addr));
}

cppnes::Subroutine &cppnes::bblocks::setAddrWord(Subroutine &sub, AbsAddress addr, uint16_t w)
{
  if (PPUADDR == addr) {
    LOG_MSG << "bblocks::setAddrWord: PPU register - write both bytes to the same address";
    sub
      .lda(abs(PPUSTATUS)) // reset latch
      //.lda(imm(w >> 8))
      //.sta(abs(addr))
      .bblocks().setAddrByte(addr, w >> 8)
      //.lda(imm(w & 0xFF))
      //.sta(abs(addr));
      .bblocks().setAddrByte(addr, w & 0xFF);
  } else {
    // Standard little-endian RAM layout
    sub
      //.lda(imm(w & 0xFF))
      //.sta(abs(addr))
      .bblocks().setAddrByte(addr, w & 0xFF)
      //.lda(imm((w >> 8) & 0xFF))
      //.sta(abs(addr + 1));
      .bblocks().setAddrByte(addr + 1, (w >> 8) & 0xFF);
  }
  return sub;
}

/*
Used for:
  - Nametables
  - Attribute tables
  - Pattern updates
  - Partial VRAM updates
*/
cppnes::Subroutine &cppnes::bblocks::ppuWriteBytes(Subroutine &sub, const Label &src, uint8_t count)
{
  static int id = 0;
  int n = id++;
  Label loop("@ppuWriteBytes_" + std::to_string(n));
  sub
    .ldx(immZero)
    .label(loop)
    .lda(absx(src))
    .sta(abs(PPUDATA))
    .inx()
    .cpx(imm(count))
    .bne(loop);

  return sub;
}

/*
Runtime pointer version. Needed for dynamic updates.
*/
cppnes::Subroutine &cppnes::bblocks::ppuWriteBytesZpPtr(Subroutine &sub, ZpAddress ptr, uint8_t count)
{
  static int id = 0;
  Label loop("@ppuWriteBytesZpPtr_" + std::to_string(id++));
  sub
    .ldy(immZero)
    .label(loop)
    .lda(indy(ptr))
    .sta(abs(PPUDATA))
    .iny()
    .cpy(imm(count))
    .bne(loop);
  return sub;
}

/*
Fill VRAM with a byte
*/
cppnes::Subroutine &cppnes::bblocks::ppuFill(Subroutine &sub, uint8_t value, uint8_t count)
{
  static int id = 0;
  int n = id++;
  Label loop("@ppuFill_" + std::to_string(n));
  sub
    .lda(imm(value))
    .ldx(immZero)
    .label(loop)
    .sta(abs(PPUDATA))
    .inx()
    .cpx(imm(count))
    .bne(loop);

  return sub;
}

/*
memcpy: Copy memory block using indirect indexed addressing
Uses Y as per-page counter, increments high bytes only on page wrap
Used for:
  - OAM buffer updates
  - Shadow buffer copies
  - Decompression staging
*/
cppnes::Subroutine &cppnes::bblocks::memcpy(Subroutine &sub,
  ZpAddress src,
  ZpAddress dst,
  uint16_t count,
  ZpAddress counter)
{
  static int id = 0;
  int n = id++;
  Label loop("@memcpy_" + std::to_string(n));

  sub
    // Set up counter (16-bit)
    .lda(imm(count & 0xFF))
    .sta(zp(counter))
    .lda(imm((count >> 8) & 0xFF))
    .sta(zp(counter + 1))

    .ldy(immZero)

    .label(loop)
    // Load from (src),Y
    .lda(indy(src))
    // Store to (dst),Y
    .sta(indy(dst))
    // Increment Y
    .iny()
    .bne(loop)  // loop while Y < 256

    // Next page
    .inc(zp(src + 1))
    .inc(zp(dst + 1))

    // Decrement counter (16-bit)
    .dec(zp(counter))
    .bne(loop)
    .dec(zp(counter + 1))
    .bne(loop);

  return sub;
}

cppnes::Subroutine &cppnes::bblocks::enableRendering(Subroutine &sub, bool enable)
{
  // Bits 3 & 4 = BG & sprites
  //0x18 = 00011000
  //0xE7 = 11100111
  if (enable) {
    bblocks::setPPUMaskBits(sub, 0x18, 0x00);
    sub.commentPrev("disable rendereing");
  } else {
    bblocks::setPPUMaskBits(sub, 0x00, 0x18);
    sub.commentPrev("enable rendereing");
  }
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::setPPUMaskBits(Subroutine &sub, uint8_t bitsToSet, uint8_t bitsToClear)
{
  sub.lda(abs(PPUMASK))
    .and_(imm(static_cast<uint8_t>(~bitsToClear)))
    .ora(imm(bitsToSet))
    .sta(abs(PPUMASK));
  return sub;
}

cppnes::Subroutine &cppnes::bblocks::enableNMI(Subroutine &sub)
{
  return setAddrByte(sub, PPUCTRL, 0b10000000);
}

cppnes::Subroutine &cppnes::bblocks::initPadCallback(Subroutine &sub, ZpAddress buttons, std::function<void(Subroutine &, uint8_t)> callback)
{
  assert(callback);
  sub
    .lda(zp(buttons))
    .and_(imm(BTN_UP))
    .beq("@not_up");
  callback(sub, BTN_UP);
  sub.label("@not_up")

    .lda(zp(buttons))
    .and_(imm(BTN_DOWN))
    .beq("@not_down");
  callback(sub, BTN_DOWN);
  sub.label("@not_down")

    .lda(zp(buttons))
    .and_(imm(BTN_LEFT))
    .beq("@not_left");
  callback(sub, BTN_LEFT);
  sub.label("@not_left")

    .lda(zp(buttons))
    .and_(imm(BTN_RIGHT))
    .beq("@not_right");
  callback(sub, BTN_RIGHT);
  sub.label("@not_right")

    .lda(zp(buttons))
    .and_(imm(BTN_A))
    .beq("@not_a");
  callback(sub, BTN_A);
  sub.label("@not_a")

    .lda(zp(buttons))
    .and_(imm(BTN_B))
    .beq("@not_b");
  callback(sub, BTN_B);
  sub.label("@not_b")

    .lda(zp(buttons))
    .and_(imm(BTN_SELECT))
    .beq("@not_select");
  callback(sub, BTN_SELECT);
  sub.label("@not_select")

    .lda(zp(buttons))
    .and_(imm(BTN_START))
    .beq("@not_start");
  callback(sub, BTN_START);
  sub.label("@not_start")
    ;

  return sub;
}

