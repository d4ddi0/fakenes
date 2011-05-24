/* FakeNES - A free, portable, Open Source NES emulator.

   background.hpp: Declarations for the PPU background renderer.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _RENDERER__BACKGROUND_HPP
#define _RENDERER__BACKGROUND_HPP
#include "../include/common.h"
#include "renderer.hpp"

namespace Renderer {

extern void BackgroundInit();
extern void BackgroundLine();
extern void BackgroundPixel();
extern void BackgroundPixelStub();
extern void BackgroundPixelSkip();

} // namespace Renderer

#endif //!_RENDERER__BACKGROUND_HPP
