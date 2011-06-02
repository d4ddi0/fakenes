/* FakeNES - A free, portable, Open Source NES emulator.

   renderer.cpp: Implementation of the PPU renderer.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <cstdlib>
#include <cstring>
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

// TODO: Emulate "rainbow palette" quirk.

namespace Renderer {

namespace {

void Clear() {
   render.buffer = NULL;
   render.line = PPU_FIRST_LINE;
   render.pixel = 0;
   render.clock = 1; // Always starts from 1
   render.isOddClock = true;

   render.spriteCount = 0;
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
 
   Background::Frame();
}

/* This only gets called from PPU_FIRST_VISIBLE_LINE to PPU_LAST_VISIBLE_LINE.
   In other words, only for visible lines. */
void Line(const int line) {
   render.buffer = PPU__GET_LINE_ADDRESS(video_buffer, line);

   render.line = line;
   render.pixel = 0;
   render.clock = 1;
   render.isOddClock = true;

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
   if(ppu__enabled && !render.buffer)
      render.buffer = PPU__GET_LINE_ADDRESS(video_buffer, render.line);
 
   /* This is set if frame skipping is enabled, and rendering has not been forced
      (i.e for sprite #0 hitscan or lightgun emulation) */
   const bool stubify = !ppu__enable_rendering && !ppu__force_rendering;

   if(ppu__enable_background) {
      if(stubify)
         Background::PixelStub();
      else
         Background::Pixel();
   }
   else {
      // Clear rendering buffers
      R_ClearBackgroundPixel();
      R_ClearFramePixel();
   }

   if(ppu__enable_sprites) {
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
   if(ppu__enable_background)
      Background::Clock();
   if(ppu__enable_sprites)
      Sprites::Clock();

   render.clock++;
   render.isOddClock = !render.isOddClock;
}

#if !defined(INLINE_MA6R)

void Load(PACKFILE* file, const int version) {
   RT_ASSERT(file);

   // General
   render.line = pack_igetw(file);
   render.pixel = pack_getc(file);
   render.clock = pack_igetw(file);
   render.isOddClock = Boolean(pack_getc(file));

   // Background
   RenderBackgroundContext& background = render.background;

   // Sprites
   for(int i = 0; i < SpritesPerLine; i++) {
      RenderSpriteContext& sprite = render.sprites[i];
      sprite.index = pack_getc(file);
      sprite.lowShift = pack_getc(file);
      sprite.highShift = pack_getc(file);
      sprite.latch = pack_getc(file);
      sprite.counter = pack_getc(file);
      sprite.dead = Boolean(pack_getc(file));
   }

   // Sprite evaluation
   RenderSpriteEvaluation& e = render.spriteEvaluation;
   for(int i = 0; i < SpritesPerLine; i++)
      e.indices[i] = pack_getc(file);

   e.state = pack_getc(file);
   e.substate = pack_getc(file);
   e.count = pack_getc(file);
   e.n = pack_getc(file);
   e.m = pack_getc(file);
   e.locked = Boolean(pack_getc(file));
   e.data = pack_getc(file);

   // OAM
   for(unsigned i = 0; i < SecondaryOAMSize; i++)
      render.secondaryOAM[i] = pack_getc(file);

   render.spriteCount = pack_getc(file);
}

void Save(PACKFILE* file, const int version) {
   RT_ASSERT(file);

   // General
   pack_iputw(render.line, file);
   pack_putc(render.pixel, file);
   pack_iputw(render.clock, file);
   pack_putc(Binary(render.isOddClock), file);

   // Background
   const RenderBackgroundContext& background = render.background;

   // Sprites
   for(int i = 0; i < SpritesPerLine; i++) {
      const RenderSpriteContext& sprite = render.sprites[i];
      pack_putc(sprite.index, file);
      pack_putc(sprite.lowShift, file);
      pack_putc(sprite.highShift, file);
      pack_putc(sprite.latch, file);
      pack_putc(sprite.counter, file);
      pack_putc(Binary(sprite.dead), file);
   }

   // Sprite evaluation
   const RenderSpriteEvaluation& e = render.spriteEvaluation;
   for(int i = 0; i < SpritesPerLine; i++)
      pack_putc(e.indices[i], file);

   pack_putc(e.state, file);
   pack_putc(e.substate, file);
   pack_putc(e.count, file);
   pack_putc(e.n, file);
   pack_putc(e.m, file);
   pack_putc(Binary(e.locked), file);
   pack_putc(e.data, file);

   // OAM
   for(unsigned i = 0; i < SecondaryOAMSize; i++)
      pack_putc(render.secondaryOAM[i], file);

   pack_putc(render.spriteCount, file);

}

#endif // !INLINE_MA6R

} // namespace Renderer
