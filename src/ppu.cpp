/* FakeNES - A free, portable, Open Source NES emulator.

   ppu.cpp: Implementation of the PPU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */
 
#include <allegro.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "common.h"
#include "cpu.h"
#include "input.h"
#include "mmc.h"
#include "ppu.h"
#include "ppu_int.h"
#include "rom.h"
#include "renderer/renderer.hpp"
#include "renderer/background.hpp"
#include "shared/crc32.h"
#include "timing.h"
#include "types.h"
#include "video.h"

static void do_spr_ram_dma(uint8 page);
static void synchronize(void);

static cpu_time_t ppu_clock_counter = 0;
static cpu_time_t ppu_clock_buffer = 0;

static int16 ppu_scanline_timer = 0;
static int16 ppu_scanline = 0;

/* IRQ prediction. */
static cpu_time_t ppu_prediction_timestamp = 0;
static cpu_time_t ppu_prediction_cycles = 0;

static void ppu_repredict_nmi(void);

static bool ppu_odd_frame = false;
static int ppu_mirroring;

BOOL ppu__rendering_enabled = TRUE;
BOOL ppu__force_rendering = FALSE;
static BOOL ppu_cache_rendering_enabled = TRUE;

UINT8 ppu_register_2000 = 0;
UINT8 ppu_register_2001 = 0;
BOOL ppu_enable_sprite_layer_a = TRUE;
BOOL ppu_enable_sprite_layer_b = TRUE;
BOOL ppu_enable_background_layer = TRUE;

BOOL background_enabled = FALSE;
BOOL sprites_enabled = FALSE;
BOOL ppu__sprite_collision = FALSE;
BOOL ppu__sprite_overflow = FALSE;

UINT8* one_screen_base_address = NULL;

/* VRAM and sprite RAM. */
/* Note that array sizes are defined in ppu_int.h. */
UINT8* ppu_vram_block_read_address[PPU_VRAM_BLOCK_READ_ADDRESS_SIZE];
UINT8* ppu_vram_block_background_cache_address[PPU_VRAM_BLOCK_BACKGROUND_CACHE_ADDRESS_SIZE];
UINT8* ppu_vram_block_background_cache_tag_address[PPU_VRAM_BLOCK_BACKGROUND_CACHE_TAG_ADDRESS_SIZE];
UINT8* ppu_vram_block_sprite_cache_address[PPU_VRAM_BLOCK_SPRITE_CACHE_ADDRESS_SIZE];
UINT8* ppu_vram_block_sprite_cache_tag_address[PPU_VRAM_BLOCK_SPRITE_CACHE_TAG_ADDRESS_SIZE];
UINT8* ppu_vram_block_write_address[PPU_VRAM_BLOCK_WRITE_ADDRESS_SIZE];

UINT32 ppu_vram_block[PPU_VRAM_BLOCK_SIZE];

INT32 ppu_vram_dirty_set_begin[PPU_VRAM_DIRTY_SET_BEGIN_SIZE];
INT32 ppu_vram_dirty_set_end[PPU_VRAM_DIRTY_SET_END_SIZE];
INT8 ppu_vram_cache_needs_update;

UINT8 ppu_vram_dummy_write[PPU_VRAM_DUMMY_WRITE_SIZE];

UINT8 ppu_pattern_vram[PPU_PATTERN_VRAM_SIZE];
UINT8 ppu_pattern_vram_cache[PPU_PATTERN_VRAM_CACHE_SIZE];
UINT8 ppu_pattern_vram_cache_tag[PPU_PATTERN_VRAM_CACHE_TAG_SIZE];

UINT8 ppu_name_table_vram[PPU_NAME_TABLE_VRAM_SIZE];
UINT8* name_tables_read[PPU_NAME_TABLES_READ_SIZE];
UINT8* name_tables_write[PPU_NAME_TABLES_WRITE_SIZE];

/* Table containing expanded name/attribute data.  Used for MMC5. */
/* Use ppu_set_expansion_table_address(block) to set this, or ppu_set_expansion_table_address(NULL) to clear. */
/* The format should be identical to that used by MMC5. */
UINT8 *ppu_expansion_table = NULL;

UINT8 ppu_palette[PPU_PALETTE_SIZE];

UINT8 ppu_spr_ram[PPU_SPR_RAM_SIZE];


unsigned vram_address = 0;
UINT8 buffered_vram_read = 0;

int address_write = 0;
int address_temp = 0;
int x_offset = 0;
int address_increment = 1;

UINT8 spr_ram_address = 0;
int sprite_height = 8;

BOOL want_vblank_nmi = FALSE;

BOOL vblank_occurred = FALSE;

UINT16 background_tileset = 0;
UINT16 sprite_tileset = 0;

#ifdef ALLEGRO_I386
UINT32 attribute_table[ATTRIBUTE_TABLE_SIZE];
#else
UINT8 attribute_table[ATTRIBUTE_TABLE_SIZE];
#endif

INT8 background_pixels[BACKGROUND_PIXELS_SIZE];

UINT8 palette_mask = 0x3f;

#include "ppu/tiles.h"

static UINT8 vram_read(UINT16 address)
{
   if(address < 0x2000)
      return ppu_vram_block_read_address[address >> 10][address & 0x3FF];
   else
      return name_tables_read[(address >> 10) & 0x03][address & 0x3FF];
}

