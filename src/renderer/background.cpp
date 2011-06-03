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

// Logic that takes place before a pixel is built.
inline void Prelogic()
{
   if(background.counter > 0)
      background.counter--;

   if(background.counter == 0) {
      /* Reload shift registers, latch and counter. This happens synchronously with the
         data fetches performed by Background::Clock(). */
      background.lowFeed = evaluation.pattern1;
      background.highFeed = evaluation.pattern2;
      background.latch = evaluation.attribute;
      background.counter = TileWidth;
   }
}

// Logic that takes place after a pixel is built.
inline void Postlogic()
{
   /* Clock the shift registers, moving the next pixel up.
      Shifting to the left brings it closer to the raster position. */
   background.lowShift <<= 1;
   background.lowShift |= (background.lowFeed & _10000000b) >> 7;
   background.lowFeed <<= 1;

   background.highShift <<= 1;
   background.highShift |= (background.highFeed & _10000000b) >> 7;
   background.highFeed <<= 1;
}

/* This function just performs minimal logic for the background.
   It's used when frame skipping. */
inline void Logic()
{
   Prelogic();
   Postlogic();
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
   // Clock the counter.
   Prelogic();

   /* Bring the two shift registers together along with the fine X scrolling offset to
      form a pixel for this position on the line. */
   const int pixel = (((background.lowShift << ppu__fine_scroll) & _10000000b) >> 7) |
                     (((background.highShift << ppu__fine_scroll) & _10000000b) >> 6);

   // Clock the shift registers.
   Postlogic();

   /* Check for a transparent or clipped background pixel. In these cases, the backdrop
      or overscan color is written to the framebuffer instead. */
   if((pixel == 0) ||
      ((render.pixel <= 7) && ppu__clip_background)) {
      R_ClearBackgroundPixel();
      R_ClearFramePixel();
      return;
   }

   // Write the pixel (without a palette) to the background scanline buffer.
   R_PutBackgroundPixel(pixel);

   // If drawing of the background layer is disabled, no need to continue.
   if(!ppu__enable_background_layer)
      return;

   /* Determine palette index from the attribute byte. The encoding is somewhat tricky:
      Each two bits (starting with the leftmost bit) form the attributes for each
      16x16 pixel segment of a 32x32 pixel segment of the screen, represented as a 2x2
      grid of the format AABBCCDD with AA, BB, CC, and DD being top left, top right,
      bottom left and bottom right, respectively. */
   const unsigned AttributeMaskTable[] = { _11000000b, _00110000b, _00001100b, _00000011b };
   const int AttributeShiftTable[] = { 6, 4, 2, 0 };
   const int x = (render.pixel / 32) & 1;
   const int y = (render.line / 32) & 1;
   // Combining bits together is faster than multiplying.
   const int index = (y << 1) | x;
   const int palette = (background.latch & AttributeMaskTable[index]) >> AttributeShiftTable[index];
 
   // Write the finished pixel to the frame buffer.     
   R_PutFramePixel( PPU__BACKGROUND_PALETTE(palette, pixel) );
}

// Frame-skipping variant of Background::Pixel().
inline void PixelStub()
{
   // Perform minimal logic for this pixel.
   Logic();
   /* As nothing is being rendered, produce a transparent pixel. The video buffer is not
      updated this time, as we are frame-skipping. */
   R_ClearBackgroundPixel();
}

