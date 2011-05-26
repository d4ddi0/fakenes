/* FakeNES - A free, portable, Open Source NES emulator.

   sprites.cpp: Implementation of the PPU sprites renderer.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <cstdlib>
#include <cstring>
#include "../include/common.h"
#include "../include/ppu.h"
#include "../include/ppu_int.h"
#include "../include/types.h"
#include "renderer.hpp"
#include "sprites.hpp"

// TODO: MMC latches support

namespace Renderer {
namespace Sprites {

namespace {

const int SpriteCount = 64;

// Data access.
const int BytesPerTile = 16;

const int Sprite_YPosition  = 0;
const int Sprite_TileIndex  = 1;
const int Sprite_Attributes = 2;
const int Sprite_XPosition  = 3;

#define SPRITE_DATA(_oam, _index, _data) \
   ( (_oam)[((_index) * BytesPerSprite) + (_data)] )

#define SPRITE_X_POSITION(_oam, _index) ( SPRITE_DATA((_oam), (_index), Sprite_XPosition) )
#define SPRITE_Y_POSITION(_oam, _index) ( SPRITE_DATA((_oam), (_index), Sprite_YPosition) )
#define SPRITE_TILE_INDEX(_oam, _index) ( SPRITE_DATA((_oam), (_index), Sprite_TileIndex) )
#define SPRITE_ATTRIBUTES(_oam, _index) ( SPRITE_DATA((_oam), (_index), Sprite_Attributes) )

#define PRIMARY_OAM     ( ppu_spr_ram )
#define SECONDARY_OAM	( render.secondaryOAM )

// Evaluation timings.
const int ClearCycleFirst      = 1;
const int ClearCycleLast       = ClearCycleFirst      + 63;	// 64 clocks
const int EvaluationCycleFirst = ClearCycleLast       + 1;
const int EvaluationCycleLast  = EvaluationCycleFirst + 191;	// 192 clocks
const int FetchCycleFirst      = EvaluationCycleLast  + 1;
const int FetchCycleLast       = FetchCycleFirst      + 63;	// 64 clocks
const int PipelineCycleFirst   = FetchCycleLast       + 1;
const int PipelineCycleLast    = PipelineCycleFirst   + 20;	// 21 clocks

const unsigned OAM_Bank = 1 << 0;
const unsigned OAM_Tile = 0xFF & ~OAM_Bank;

const unsigned Attribute_Palette  = 0x03;
const unsigned Attribute_Priority = 1 << 5;
const unsigned Attribute_HFlip    = 1 << 6;
const unsigned Attribute_VFlip    = 1 << 7;

inline void ClearSprites() {
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

inline void ClearEvaluation() {
   RenderEvaluationContext& e = render.evaluation;

   e.state = 1;
   e.substate = 1;
   e.count = 0;
   e.n = 0;
   e.m = 0;
   e.locked = FALSE;
   e.data = 0x00;
}

linear void Clear()
{
    ClearSprites();
    ClearEvaluation();

    memset(render.secondaryOAM, 0xFF, SecondaryOAMSize);
}

/* This function just performs minimal logic for a sprite. It's used when frame skipping,
    or when the frame buffer has been locked for writes. */
inline void Logic(RenderSpriteContext& sprite) {
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

} // namespace anonymous

// ----------------------------------------------------------------------
// PUBLIC INTERFACE
// ----------------------------------------------------------------------

#if !defined(INLINE_P2EC)

void Initialize() {
   Clear();
}

void Line() {
   // Copy evaluation count to spriteCount, as it will be overwritten
   render.spriteCount = render.evaluation.count;
}

#endif

/* Every cycle, the 8 x-position counters for the sprites are decremented by one. For each sprite, if the counter is still nonzero, nothing else happens. If the counter is zero, the sprite becomes "active", and the respective pair of shift registers for the sprite is shifted once every cycle. This output accompanies the data in the sprite's latch, to form a pixel. The current pixel for each "active" sprite is checked (from highest to lowest priority), and the first non-transparent pixel moves on to a multiplexer, where it joins the BG pixel.
    * If the sprite has foreground priority, the sprite pixel is output.
    * If the sprite has background priority:
          * If the BG pixel is zero, the sprite pixel is output.
          * If the BG pixel is nonzero, the BG pixel is output. */