void ppu_free_chr_rom(ROM *rom)
{
   RT_ASSERT(rom);

   if(rom->chr_rom) {
      free(rom->chr_rom);
      rom->chr_rom = NULL;
   }

   if(rom->chr_rom_cache) {
      free(rom->chr_rom_cache);
      rom->chr_rom_cache = NULL;
   }

   if(rom->chr_rom_cache_tag) {
      free(rom->chr_rom_cache_tag);
      rom->chr_rom_cache_tag = NULL;
   }
}

UINT8* ppu_get_chr_rom_pages(ROM *rom)
{
   RT_ASSERT(rom);

   int num_pages = rom -> chr_rom_pages;
   int copycount, missing, count, next, pages_mirror_size;

   /* Compute a mask used to wrap invalid CHR ROM page numbers.
    *  As CHR ROM uses a 8k page size, this mask is based
    *  on a 8k page size.
    */
   if(((num_pages * 2 - 1) & (num_pages - 1)) == (num_pages - 1)) {
      /* compute mask for even power of two */
      pages_mirror_size = num_pages;
   }
   else {
      /* compute mask */
      int i;

      /* compute the smallest even power of 2 greater than
         CHR ROM page count, and use that to compute the mask */
      for (i = 0; (num_pages >> i) > 0; i++);

      pages_mirror_size = (1 << i);
   }

   rom->chr_rom_page_overflow_mask = pages_mirror_size - 1;

   /* identify-map all the present pages */
   for(copycount = 0; copycount < num_pages; copycount++)
      rom->chr_rom_page_lookup[copycount] = copycount;


    /* mirror-map all the not-present pages */
    for (next = num_pages, missing = pages_mirror_size - num_pages,
        count = 1; missing; count <<= 1, missing >>= 1)
    {
        if (missing & 1)
        {
            for (copycount = count; copycount; copycount--, next++)
            {
                rom -> chr_rom_page_lookup[next] =
                    rom -> chr_rom_page_lookup[next - count];
            }
        }
    }


    /* 8k CHR ROM page size */
    rom -> chr_rom = (UINT8*)malloc (num_pages * 0x2000);

    /* 2-bit planar tiles converted to 8-bit chunky */
    rom -> chr_rom_cache = (UINT8*)malloc ((num_pages * 0x2000) / 2 * 8);
    rom -> chr_rom_cache_tag = (UINT8*)malloc ((num_pages * 0x2000) / 2);

    if (rom -> chr_rom == 0 || rom -> chr_rom_cache == 0 ||
     rom -> chr_rom_cache_tag == 0)
    {
        if (rom -> chr_rom) free (rom -> chr_rom);
        if (rom -> chr_rom_cache) free (rom -> chr_rom_cache);
        if (rom -> chr_rom_cache_tag) free (rom -> chr_rom_cache_tag);

        rom -> chr_rom = NULL;
        rom -> chr_rom_cache = NULL;
        rom -> chr_rom_cache_tag = NULL;
    }
    else
    {
        /* initialize to a known value for areas not present in image */
        memset (rom -> chr_rom, 0xFF, (num_pages * 0x2000));
    }

    return rom -> chr_rom;
}

void ppu_set_ram_1k_pattern_vram_block(UINT16 block_address, int vram_block)
{
   synchronize();

   ppu_vram_block [block_address >> 10] = vram_block;

   ppu_vram_block_read_address [block_address >> 10] =
      ppu_pattern_vram + (vram_block << 10);
   ppu_vram_block_write_address [block_address >> 10] =
     ppu_pattern_vram + (vram_block << 10);

   ppu_vram_block_background_cache_address [block_address >> 10] =
   ppu_vram_block_sprite_cache_address [block_address >> 10] =
      ppu_pattern_vram_cache + ((vram_block << 10) / 2 * 8);

   ppu_vram_block_background_cache_tag_address [block_address >> 10] =
   ppu_vram_block_sprite_cache_tag_address [block_address >> 10] =
      ppu_pattern_vram_cache_tag + ((vram_block << 10) / 2);
}

void ppu_set_ram_1k_pattern_vrom_block(UINT16 block_address, int vrom_block)
{
   synchronize();

   vrom_block = (vrom_block & 7) + ROM_CHR_ROM_PAGE_LOOKUP
      [(vrom_block / 8) & ROM_CHR_ROM_PAGE_OVERFLOW_MASK] * 8;

   ppu_vram_block [block_address >> 10] =
      FIRST_VROM_BLOCK + vrom_block;

   ppu_vram_block_read_address [block_address >> 10] =
      ROM_CHR_ROM + (vrom_block << 10);
   ppu_vram_block_write_address [block_address >> 10] =
      ppu_vram_dummy_write;

   ppu_vram_block_background_cache_address [block_address >> 10] =
   ppu_vram_block_sprite_cache_address [block_address >> 10] =
      ROM_CHR_ROM_CACHE + ((vrom_block << 10) / 2 * 8);

   ppu_vram_block_background_cache_tag_address [block_address >> 10] =
   ppu_vram_block_sprite_cache_tag_address [block_address >> 10] =
      ROM_CHR_ROM_CACHE_TAG + ((vrom_block << 10) / 2);
}

