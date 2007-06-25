/* FakeNES - A free, portable, Open Source NES emulator.

   background.cpp: Implementation of the PPU background renderer.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _RENDERER__BACKGROUND_HPP
#define _RENDERER__BACKGROUND_HPP
#include "../include/common.h"
#include "renderer.hpp"

extern void rendererRenderBackgroundLine(int line);
extern void rendererRenderBackgroundPixel(void);

#endif //!_RENDERER__BACKGROUND_HPP
