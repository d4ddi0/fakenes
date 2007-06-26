/* FakeNES - A free, portable, Open Source NES emulator.

   background.cpp: Implementation of the PPU background renderer.

   Copyright (c) 2001-2007, FakeNES Team.
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
#include "background.hpp"
#include "renderer.hpp"

/* VRAM address bit layout:
      -YYY VHyy yyyx xxxx
         x = x tile offset in name table
         y = y tile offset in name table
         H = horizontal name table
         V = vertical name table
         Y = y line offset in tile */

// Define some constants.
static const int TileWidth = 8;
static const int TileHeight = 8;

static const int RenderWidth = 256;
static const int RenderWidthTiles = (RenderWidth / TileWidth);

static const int RenderHeight = 240;

// amount of space a tile consumes in CHR
static const int BytesPerTile = (TileWidth * TileHeight) / 4; // 4 pixels per byte

// offset of attributes in nametables
static const unsigned AttributeBase = (RenderWidth / TileWidth) * (RenderHeight / TileHeight);

static const unsigned AttributeMask = 0x03;

// shifts/mask for extended attributes in MMC5 ExRAM
static const int ExpansionAttributeShifts = 6;
static const unsigned ExpansionAttributeMask = 0x03;

// Variables specific to rendering.
static uint8* plotBuffer = null;
static int plotPixel = 0;

static unsigned vramAddressBG = 0;

static unsigned attributeOffset = 0;
static int nametable = 0;
static uint8* nametableAddress = null;
static uint8 attributeByte = 0;

// used for left edge clipping etc
static int x = 0;

static int scrollTileX = 0, scrollTileXOffset = 0;
static int scrollTileY = 0, scrollTileYOffset = 0;

static uint8 tileName = 0x00;
static unsigned tileAddress = 0;
static uint8* cacheAddress = null;
static unsigned cacheBank = 0, cacheIndex = 0;
static uint8 cacheTag = 0;
static uint8 attribute = 0x00;

/* this is currently just an inefficient line renderer built on top of a pixel engine
   more of a proof of concept than anything right now */
void rendererRenderBackgroundLine(int line)
{
   plotBuffer = PPU_GET_LINE_ADDRESS(video_buffer, line);
 
   // start with completely empty scene
   memset(plotBuffer, (ppu_background_palette[0] & palette_mask) + PALETTE_ADJUST, RenderWidth);

   // used for sprite pixel allocation and collision detection
   memset(background_pixels + (1 * TileWidth), 0, RenderWidth);

   // start a new line
   plotPixel = 0;

   for(x = 0; x < (RenderWidthTiles + 1); x++) {
      for(int pixel = 0; pixel < TileWidth; pixel++) {
         // determine which nametable to use
         nametable = (vram_address >> 10) & 0x03;
         nametableAddress = name_tables_read[nametable];

         // derive scrolling values from the VRAM address
         scrollTileX = (vram_address & 0x1F);
         scrollTileXOffset = x_offset;

         scrollTileY = (vram_address >> 5) & 0x1F;
         scrollTileYOffset = (vram_address >> 12) & 0x07;

         // nametable/exram entry
         vramAddressBG = vram_address & 0x3FF;

         // fetch tile from nametable
         tileName = nametableAddress[vramAddressBG];
         tileAddress = ((tileName * BytesPerTile) + background_tileset);

         // give the mapper a chance to do banking or other related tasks prior to accessing the CHR data
         if(mmc_check_latches) {
            // Note: This is currently rigged for MMC2/4 ONLY.
            if((tileName >= 0xFD) && (tileName <= 0xFE))
               mmc_check_latches(tileAddress);
         }

         // CHR and attribute stuff.....
         cacheBank = tileAddress >> 10;
         cacheIndex = ((tileAddress & 0x3FF) / 2) + scrollTileYOffset;

         unsigned vromBlock;
         if(ppu_expansion_table) {
            const unsigned exramBlock = ppu_expansion_table[vramAddressBG] & 0x3F;

            vromBlock = ((tileAddress >> 10) & 3) + (exramBlock << 2);
            vromBlock = (vromBlock & 7) +
               ROM_CHR_ROM_PAGE_LOOKUP[(vromBlock / 8) &
               ROM_CHR_ROM_PAGE_OVERFLOW_MASK] * 8;

            cacheTag = ( ROM_CHR_ROM_CACHE_TAG + ((vromBlock << 10) / 2) ) [cacheIndex];
         }
         else
            cacheTag = ppu_vram_block_background_cache_tag_address[cacheBank][cacheIndex];

         if(ppu_expansion_table) {
            cacheAddress = ROM_CHR_ROM_CACHE + ((vromBlock << 10) / 2 * 8) + cacheIndex * 8;

            attribute = attribute_table[(ppu_expansion_table[vramAddressBG] >> ExpansionAttributeShifts) &
               ExpansionAttributeMask];
         }
         else {
            cacheAddress = ppu_vram_block_background_cache_address[cacheBank] + cacheIndex * 8;

            // Attribute bytes store color data for 32x32 pixels worth of tiles, so 32 is our divisor.
            attributeOffset = ((((scrollTileY * TileWidth) + scrollTileYOffset) / 32) *
                (RenderWidth / 32)) +
               (((scrollTileX * TileWidth) + scrollTileXOffset) / 32);

            attributeByte = nametableAddress[AttributeBase + attributeOffset];

            /* Attribute shift table:
                  X Odd   Y Odd   0 shifts
                  X Even  Y Odd   2 shifts
                  X Odd   Y Even  4 shifts
                  X Even  Y Even  6 shifts
               We can get the same behavior simply by ORing the masked bits together. =) */
            const int shifts = (scrollTileX & 2) | ((scrollTileY & 2) << 1);
            attribute = attribute_table[(attributeByte & (AttributeMask << shifts)) >> shifts];
         }

         // finally, plot our pixel
         rendererRenderBackgroundPixel();

         // subtile(per pixel) scrolling
         x_offset++;
         if(x_offset >= TileWidth) {
            x_offset = 0;

            // on to the next tile.....
            vramAddressBG++;
            if((vramAddressBG & 0x1F) == 0) {
               // horizontal name table toggle
               nametable ^= 1;
               // handle address wrap
               vramAddressBG = (vramAddressBG - (1 << 5));
            }
         }

         // repack changes into vram_address, to be decoded again on the next pixel
         vram_address = vramAddressBG + (nametable << 10) + (scrollTileYOffset << 12);

         // on to the next pixel, I say! (It's about time.)
         plotPixel++;
      }
   }
}

void rendererRenderBackgroundPixel(void)
{
   if(!cacheTag) {
      // Current tile is completely transparent - skip it.
      return;
   }

   const uint8 color = cacheAddress[scrollTileXOffset] & attribute;
   if(color == 0) {
      // Pixel is transparent - skip it.
      return;
   }

   background_pixels[(1 * TileWidth) + plotPixel] = color;

   if(!ppu_enable_background_layer)
      return;

   if((x == 0) &&
      PPU_BACKGROUND_CLIP_ENABLED)
      return;

   plotBuffer[plotPixel] = (ppu_background_palette[color] & palette_mask) + PALETTE_ADJUST;
}