void ppu_set_ram_1k_pattern_vrom_block_ex(UINT16 block_address,
 int vrom_block, int map_type)
{
   synchronize();

   vrom_block = (vrom_block & 7) + ROM_CHR_ROM_PAGE_LOOKUP
      [(vrom_block / 8) & ROM_CHR_ROM_PAGE_OVERFLOW_MASK] * 8;

   if(map_type & PPU_MAP_RAM) {
      ppu_vram_block [block_address >> 10] =
         FIRST_VROM_BLOCK + vrom_block;

      ppu_vram_block_read_address [block_address >> 10] =
         ROM_CHR_ROM + (vrom_block << 10);
      ppu_vram_block_write_address [block_address >> 10] =
         ppu_vram_dummy_write;
   }

   if(map_type & PPU_MAP_BACKGROUND) {
      ppu_vram_block_background_cache_address [block_address >> 10] =
         ROM_CHR_ROM_CACHE + ((vrom_block << 10) / 2 * 8);

      ppu_vram_block_background_cache_tag_address [block_address >> 10] =
         ROM_CHR_ROM_CACHE_TAG + ((vrom_block << 10) / 2);
   }

   if(map_type & PPU_MAP_SPRITES) {
      ppu_vram_block_sprite_cache_address [block_address >> 10] =
         ROM_CHR_ROM_CACHE + ((vrom_block << 10) / 2 * 8);

      ppu_vram_block_sprite_cache_tag_address [block_address >> 10] =
         ROM_CHR_ROM_CACHE_TAG + ((vrom_block << 10) / 2);
   }
}

void ppu_set_ram_8k_pattern_vram(void)
{
   synchronize();

   ppu_set_ram_1k_pattern_vram_block(0x0000, 0);
   ppu_set_ram_1k_pattern_vram_block(0x0400, 1);
   ppu_set_ram_1k_pattern_vram_block(0x0800, 2);
   ppu_set_ram_1k_pattern_vram_block(0x0C00, 3);
   ppu_set_ram_1k_pattern_vram_block(0x1000, 4);
   ppu_set_ram_1k_pattern_vram_block(0x1400, 5);
   ppu_set_ram_1k_pattern_vram_block(0x1800, 6);
   ppu_set_ram_1k_pattern_vram_block(0x1C00, 7);
}

int ppu_init(void)
{
   int i;

   /* compute CRC32 for CHR ROM */
   if(global_rom.chr_rom_pages > 0) {
      global_rom.chr_rom_crc32 = make_crc32 (global_rom.chr_rom,
        (global_rom.chr_rom_pages * 0x2000));
   }

   /* calculate the attribute lookup table */
   for (i = 0; i < 4; i++) {
#ifdef ALLEGRO_I386
      UINT32 attribute = (i << 2) | 3;

      attribute |= (attribute << 8) | (attribute << 16) |
         (attribute << 24);

      attribute_table [i] = attribute;
#else
      attribute_table [i] = (i << 2) | 3;
#endif
   }

   ppu_cache_init();
   ppu_cache_chr_rom_pages();

   ppu_reset();

   Renderer_Initialize();

   return 0;
}

void ppu_exit (void)
{
   FILE* dump_file;

#ifdef DEBUG
   dump_file = fopen("ppudump.ram", "wb");
   if(dump_file) {
      fwrite(ppu_name_table_vram, 1, sizeof(ppu_name_table_vram), dump_file);
      fwrite(ppu_pattern_vram, 1, sizeof(ppu_pattern_vram), dump_file);
      fclose(dump_file);
   }

   dump_file = fopen("ppudump.spr", "wb");
   if(dump_file) {
      fwrite(ppu_spr_ram, 1, sizeof(ppu_spr_ram), dump_file);
      fclose (dump_file);
   }
#endif
}

int ppu_get_mirroring(void)
{
   synchronize();
 
   return ppu_mirroring;
}

BOOL ppu_is_rendering(void) {
   if((ppu_scanline >= PPU_FIRST_DISPLAYED_LINE) &&
      (ppu_scanline <= PPU_LAST_DISPLAYED_LINE))
      return TRUE;
   else
      return FALSE;
}

void ppu_set_name_table_internal(int table, int select)
{
   synchronize();

   ppu_set_name_table_address(table, ppu_name_table_vram + (select << 10));
}

void ppu_set_name_table_address(int table, UINT8* address)
{
   synchronize();

   name_tables_read[table] = address;
   name_tables_write[table] = address;
}

void ppu_set_name_table_address_rom (int table, UINT8* address)
{
   synchronize();

   name_tables_read[table] = address;
   name_tables_write[table] = ppu_vram_dummy_write;
}

void ppu_set_name_table_address_vrom(int table, int vrom_block)
{
   synchronize();

   vrom_block = (vrom_block & 7) + ROM_CHR_ROM_PAGE_LOOKUP
      [(vrom_block / 8) & ROM_CHR_ROM_PAGE_OVERFLOW_MASK] * 8;

   ppu_set_name_table_address_rom (table, ROM_CHR_ROM + (vrom_block << 10));
}

void ppu_set_expansion_table_address(UINT8* address)
{
   synchronize();

   ppu_expansion_table = address;
}

void ppu_set_mirroring_one_screen(void)
{
   synchronize();

    ppu_set_name_table_address(0, one_screen_base_address);
    ppu_set_name_table_address(1, one_screen_base_address);
    ppu_set_name_table_address(2, one_screen_base_address);
    ppu_set_name_table_address(3, one_screen_base_address);
}

