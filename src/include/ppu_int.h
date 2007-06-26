/* FakeNES - A free, portable, Open Source NES emulator.

   ppu_int.h: Internal declarations for the PPU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef PPU_INT_H_INCLUDED
#define PPU_INT_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "cpu.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* delay for sprite 0 collision detection in PPU clocks */
/* should be <= SCANLINE_CLOCKS - 256 */
#define DOTS_HBLANK_BEFORE_RENDER 0

/* VRAM and sprite RAM. */
#define PPU_VRAM_BLOCK_READ_ADDRESS_SIZE 8
#define PPU_VRAM_BLOCK_BACKGROUND_CACHE_ADDRESS_SIZE 8
#define PPU_VRAM_BLOCK_BACKGROUND_CACHE_TAG_ADDRESS_SIZE 8
#define PPU_VRAM_BLOCK_SPRITE_CACHE_ADDRESS_SIZE 8
#define PPU_VRAM_BLOCK_SPRITE_CACHE_TAG_ADDRESS_SIZE 8
#define PPU_VRAM_BLOCK_WRITE_ADDRESS_SIZE 8
extern UINT8* ppu_vram_block_read_address[PPU_VRAM_BLOCK_READ_ADDRESS_SIZE];
extern UINT8* ppu_vram_block_background_cache_address[PPU_VRAM_BLOCK_BACKGROUND_CACHE_ADDRESS_SIZE];
extern UINT8* ppu_vram_block_background_cache_tag_address[PPU_VRAM_BLOCK_BACKGROUND_CACHE_TAG_ADDRESS_SIZE];
extern UINT8* ppu_vram_block_sprite_cache_address[PPU_VRAM_BLOCK_SPRITE_CACHE_ADDRESS_SIZE];
extern UINT8* ppu_vram_block_sprite_cache_tag_address[PPU_VRAM_BLOCK_SPRITE_CACHE_TAG_ADDRESS_SIZE];
extern UINT8* ppu_vram_block_write_address[PPU_VRAM_BLOCK_WRITE_ADDRESS_SIZE];

/*
 vram block identifiers
  0-7 = pattern VRAM
  8+  = pattern VROM
*/
#define FIRST_VROM_BLOCK 8

#define PPU_VRAM_BLOCK_SIZE 8
extern UINT32 ppu_vram_block[PPU_VRAM_BLOCK_SIZE];

#define PPU_VRAM_DIRTY_SET_BEGIN_SIZE 8
#define PPU_VRAM_DIRTY_SET_END_SIZE 8
extern INT32 ppu_vram_dirty_set_begin[PPU_VRAM_DIRTY_SET_BEGIN_SIZE];
extern INT32 ppu_vram_dirty_set_end[PPU_VRAM_DIRTY_SET_END_SIZE];
extern INT8 ppu_vram_cache_needs_update;

#define PPU_VRAM_DUMMY_WRITE_SIZE 1024
extern UINT8 ppu_vram_dummy_write[PPU_VRAM_DUMMY_WRITE_SIZE];

#define PPU_PATTERN_VRAM_SIZE 8 * 1024
#define PPU_PATTERN_VRAM_CACHE_SIZE 8 * 1024 / 2 * 8
#define PPU_PATTERN_VRAM_CACHE_TAG_SIZE 8 * 1024 / 2
extern UINT8 ppu_pattern_vram[PPU_PATTERN_VRAM_SIZE];
extern UINT8 ppu_pattern_vram_cache[PPU_PATTERN_VRAM_CACHE_SIZE];
extern UINT8 ppu_pattern_vram_cache_tag[PPU_PATTERN_VRAM_CACHE_TAG_SIZE];

#define PPU_NAME_TABLE_VRAM_SIZE 4 * 1024
#define PPU_NAME_TABLES_READ_SIZE 4
#define PPU_NAME_TABLES_WRITE_SIZE 4
extern UINT8 ppu_name_table_vram[PPU_NAME_TABLE_VRAM_SIZE];
extern UINT8* name_tables_read[PPU_NAME_TABLES_READ_SIZE];
extern UINT8* name_tables_write[PPU_NAME_TABLES_WRITE_SIZE];

/* Table containing expanded name/attribute data.  Used for MMC5. */
/* Use ppu_set_expansion_table_address(block) to set this, or ppu_set_expansion_table_address(NULL) to clear. */
/* The format should be identical to that used by MMC5. */
extern UINT8* ppu_expansion_table;

#define PPU_PALETTE_SIZE 32
extern UINT8 ppu_palette[PPU_PALETTE_SIZE];

#define PPU_SPR_RAM_SIZE 256
extern UINT8 ppu_spr_ram[PPU_SPR_RAM_SIZE];

#define ppu_background_palette ppu_palette
#define ppu_sprite_palette (ppu_palette + 16)

extern int ppu_mirroring;

#define PPU_GET_LINE_ADDRESS(bitmap, y)   (bitmap->line[y])
#define PPU_PUTPIXEL(bitmap, x, y, color) (bitmap->line[y][x] = color)
#define PPU_GETPIXEL(bitmap, x, y)        (bitmap->line[y][x])

extern unsigned vram_address;
extern UINT8 buffered_vram_read;

extern int address_write;
extern int address_temp;
extern int x_offset;
extern int address_increment;

extern UINT8 spr_ram_address;
extern int sprite_height;

extern BOOL want_vblank_nmi;

extern BOOL vblank_occurred;

extern UINT8 hit_first_sprite;
extern cpu_time_t first_sprite_this_line;

extern UINT16 background_tileset;
extern UINT16 sprite_tileset;

#define ATTRIBUTE_TABLE_SIZE 4
#ifdef ALLEGRO_I386
extern UINT32 attribute_table[ATTRIBUTE_TABLE_SIZE];
#else
extern UINT8 attribute_table[ATTRIBUTE_TABLE_SIZE];
#endif

#define BACKGROUND_PIXELS_SIZE 8 + 256 + 8
extern INT8 background_pixels[BACKGROUND_PIXELS_SIZE];

extern UINT8 palette_mask;

#ifdef __cplusplus
}
#endif
#endif /* !PPU_INT_H_INCLUDED */
