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

namespace Renderer {

namespace {

// Amount of space a tile consumes in CHR (at 4 pixels per byte).
const int BytesPerTile = (TileWidth * TileHeight) / 4;

// Offset of attributes in nametables, and associated mask.
const unsigned AttributeBase = (DisplayWidth / TileWidth) * (DisplayHeight / TileHeight);
const unsigned AttributeMask = 0x03;

// Shifts and mask for extended attributes in MMC5 ExRAM.
const int ExpansionAttributeShifts = 6;
const unsigned ExpansionAttributeMask = 0x03;

void Clear() {
   render.background.tile = 0;
   render.background.pixel = 0;
}

void UpdatePPU( int nametable, const int scrollTileYOffset, unsigned vramAddressBG )
{
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
}

void Step()
{
   RenderBackgroundContext& current = render.background;

   // Proceed to the next pixel of the current tile.
   current.pixel++;
   if(current.pixel >= TileWidth) {
      // Done with this tile, on to the next one.
      current.tile++;
      current.pixel = 0;
   }
}

} // namespace anonymous

void BackgroundInit()
{
   Clear();
}

void BackgroundLine()
{
   Clear();
}

void BackgroundPixel()
{
   /* VRAM address bit layout:
         -YYY VHyy yyyx xxxx
            x = x tile offset in name table
            y = y tile offset in name table
            H = horizontal name table
            V = vertical name table
            Y = y line offset in tile */

   // determine which nametable to use
   const int nametable = (vram_address >> 10) & 0x03;
   const uint8* nametableAddress = name_tables_read[nametable];

   // derive scrolling values from the VRAM address
   const int scrollTileX = (vram_address & 0x1F);
   const int scrollTileXOffset = x_offset;

   const int scrollTileY = (vram_address >> 5) & 0x1F;
   const int scrollTileYOffset = (vram_address >> 12) & 0x07;

   // nametable/exram entry
   const unsigned vramAddressBG = vram_address & 0x3FF;

   // fetch tile from nametable
   const uint8 tileName = nametableAddress[vramAddressBG];
   const unsigned tileAddress = ((tileName * BytesPerTile) + background_tileset);

   // give the mapper a chance to do banking or other related tasks prior to accessing the CHR data
   if(mmc_check_latches) {
      // Note: This is currently rigged for MMC2/4 ONLY.
      if((tileName >= 0xFD) && (tileName <= 0xFE))
         mmc_check_latches(tileAddress);
   }

   // CHR and attribute stuff.....
   const unsigned cacheBank = tileAddress >> 10;
   const unsigned cacheIndex = ((tileAddress & 0x3FF) / 2) + scrollTileYOffset;

   uint8 cacheTag = 0;
   uint8* cacheAddress = null;
   uint8 attribute = 0x00;
   if(ppu_expansion_table) {
      const unsigned exramBlock = ppu_expansion_table[vramAddressBG] & 0x3F;

      unsigned vromBlock;
      vromBlock = ((tileAddress >> 10) & 3) + (exramBlock << 2);
      vromBlock = (vromBlock & 7) +
         ROM_CHR_ROM_PAGE_LOOKUP[(vromBlock / 8) &
         ROM_CHR_ROM_PAGE_OVERFLOW_MASK] * 8;

      cacheTag = ( ROM_CHR_ROM_CACHE_TAG + ((vromBlock << 10) / 2) ) [cacheIndex];
      cacheAddress = ROM_CHR_ROM_CACHE + ((vromBlock << 10) / 2 * 8) + cacheIndex * 8;

      attribute = attribute_table[(ppu_expansion_table[vramAddressBG] >> ExpansionAttributeShifts) &
         ExpansionAttributeMask];
   }
   else {
      cacheTag = ppu_vram_block_background_cache_tag_address[cacheBank][cacheIndex];
      cacheAddress = ppu_vram_block_background_cache_address[cacheBank] + cacheIndex * 8;

      // Attribute bytes store color data for 32x32 pixels worth of tiles, so 32 is our divisor.
      unsigned attributeOffset = ((((scrollTileY * TileWidth) + scrollTileYOffset) / 32) *
          (DisplayWidth / 32)) +
         (((scrollTileX * TileWidth) + scrollTileXOffset) / 32);

      const uint8 attributeByte = nametableAddress[AttributeBase + attributeOffset];

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
   bool transparent = true;
   uint8 color = 0;

   if(cacheTag) {
      // Non-transparent tile.
      color = cacheAddress[scrollTileXOffset] & attribute;
      if(color != 0) {
         // Non-transparent pixel.
         if((render.background.tile > 0) || !PPU_BACKGROUND_CLIP_ENABLED)
            // Non-clipped pixel.
            transparent = false;
      }
   }

   if(transparent) {
      PPU__PUT_BACKGROUND_PIXEL(render.pixel, 0);
      render.buffer[render.pixel] = PPU__BACKGROUND_PALETTE(0);
   }
   else {
      PPU__PUT_BACKGROUND_PIXEL(render.pixel, color);
      render.buffer[render.pixel] = ppu_enable_background_layer ?
         PPU__BACKGROUND_PALETTE(color) : PPU__BACKGROUND_PALETTE(0);
   }

   // Update internal registers
   UpdatePPU( nametable, scrollTileYOffset, vramAddressBG );

   // Move on to the next pixel
   Step();
}

// Frame-skipping variant of BackgroundPixel()
void BackgroundPixelStub() {

   // Emulate a bare subset of the full pipeline.
   const int nametable = (vram_address >> 10) & 0x03;
   const int scrollTileYOffset = (vram_address >> 12) & 0x07;
   const unsigned vramAddressBG = vram_address & 0x3FF;

   UpdatePPU( nametable, scrollTileYOffset, vramAddressBG );

   /* As nothing is being rendered, produce a transparent pixel. The video
      buffer is not updated this time, as we are frame-skipping. */
   PPU__PUT_BACKGROUND_PIXEL(render.pixel, 0);

   // Move on to the next pixel
   Step();
}

/* PPU sleep mode - just do minimal processing, do not draw any pixels
   or affect any registers. */
void BackgroundPixelSkip() {
   Step();
}

} // namespace Renderer