// This is the only place where ppu_mirroring may change.
void ppu_set_mirroring(int mirroring)
{
   synchronize();

   ppu_mirroring = mirroring;

   switch(ppu_mirroring) {
      case MIRRORING_ONE_SCREEN: {
         ppu_set_mirroring_one_screen();
         break;
      }

      case MIRRORING_ONE_SCREEN_2000: {
         one_screen_base_address = ppu_name_table_vram;
         ppu_set_mirroring_one_screen();

         break;
      }

      case MIRRORING_ONE_SCREEN_2400: {
         one_screen_base_address = ppu_name_table_vram + 0x400;
         ppu_set_mirroring_one_screen();

         break;
      }

      case MIRRORING_ONE_SCREEN_2800: {
         one_screen_base_address = ppu_name_table_vram + 0x800;
         ppu_set_mirroring_one_screen();

         break;
      }

      case MIRRORING_ONE_SCREEN_2C00: {
         one_screen_base_address = ppu_name_table_vram + 0xC00;
         ppu_set_mirroring_one_screen();

         break;
      }

      case MIRRORING_VERTICAL: {
         ppu_set_name_table_internal(0, 0);
         ppu_set_name_table_internal(1, 1);
         ppu_set_name_table_internal(2, 0);
         ppu_set_name_table_internal(3, 1);

         break;
      }

      case MIRRORING_HORIZONTAL: {
         ppu_set_name_table_internal(0, 0);
         ppu_set_name_table_internal(1, 0);
         ppu_set_name_table_internal(2, 1);
         ppu_set_name_table_internal(3, 1);

         break;
      }

      case MIRRORING_FOUR_SCREEN: {
         ppu_set_name_table_internal(0, 0);
         ppu_set_name_table_internal(1, 1);
         ppu_set_name_table_internal(2, 2);
         ppu_set_name_table_internal(3, 3);

         break;
      }

      default: {
         WARN_GENERIC();
         break;
      }
   }
}

void ppu_invert_mirroring(void)   /* '/' key. */
{
   synchronize();

   switch(ppu_mirroring) {
      case MIRRORING_HORIZONTAL: {
         ppu_set_mirroring(MIRRORING_VERTICAL);
         break;
      }

      case MIRRORING_VERTICAL: {
         ppu_set_mirroring(MIRRORING_HORIZONTAL);
         break;
      }
   }
}

static UINT8 ppu_vram_read(void)
{
   // no syncing necessary here, its done by ppu_read/write()
   UINT16 address = vram_address & 0x3FFF;
   UINT8 temp = buffered_vram_read;

   /* VRAM Read I/O. */
   if(address >= 0x2000) {
      if (address >= 0x3F00) {
         /* palettes */
         if((address & 0x03) == 0)
            temp = ppu_palette[0] & palette_mask;
         else
            temp = ppu_palette[address & 0x1F] & palette_mask;
      }
      else {
         /* name tables */
         buffered_vram_read = name_tables_read[(address >> 10) & 3]
            [address & 0x3FF];
      }
   }
   else {
      /* pattern tables */
  
      /* If the mapper's behavior is affected by PPU bus activity, *
       *  it will need to install a handler in the function        *
       *  pointer mmc_check_latches.                               */

      /* This code is currently configured to report accesses to   *
       *  0FD0-0FEF and 1FD0-1FEF only, for MMC2 and MMC4.         */

      /* If a mapper needs to watch other PPU address ranges,      *
       *  this code will need to be changed.                       */
      if(mmc_check_latches) {
         if(((address & 0xFFF) >= 0xFD0) &&
            ((address & 0xFFF) <= 0xFEF))
            mmc_check_latches(address);
      }

      buffered_vram_read =
         ppu_vram_block_read_address[address >> 10][address & 0x3FF];
   }

   vram_address += address_increment;

   return temp;
}

static void ppu_vram_write(UINT8 value)
{
   // no syncing necessary here, its done by ppu_read/write()
   UINT16 address = vram_address & 0x3FFF;

   /* VRAM Write I/O. */
   if(address >= 0x2000) {
      if(address >= 0x3F00) {
         /* palettes */
         if((address & 0x03) == 0) {
            ppu_background_palette[(address & 0x0F)] = value;
            ppu_sprite_palette[(address & 0x0F)] = value;
         }
         else
            ppu_palette[address & 0x1F] = value;
      }
      else {
         /* name tables */
         name_tables_write[(address >> 10) & 3][address & 0x3FF] = value;
      }
   }
   else {
      /* pattern tables */
      int vram_block;

      if(mmc_check_latches) {
         if(((address & 0xFFF) >= 0xFD0) &&
            ((address & 0xFFF) <= 0xFEF))
            mmc_check_latches(address);
      }

      vram_block = ppu_vram_block[address >> 10];

      if(vram_block < FIRST_VROM_BLOCK) {
         /* if block is writable (VRAM) */
         int this_tile;

         if(ppu_vram_block_write_address[address >> 10]
            [address & 0x3FF] != value) {
            /* VRAM changed, track for cache update */
            ppu_vram_block_write_address[address >> 10]
               [address & 0x3FF] = value;

            this_tile = (address & 0x3FF) / 16;

            if(ppu_vram_dirty_set_end[vram_block] != this_tile) {
               if(ppu_vram_dirty_set_end[vram_block] == this_tile - 1)
                  ppu_vram_dirty_set_end[vram_block] ++;
               else {
                  if(vram_set_needs_recache(vram_block))
                     recache_vram_set(vram_block);

                  ppu_vram_dirty_set_begin[vram_block] =
                     ppu_vram_dirty_set_end[vram_block] = this_tile;

                  ppu_vram_cache_needs_update = TRUE;
               }
            }
         }
      }
   }

   vram_address += address_increment;
}

