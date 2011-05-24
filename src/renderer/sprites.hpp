/* FakeNES - A free, portable, Open Source NES emulator.

   sprites.hpp: Declarations for the PPU background renderer.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _RENDERER__SPRITES_HPP
#define _RENDERER__SPRITES_HPP
#include "../include/common.h"
#include "renderer.hpp"

namespace Renderer {

extern void SpriteInit();
extern void SpriteLine();
extern void SpritePixel();
extern void SpritePixelStub();
extern void SpritePixelSkip();

} // namespace Renderer

#endif //!_RENDERER__SPRITES_HPP
