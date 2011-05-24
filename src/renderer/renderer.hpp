/* FakeNES - A free, portable, Open Source NES emulator.

   renderer.cpp: Declarations for the PPU renderer.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _RENDERER__RENDERER_HPP
#define _RENDERER__RENDERER_HPP
#include "../include/common.h"

// Start of the NES palette in the global 256-color palette.
#define PALETTE_ADJUST 1

namespace Renderer {

extern const int TileWidth;
extern const int TileHeight;

extern const int DisplayWidth;
extern const int DisplayWidthTiles;
extern const int DisplayWidthTilesBuffered;
extern const int DisplayHeight;

typedef struct _RenderBackgroundContext {
   // Current tile and subtile positions.
   int tile, pixel;

} RenderBackgroundContext;

typedef struct _RenderSpriteContext {

} RenderSpriteContext;

typedef struct _RenderContext {
   uint8* buffer;
   int line, pixel;

   RenderBackgroundContext background;
   RenderSpriteContext sprite;

} RenderContext;

extern RenderContext render;

} // namespace Renderer

extern void Renderer_Initialize();
extern void Renderer_Frame();
extern void Renderer_Line(int line);
extern void Renderer_Pixel();

#endif //!_RENDERER__RENDERER_HPP
