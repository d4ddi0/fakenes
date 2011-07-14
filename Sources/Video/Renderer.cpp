/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "Background.hpp"
#include "Local.hpp"
#include "Renderer.hpp"
#include "Sprites.hpp"
#include "Video.h"

#define INLINE_8EWR
#define INLINE_P2EC
#include "Background.cpp"
#include "Sprites.cpp"

// TODO: Emulate "rainbow palette" quirk.

namespace Renderer {

namespace {

void Clear()
{
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

const int DisplayWidth = PPU_RENDER_CLOCKS;			// 256
const int DisplayWidthTiles = DisplayWidth / TileWidth;		// (256 / 8) = 32
const int DisplayHeight = PPU_DISPLAY_LINES;			// 240
const int DisplayHeightTiles =  DisplayHeight / TileHeight;	// (240 / 8) = 30

RenderContext render;

void Initialize()
{
   Clear();

   Background::Initialize();
   Sprites::Initialize();
}

/* This gets called at the start of each frame, on PPU_FIRST_LINE, which
   is the dummy (non-visible) sprite evaluation line. */
void Frame()
{
   Clear();
 
   Background::Frame();
}

/* This only gets called from PPU_FIRST_VISIBLE_LINE to PPU_LAST_VISIBLE_LINE.
   In other words, only for visible lines. */
void Line(const int line)
{
   render.buffer = PPU__GET_LINE_ADDRESS(video_get_render_buffer(), line);

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
force_inline void Pixel()
{
   /* This is set if frame skipping is disabled, or when rendering has been forced
      (i.e for sprite #0 hitscan or lightgun emulation) */
   const bool rendering = ppu__enable_rendering || ppu__force_rendering;

   /* After loading a state, the render buffer is null and must be reloaded, and
      all previously rendered pixels are lost. This can causes momentary, but harmless
      flicker when first loading a save state. */
   if(rendering && !render.buffer)
      render.buffer = PPU__GET_LINE_ADDRESS(video_get_render_buffer(), render.line);

   // Check if the PPU is completely disabled.
   if(!ppu__enabled) {
      // Skip further processing.
      Background::PixelStub(rendering);
      render.pixel++;
      return;
   }

   // Check if the background is enabled.
   if(ppu__enable_background)
      Background::Pixel(rendering);
   else
      // When the background is disabled, we need to produce a backdrop pixel.
      Background::PixelStub(rendering);

   // Check if sprite rendering is enabled.
   if(ppu__enable_sprites)
      Sprites::Pixel(rendering);

   // Advance to the next pixel position.
   render.pixel++;
}

/* This is called for each clock cycle of every line, whether the PPU is within
   the rendering stage or not, except during vblank. The PPU idle line (240) also
   does not trigger this function. */
force_inline void Clock()
{
   // Clock the background and sprite generators.
   if(ppu__enable_background)
      Background::Clock();
   if(ppu__enable_sprites)
      Sprites::Clock();

   // Advance to the next clock cycle.
   render.clock++;
   render.isOddClock = !render.isOddClock;
}

#if !defined(INLINE_MA6R)

void Load(PACKFILE* file, const int version)
{
   RT_ASSERT(file);

   // General
   render.line = pack_igetw(file);
   render.pixel = pack_getc(file);

   render.clock = pack_igetw(file);
   render.isOddClock = LOAD_BOOLEAN(pack_getc(file));

   // Background
   RenderBackgroundContext& background = render.background;

   background.lowShift = pack_getc(file);
   background.lowFeed = pack_getc(file);
   background.highShift = pack_getc(file);
   background.highFeed = pack_getc(file);

   background.buffer = pack_getc(file);
   background.bufferTag = pack_getc(file);
   background.latch = pack_getc(file);
   background.latchTag = pack_getc(file);

   background.counter = pack_getc(file);

   // Background evaluation
   RenderBackgroundEvaluation& backgroundEvaluation = render.backgroundEvaluation;

   backgroundEvaluation.name = pack_getc(file);

   backgroundEvaluation.attribute = pack_getc(file);
   backgroundEvaluation.tag = pack_getc(file);

   backgroundEvaluation.pattern1 = pack_getc(file);
   backgroundEvaluation.pattern2 = pack_getc(file);

   backgroundEvaluation.row = pack_getc(file);

   // Sprites
   for(int i = 0; i < SpritesPerLine; i++) {
      RenderSpriteContext& sprite = render.sprites[i];

      sprite.index = pack_getc(file);

      sprite.lowShift = pack_getc(file);
      sprite.highShift = pack_getc(file);

      sprite.latch = pack_getc(file);

      sprite.counter = pack_getc(file);

      sprite.dead = LOAD_BOOLEAN(pack_getc(file));
   }

   // Sprite evaluation
   RenderSpriteEvaluation& spriteEvaluation = render.spriteEvaluation;

   for(int i = 0; i < SpritesPerLine; i++)
      spriteEvaluation.indices[i] = pack_getc(file);

   spriteEvaluation.state = pack_getc(file);
   spriteEvaluation.substate = pack_getc(file);

   spriteEvaluation.count = pack_getc(file);

   spriteEvaluation.n = pack_getc(file);
   spriteEvaluation.m = pack_getc(file);

   spriteEvaluation.locked = LOAD_BOOLEAN(pack_getc(file));

   spriteEvaluation.data = pack_getc(file);

   // OAM
   for(unsigned i = 0; i < SecondaryOAMSize; i++)
      render.secondaryOAM[i] = pack_getc(file);

   render.spriteCount = pack_getc(file);
}

void Save(PACKFILE* file, const int version)
{
   RT_ASSERT(file);

   // General
   pack_iputw(render.line, file);
   pack_putc(render.pixel, file);

   pack_iputw(render.clock, file);
   pack_putc(SAVE_BOOLEAN(render.isOddClock), file);

   // Background
   const RenderBackgroundContext& background = render.background;

   pack_putc(background.lowShift, file);
   pack_putc(background.lowFeed, file);
   pack_putc(background.highShift, file);
   pack_putc(background.highFeed, file);

   pack_putc(background.buffer, file);
   pack_putc(background.bufferTag, file);
   pack_putc(background.latch, file);
   pack_putc(background.latchTag, file);

   pack_putc(background.counter, file);

   // Background evaluation
   const RenderBackgroundEvaluation& backgroundEvaluation = render.backgroundEvaluation;

   pack_putc(backgroundEvaluation.name, file);

   pack_putc(backgroundEvaluation.attribute, file);
   pack_putc(backgroundEvaluation.tag, file);

   pack_putc(backgroundEvaluation.pattern1, file);
   pack_putc(backgroundEvaluation.pattern2, file);

   pack_putc(backgroundEvaluation.row, file);

   // Sprites
   for(int i = 0; i < SpritesPerLine; i++) {
      const RenderSpriteContext& sprite = render.sprites[i];

      pack_putc(sprite.index, file);

      pack_putc(sprite.lowShift, file);
      pack_putc(sprite.highShift, file);

      pack_putc(sprite.latch, file);

      pack_putc(sprite.counter, file);

      pack_putc(SAVE_BOOLEAN(sprite.dead), file);
   }

   // Sprite evaluation
   const RenderSpriteEvaluation& spriteEvaluation = render.spriteEvaluation;

   for(int i = 0; i < SpritesPerLine; i++)
      pack_putc(spriteEvaluation.indices[i], file);

   pack_putc(spriteEvaluation.state, file);
   pack_putc(spriteEvaluation.substate, file);

   pack_putc(spriteEvaluation.count, file);

   pack_putc(spriteEvaluation.n, file);
   pack_putc(spriteEvaluation.m, file);

   pack_putc(SAVE_BOOLEAN(spriteEvaluation.locked), file);

   pack_putc(spriteEvaluation.data, file);

   // OAM
   for(unsigned i = 0; i < SecondaryOAMSize; i++)
      pack_putc(render.secondaryOAM[i], file);

   pack_putc(render.spriteCount, file);

}

#endif // !INLINE_MA6R

} // namespace Renderer
