/* FakeNES - A free, portable, Open Source NES emulator.

   background.cpp: Implementation of the PPU background renderer.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../include/common.h"
#include "../include/binary.h"
#include "../include/mmc.h"
#include "../include/ppu.h"
#include "../include/ppu_int.h"
#include "../include/types.h"
#include "background.hpp"
#include "renderer.hpp"

// TODO: Frame skipping support.
// TODO: MMC2 and MMC4 latches support.
// TODO: MMC5 ExRAM and attribute support.
// TODO: State saving support.

namespace Renderer {
namespace Background {

namespace {

// Amount of space a tile consumes in CHR (at 4 pixels per byte).
const int BytesPerTile = (TileWidth * TileHeight) / 4;

// Offset of attributes in name tables, and associated mask.
const unsigned AttributeBase = (DisplayWidth / TileWidth) * (DisplayHeight / TileHeight);
const unsigned AttributeMask = 0x03;

// Shifts and mask for extended attributes in MMC5 ExRAM.
const int ExpansionAttributeShifts = 6;
const unsigned ExpansionAttributeMask = 0x03;

// Evaluation timings.
const int FetchCycleFirst    = 1;
const int FetchCycleLast     = FetchCycleFirst    + 255;	// 256 clocks
const int SpriteCycleFirst   = FetchCycleLast     + 1;
const int SpriteCycleLast    = SpriteCycleFirst   + 63;		// 64 clocks
const int PrefetchCycleFirst = SpriteCycleLast    + 1;
const int PrefetchCycleLast  = PrefetchCycleFirst + 15;		// 16 clocks
const int ExtraCycleFirst    = PrefetchCycleLast  + 1;
const int ExtraCycleLast     = ExtraCycleFirst    + 3;		// 4 clocks
const int IdleCycle          = ExtraCycleLast     + 1;

// Indices for the fetch table. See Initialize() for further details.
enum {
   Fetch_None = 0,
   Fetch_Visible,
   Fetch_Always
};

// Aliases for the renderer contexts, for improved readability.
RenderBackgroundContext& background = render.background;
RenderBackgroundEvaluation& evaluation = render.backgroundEvaluation;

void ClearBackground()
{
   memset(&background, 0, sizeof(background));
}

void ClearEvaluation()
{
   memset(&evaluation, 0, sizeof(evaluation));
}

void Clear() {
   ClearBackground();
   ClearEvaluation();
}

} // namespace anonymous

// --------------------------------------------------------------------------------
// PUBLIC INTERFACE
// --------------------------------------------------------------------------------

#if !defined(INLINE_8EWR)

R_LookupTable( FetchTable );
R_LookupTable( SequenceTable );

void Initialize()
{
   Clear();

   // Clear lookup tables.
   R_ClearLookupTable( FetchTable );
   R_ClearLookupTable( SequenceTable );

   /* Build our fetch table. This tells us whether or not we should do any processing on a
      given clock cycle. Fetches only occur on certain cycles of a scanline. */
   for(int i = 1; i <= PPU_SCANLINE_CLOCKS; i++) {
      int fetch;
      if((i % 2) != 0) {
         /* As each fetch takes exactly 2 PPU clocks to complete, and the clock counter
            starts at one, we only have to process on even cycles. */
         fetch = Fetch_None;
      }
      else if(i <= FetchCycleLast) {
         // We only perform normal fetching during visible lines.
         fetch = Fetch_Visible;
      }
      else if((i >= PrefetchCycleFirst) && (i <= PrefetchCycleLast)) {
         /* Prefetching is performed on all visible lines, plus the sprite evaluation line -1.
            This process occurs during HBlank, towards the end of the line. */
         fetch = Fetch_Always;
      }
      else
         // No fetching occurs during this cycle.
         fetch = Fetch_None;

      FetchTable[i] = fetch;
   }

   /* Build our sequence table. This tells us which type of data to fetch on each clock cycle.
      Each tile fetch is exactly 8 cycles long. */
   for(int i = 0; i < PPU_SCANLINE_CLOCKS; i++) {
      const int tile = i / 8;
      SequenceTable[i + 1] = (((i / 8.f) - tile) * 4) + 1; // 1-4
   }
}

void Frame()
{
   Clear();
}

void Line()
{
   // Do nothing.
}

#endif

inline void Pixel()
{
   if(background.counter > 0)
      background.counter--;
   if(background.counter == 0)
   {
      background.lowFeed = evaluation.pattern1;
      background.highFeed = evaluation.pattern2;
      background.latch = evaluation.attribute;
      background.counter += TileWidth;
   }

   uint8 pixel = (((background.lowShift << ppu__fine_scroll) & 0x8000) >> 15) |
                 (((background.highShift << ppu__fine_scroll) & 0x8000) >> 14);

   /* Clock the shift registers, moving the next pixel up.
      Shifting to the left brings it closer to the raster position. */
   background.lowShift <<= 1;
   background.highShift <<= 1;
   background.lowShift |= (background.lowFeed & 0x80) >> 7;
   background.highShift |= (background.highFeed & 0x80) >> 7;
   background.lowFeed <<= 1;
   background.highFeed <<= 1;

   const int megatile_x = render.pixel - ((render.pixel / 32) * 32);
   const int megatile_y = render.line - ((render.line / 32) * 32);
   const int x = megatile_x / 16;
   const int y = megatile_y / 16;
   uint8 attrib;
   if((x == 0) && (y == 0))
      // top left
      attrib = background.latch & _00000011b;
   else if((x == 1) && (y == 0))
      // top right
      attrib = (background.latch & _00001100b) >> 2;
   else if((x == 0) && (y == 1))
      // bottom left
      attrib = (background.latch & _00110000b) >> 4;
   else if((x == 1) && (y == 1))
      // bottom right
      attrib = (background.latch & _11000000b) >> 6;
      
   R_PutBackgroundPixel(pixel);
   bool skip = false;
   if( pixel == 0)
      skip  =true;
   if((render.pixel < 8) && ppu__clip_background)
      skip = true;

   if( !skip)
      R_PutFramePixel(PPU__BACKGROUND_PALETTE(attrib, pixel));
   else
      R_ClearFramePixel();

}