UINT8 ppu_get_background_color(void)
{
   synchronize();

   /* Returns the current PPU background color - for drawing overscan for e.g NTSC */
   /* In the future, this should be rendered by the PPU itself into a special kind of buffer. */
   /* Returned as an index into the 256 color palette */
   return (ppu_palette[0] & palette_mask) + PALETTE_ADJUST;
}

static UINT8 last_ppu_write_value;

UINT8 ppu_read (UINT16 address)
{
   synchronize();

   /* Handle register mirroring. */
   switch(address & 7) {
      case 0x2000 & 7:
      case 0x2001 & 7:
      case 0x2003 & 7:
      case 0x2005 & 7:
      case 0x2006 & 7:
         return last_ppu_write_value;

      case 0x2002 & 7: {
         /* PPU status register. */

         UINT8 data = 0x00;

         if(vblank_occurred) {
            data |= PPU_VBLANK_FLAG_BIT;
            vblank_occurred = FALSE;
         }

         if(ppu__sprite_collision)
            data |= PPU_SPRITE_0_COLLISION_BIT;
         if(ppu__sprite_overflow)
            data |= PPU_SPRITE_OVERFLOW_BIT;

         address_write = FALSE;

         return (data | (last_ppu_write_value & 0x1F));
      }

      case 0x2004 & 7: {
         /* Sprite RAM I/O. */
         return ppu_spr_ram [spr_ram_address];
      }

      case 0x2007 & 7: {
         /* VRAM Read I/O. */
        return ppu_vram_read();
      }

      default:
         break;
   }

   return 0x00;
}

void ppu_write(UINT16 address, UINT8 value)
{
   synchronize();

   if(address == 0x4014) {
      /* Sprite RAM DMA. */
      do_spr_ram_dma(value);
      return;
   }

   last_ppu_write_value = value;

   /* Handle register mirroring. */
   switch(address & 7) {
      case 0x2000 & 7: {
         /* Control register #1. */
         ppu_register_2000 = value;

         sprite_height = (value & PPU_SPRITE_SIZE_BIT) ? 16 : 8;

         want_vblank_nmi = value & PPU_VBLANK_NMI_FLAG_BIT;

         address_increment = (value & PPU_ADDRESS_INCREMENT_BIT) ? 32 : 1;

         background_tileset =
            (value & PPU_BACKGROUND_TILESET_BIT) ? 0x1000 : 0x0000;
         sprite_tileset =
            (value & PPU_SPRITE_TILESET_BIT) ? 0x1000 : 0x0000;

         address_temp = (address_temp & ~(3 << 10)) | ((value & 3) << 10);

         ppu_repredict_nmi();

         break;
      }

      case 0x2001 & 7: {
         /* Control register #2. */
         ppu_register_2001 = value;
         if(ppu_register_2001 & PPU_MONOCHROME_DISPLAY_BIT)
            palette_mask = 0x30;
         else
            palette_mask = 0x3f;

         break;
      }

      case 0x2003 & 7: {
         /* Sprite RAM address. */
         spr_ram_address = value;
         break;
      }

      case 0x2004 & 7: {
         /* Sprite RAM I/O. */
         ppu_spr_ram[spr_ram_address] = value;
         spr_ram_address++;

         break;
      }

      case 0x2005 & 7: {
         /* Horizontal / Vertical offset. */
         if(!address_write) {
            /* Horizontal offset. */
           address_write = TRUE;

           address_temp = (address_temp & ~0x1F) | ((value >> 3) & 0x1F);
           x_offset = value & 7;
        }
        else  {
           /* Vertical offset. */
           address_write = FALSE;

           address_temp =
              (address_temp & ~(0x1F << 5)) | (((value >> 3) & 0x1F) << 5);
           address_temp =
              (address_temp & ~(7 << 12)) | ((value & 7) << 12);
         }
 
        break;
     }

     case 0x2006 & 7: {
        /* VRAM address. */
        if(!address_write) {
           address_write = TRUE;
           address_temp = (address_temp & 0xFF) | ((value & 0x3F) << 8);
         }
         else {
            address_write = FALSE;

            address_temp = (address_temp & ~0xFF) | value;
            vram_address = address_temp;
         }

         break;
      }

      case 0x2007 & 7: {
         /* VRAM Write I/O. */
         ppu_vram_write(value);
         break;
      }

      default:
         break;
   }
}

static void do_spr_ram_dma(UINT8 page)
{
    /* Sprite RAM DMA. */
   unsigned address = page * 0x100;

   /* Steal 2 CPU cycles. */
   cpu_burn(2 * CPU_CLOCK_MULTIPLIER);

   for(int index = 0; index < 256; index++) {
      int value = cpu_read(address + index);

      /* Steal 2 CPU cycles. */
      cpu_burn(2 * CPU_CLOCK_MULTIPLIER);

      ppu_spr_ram[spr_ram_address] = value;
      spr_ram_address++;
   }
}

