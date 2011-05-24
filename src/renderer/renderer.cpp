/* FakeNES - A free, portable, Open Source NES emulator.

   renderer.cpp: Implementation of the PPU renderer.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <cstring>
#include <cstdlib>
#include "../include/common.h"
#include "../include/mmc.h"
#include "../include/ppu.h"
#include "../include/ppu_int.h"
#include "../include/types.h"
#include "../include/video.h"
#include "background.hpp"
#include "renderer.hpp"
#include "sprites.hpp"

namespace Renderer {

const int TileWidth = 8;
const int TileHeight = 8;

const int DisplayWidth = 256;
const int DisplayWidthTiles = (DisplayWidth / TileWidth);
const int DisplayWidthTilesBuffered = DisplayWidthTiles + 1;
const int DisplayHeight = 240;

RenderContext render;

} // namespace Renderer

using namespace Renderer;

void Renderer_Initialize() {
   render.buffer = NULL;
   render.line = 0;
   render.pixel = 0;

   BackgroundInit();
   SpriteInit();
}

void Renderer_Frame() {
}

void Renderer_Line(int line) {
   render.buffer = PPU_GET_LINE_ADDRESS(video_buffer, line);
   render.line = line;
   render.pixel = 0;

   // TODO: Move this to a per-pixel basis.
   memset(render.buffer, (ppu_background_palette[0] & palette_mask) + PALETTE_ADJUST, DisplayWidth);
   memset(background_pixels + (1 * TileWidth), 0, DisplayWidth);

   BackgroundLine();
   SpriteLine();
}

void Renderer_Pixel() {
   if(!render.buffer) {
      /* After loading a state, the render buffer is null and must be reloaded, and
         all previously rendered pixels are lost. This can causes momentary, but harmless
         flicker when first loading a save state. */
      render.buffer = PPU_GET_LINE_ADDRESS(video_buffer, render.line);
   }

   if(PPU_ENABLED) {
      /* This is set if frame skipping is enabled, and rendering has not been forced
         (i.e for sprite #0 hitscan or lightgun emulation) */
      const bool quick = !ppu__rendering_enabled && !ppu__force_rendering;
      if(PPU_BACKGROUND_ENABLED && !quick)
         BackgroundPixel();
      else
         BackgroundPixelStub();

      if(PPU_SPRITES_ENABLED && !quick)
         SpritePixel();
      else
         SpritePixelStub();
   }
   else {
      // When the PPU is disabled, we don't do anything at all
      BackgroundPixelSkip();
      SpritePixelSkip();
   }

   render.pixel++;
}

void Renderer_Load(PACKFILE* file, const int version) {
   RT_ASSERT(file);

   pack_putc(render.line, file);
   pack_putc(render.pixel, file);

   pack_putc(render.background.tile, file);
   pack_putc(render.background.pixel, file);
}

void Renderer_Save(PACKFILE* file, const int version) {
   RT_ASSERT(file);

   render.line = pack_getc(file);
   render.pixel = pack_getc(file);

   render.background.tile = pack_getc(file);
   render.background.pixel = pack_getc(file);
}
