/* FakeNES - A free, portable, Open Source NES emulator.

   renderer.cpp: Declarations for the PPU renderer.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _RENDERER__RENDERER_HPP
#define _RENDERER__RENDERER_HPP
#include <allegro.h>
#include "../include/common.h"
#include "../include/ppu.h"
#include "../include/ppu_int.h"
#include "../include/types.h"

/* This macro helps with creating scanline-length arrays for storing the results of calculations
   for use on a per-clock basis to improve performance. */
#define R_LookupTable(_name) \
   int _name[1 + PPU_SCANLINE_CLOCKS]

// This macro helps with clearing a lookup table before use.
#define R_ClearLookupTable(_name) { \
   for(int _i = 0; _i < 1 + PPU_SCANLINE_CLOCKS; _i++) \
      _name[_i] = 0; \
}

// These macros write to the special background scanline buffer.
#define R_GetBackgroundPixel(_pixel) \
   ( ppu__background_pixels[render.pixel] )
#define R_PutBackgroundPixel(_pixel) \
   ( ppu__background_pixels[render.pixel] = (_pixel) )
#define R_ClearBackgroundPixel() \
   ( R_PutBackgroundPixel(0) )

// These macros write to the framebuffer.
#define R_PutFramePixel(_pixel) \
   ( render.buffer[render.pixel] = (_pixel) )
#define R_ClearFramePixel() \
   ( R_PutFramePixel(PPU__BACKGROUND_COLOR) )

// --------------------------------------------------------------------------------

namespace Renderer {

// These have to be here, rather than in a file, due to compiler limitations.
static const int SpritesPerLine = 8;
static const int BytesPerSprite = PPU__BYTES_PER_SPRITE;
static const unsigned SecondaryOAMSize = BytesPerSprite * SpritesPerLine;

extern const int TileWidth;
extern const int TileHeight;
extern const int DisplayWidth;
extern const int DisplayWidthTiles;
extern const int DisplayHeight;
extern const int DisplayHeightTiles;

typedef struct _RenderBackgroundContext {
   uint8 lowShift, highShift;
   uint8 lowFeed, highFeed;
   uint8 buffer, bufferTag;
   uint8 latch, latchTag;
   uint8 counter;

} RenderBackgroundContext;

typedef struct _RenderBackgroundEvaluation {
   uint8 name;
   uint8 attribute, tag;
   uint8 pattern1, pattern2;
   uint8 row;

} RenderBackgroundEvaluation;

typedef struct _RenderSpriteContext {
   uint8 index;
   uint8 lowShift, highShift;
   uint8 latch;
   uint8 counter;
   bool dead;

} RenderSpriteContext;

typedef struct _RenderSpriteEvaluation {
   uint8 indices[SpritesPerLine];
   uint8 state, substate;
   uint8 count;
   uint8 n, m;
   BOOL locked;
   uint8 data;

} RenderSpriteEvaluation;

typedef struct _RenderContext {
   uint8* buffer;
   int16 line;
   uint8 pixel;
   uint16 clock;
   bool isOddClock;

   RenderBackgroundContext background;
   RenderBackgroundEvaluation backgroundEvaluation;

   RenderSpriteContext sprites[SpritesPerLine];
   RenderSpriteEvaluation spriteEvaluation;
   uint8 secondaryOAM[SecondaryOAMSize];
   uint8 spriteCount;

} RenderContext;

extern RenderContext render;

// --------------------------------------------------------------------------------

extern void Initialize();
extern void Frame();
extern void Line(const int line);
extern void Pixel();
extern void Clock();
extern void Load(PACKFILE* file, const int version);
extern void Save(PACKFILE* file, const int version);

} // namespace Renderer

#endif //!_RENDERER__RENDERER_HPP