void ppu_reset(void)
{
   int i;

   // clear the clock counter and the buffer
   ppu_clock_counter = 0;
   ppu_clock_buffer = 0;

   // queue a scanline cycle and reset the scanline counter
   ppu_scanline_timer = PPU_SCANLINE_CLOCKS;
   ppu_scanline = PPU_FIRST_LINE;

   // clear prediction vectors
   ppu_prediction_timestamp = 0;
   ppu_prediction_cycles = 0;

   // enable rendering
   ppu__rendering_enabled = TRUE;
   ppu__force_rendering = FALSE;
   ppu_cache_rendering_enabled = TRUE;

   // clear sprite collision and overflow flags
   ppu__sprite_collision = FALSE;
   ppu__sprite_overflow = FALSE;

   // start on an even frame
   ppu_odd_frame = false;

   // clear vblank flag
   vblank_occurred = FALSE;

   memset(ppu_pattern_vram, NULL, sizeof(ppu_pattern_vram));
   memset(ppu_name_table_vram, NULL, sizeof(ppu_name_table_vram));
   memset(ppu_spr_ram, NULL, sizeof(ppu_spr_ram));

   ppu_cache_all_vram();

   ppu_write(0x2000, 0x00);

   ppu_write(0x2001,
      PPU_BACKGROUND_SHOW_LEFT_EDGE_BIT |
      PPU_SPRITES_SHOW_LEFT_EDGE_BIT |
      PPU_BACKGROUND_ENABLE_BIT | PPU_SPRITES_ENABLE_BIT);

   spr_ram_address = 0;

   vram_address = 0;
   address_temp = 0;

   x_offset = 0;

   address_write = FALSE;

   buffered_vram_read = 0;

   ppu_set_expansion_table_address(NULL);

   ppu_set_mirroring(ppu_mirroring);
}

void ppu_sync_update(void)
{
   synchronize();
}

void ppu_disable_rendering(void)
{
   ppu_cache_rendering_enabled = FALSE;
}

void ppu_enable_rendering(void)
{
   ppu_cache_rendering_enabled = TRUE;
}

void ppu_clear_palette(void)
{
   /* Clears the palette - easier than writing to VRAM when the palette needs
      to be cleared from an external source. */
   memset(&ppu_palette, 0, sizeof(ppu_palette));
}

void ppu_save_state(PACKFILE* file, int version)
{
   synchronize();

   pack_iputl(ppu_clock_counter, file);
   pack_iputl(ppu_clock_buffer, file);

   pack_iputw(ppu_scanline_timer, file);
   pack_iputw(ppu_scanline, file);

   pack_iputl(ppu_prediction_timestamp, file);
   pack_iputl(ppu_prediction_cycles, file);

   // --

   pack_putc(ppu_register_2000, file);
   pack_putc(ppu_register_2001, file);

   pack_putc(ppu_mirroring, file);

   pack_putc(spr_ram_address, file);
 
   pack_iputw(vram_address, file);
   pack_putc(buffered_vram_read, file);

   pack_putc(address_write, file);
   pack_iputw(address_temp, file);
   pack_putc(x_offset, file);

   pack_putc(ppu_odd_frame ? 1 : 0, file);

   pack_putc(vblank_occurred ? 1 : 0, file);

   pack_putc(ppu__sprite_collision ? 1 : 0, file);
   pack_putc(ppu__sprite_overflow ? 1 : 0, file);

   Renderer_Save(file, version);

   pack_putc(mmc_get_name_table_count(), file);
   pack_putc(mmc_uses_pattern_vram(), file);

   /* save palette RAM */
   pack_fwrite(ppu_palette, PPU_PALETTE_SIZE, file);

   /* save sprite RAM */
   pack_fwrite(ppu_spr_ram, PPU_SPR_RAM_SIZE, file);

   /* mmc_get_name_table_count() MUST be <= 4 */
   /* values of 2 and 4 are expected */
   if(mmc_get_name_table_count()) {
      pack_fwrite(ppu_name_table_vram, 1024 * mmc_get_name_table_count(),
         file);
   }

   if(mmc_uses_pattern_vram())
      pack_fwrite(ppu_pattern_vram, PPU_PATTERN_VRAM_SIZE, file);
}

void ppu_load_state(PACKFILE* file, int version)
{
   ppu_clock_counter = pack_igetl(file);
   ppu_clock_buffer = pack_igetl(file);

   ppu_scanline_timer = pack_igetw(file);
   ppu_scanline = pack_igetw(file);

   ppu_prediction_timestamp = pack_igetl(file);
   ppu_prediction_cycles = pack_igetl(file);

   // --

   ppu_register_2000 = pack_getc(file);
   ppu_write (0x2000, ppu_register_2000);

   ppu_register_2001 = pack_getc(file);
   ppu_write (0x2001, ppu_register_2001);

   ppu_mirroring = pack_getc(file);
   ppu_set_mirroring(ppu_mirroring);

   spr_ram_address = pack_getc(file);

   vram_address = pack_igetw(file);
   buffered_vram_read = pack_getc(file);

   address_write = pack_getc(file);
   address_temp = pack_igetw(file);
   x_offset = pack_getc(file);

   ppu_odd_frame = TRUE_OR_FALSE(pack_getc(file));

   vblank_occurred = TRUE_OR_FALSE(pack_getc(file));

   ppu__sprite_collision = TRUE_OR_FALSE(pack_getc(file));
   ppu__sprite_overflow = TRUE_OR_FALSE(pack_getc(file));

   Renderer_Load(file, version);

   const int state_name_table_count = pack_getc(file);
   const int state_contains_pattern_vram = pack_getc(file);

   /* load palette RAM */
   pack_fread(ppu_palette, PPU_PALETTE_SIZE, file);

   /* load sprite RAM */
   pack_fread(ppu_spr_ram, PPU_SPR_RAM_SIZE, file);

   /* state_name_table_count MUST be <= 4 */
   /* values of 2 and 4 are expected */
   if(state_name_table_count)
      pack_fread(ppu_name_table_vram, 1024 * state_name_table_count, file);

   if(state_contains_pattern_vram) {
      pack_fread(ppu_pattern_vram, PPU_PATTERN_VRAM_SIZE, file);
      ppu_cache_all_vram();
   }
}

