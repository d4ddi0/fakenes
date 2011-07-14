/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "Internals.h"
#include "Local.hpp"
#include "Renderer.hpp"
#include "Sprites.hpp"

namespace Renderer {
namespace Sprites {

namespace {

// Total number of sprites in OAM.
const int SpriteCount = PPU__SPRITE_COUNT;

// Bytes that each tile takes up in a pattern table.
const int BytesPerTile = 16;

// Offsets into OAM for various sprite data, relative to the sprite entry.
const int Sprite_YPosition  = 0;
const int Sprite_TileIndex  = 1;
const int Sprite_Attributes = 2;
const int Sprite_XPosition  = 3;

// Evaluation timings.
const int ClearCycleFirst      = 1;
const int ClearCycleLast       = ClearCycleFirst      + 63;	// 64 clocks
const int EvaluationCycleFirst = ClearCycleLast       + 1;
const int EvaluationCycleLast  = EvaluationCycleFirst + 191;	// 192 clocks
const int FetchCycleFirst      = EvaluationCycleLast  + 1;
const int FetchCycleLast       = FetchCycleFirst      + 63;	// 64 clocks
const int PipelineCycleFirst   = FetchCycleLast       + 1;
const int PipelineCycleLast    = PipelineCycleFirst   + 20;	// 21 clocks

// Masks for 8x16 sprites.
const unsigned OAM_Bank = _00000001b;
const unsigned OAM_Tile = 0xFF & ~OAM_Bank;

// Masks for sprite attributes.
const unsigned Attribute_Palette  = _00000011b;
const unsigned Attribute_Priority = _00100000b;
const unsigned Attribute_HFlip    = _01000000b;
const unsigned Attribute_VFlip    = _10000000b;

force_inline void ClearSprites() {
    for(int i = 0; i < SpritesPerLine; i++) {
       RenderSpriteContext& sprite = render.sprites[i];
       sprite.index = 0;
       sprite.lowShift = 0x00;
       sprite.highShift = 0x00;
       sprite.latch = 0x00;
       sprite.counter = 0;
       sprite.dead = TRUE;
    }

    render.spriteCount = 0;
}

force_inline void ClearEvaluation() {
   RenderSpriteEvaluation& e = render.spriteEvaluation;

   e.state = 1;
   e.substate = 1;
   e.count = 0;
   e.n = 0;
   e.m = 0;
   e.locked = FALSE;
   e.data = 0x00;
}

exclusive void Clear()
{
    ClearSprites();
    ClearEvaluation();

    // Writing $FF produces a hidden sprite.
    memset(render.secondaryOAM, 0xFF, SecondaryOAMSize);
}

/* This function just performs minimal logic for a sprite. It's used when frame skipping,
    or when the frame buffer has been locked for writes. */
force_inline void Logic(RenderSpriteContext& sprite) {
    if(sprite.dead)
        return;

    if(sprite.counter > 0) {
       sprite.counter--;
       return;
   }

    if(sprite.latch & Attribute_HFlip) {
       sprite.lowShift >>= 1;
       sprite.highShift >>= 1;
    }
    else {
       sprite.lowShift <<= 1;
       sprite.highShift <<= 1;
    }

    if((sprite.lowShift + sprite.highShift) == 0x00)
       sprite.dead = TRUE;
}

force_inline uint8 ReadOAM(const int index, const unsigned offset)
{
   /* Read a byte from primary OAM, updating the OAM address accordingly. This step is neccessary
      for games that detect OAM access during sprite evaluation, such as Micro Machines. */
   ppu__oam_address = (index * BytesPerSprite) + offset;
   return ppu__sprite_vram[ppu__oam_address];
}

force_inline uint8 ReadSOAM(const int index, const unsigned offset)
{
   // Reads a byte from secondary OAM.
   const unsigned address = (index * BytesPerSprite) + offset;
   return render.secondaryOAM[address];
}

force_inline void WriteSOAM(const int index, const unsigned offset, const uint8 data)
{
   // Writes a byte to secondary OAM.
   const unsigned address = (index * BytesPerSprite) + offset;
   render.secondaryOAM[address] = data;
}

} // namespace anonymous

// ----------------------------------------------------------------------
// PUBLIC INTERFACE
// ----------------------------------------------------------------------

#if !defined(INLINE_P2EC)

R_LookupTable( IndexTable );
R_LookupTable( SequenceTable );

void Initialize() {
   Clear();

   // Clear lookup tables.
   R_ClearLookupTable( IndexTable );
   R_ClearLookupTable( SequenceTable );

   /* Build our index table. This gives the *secondary* OAM index (0-7) of the sprite being
      processed at a given clock cycle during data fetching. */
   for(int i = 1; i <= PPU_SCANLINE_CLOCKS; i++) {
      const int position = i - FetchCycleFirst;	// 0-63
      IndexTable[i] = position / 8;
   }

   /* Build our sequence table. This tells us which type of data to fetch on each clock cycle
      of the data fetching phase. Each sprite fetch is exactly 8 cycles long. */
   for(int i = 1; i <= PPU_SCANLINE_CLOCKS; i++) {
      const int position = i - FetchCycleFirst;			// 0-63
      const int index = position / 8;				// 0-7
      SequenceTable[i] = ((position - (index * 8)) / 2) + 1;	// 1-4
   }
}

void Line() {
   // Copy evaluation count to spriteCount, as it will be overwritten
   render.spriteCount = render.spriteEvaluation.count;
}

#endif

/* Every cycle, the 8 x-position counters for the sprites are decremented by one. For each sprite, if the counter is still nonzero, nothing else happens. If the counter is zero, the sprite becomes "active", and the respective pair of shift registers for the sprite is shifted once every cycle. This output accompanies the data in the sprite's latch, to form a pixel. The current pixel for each "active" sprite is checked (from highest to lowest priority), and the first non-transparent pixel moves on to a multiplexer, where it joins the BG pixel.
    * If the sprite has foreground priority, the sprite pixel is output.
    * If the sprite has background priority:
          * If the BG pixel is zero, the sprite pixel is output.
          * If the BG pixel is nonzero, the BG pixel is output. */
force_inline void Pixel(const bool rendering)
{
    // Check if we are frame-skipping.
    if(!rendering) {
       // When not rendering, just perform minimal logic for each sprite.
       for(int i = 0; i < render.spriteCount; i++)
          Logic(render.sprites[i]);

       return;
    }

    // Check if we should clip sprites on the left side of the screen
    bool clipping = false;
    if((render.pixel <= 7) && ppu__clip_sprites)
       clipping = true;

    // Whether or not pixel buffer can be written (for sprite priority). See below.
    bool locked = false;

    for(int i = 0; i < render.spriteCount; i++) {
       // If the framebuffer has been locked, we just do minimal processing.
       if(locked) {
          Logic(render.sprites[i]);
          continue;
       }

       RenderSpriteContext& sprite = render.sprites[i];

       /* Dead sprites are those with transparent bitmaps, which don't get rendered
          Setting and checking a flag is faster than checking the bitmap. */
       if(sprite.dead)
           continue;

       /* Rather than repeatedly check the X coordinate of the sprite, we simply decrement it.
          When it becomes zero, it has reached the current raster position. */
       if(sprite.counter > 0) {
          sprite.counter--;
          continue;
      }

       int pixel = 0;
       if(sprite.latch & Attribute_HFlip) {
          /* The pixel is formed of two bits taken from each shift register,
             giving a possible color range from 0-3. */
          pixel = (sprite.lowShift & _00000001b) | ((sprite.highShift & _00000001b) << 1);

          sprite.lowShift >>= 1;
          sprite.highShift >>= 1;
       }
       else {
          pixel = ((sprite.lowShift & _10000000b) >> 7) | ((sprite.highShift & _10000000b) >> 6);

          /* Clock the shift registers, moving the next pixel up.
             Shifting to the left brings it closer to the raster position. */
          sprite.lowShift <<= 1;
          sprite.highShift <<= 1;
       }

       /* When the sprite's bitmap becomes transparent, we mark it as dead. The easiest way to check this
          is to add the two bitmaps together, then check for non-zero (i.e some bits are set). */
       if((sprite.lowShift + sprite.highShift) == 0x00)
          sprite.dead = TRUE;

       // Don't draw transparent pixels.
       if(pixel == 0)
          continue;

       /* FRAMEBUFFER LOCKING:
          Note that the PPU only renders a single sprite per pixel, even if it is back-priority and
          "hidden" by the background. which allows sprite priority to work properly. So we need to set a
          flag when a sprite has been rendered, to prevent further (lesser priority) sprites from being
          rendered in the same position. This locks the framebuffer from further writes. */
       locked = true;

       // All logic is done by this point, so it is safe to check for clipping
       if(clipping)
          continue;

       /* Sprite #0 hit test:
          When the raster gun meets a non-transparent sprite #0 pixel that is overlapping a
          non-transparent background pixel, the sprite #0 hit flag is set. This does not occur if either
          the background or sprites are disabled, or if clipping is enabled for either in the area.
          This test is also not affected by background priority.

          As our background rendering code automatically produces transparent (color #0) values in the
          special background pixel buffer whenever the background is disabled or clipped, all we have
          to do here is check for three conditions:
             1) The current sprite is #0
             2) The sprite is not transparent or clipped (already checked)
             3) The background is opaque */
       if((i == 0) && (sprite.index == 0) && // for sprite #0, i always == 0
          !ppu__sprite_collision) {
          if(R_GetBackgroundPixel() != 0)
             ppu__sprite_collision = TRUE;
       }

       if(sprite.latch & Attribute_Priority) {
          /* This is a back-priority sprite, so we should only plot pixels
             in areas where the background was transparent. */
          if(R_GetBackgroundPixel() != 0)
             continue;

          // We don't want to draw if the back-priority layer is disabled, either.
          if(!ppu__enable_sprite_back_layer)
             continue;
       }
       else
          /* Similarly, don't draw if the front-priority layer is disabled,
             for front-priority sprites. */
          if(!ppu__enable_sprite_front_layer)
             continue;

       /* We can only plot pixels when:
             1) Sprite clipping is disabled, or the current pixel position is 8 or higher
             2) The sprite pixel is not transparent (color #0)
             3) The background pixel is transparent (for back-priority sprites)
             4) The frame buffer is not locked for writes
             5) Drawing of the associated sprite layer is not disabled */
       const int palette = sprite.latch & Attribute_Palette;
       R_PutFramePixel( PPU__SPRITE_PALETTE(palette, pixel) );
    }
}

#define MAIN_LOOP	main_loop
#define STATE1_LOOP	state1_loop
#define STATE3_LOOP	state3_loop
#define END_LOOP	end_loop

#define READ_HELPER(_INDEX, _OFFSET) { \
   if(!cycle_is_even) \
      goto END_LOOP; \
   e.data = ReadOAM((_INDEX), (_OFFSET)); \
}

#define WRITE_HELPER(_INDEX, _OFFSET) { \
   if(!cycle_is_odd) \
      goto END_LOOP; \
   if(!e.locked) \
      WriteSOAM((_INDEX), (_OFFSET), e.data); \
}

