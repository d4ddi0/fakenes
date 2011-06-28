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
namespace Background {

extern R_LookupTable( FetchTable );
extern R_LookupTable( SequenceTable );

extern void Initialize();
extern void Frame();
extern void Line();
extern void Pixel(const bool rendering);
extern void PixelStub(const bool rendering);
extern void Clock();

} // namespace Background
} // namespace Renderer

#endif //!_RENDERER__BACKGROUND_HPP