// Internal functions follow.
static void predict_nmi_slave(cpu_time_t cycles)
{
   // Clear pending interrupt just in case.
   cpu_unqueue_interrupt(CPU_INTERRUPT_NMI);

   // NMIs only occur if the flag is set.
   if(!want_vblank_nmi)
      return;

   // Save variables since we just want to simulate.
   const int16 saved_scanline_timer = ppu_scanline_timer;
   const int16 saved_scanline = ppu_scanline;

   for(cpu_time_t offset = 0; offset < cycles; offset++) {
      // Get current scanline clock cycle (starting at 1).
      const cpu_time_t cycle = (PPU_SCANLINE_CLOCKS - ppu_scanline_timer) + 1;

      // VBlank NMI occurs on the 1st cycle of the line after the VBlank flag is set.
      if((cycle == 1) &&
         (ppu_scanline == PPU_FIRST_VBLANK_LINE))
         cpu_queue_interrupt(CPU_INTERRUPT_NMI, ppu_prediction_timestamp + (offset * PPU_CLOCK_MULTIPLIER));

      if(ppu_scanline_timer > 0)
         ppu_scanline_timer--;
      if(ppu_scanline_timer <= 0) {
         ppu_scanline_timer += PPU_SCANLINE_CLOCKS;

         // move on to next scanline
         ppu_scanline++;
         if(ppu_scanline > PPU_LAST_LINE)
            ppu_scanline = PPU_FIRST_LINE;
      }
   }

   // Restore variables.
   ppu_scanline_timer = saved_scanline_timer;
   ppu_scanline = saved_scanline;
}

void ppu_predict_nmi(cpu_time_t cycles)
{
   synchronize();

   // Save parameters for re-prediction if a mid-scanline change occurs.
   ppu_prediction_timestamp = cpu_get_cycles();
   // We'll actually emulate a little bit longer than requested, since it doesn't hurt to do so.
   ppu_prediction_cycles = cycles + PREDICTION_BUFFER_CYCLES + (1 * PPU_CLOCK_MULTIPLIER);

   // Convert from master clock to APU clock.
   const cpu_time_t ppu_cycles = cycles / PPU_CLOCK_DIVIDER;
   if(ppu_cycles == 0)
      return;

   predict_nmi_slave(ppu_cycles);
}

static void ppu_repredict_nmi(void)
{
   synchronize();

   const cpu_time_t cycles = cpu_get_cycles();

   // Determine how many cycles are left to simulate for this execution cycle.
   cpu_time_t cycles_remaining = (cpu_rtime_t)cycles - (cpu_rtime_t)ppu_prediction_timestamp;
   if(cycles_remaining <= 0)
      return;

   // Cap the number of cycles to simulate at the amount given in the last prediction request.
   if(cycles_remaining > ppu_prediction_cycles)
      cycles_remaining = ppu_prediction_cycles;

   // Convert from master clock to PPU clock.
   const cpu_time_t ppu_cycles_remaining = cycles_remaining / PPU_CLOCK_DIVIDER;
   if(ppu_cycles_remaining == 0)
      return;

   predict_nmi_slave(ppu_cycles_remaining);
}

#if 0
            // TODO: Sprite #0 hit test support here on skipped frames
            // TODO: Find out what is wrong with Zapper hitscan code and fix it (crashes)
            if(input_enable_zapper)
               input_update_zapper_offsets();

            // We always need to perform rendering when performing the light gun hit-test
            bool hitscan = false;
            if(input_enable_zapper && input_zapper_on_screen &&
               (input_zapper_y_offset == ppu_scanline) &&
               (input_zapper_x_offset == Renderer::render.pixel)) {
               hitscan = true;
               ppu__force_rendering = TRUE;
            }

            Renderer_Pixel();

            if(hitscan)
               input_update_zapper();

            if(ppu__force_rendering)
               ppu__force_rendering = FALSE;
#endif

static void start_frame() {
   // Clear VBlank flag
   vblank_occurred = FALSE;

   // clear sprite #0 collision flag
   ppu__sprite_collision = FALSE;

   Renderer_Frame();
}

static void start_scanline() 
{
   if((ppu_scanline >= PPU_FIRST_DISPLAYED_LINE) &&
      (ppu_scanline <= PPU_LAST_DISPLAYED_LINE)) {

      if((ppu_scanline == PPU_FIRST_DISPLAYED_LINE) &&
         PPU_ENABLED)
         vram_address = address_temp;

      // clear sprite #0 overflow flag
      ppu__sprite_overflow = FALSE;

      Renderer_Line(ppu_scanline);
   }
   else if(ppu_scanline == PPU_IDLE_LINE) {
      // clear sprite #0 overflow flag from the last line
      ppu__sprite_overflow = FALSE;
   }
   else if(ppu_scanline == PPU_FIRST_VBLANK_LINE) {
      // vblank
      vblank_occurred = TRUE;

      // VBlank NMI
      // This is now handled by ppu_predict_nmi() instead.
      // if(want_vblank_nmi)
      //   cpu_interrupt(CPU_INTERRUPT_NMI);
   }

   // call start scanline interrupt for MMC
   if(mmc_scanline_start)
      cpu_interrupt(mmc_scanline_start(ppu_scanline));
}