#define CONTINUE_1 { \
   e.substate++; \
   goto STATE1_LOOP; \
}

#define CONTINUE_3 { \
   e.substate++; \
   goto STATE3_LOOP; \
}

#define JUMP(_STATE) { \
   e.state = (_STATE); \
   e.substate = 1; \
   goto MAIN_LOOP; \
}

#define JUMP_1	JUMP(1)
#define JUMP_2	JUMP(2)
#define JUMP_3	JUMP(3)
#define JUMP_4	JUMP(4)

force_inline void Clock() {
  /* Kind of ugly, but it works. Note that emulation starts on an odd cycle (1),
      and most writes take place on even cycles (2, 4, 6, etc.). */
   const int cycle = render.clock;
   const bool cycle_is_even = !render.isOddClock;
   const bool cycle_is_odd = render.isOddClock;

   const int line = render.line + 1;

   if(cycle <= ClearCycleLast) {
      /* Cycles 0-63: Secondary OAM (32-byte buffer for current sprites on scanline) is initialized to $FF -
         attempting to read $2004 will return $FF */

      // Note: Each byte takes 2 PPU cycles to clear.
      if(cycle_is_even)
         render.secondaryOAM[(cycle / 2) - 1] = 0xFF;
   }
   else if(cycle <= EvaluationCycleLast) {
      /* Cycles 64-255: Sprite evaluation
         * On even cycles, data is read from (primary) OAM
         * On odd cycles, data is written to secondary OAM (unless writes are inhibited, in which case it will read the value in secondary OAM instead)
 	 * 1. Starting at n = 0, read a sprite's Y-coordinate (OAM[n][0], copying it to the next open slot in secondary OAM (unless 8 sprites have been found, in which case the write is ignored).
         * 1a. If Y-coordinate is in range, copy remaining bytes of sprite data (OAM[n][1] thru OAM[n][3]) into secondary OAM.
         * 2. Increment n
         * 2a. If n has overflowed back to zero (all 64 sprites evaluated), go to 4
         * 2b. If less than 8 sprites have been found, go to 1
         * 2c. If exactly 8 sprites have been found, disable writes to secondary OAM. This causes sprites in back to drop out.
         * 3. Starting at m = 0, evaluate OAM[n][m] as a Y-coordinate.
         * 3a. If the value is in range, set the sprite overflow flag in $2002 and read the next 3 entries of OAM (incrementing 'm' after each byte and incrementing 'n' when 'm' overflows); if m = 3, increment n
         * 3b. If the value is not in range, increment n AND m (without carry). If n overflows to 0, go to 4; otherwise go to 3
         * 4. Attempt (and fail) to copy OAM[n][0] into the next free slot in secondary OAM, and increment n (repeat until HBLANK is reached) 
      */

      /* This was very difficult to implement. It had to be written as a state machine so
         that it could drop in and out of execution at any time to wait for PPU cycles
         to pass for reads and writes for proper synchronization. */

      if(cycle == EvaluationCycleFirst)
         ClearEvaluation();

      RenderSpriteEvaluation& e = render.spriteEvaluation;

   MAIN_LOOP:
      if(e.state == 1) {
         /* 1. Starting at n = 0, read a sprite's Y-coordinate (OAM[n][0],
               copying it to the next open slot in secondary OAM (unless 8 sprites have been found,
               in which case the write is ignored). */
      STATE1_LOOP:
         switch(e.substate) {
            // Y-Position
            case 1: {
               READ_HELPER(e.n, Sprite_YPosition);
               CONTINUE_1
            }
            case 2: {
               WRITE_HELPER(e.count, Sprite_YPosition);

               /* 1a. If Y-coordinate is in range,
                      copy remaining bytes of sprite data (OAM[n][1] thru OAM[n][3]) into secondary OAM. */

               /* Note that adding ppu__sprite_height == 8 to a Y position of 0 would result in 8,
                  which is wrong. The proper result would be 7, which requires a subtraction by one.
                  We can avoid it by using a < rather than <= comparison. */
               if((line >= e.data) && (line < (e.data + ppu__sprite_height)))
                  CONTINUE_1
               else
                  JUMP_2
            }

            // Tile index
            case 3: {
               READ_HELPER(e.n, Sprite_TileIndex);
               CONTINUE_1
            }
            case 4: {
               WRITE_HELPER(e.count, Sprite_TileIndex);
               CONTINUE_1
            }

            // Attribute
            case 5: {
               READ_HELPER(e.n, Sprite_Attributes);
               CONTINUE_1
            }
            case 6: {
               WRITE_HELPER(e.count, Sprite_Attributes);
               CONTINUE_1
            }

            // X-Position
            case 7: {
               READ_HELPER(e.n, Sprite_XPosition);
               CONTINUE_1
            }
            case 8: {
               WRITE_HELPER(e.count, Sprite_XPosition);

               // Sprite copy complete.
               e.indices[e.count] = e.n;
               e.count++;

               JUMP_2
            }
         }
      }
      else if(e.state == 2) {
         /* 2. Increment n */
         e.n++;

         /* 2a. If n has overflowed back to zero (all 64 sprites evaluated), go to 4 */
         if(e.n >= SpriteCount) {
            e.n = 0;
            JUMP_4
         }

         /* 2b. If less than 8 sprites have been found, go to 1 */
         if(e.count < SpritesPerLine)
            JUMP_1

         /* 2c. If exactly 8 sprites have been found, disable writes to secondary OAM.
                This causes sprites in back to drop out. */
         else if(e.count == SpritesPerLine)
            e.locked = TRUE;

         JUMP_3
      }
      else if(e.state == 3) {
      STATE3_LOOP:
         switch(e.substate) {
            case 1: {
                  /* What has effectively happened by this point is that 8 sprites are on a scanline.
                     This breaks the logic, resulting in the evaluation of tile indices, attributes
                     and X-positions of sprites rather than their Y-positions. This is because on
                     each loop, n=n+1 but also sometimes m>0. */

               /* 3. Starting at m = 0,
                     evaluate OAM[n][m] as a Y-coordinate.  */
               READ_HELPER(e.n, e.m);
               CONTINUE_3
            }

            case 2: {
               if((line >= e.data) && (line < (e.data + ppu__sprite_height))) {
                  /* 3a. If the value is in range,
                     set the sprite overflow flag in $2002 and ... (CONTINUED)  */
                  ppu__sprite_overflow = TRUE;
                  CONTINUE_3
               }
               else {
                  /* 3b. If the value is not in range, increment n AND m (without carry).
                         If n overflows to 0, go to 4; otherwise go to 3 */
                  e.n++;
                  e.m++;
                  if(e.n >= SpriteCount) {
                     e.n = 0;
                     JUMP_4
                  }
                  else
                     JUMP_3
               }
            }

            /* 3a. If the value is in range,
                   set the sprite overflow flag in $2002 and read the next 3 entries of OAM
                   (incrementing 'm' after each byte and incrementing 'n' when 'm' overflows);
                   if m = 3, increment n */
            case 3:
            case 4:
            case 5: {
               READ_HELPER(e.n, e.m);

               e.m++;
               /* Increment n when m overflows (from 0-3 -> 4) seems more correct than
                  incrementing n when m is equal to 3, despite the above. */
               if(e.m > 3) {
                  e.m = 0;
                  e.n++;
               }

               CONTINUE_3
            }

            case 6:
               JUMP_4
         }
      }
      // Stage four does not need to be emulated, as it results in nothing.
      else if(e.state == 4) {
         /* 4. Attempt (and fail) to copy OAM[n][0] into the next free slot in secondary OAM,
               and increment n (repeat until HBLANK is reached) */
      }

    END_LOOP:
      /* Ideally we'd do nothing here, but newer compilers will complain if nothing comes
         after the label, because there is nothing to jump to I guess. */
      const int garbage = 0;
   }
   else if(cycle <= FetchCycleLast) {
      /* The tile data for the sprites on the next scanline are fetched here.
         Again, each memory access takes 2 PPU cycles to complete, and 4 are performed for each of the 8 sprites:
         1. Garbage nametable byte
         2. Garbage nametable byte
         3. Tile bitmap A
         4. Tile bitmap B (+8 bytes from tile bitmap A) */

      if(cycle == FetchCycleFirst)
         // We're already in HBlank, so it's safe to clear data here.
         ClearSprites();

      /* As each fetch takes 2 cycles to complete, we only care about even cycles,
         emulation-wise, at least. */
      if(!cycle_is_even)
         return;

      // Determine which sprite we are processing for this cycle.
      const int index = IndexTable[cycle];

      // To make things a little cleaner, we'll get a direct reference.
      RenderSpriteContext& sprite = render.sprites[index];

      /* We need to keep track of the original index (0-63) for sprite #0 hit detection.
         This is filled in during sprite evaluation for each sprite. */
      sprite.index = render.spriteEvaluation.indices[index];

      /* Determine the type of data being fetched:
            1 - Garbage fetch (1)
            2 - Garbage fetch (2)
            3 - Pattern byte 1
            4 - Pattern byte 2 */
      const int type = SequenceTable[cycle];

      // Check for the first cycle of the sequence.
      if(type == 1) {
         /* The exact time when the attribute byte and X position are loaded into the 
            latch and counter (respectively) is unknown, however we have a perfectly
            good oppertunity to do it here with the tile data, so we will. */
         sprite.latch = ReadSOAM(index, Sprite_Attributes);
         sprite.counter = ReadSOAM(index, Sprite_XPosition);
      }

      switch(type) {
         case 1:
         case 2: {
            /* The PPU makes garbage fetches to the name tables so that the same hardware that
               fetches data for the background could also be used for sprites. Normally we
               don't have to emulate this, but if the MMC has a hook installed, we do. */
             if(!mmc_check_address_lines)
                break;

             // Simulate a name table fetch.
             const unsigned address = 0x2000 + (ppu__vram_address & 0x0FFF);
             mmc_check_address_lines(address);

             break;
         }

         case 3:	
         case 4: {
            // Fetch tile bitmaps.
            if(index >= render.spriteEvaluation.count) {
               /* If there are less than 8 sprites on the next scanline, then dummy fetches to
                  tile $FF occur for the left-over sprites, because of the dummy sprite data
                  in the secondary OAM (see sprite evaluation). This data is then discarded,
                  and the sprites are loaded with a transparent bitmap instead. */

               // There's no reason to emulate this if a mapper doesn't depend on it.
               if(!mmc_check_address_lines)
                  return;

               const unsigned address = (0xFF * BytesPerTile) + ppu__sprite_tileset;
               mmc_check_address_lines(address);

               /* Due to having cleared the evaluation state at the start of HBlank, unused
                  sprites are already transparent and disabled by default, so there's
                  nothing else to be done here. */
               return;
            }

            const int tile = ReadSOAM(index, Sprite_TileIndex);

            unsigned address;
            if(ppu__sprite_height == 8) {
               // Render 8x8 sprites.
               address = (tile * BytesPerTile) + ppu__sprite_tileset;
            }
            else {
               // Render 8x16 sprites.
               unsigned bank = 0x0000;
               if(tile & OAM_Bank)
                  bank = 0x1000;

               address = ((tile & OAM_Tile) * BytesPerTile) + bank;
            }

            /* Calculate which line of the sprite's bitmap to fetch. */
            const int y = ReadSOAM(index, Sprite_YPosition);

            int row;
            if(sprite.latch & Attribute_VFlip)
               row = (y + (ppu__sprite_height - 1)) - line;
            else
               row = line - y;

            /* For 8x16 sprites, we may need to jump to the next tile.
               This occurs on row indices 8-15, which then become 0-7 after the offset. */
            if(row >= 8) {
               address += BytesPerTile;
               row -= 8;
            }

            /* Each line of the plane data for the tile bitmap is a single byte, so this is
               simply used as a byte offset. */
            address += row;

            /* For the second fetch, we need to increment the address to access the second bit plane.
               As each tile is made up of 16 bytes, split into two planes, the offset for the 
               second bit plane is 8 bytes, or BytesPerTile / 2. */
            if(type == 4)
               address += BytesPerTile / 2;

            // Now that we have the complete address...
            if(mmc_check_address_lines)
               mmc_check_address_lines(address);

            /* The PPU manages memory using 1 kB pages, so we first have to find the proper page,
               then calculate the offset of the bytes containing the data for the two separate
               planes for this line of the tile bitmap. */
            const int page = address / PPU__PATTERN_TABLE_PAGE_SIZE;
            const uint8 *data = ppu__sprite_pattern_tables_read[page];

            unsigned offset = address & PPU__PATTERN_TABLE_PAGE_MASK;

            if(type == 3)
               // First tile bitmap.
               sprite.lowShift = data[offset];
            else
               // Second tile bitmap.
               sprite.highShift = data[offset];

            /* Mark sprite as active. This is only done when the sprite's bitmap is not
               transparent, otherwise it is ignored by the renderer. */
            if((sprite.lowShift + sprite.highShift) != 0x00)
               sprite.dead = FALSE;

            break;
         }
      }
   }
}

} // namespace Sprites
} // namespace Renderer