/* PPU background rendering as I understand it:

   The unit possess two internal VRAM address registers, an active one (V) that the
   software can directly modify (via $2006) and a "shadow" or latch (T), which is
   periodically loaded into V in three scearios: At the start of a frame, at the
   start of HBlank (though only partially), and at the second write to $2006
   (PPUADDR). The per-frame and per-scanline updates do not occur if rendering is
   disabled. All normal operations, such as setting the scrolling positions,
   normally modify T, and not V. Only writes to $2006 modify V directly, which is
   actually done by modifying T internally and then copying it into V.

   V is encoded so that it contains a considerable amount of encoded information
   while still being directly useable as a VRAM address for purposes such as name
   table access. The contents of V are encoded as:
      Bit 15 xDDDCCBBBBBAAAAA Bit 0
      A = Coarse X scroll (X), or more specifically the X tile position (0-31)
      B = Course Y scroll (Y), or more specifically the Y tile position (0-29)
      C = Name table (N, 0-3), these bits come from bits0-1 of $2000 (PPUCTRL)
      D = Fine vertical scroll (Ys), which is the tile Y offset (0-7)
      x = Unused, making the register effectively only 15 bits wide

   There has been some suggestions that V is actually a set of daisy-chained scroll
   registers inside of the PPU, treaded as a single unit at times due to the
   daisy-chaining, which does seem to be the case (see below).

   Some important things to note are that the fine X scroll (Xs) is handled by a
   separate pipeline from the one which fetches data, thus it is stored in a
   separate, 3-bit wide register, which is fed into the shifters during
   rasterization. Furthermore, while Y is 5-bits wide and can contain a value of
   0-31 like X, the PPU normally wraps it around at 29 (see below). V is
   structured in such a way that it can be directly incremented after each tile;
   this initially adds 1 to X, but eventually X will wrap around to 0 (after 32
   tiles have been fetched) and overflow into Y, incrementing it by 1 as well.
   This simulates a 2-D tile matrix with only simple logic.

   At the end of the rendering portion of each scanline, starting with the sprite
   evaluation scanline -1 (or 240 if you  prefer), the PPU fetches the first two
   tiles for the next scanline, tiles #0 and #1. This is so they an be loaded into
   the background shift registers and latches at the start of the next line which
   makes fine X scrolling immediately possible. This means that the first tile to
   get fetched on each line is actually tile #2. As the unit performs fetches for
   272 clock cycles per scanline (256 during rasterization, 16 during HBlank),
   and each tile takes 8 cycles to fetch completely, 34 tiles total are fetched
   per scanline. This is because of Xs, when non-zero, may cause the playing field
   to overflow beyond the screen limits, making the minimal amount of tiles that
   the PPU can fetch 33 when fine X scrolling is required. The 34th tile is simply
   a garbage fetch that is not used for anything.

   In order to simulate the effect of a camera panning over a large background,
   the horiontal name table bit (Xt, or bit 10) of V is inverted whenever X wraps
   around from 31 to 0. Consider the 33rd fetch of a line, where X has wrapped
   around and Xt is inverted, the tile fetched is actually the first tile of the
   next name table, on the same line, making it appear as if there is seamless
   horizontal scrolling. Similarly, when Y wraps around from 29 to 0, the vertical
   name table bit (Yt, or bit 11) is inverted. It is important to note that this
   *only* happens when Y wraps around from 29 to 0. If you set Y to a higher value
   manually using $2006, it will wrap at 31 instead and not invert Yt. This causes
   vertical scrolling offsets higher than 239 to appear negative, although with
   graphical glitches due to data being fetched incorrectly.

   There is one major gotcha: As stated before, T is partially loaded into V at the
   beginning of HBlank, when the fetch for tile #0 is performed, effectively
   resetting X and Xt to the values which were last set by software (via $2006),
   which makes tile fetches start from the same horizontal location every scanline.
   Bcause of this, the first tile fetched on the "next" scanline is actually (X+2),
   due to the prefetching that occurs here. The rest of V is left unmodified.

   An issue with vertical scrolling is that changes to the low byte of $2005
   (Y scroll position) do not take effect until the next frame, as they only modify
   T and it doesn't get copied into V except on writes to $2006. So the changes to
   vertical scrolling only show up when T is copied into V at frame start.
*/
inline void Clock()
{
   /* The data for each tile is fetched during this phase.
      Each memory access takes 2 PPU cycles to complete,
      and 4 must be performed per tile: 
         * Nametable byte 
         * Attribute table byte 
         * Tile bitmap A 
         * Tile bitmap B (+8 bytes from tile bitmap A) */

   // Check if we need to do any data fetching for this clock cycle.
   switch( FetchTable[render.clock] ) {
      case Fetch_None:
         return;
      case Fetch_Visible:
         if(render.line < PPU_FIRST_DISPLAYED_LINE)
            return;
         break;
      case Fetch_Always:
         break;

      default:
         WARN_GENERIC();
         return;
   }
   
   const int cycle = render.clock;

   /* Determine which type of data to fetch:
         1 - Name byte
         2 - Attribute byte
         3 - First pattern byte
         4 - Second pattern byte */
   const int type = SequenceTable[cycle];

   switch(type) {
      case 1: {
         // Fetch name table byte.
          const int table = (ppu__vram_address >> 10) & 3;
          const uint8 *data = ppu__name_tables_read[table];
          evaluation.name = data[ppu__vram_address & PPU__NAME_TABLE_PAGE_MASK];
          break;
      }

      /* VRAM address bit layout:
            -YYY VHyy yyyx xxxx
               x = x tile offset in name table
               y = y tile offset in name table
               H = horizontal name table
               V = vertical name table
               Y = y line offset in tile */
      case 2: {
         // Fetch attribute byte. This is also when the VRAM address is updated.
         const int table = (ppu__vram_address >> 10) & 3;
         int x = ppu__vram_address & _00011111b;
         int y = (ppu__vram_address >> 5) & _00011111b;
         const int attributeX = x / 4;
         const int attributeY = y / 4;
         const unsigned address = AttributeBase + (attributeY * (DisplayWidth / 32)) + attributeX;
         const uint8* data = ppu__name_tables_read[table];
         evaluation.attribute = data[address & PPU__NAME_TABLE_PAGE_MASK];

         // Unpack the rest of the VRAM address so we can update it.
         unsigned bit10 = (ppu__vram_address >> 10) & 1;
         unsigned bit11 = (ppu__vram_address >> 11) & 1;
         int row = (ppu__vram_address >> 12) & _00000111b;

         // We need to set this here so that the pattern data fetches can get at it.
         evaluation.row = row;

         /* Increment VRAM address. We do this the hard way for now, later on I'll figure out the
            proper calculations for doing it in a bitwise manner. */
         x++;
         if(x > 31) {
            x = 0;
            bit10 ^= 1;

            row++;
            if(row > 7) {
               row = 0;

               /* if you manually set the value above 29 (from either 2005 or
                  2006), the wrapping from 29 obviously won't happen, and attrib data will be
                  used as name table data.  the "y scroll" still wraps to 0 from 31, but
                  without switching bit 11.  this explains why writing 240+ to 'Y' in 2005
                  appeared as a negative scroll value. */
               y++;
               if(y == 30) {
                  y = 0;
                  bit11 ^= 1;
               }
               else if(y > 31)
                  y = 0;
            }
         }

         ppu__vram_address = (row << 12) | (bit11 << 11) | (bit10 << 10) | (y << 5) | x;

;
      }

      case 3:
      case 4: {
         // Fetch pattern table bytes.
         const unsigned address = (evaluation.name * BytesPerTile) + ppu__background_tileset;
         const unsigned page = address / PPU__PATTERN_TABLE_PAGE_SIZE;
         const uint8 *data = ppu__background_pattern_tables_read[page];
         unsigned offset = address & PPU__PATTERN_TABLE_PAGE_MASK;

         switch(type) {
            case 3:
               evaluation.pattern1 = data[offset + evaluation.row];
               break;

            case 4: {
               evaluation.pattern2 = data[offset + (BytesPerTile / 2) + evaluation.row];

               /* Background::Pixel() does not get called during HBlank, so we need to load the
                  shift registers, latch and counter manually here. */
               if(cycle >= PrefetchCycleFirst) {
                  background.lowShift <<= 8;
                  background.lowShift |= evaluation.pattern1;
                  background.highShift <<= 8;
                  background.highShift |= evaluation.pattern2;
                  background.latch = evaluation.attribute;
                  background.counter = TileWidth;
               }

               break;
            }
         }

         break;
      }
   }
}

} // namespace Background
} // namespace Renderer