/* This is only called for scanlines -1 to 239, as the PPU is idle during other lines
   (excepting what is handled by start_scanline(), of course). */
static void start_scanline_cycle(const cpu_time_t cycle) {
   Renderer_Clock();

   // Continue for visible lines only
   if(ppu_scanline < PPU_FIRST_DISPLAYED_LINE)
      return;

   // The PPU renders one pixel per clock for the first 256 clock cycles
   if(cycle <= PPU_RENDER_CLOCKS) {
      Renderer_Pixel();
   }
   // HBlank start.
   else if((cycle == PPU_HBLANK_START) &&
           mmc_hblank_start) {

      // call hblank start interrupt for MMC
      cpu_interrupt(mmc_hblank_start(ppu_scanline));
   }
   /* Mid-HBlank VRAM address fixup.

      This is actually when the PPU begins fetching data for the first two tiles of the
      next line (as the PPU start fetching from tile 3 at the beginning of each line),
      but unfortunately we don't emulate PPU background timing yet. */

   else if((cycle == PPU_HBLANK_PREFETCH) &&
           PPU_ENABLED) {

      vram_address += 0x1000;

      if((vram_address & (7 << 12)) == 0) {
         vram_address += 32;

         switch(vram_address & (0x1F << 5)) {
            case 0:
               vram_address -= (1 << 10);
               break;
           case (30 << 5):
               vram_address = (vram_address - (30 << 5)) ^ (1 << 11);
               break;
         }
      }
   }
}

static void end_scanline() {
   // call end scanline interrupt for MMC
   if(mmc_scanline_end)
      cpu_interrupt(mmc_scanline_end(ppu_scanline));
}

static void end_frame() {
   // Toggle even/odd frame flag
   ppu_odd_frame = !ppu_odd_frame;

   /* Current frame has ended, but we only have to draw the buffer to the screen
      if rendering had been enabled (i.e this was not a skipped frame). */
   if(ppu__rendering_enabled)
      video_blit(screen);

   // This is the only time its safe to enable/disable rendering directly
   if(ppu__rendering_enabled != ppu_cache_rendering_enabled)
      ppu__rendering_enabled = ppu_cache_rendering_enabled;
}

static void synchronize()
{
   // Calculate the delta period.
   const cpu_time_t elapsed_cycles = cpu_get_elapsed_cycles(&ppu_clock_counter) + ppu_clock_buffer;
   if(elapsed_cycles <= 0) { // Always > 0 when ppu_clock_buffer > 0
      // Nothing to do. 
      return;
   }

   // Scale from master clock to PPU and buffer the remainder to avoid possibly losing cycles.
   const cpu_time_t elapsed_ppu_cycles = elapsed_cycles / PPU_CLOCK_DIVIDER;
   ppu_clock_buffer = elapsed_cycles - (elapsed_ppu_cycles * PPU_CLOCK_DIVIDER);

   if(elapsed_ppu_cycles == 0)
      return;

   for(cpu_time_t count = 0; count < elapsed_ppu_cycles; count++) {
      // Get current scanline clock cycle (starting at 1).
      const cpu_time_t cycle = (PPU_SCANLINE_CLOCKS - ppu_scanline_timer) + 1;

      /* Not entirely sure how the cache system works, so for now we'll call this every cycle
         just to make sure, even if that ends up being a little slow. */
      if(ppu_vram_cache_needs_update)
         recache_vram_sets();

      // Scanline start.
      if(cycle == 1) {

         if(ppu_scanline == PPU_FIRST_LINE)
            // start a new frame
            start_frame();

         // call handler
         start_scanline();
      }

      // Renderered lines (-1 to 239)
      if((ppu_scanline >= PPU_FIRST_LINE) &&
         (ppu_scanline <= PPU_LAST_DISPLAYED_LINE))
         start_scanline_cycle(cycle);

      if((ppu_scanline == PPU_CLOCK_SKIP_LINE) && (cycle == PPU_CLOCK_SKIP_CYCLE) &&
          ppu_odd_frame && PPU_BACKGROUND_ENABLED) {
         /* From nesdev wiki:

            The PPU has an even/odd flag that is toggled every frame,
            regardless of whether the BG is enabled or disabled.

            With BG disabled, each PPU frame is 341*262=89342 PPU clocks long.
            There is no skipped clock every other frame.

            With BG enabled, each odd PPU frame is one PPU clock shorter than
            normal. I've timed this to occur around PPU clock 328 on
            scanline 20, but haven't written a test ROM for it yet. */

         /* Steal a clock from the scanline timer and move it to the clock
            buffer for the next frame */
         ppu_scanline_timer--;
         ppu_clock_buffer++;
      }

      if(ppu_scanline_timer > 0)
         ppu_scanline_timer--;
      if(ppu_scanline_timer <= 0) {
         // end of scanline
         ppu_scanline_timer += PPU_SCANLINE_CLOCKS;

         // call handler         
         end_scanline();

         // move on to next scanline
         ppu_scanline++;
         if(ppu_scanline > PPU_LAST_LINE) {
            // end of frame
            ppu_scanline = PPU_FIRST_LINE;

            // call handler         
            end_frame();
         }
      }
   }
}
