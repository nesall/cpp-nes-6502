# NES Background Rendering and Level Storage Overview

---

## 1. Nametables
- A nametable is a **1KB region in PPU VRAM** defining a 32×30 tile layout.
- It stores **tile indices** that reference CHR pattern tables.
- Full screen size:
  - 960 bytes: tile indices (32×30)
  - 64 bytes: attribute table
  - **Total = 1024 bytes**

---

## 2. Switching Between Title and Game Screens
- You **do not need multiple VRAM nametables**.
- Store separate layouts in PRG ROM and **copy the desired one into VRAM** during vblank.
- Typical steps:
  1. Wait for vblank
  2. Disable rendering
  3. Copy nametable + attributes
  4. Re-enable rendering

---

## 3. Metatile-Based Level Storage
- Instead of storing full 960-tile screens, games use **16×16 metatiles** (each = 2×2 tiles).
- Screen in metatiles:
  - 16 columns × 15 rows = 240 bytes
- Saves significant PRG ROM space.

---

## 4. Metatile Expansion
- Each metatile expands into **4 tile indices**:
  - Top-Left, Top-Right, Bottom-Left, Bottom-Right
- Process for rendering:
  1. Read metatile ID from level array
  2. Lookup tile indices in MetatileTable
  3. Compute nametable address
  4. Write 4 tiles to VRAM via `$2006/$2007`

---

## 5. Nametable Address Formula
- For metatile coordinates `(mx, my)`:
  - `tx = mx * 2`
  - `ty = my * 2`
- Nametable address = `$2000 + (ty * 32) + tx`
- Bottom row of the metatile = address + 32

---

## 6. Why This Matters
- Full screen per level: 1024 bytes
- Metatile-based screen: ~240 bytes
- With RLE compression: often <150 bytes
- **Major PRG ROM savings**

---

## 7. Optional: Attribute Table Handling
- Each 16×16 metatile fits inside an **attribute quadrant**
- Metatile definitions can include **palette ID**
- Attribute bytes can be built while expanding metatiles to VRAM

---

## Metatile Example

Suppose your CHR pattern table has these tiles:
```
Tile 0: sky top-left
Tile 1: sky top-right
Tile 2: sky bottom-left
Tile 3: sky bottom-right
Tile 4: brick top-left
Tile 5: brick top-right
Tile 6: brick bottom-left
Tile 7: brick bottom-right
```

A metatile table could be:
```
; id 0 = sky block
.byte $00, $01, $02, $03
; id 1 = brick block
.byte $04, $05, $06, $07

```

Then the level array just stores metatile IDs:
```
Level1:
.byte 0,0,1,1,0,0,...
```

### Basic Concept

- NES tiles are 8×8 pixels.
- Metatile = usually 2×2 tiles (16×16 pixels), sometimes larger.
- A metatile stores:
  - The tile indices for its constituent tiles (top-left, top-right, bottom-left, bottom-right)
  - Palette info (often 2 bits, since NES palettes are 4 colors)
  
```
Metatile ID = 1 (Brick Block)
16x16 pixels = 2x2 tiles
+-------------------+
| TL | TR           |  <- Top row of tiles
|----+----|
| BL | BR           |  <- Bottom row of tiles
+-------------------+

Tile indices in CHR pattern table:
---------------------------
| Index | Description     |
|-------|----------------|
| $04   | Top-Left brick  |
| $05   | Top-Right brick |
| $06   | Bottom-Left     |
| $07   | Bottom-Right    |

Level array stores:
[1, 1, 0, 2,...]   ; 1 = this metatile

When rendering:
- Game looks up ID 1 in MetatileTable
- Writes $04 → nametable at (tx, ty)
- Writes $05 → nametable at (tx+1, ty)
- Writes $06 → nametable at (tx, ty+1)
- Writes $07 → nametable at (tx+1, ty+1)
```

How it ties to CHR:
```
CHR memory (pattern table $0000):
+----+----+----+----+----+----+----+----+
| $00 | $01 | $02 | $03 | $04 | $05 | $06 | $07 | ...
+----+----+----+----+----+----+----+----+
Tiles 0-3: sky
Tiles 4-7: brick
```


## Top down structure of a NES screen

```
SCREEN (256×240 pixels)
  ↓
NAMETABLE (32×30 tiles)
  - Each entry: index to a tile in CHR-ROM (0-255)
  - Size: 32×30 = 960 bytes
  ↓
ATTRIBUTE TABLE (16×16 blocks, packed into 64 bytes)
  - Each 16×16 block (2×2 tiles) → 2-bit palette selection (0-3)
  - Size: 64 bytes
  ↓
CHR-ROM (tile data)
  - Each tile: 8×8 pixels, 2-bit color (4 colors per tile)
  - NES format: 16 bytes per tile (2 bitplanes, 8 bytes each)
  - Max 256 tiles = 4096 bytes
  ↓
PALETTES (4 background palettes)
  - Each palette: 4 colors (indices into NES master palette 0-63)
  - Total: 4×4 = 16 bytes

 ```