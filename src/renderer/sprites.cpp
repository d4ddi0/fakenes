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

namespace Renderer {

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

// TODO: Double check these
const unsigned OAM_Bank = 1 << 1;

const unsigned Attribute_HFlip    = 1 << 7;
const unsigned Attribute_VFlip    = 1 << 6;
const unsigned Attribute_Priority = 1 << 5;

void ClearSprites() {
    for(int i = 0; i < SpritesPerLine; i++) {
       RenderSpriteContext& sprite = render.sprites[i];
       sprite.lowShift = 0;
       sprite.highShift = 0;
       sprite.latch = 0;
       sprite.counter = 0;
    }

    render.spriteCount = 0;
}

void ClearEvaluation() {
   RenderEvaluationContext& e = render.evaluation;

   e.state = 1;
   e.substate = 1;
   e.count = 0;
   e.n = 0;
   e.m = 0;
   e.locked = FALSE;
   e.data = 0x00;
}

void Clear()
{
    ClearSprites();
    ClearEvaluation();

    memset(render.secondaryOAM, 0xFF, SecondaryOAMSize);
}

} // namespace anonymous

void SpriteInit() {
   Clear();
}

void SpriteLine() {
   // Copy evaluation count to spriteCount, as it will be overwritten
   render.spriteCount = render.evaluation.count;
}

void SpritePixel() {
    for(int i = 0; i < render.spriteCount; i++) {
       RenderSpriteContext& sprite = render.sprites[i];

       if(sprite.counter > 0)
          sprite.counter--;
       if(sprite.counter > 0)
          continue;

       uint8 pixel = (sprite.lowShift & 0x01) | ((sprite.highShift & 0x01) << 1);
       if(pixel != 0)
          render.buffer[render.pixel] = pixel;

       sprite.lowShift >>= 1;
       sprite.highShift >>= 1;
    }
}

void SpritePixelStub() {
}

void SpritePixelSkip() {
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

void SpriteClock() {
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
               if((line >= e.data) && (line <= (e.data + ppu__sprite_height)))
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

               if((line >= data) && (line <= (data + ppu__sprite_height))) {
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
         //if(index < render.evaluation.count) {
         if(1) {
            const int type = ((position - (index * 8)) / 2) + 1; // 1-4

            RenderSpriteContext& sprite = render.sprites[index];

            /* The exact time when the attribute byte and X position are loaded into the
               latch and counter (respectively) is unknown, however we have a perfectly
               good oppertunity to do it here with the tile data, so we will. */
            switch(type) {
               case 1:		// Garbage
                  // Load latch and counter
                  sprite.latch = SPRITE_ATTRIBUTES(SECONDARY_OAM, index);
                  sprite.counter = SPRITE_X_POSITION(SECONDARY_OAM, index);
                  break;

               case 3:		// Tile bitmap A
               case 4: {	// Tile bitmap B
                  /* It's inefficient to execute all of this twice, but there isn't really any other way
                     while still keeping it aligned to the proper clock cycles (for now).  */

                  const int tile = SPRITE_TILE_INDEX(SECONDARY_OAM, index);

                  unsigned address;
                  if(ppu__sprite_height == 8)
                     // Render 8x8 sprites.
                     address = (tile * BytesPerTile) + ppu__sprite_tileset;
                  else {
                     // Render 8x16 sprites.
                     if(tile & OAM_Bank)
                        // Use bank starting at $1000
                        address = ((tile - 1) * BytesPerTile) + 0x1000;
                     else
                        // Use bank starting at $0000
                        address = tile * BytesPerTile;
                  }

                  const int y = SPRITE_Y_POSITION(SECONDARY_OAM, index);
                  /* Each line of the plane data for the tile bitmap is a single byte, so this is
                     simply used as a byte offset. */
                  const int row = line - y;

                  /* The PPU manages memory using 1 kB pages, so we first have to find the proper page,
                     then calculate the offset of the bytes containing the data for the two separate
                     planes for this line of the tile bitmap. */
                  const unsigned page = address >> 10;
                  const uint8 *data = ppu_vram_block_read_address[page];
                  const unsigned offset = address - (page << 10);

                  // TODO: MMC latches support

                  if(type == 3)
                     // Tile bitmap A
                     sprite.lowShift = data[offset + row];
                  else
                     // Tile bitmap B
                     sprite.highShift = data[offset + (BytesPerTile / 2) + row];

                  break;
               }
            }
         }
      }
   }
}

} // namespace Renderer
