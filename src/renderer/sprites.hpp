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
namespace Sprites {

extern void Initialize();
extern void Line();
extern void Pixel();
extern void PixelStub();
extern void Clock();

} // namespace Sprites
} // namespace Renderer

#endif //!_RENDERER__SPRITES_HPP
