/* FakeNES - A free, portable, Open Source NES emulator.

   renderer.cpp: Declarations for the PPU renderer.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _RENDERER__RENDERER_HPP
#define _RENDERER__RENDERER_HPP
#include <allegro.h>
#include "../include/common.h"
#include "../include/types.h"

namespace Renderer {

// These have to be here, rather than in a file, due to compiler limitations.
static const int SpritesPerLine = 8;

static const int BytesPerSprite = 4;
static const unsigned SecondaryOAMSize = BytesPerSprite * SpritesPerLine;

extern const int TileWidth;
extern const int TileHeight;

extern const int DisplayWidth;
extern const int DisplayWidthTiles;
extern const int DisplayHeight;

typedef struct _RenderBackgroundContext {
   // Current tile and subtile positions.
   uint8 tile, pixel;

} RenderBackgroundContext;

typedef struct _RenderSpriteContext {
   uint8 lowShift, highShift;
   uint8 latch;
   uint8 counter;
   BOOL dead;

} RenderSpriteContext;

typedef struct _RenderEvaluationContext {
   uint8 state, substate;
   uint8 count;
   uint8 n, m;
   BOOL locked;
   uint8 data;

} RenderEvaluationContext;

typedef struct _RenderContext {
   uint8* buffer;
   int16 line;
   uint8 pixel;
   uint16 clock;

   uint8 secondaryOAM[SecondaryOAMSize];
   uint8 spriteCount;

   RenderBackgroundContext background;
   RenderSpriteContext sprites[SpritesPerLine];
   RenderEvaluationContext evaluation;

} RenderContext;

extern RenderContext render;

} // namespace Renderer

extern void Renderer_Initialize();
extern void Renderer_Frame();
extern void Renderer_Line(int line);
extern void Renderer_Pixel();
extern void Renderer_Clock();
extern void Renderer_Load(PACKFILE* file, const int version);
extern void Renderer_Save(PACKFILE* file, const int version);

#endif //!_RENDERER__RENDERER_HPP
