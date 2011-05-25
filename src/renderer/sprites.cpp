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

const int SpriteCount = 64;

const int BytesPerSprite = 4;
const int Sprite_YPosition = 0;
const int Sprite_TileIndex = 1;
const int Sprite_Attributes = 2;
const int Sprite_XPosition = 3;

const unsigned HFlipMask = 0x80;
const unsigned VFlipMask = 0x40;
const unsigned PriorityMask = 0x20;

void Clear()
{
    for(int i = 0; i < SpritesPerLine; i++) {
       RenderSpriteContext& sprite = render.sprites[i];
       sprite.lowShift = 0;
       sprite.highShift = 0;
       sprite.latch = 0;
       sprite.counter = 0;
    }
}

} // namespace anonymous

void SpriteInit() {
   Clear();
}

void SpriteLine() {
#if 0
    for(int i = 0; i < SpritesPerLine; i++) {
       shifts0[i] = 0;
       shifts1[i] = 0;
       latches[i] = 0;
       counters[i] = 0;
    }

    int sprites = 0;
    for (sprite = 0; sprite < SpriteCount; sprite++)
    {
        int first_y, last_y;

        first_y = ppu_spr_ram [(sprite * BytesPerSprite) + SpriteYPosition] + 1;
        last_y = first_y + ppu__sprite_height - 1;

        /* vertical clipping */
         // todo: use macros here
        if (last_y >= 239) last_y = 239;

        for (line = first_y; line <= last_y; line++)
        {
           if( line == render.line) {
              if( sprites >= 8 ) {
                 ppu__sprite_overflow = TRUE;
                 continue;
              }

              counters[sprites] = ppu_spr_ram [(sprite * BytesPerSprite) + SpriteXPosition] ;
              int address;
              int tile = ppu_spr_ram [(sprite * BytesPerSprite) + SpriteTileIndex] ;
              if(ppu__sprite_height == 8 ) 
              address = ((tile * 16) + ppu__sprite_tileset);
              else
        if (! (tile & 1))
        {
            address = (tile * 16);
        }
        else
        {
            address = (((tile - 1) * 16) + 0x1000);
        }

 UINT8 *cache_address;
cache_address = ppu_vram_block_sprite_cache_address [address >> 10] +
        ((address & 0x3FF) / 2 * 8) + (y * 8);

              uint8 attribute;
  attribute = attribute_table [ppu_spr_ram [(sprite * BytesPerSprite) + SpriteAttribute] & 3];

              latches[sprites] = attribute;
              sprites++;
           }
        }
    }
#endif
}

void SpritePixel() {
#if 0
   for(int i = 0; i < SpritesPerLine; i++) {
      if(counters[i] > 0)
         counters[i]--;
      if(counters[i] >= 0)
         continue;

      shifts0[i] <<= 1;
      shifts1[i] <<= 1;
   }
#endif
}

void SpritePixelStub() {
}

void SpritePixelSkip() {
}

void SpriteClock() {
//   if(clock == 1)
}

} // namespace Renderer