// Frame-skipping variant of BackgroundPixel()
inline void PixelStub()
{
   /* As nothing is being rendered, produce a transparent pixel. The video buffer is not
      updated this time, as we are frame-skipping. */
   R_ClearBackgroundPixel();
}

inline void Clock()
{
   /* The data for each tile is fetched during this phase.
      Each memory access takes 2 PPU cycles to complete,
      and 4 must be performed per tile: 
         * Nametable byte 
         * Attribute table byte 
         * Tile bitmap A 
         * Tile bitmap B (+8 bytes from tile bitmap A) */

      if(render.clock == PrefetchCycleFirst) {
         evaluation.tile = 0;
       }

   // Check if we need to do any data fetching for this clock cycle.
   switch( FetchTable[render.clock] ) {
      case Fetch_None:
         return;
      case Fetch_Visible:
         //if(render.line < PPU_FIRST_DISPLAYED_LINE)
          //  return;
         break;
      case Fetch_Always:
         break;

      default:
         WARN_GENERIC();
         return;
   }
   
   const int cycle = render.clock;

   //int line = render.line;
   //if(cycle >= PrefetchCycleFirst)
      // Prefetching affects the next scanline, not the current.
     // line++;

   /* Determine which type of data to fetch:
         1 - Name byte
         2 - Attribute byte
         3 - First pattern byte
         4 - Second pattern byte

      As a consequence, #4 also advances the tile position to the next tile, as the
      fetching for the current tile has been completed by that point. */
   const int type = SequenceTable[cycle];

   /* VRAM address bit layout:
         -YYY VHyy yyyx xxxx
            x = x tile offset in name table
            y = y tile offset in name table
            H = horizontal name table
            V = vertical name table
            Y = y line offset in tile */
   switch(type) {
      // Name byte
      case 1: {
          const int name_table = (ppu__vram_address >> 10) & _00000011b;
          unsigned address = ppu__vram_address & 0x03FF; // 0-1023
          const uint8 *data = ppu__name_tables_read[name_table];
          evaluation.name = data[address];
          break;
      }

      // Attribute byte
      case 2: {

           int x = ppu__vram_address & _00011111b; // 0-31
          int y = (ppu__vram_address >> 5) & _00011111b; // 0-29

          const int name_table = (ppu__vram_address >> 10) & _00000011b;

         x = x / 4;
         y = y / 4;
         const unsigned address = AttributeBase + (y * (DisplayWidthTiles / 4)) + x;
         const uint8* data = ppu__name_tables_read[name_table];
         evaluation.attribute = data[address];
         break;
      }

      // First pattern byte
      case 3:
      // Second Pattern byte
      case 4: {
           int row = (ppu__vram_address >> 12) & _00000011b; //0-7


         const unsigned address = ppu__background_tileset + (evaluation.name * BytesPerTile);

         const unsigned page = address / PPU__PATTERN_TABLE_PAGE_SIZE;
         const uint8 *data = ppu__background_pattern_tables_read[page];
         unsigned offset = address - (page * PPU__PATTERN_TABLE_PAGE_SIZE);

         // TODO: smooth Y scrolling
         row = evaluation.row;

         if(type == 3) {
            evaluation.pattern1 = data[offset + row];
         }
         else {
            evaluation.pattern2 = data[offset + (BytesPerTile / 2) + row];

            if(cycle >= PrefetchCycleFirst) {
               background.lowShift <<= 8;
               background.highShift <<= 8;
               background.lowShift |= evaluation.pattern1;
               background.highShift |= evaluation.pattern2;
               background.latch = evaluation.attribute;
               background.counter = TileWidth;
            }

           int x = ppu__vram_address & _00011111b; // 0-31
           int y = (ppu__vram_address >> 5) & _00011111b; // 0-29
           unsigned bit10 = (ppu__vram_address >> 10) & 0x01;
           unsigned bit11 = (ppu__vram_address >> 11) & 0x01;

            x++;
            if(x > 31) {
               bit10 ^= 1;
               x = 0;
               evaluation.row++;
               if(evaluation.row > 7) {
                  evaluation.row = 0;
                  y++;
                  /* if you manually set the value above 29 (from either 2005 or
                     2006), the wrapping from 29 obviously won't happen, and attrib data will be
                     used as name table data.  the "y scroll" still wraps to 0 from 31, but
                     without switching bit 11.  this explains why writing 240+ to 'Y' in 2005
                     appeared as a negative scroll value. */
                  if(y == 29) {
                     bit11 ^= 1;
                     y = 0;
                  }
                  else if(y > 31)
                     y = 0;
               }
            }

             ppu__vram_address = (row << 12) | (bit11 << 11) | (bit10 << 10) | (y << 5) | x;

            // Advance to the next tile.
            evaluation.tile++;
            evaluation.tile &= DisplayWidthTiles - 1;
         }

         break;
      }
   }
}

} // namespace Background
} // namespace Renderer