inline void Pixel() {
    // Check if we should clip sprites on the left side of the screen
    bool clipping = false;
    if((render.pixel <= 7) && PPU_SPRITES_CLIP_ENABLED)
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

       uint8 pixel = 0;
       if(sprite.latch & Attribute_HFlip) {
          /* The pixel is formed of two bits taken from each shift register,
             giving a possible color range from 0-3. */
          pixel = (sprite.lowShift & 0x01) | ((sprite.highShift & 0x01) << 1);

          sprite.lowShift >>= 1;
          sprite.highShift >>= 1;
       }
       else {
          pixel = ((sprite.lowShift & 0x80) >> 7) | ((sprite.highShift & 0x80) >> 6);

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
          if(ppu__background_pixels[render.pixel] != 0)
             ppu__sprite_collision = TRUE;
       }

       if(sprite.latch & Attribute_Priority) {
          /* This is a back-priority sprite, so we should only plot pixels
             in areas where the background was transparent. */
          if(ppu__background_pixels[render.pixel] != 0)
             continue;

          // We don't want to draw if the back-priority layer is disabled, either.
          if(!ppu_enable_sprite_layer_a)
             continue;
       }
       else
          /* Similarly, don't draw if the front-priority layer is disabled,
             for front-priority sprites. */
          if(!ppu_enable_sprite_layer_b)
             continue;

       /* We can only plot pixels when:
             1) Sprite clipping is disabled, or the current pixel position is 8 or higher
             2) The sprite pixel is not transparent (color #0)
             3) The background pixel is transparent (for back-priority sprites)
             4) The frame buffer is not locked for writes
             5) Drawing of the associated sprite layer is not disabled */
       const int colormap = sprite.latch & Attribute_Palette;
       render.buffer[render.pixel] = PPU__SPRITE_PALETTE(colormap, pixel);
    }
}

inline void PixelStub() {
    for(int i = 0; i < render.spriteCount; i++)
       Logic(render.sprites[i]);
}

#define MAIN_LOOP	main_loop
#define STATE1_LOOP	state1_loop
#define STATE3_LOOP	state3_loop
#define END_LOOP	end_loop

#define READ_HELPER(_value) { \
   if(!cycle_is_even) \
      goto END_LOOP; \
   e.data = (_value); \
}

#define WRITE_HELPER(_value) { \
   if(!cycle_is_odd) \
      goto END_LOOP; \
   if(!e.locked) \
      (_value) = e.data; \
}

#define CONTINUE_1 { \
   e.substate++; \
   goto STATE1_LOOP; \
}

#define CONTINUE_3 { \
   e.substate++; \
   goto STATE3_LOOP; \
}

#define JUMP(_state) { \
   e.state = (_state); \
   e.substate = 1; \
   goto MAIN_LOOP; \
}

#define JUMP_1	JUMP(1)
#define JUMP_2	JUMP(2)
#define JUMP_3	JUMP(3)
#define JUMP_4	JUMP(4)

