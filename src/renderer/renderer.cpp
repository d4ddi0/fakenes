/* FakeNES - A free, portable, Open Source NES emulator.

   renderer.cpp: Implementation of the PPU renderer.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include "../include/common.h"
#include "../include/ppu.h"
#include "../include/ppu_int.h"
#include "../include/types.h"
#include "../include/video.h"
#include "background.hpp"
#include "renderer.hpp"
#include "sprites.hpp"

#define INLINE_8EWR
#define INLINE_P2EC
#include "background.cpp"
#include "sprites.cpp"

namespace Renderer {

namespace {

void Clear() {
   render.buffer = NULL;
   render.line = PPU_FIRST_LINE;
   render.pixel = 0;
   render.clock = 1; // Always starts from 1
}

} // namespace anonymous

// ----------------------------------------------------------------------
// PUBLIC INTERFACE
// ----------------------------------------------------------------------

#if !defined(INLINE_MA6R)

const int TileWidth = 8;
const int TileHeight = 8;

const int DisplayWidth = PPU_RENDER_CLOCKS;		// 256
const int DisplayWidthTiles = DisplayWidth / TileWidth;	// (256 / 8) = 32
const int DisplayHeight = PPU_DISPLAY_LINES;		// 240

RenderContext render;

void Initialize() {
   Clear();

   Background::Initialize();
   Sprites::Initialize();
}

/* This gets called at the start of each frame, on PPU_FIRST_LINE, which
   is the dummy (non-visible) sprite evaluation line. */
void Frame() {
   Clear();
}

/* This only gets called from PPU_FIRST_VISIBLE_LINE to PPU_LAST_VISIBLE_LINE.
   In other words, only for visible lines. */
void Line(const int line) {
   render.buffer = PPU_GET_LINE_ADDRESS(video_buffer, line);

   render.line = line;
   render.pixel = 0;
   render.clock = 1;

   Background::Line();
   Sprites::Line();
}

#endif // !INLINE_MA6R

/* Like Render_Line(), this is called only for visible lines, for the first 256
   clock cycles which equate to a single pixel each. */
inline void Pixel() {
   /* After loading a state, the render buffer is null and must be reloaded, and
      all previously rendered pixels are lost. This can causes momentary, but harmless
      flicker when first loading a save state. */
   if(PPU_ENABLED && !render.buffer)
      render.buffer = PPU_GET_LINE_ADDRESS(video_buffer, render.line);
 
   /* This is set if frame skipping is enabled, and rendering has not been forced
      (i.e for sprite #0 hitscan or lightgun emulation) */
   const bool stubify = !ppu__rendering_enabled && !ppu__force_rendering;

   if(PPU_BACKGROUND_ENABLED) {
      if(stubify)
         Background::PixelStub();
      else
         Background::Pixel();
   }
   else {
      // Clear framebuffer
      PPU__PUT_BACKGROUND_PIXEL(render.pixel, 0);
      render.buffer[render.pixel] = PPU__BACKGROUND_PALETTE(0);

      // Advance raster position without affecting anything else
      Background::PixelSkip();
   }

   if(PPU_SPRITES_ENABLED) {
      if(stubify)
         Sprites::PixelStub();
      else
         Sprites::Pixel();
   }

   render.pixel++;
}

/* This is called for each clock cycle of every line, whether the PPU is within
   the rendering stage or not, except during vblank. The PPU idle line (240) also
   does not trigger this function. */
inline void Clock() {
   if(PPU_SPRITES_ENABLED)
      Sprites::Clock();

   render.clock++;
}

#if !defined(INLINE_MA6R)

void Load(PACKFILE* file, const int version) {
   RT_ASSERT(file);

   // Renderer
   pack_putc(render.line, file);
   pack_putc(render.pixel, file);
   pack_iputw(render.clock, file);

   // Background
   pack_putc(render.background.tile, file);
   pack_putc(render.background.pixel, file);
}

void Save(PACKFILE* file, const int version) {
   RT_ASSERT(file);

   // Renderer
   render.line = pack_getc(file);
   render.pixel = pack_getc(file);
   render.clock = pack_igetw(file);

   // Background
   render.background.tile = pack_getc(file);
   render.background.pixel = pack_getc(file);
}

#endif // !INLINE_MA6R

} // namespace Renderer
