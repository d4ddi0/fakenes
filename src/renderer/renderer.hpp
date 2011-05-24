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
   uint8 tile, pixel; // Saved to file

} RenderBackgroundContext;

typedef struct _RenderSpriteContext {

} RenderSpriteContext;

typedef struct _RenderContext {
   uint8* buffer;
   uint8 line, pixel; // Saved to file

   RenderBackgroundContext background;
   RenderSpriteContext sprite;

} RenderContext;

extern RenderContext render;

} // namespace Renderer

extern void Renderer_Initialize();
extern void Renderer_Frame();
extern void Renderer_Line(int line);
extern void Renderer_Pixel();
extern void Renderer_Load(PACKFILE* file, const int version);
extern void Renderer_Save(PACKFILE* file, const int version);

#endif //!_RENDERER__RENDERER_HPP