inline void Clock() {
   if(render.line == PPU_LAST_DISPLAYED_LINE)
      // No need to do anything here, as the next line cannot have sprites.
      // FIXME: Is this really correct?
      return;

  /* Kind of ugly, but it works. Note that emulation starts on an odd cycle (1),
      and most writes take place on even cycles (2, 4, 6, etc.). */
   const int cycle = render.clock;
   const bool cycle_is_even = (cycle % 2) == 0;
   const bool cycle_is_odd = !cycle_is_even;

   const int line = render.line + 1;

   if(cycle <= ClearCycleLast) {
      /* Cycles 0-63: Secondary OAM (32-byte buffer for current sprites on scanline) is initialized to $FF -
         attempting to read $2004 will return $FF */

      // Note: Each byte takes 2 PPU cycles to clear.
      if(cycle_is_even)
         SECONDARY_OAM[(cycle / 2) - 1] = 0xFF;
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

      RenderEvaluationContext& e = render.evaluation;

   MAIN_LOOP:
      if(e.state == 1) {
         /* 1. Starting at n = 0, read a sprite's Y-coordinate (OAM[n][0],
               copying it to the next open slot in secondary OAM (unless 8 sprites have been found,
               in which case the write is ignored). */
      STATE1_LOOP:
         switch(e.substate) {
            // Y-Position
            case 1: {
               READ_HELPER( SPRITE_Y_POSITION(PRIMARY_OAM, e.n) );
               CONTINUE_1
            }
            case 2: {
               WRITE_HELPER( SPRITE_Y_POSITION(SECONDARY_OAM, e.count) );

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
               READ_HELPER( SPRITE_TILE_INDEX(PRIMARY_OAM, e.n) );
               CONTINUE_1
            }
            case 4: {
               WRITE_HELPER( SPRITE_TILE_INDEX(SECONDARY_OAM, e.count) );
               CONTINUE_1
            }

            // Attribute
            case 5: {
               READ_HELPER( SPRITE_ATTRIBUTES(PRIMARY_OAM, e.n) );
               CONTINUE_1
            }
            case 6: {
               WRITE_HELPER( SPRITE_ATTRIBUTES(SECONDARY_OAM, e.count) );
               CONTINUE_1
            }

            // X-Position
            case 7: {
               READ_HELPER( SPRITE_X_POSITION(PRIMARY_OAM, e.n) );
               CONTINUE_1
            }
            case 8: {
               WRITE_HELPER( SPRITE_X_POSITION(SECONDARY_OAM, e.count) );

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
               const uint8 data = SPRITE_DATA(PRIMARY_OAM, e.n, e.m);

               if((line >= data) && (line < (data + ppu__sprite_height))) {
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
            case 2:
            case 3:
            case 4: {
               READ_HELPER( SPRITE_DATA(PRIMARY_OAM, e.n, e.m) );
               e.m++;
               /* Increment n when m overflows (from 0-3 -> 4) seems more correct than
                  incrementing n when m is equal to 3, despite the above. */
               if(e.m > 3) {
                  e.m = 0;
                  e.n++;
               }

               CONTINUE_3
            }

            case 5:
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
         // We're already in HBlank, so it's safe to clear data here
         ClearSprites();

      /* As each fetch takes 2 cycles to complete, we only care about even cycles,
         emulation-wise, at least. */
      if(cycle_is_even) {
         const int position = cycle - FetchCycleFirst;	// 0-63
         const int index = position / 8;		// 0-7

         /* We don't need to bother with sprites that aren't active. They get loaded with a transparent
            bitmap instead, although we never even try to render them for performance sake. */
         if(index < render.evaluation.count) {
            const int type = ((position - (index * 8)) / 2) + 1; // 1-4

            // To make things a little cleaner, we'll get a direct reference.
            RenderSpriteContext& sprite = render.sprites[index];

            /* We need to keep track of the original index (0-63) for sprite #0 hit detection.
               This is filled in during sprite evaluation for each sprite. */
            sprite.index = render.evaluation.indices[index];

            /* The exact time when the attribute byte and X position are loaded into the
               latch and counter (respectively) is unknown, however we have a perfectly
               good oppertunity to do it here with the tile data, so we will. */
            switch(type) {
               // Garbage
               case 1:
                  // Load latch and counter
                  sprite.latch = SPRITE_ATTRIBUTES(SECONDARY_OAM, index);
                  sprite.counter = SPRITE_X_POSITION(SECONDARY_OAM, index);
                  break;

               // Tile bitmap A
               case 3:	
               // Tile bitmap B	
               case 4: {
                  /* It's inefficient to execute all of this twice, but there isn't really any other way
                     while still keeping it aligned to the proper clock cycles (for now).  */

                  const int tile = SPRITE_TILE_INDEX(SECONDARY_OAM, index);
                  const int y = SPRITE_Y_POSITION(SECONDARY_OAM, index);

                  unsigned address;
                  if(ppu__sprite_height == 8)
                     // Render 8x8 sprites.
                     address = (tile * BytesPerTile) + ppu__sprite_tileset;
                  else {
                     // Render 8x16 sprites.
                     unsigned bank = 0x0000;
                     if(tile & OAM_Bank)
                        bank = 0x1000;

                     address = ((tile & OAM_Tile) * BytesPerTile) + bank;
                  }

                  /* Each line of the plane data for the tile bitmap is a single byte, so this is
                     simply used as a byte offset. */
                  int row;
                  if(sprite.latch & Attribute_VFlip)
                     row = (y + (ppu__sprite_height - 1)) - line;
                  else
                     row = line - y;

                  /* The PPU manages memory using 1 kB pages, so we first have to find the proper page,
                     then calculate the offset of the bytes containing the data for the two separate
                     planes for this line of the tile bitmap. */
                  const unsigned page = address >> 10;
                  const uint8 *data = ppu_vram_block_read_address[page];
                  unsigned offset = address - (page << 10);

                  /* For 8x16 sprites, we may need to jump to the next tile.
                     This occurs on row indices 8-15, which then become 0-7 after the offset. */
                  if(row >= 8) {
                     offset += BytesPerTile;
                     row -= 8;
                  }

                  if(type == 3)
                     // Tile bitmap A
                     sprite.lowShift = data[offset + row];
                  else
                     // Tile bitmap B
                     sprite.highShift = data[offset + (BytesPerTile / 2) + row];

                  /* Mark sprite as active. This is only done when the sprite's bitmap is not
                     transparent, otherwise it is ignored by the renderer. */
                  if((sprite.lowShift + sprite.highShift) != 0x00)
                     sprite.dead = FALSE;

                  break;
               }
            }
         }
      }
   }
}

} // namespace Sprites
} // namespace Renderer
