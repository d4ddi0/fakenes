/* FakeNES - A free, portable, Open Source NES emulator.

   sprites.cpp: Implementation of the PPU sprites renderer.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <cstring>
#include <cstdlib>
#include "../include/common.h"
#include "../include/mmc.h"
#include "../include/ppu.h"
#include "../include/ppu_int.h"
#include "../include/types.h"
#include "../include/video.h"
#include "renderer.hpp"
#include "sprites.hpp"

namespace Renderer {

namespace {

const unsigned HFlipMask = 0x80;
const unsigned VFlipMask = 0x40;
const unsigned PriorityMask = 0x20;

} // namespace anonymous

void SpriteInit() {
}

void SpriteLine() {
}

void SpritePixel() {
}

void SpritePixelStub() {
}

void SpritePixelSkip() {
}

} // namespace Renderer
