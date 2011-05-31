/* FakeNES - A free, portable, Open Source NES emulator.

   background.cpp: Implementation of the PPU background renderer.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include "../include/common.h"
#include "../include/mmc.h"
#include "../include/ppu.h"
#include "../include/ppu_int.h"
#include "../include/types.h"
#include "background.hpp"
#include "renderer.hpp"

namespace Renderer {
namespace Background {

namespace {

// Amount of space a tile consumes in CHR (at 4 pixels per byte).
const int BytesPerTile = (TileWidth * TileHeight) / 4;

// Offset of attributes in nametables, and associated mask.
const unsigned AttributeBase = (DisplayWidth / TileWidth) * (DisplayHeight / TileHeight);
const unsigned AttributeMask = 0x03;

// Shifts and mask for extended attributes in MMC5 ExRAM.
const int ExpansionAttributeShifts = 6;
const unsigned ExpansionAttributeMask = 0x03;

void Clear() {
   render.background.tile = 0;
   render.background.pixel = 0;
}

inline void Step()
{
   RenderBackgroundContext& current = render.background;

   // Proceed to the next pixel of the current tile.
   current.pixel++;
   if(current.pixel >= TileWidth) {
      // Done with this tile, on to the next one.
      current.tile++;
      current.pixel = 0;
   }
}

} // namespace anonymous

// ----------------------------------------------------------------------
// PUBLIC INTERFACE
// ----------------------------------------------------------------------

#if !defined(INLINE_8EWR)

void Initialize()
{
   Clear();
}

void Line()
{
   Clear();
}

#endif

inline void Pixel()
{
#if 0
   if(transparent) {
      PPU__PUT_BACKGROUND_PIXEL(render.pixel, 0);
      render.buffer[render.pixel] = PPU__BACKGROUND_PALETTE(0);
   }
   else {
      PPU__PUT_BACKGROUND_PIXEL(render.pixel, color);
      render.buffer[render.pixel] = ppu_enable_background_layer ?
         PPU__BACKGROUND_PALETTE(color) : PPU__BACKGROUND_PALETTE(0);
   }
#endif

   // Move on to the next pixel
   Step();
}

// Frame-skipping variant of BackgroundPixel()
inline void PixelStub()
{
#if 0
   /* As nothing is being rendered, produce a transparent pixel. The video
      buffer is not updated this time, as we are frame-skipping. */
   PPU__PUT_BACKGROUND_PIXEL(render.pixel, 0);
#endif

   // Move on to the next pixel
   Step();
}

/* PPU sleep mode - just do minimal processing, do not draw any pixels
   or affect any registers. */
inline void PixelSkip() {
   Step();
}

} // namespace Background
} // namespace Renderer
