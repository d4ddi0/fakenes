

/*

FakeNES - A portable, open-source NES emulator.

ppu.c: Implementation of the PPU emulation.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include <stdio.h>

#include <stdlib.h>

#include <string.h>


#include "cpu.h"

#include "input.h"

#include "mmc.h"

#include "ppu.h"

#include "video.h"


#include "misc.h"


/* VRAM and sprite RAM. */

static UINT8 * ppu_vram_block_read_address [8];
static UINT8 * ppu_vram_block_cache_address [8];
static UINT8 * ppu_vram_block_write_address [8];

/*
 vram block identifiers
  0-7 = pattern VRAM
  8+  = pattern VROM
*/
#define FIRST_VROM_BLOCK 8
static UINT32 ppu_vram_block [8];

static INT32 ppu_vram_set_begin [8];
static INT32 ppu_vram_set_end [8];


static UINT8 ppu_vram_dummy_write [1024];

static UINT8 ppu_pattern_vram [8 * 1024];
static UINT8 ppu_pattern_vram_cache [8 * 1024 / 2 * 8];


static UINT8 ppu_name_table_vram [4 * 1024];
static UINT8 *name_tables_read [4];
static UINT8 *name_tables_write [4];


static UINT8 ppu_palette [32];


static UINT8 ppu_spr_ram [256];


#define ppu_background_palette ppu_palette
#define ppu_sprite_palette (ppu_palette + 16)

static void do_spr_ram_dma(UINT8 page);

static int ppu_mirroring;


/* register $2000 */
#define VBLANK_NMI_FLAG_BIT     (1 << 7)
#define PPU_SLAVE_BIT           (1 << 6)
#define SPRITE_SIZE_BIT         (1 << 5)
#define BACKGROUND_TILESET_BIT  (1 << 4)
#define SPRITE_TILESET_BIT      (1 << 3)
#define ADDRESS_INCREMENT_BIT   (1 << 2)
#define NAME_TABLE_SELECT       (3 << 0)


/* register $2001 */
#define COLOR_INTENSITY         (7 << 5)
#define SPRITES_ENABLE_BIT      (1 << 4)
#define BACKGROUND_ENABLE_BIT   (1 << 3)
#define SPRITES_CLIP_LEFT_EDGE_BIT      (1 << 2)
#define BACKGROUND_CLIP_LEFT_EDGE_BIT   (1 << 1)
#define MONOCHROME_DISPLAY_BIT  (1 << 0)


/* register $2002 */
#define VBLANK_FLAG_BIT         (1 << 7)
#define SPRITE_0_COLLISION_BIT  (1 << 6)
#define SPRITE_OVERFLOW_BIT     (1 << 5)
#define VRAM_WRITE_FLAG_BIT     (1 << 4)


#define PPU_PUTPIXEL(bitmap, x, y, color) \
    (bitmap -> line [y] [x] = color)

#define PPU_GETPIXEL(bitmap, x, y) \
    (bitmap -> line [y] [x])


static int vram_address = 0;
static int buffered_vram_read = 0;

static int address_write = 0;
static int address_temp = 0;
static int x_offset = 0;
static int address_increment = 1;


static UINT8 spr_ram_address = 0;
static int sprite_height = 8;


static int want_vblank_nmi = TRUE;

static int vblank_occured = FALSE;
static int vram_writable = FALSE;

static int hit_first_sprite = FALSE;


static int background_clip_enabled = FALSE;
static int sprites_clip_enabled = FALSE;

int ppu_clip_background = FALSE; // hack


static int first_line_this_frame = TRUE;

static int background_tileset = 0;
static int sprite_tileset = 0;

static INLINE UINT8 vram_read (UINT16 address)
{
    if (address < 0x2000)
    {
        return ppu_vram_block_read_address [address >> 10] [address & 0x3FF];
    }
    else
    {
        return name_tables_read[(address >> 10) & 3]
            [address & 0x3FF];
    }
}


static UINT8 ppu_chr_rom_pages;
static UINT8 ppu_chr_rom_page_overflow_mask;
static UINT8 * ppu_chr_rom = NULL;
static UINT8 * ppu_chr_rom_cache = NULL;


void ppu_free_chr_rom (void)
{
    if (ppu_chr_rom) free (ppu_chr_rom);
    if (ppu_chr_rom_cache) free (ppu_chr_rom_cache);

    ppu_chr_rom = NULL;
    ppu_chr_rom_cache = NULL;
}


UINT8 * ppu_get_chr_rom_pages (int num_pages)
{
    ppu_chr_rom_pages = num_pages;

    /* Compute a mask used to wrap invalid CHR ROM page numbers.
     *  As CHR ROM banking uses a 1k page size, this mask is based
     *  on a 1k page size.
     */
    if (((num_pages * 2 - 1) & (num_pages - 1)) == (num_pages - 1) ||
        (num_pages == 1))
    /* compute mask for even power of two */
    {
        ppu_chr_rom_page_overflow_mask = (num_pages * 8) - 1;
    }
    else
    /* compute mask */
    {
        int i;

        /* compute the largest even power of 2 less than
           CHR ROM page count, and use that to compute the mask */
        for (i = 0; (ppu_chr_rom_pages >> (i + 1)) > 0; i++);

        ppu_chr_rom_page_overflow_mask = ((1 << i) * 8) - 1;
    }

    /* 8k CHR ROM page size */
    ppu_chr_rom = malloc (num_pages * 0x2000);

    /* 2-bit planar tiles converted to 8-bit chunky */
    ppu_chr_rom_cache = malloc ((num_pages * 0x2000) / 2 * 8);

    if (ppu_chr_rom == 0 || ppu_chr_rom_cache == 0)
    {
        ppu_free_chr_rom ();

        return NULL;
    }

    return ppu_chr_rom;
}


UINT32 tile_decode_table_plane_0[16];
UINT32 tile_decode_table_plane_1[16];


void ppu_cache_chr_rom_pages (void)
{
    int tile, num_tiles;

    /* 8k CHR ROM page size, 2-bitplane 8x8 tiles */
    num_tiles = (ppu_chr_rom_pages * 0x2000) / (8 * 2);

    for (tile = 0; tile < num_tiles; tile++)
    {
        int y;

        for (y = 0; y < 8; y++)
        {
            UINT32 pixels0_3, pixels4_7;

            pixels0_3 = tile_decode_table_plane_0
                [(ppu_chr_rom [tile * 16 + y]) >> 4];
            pixels4_7 = tile_decode_table_plane_0
                [(ppu_chr_rom [tile * 16 + y]) & 0x0F];

            pixels0_3 |= tile_decode_table_plane_1
                [(ppu_chr_rom [tile * 16 + y + 8]) >> 4];
            pixels4_7 |= tile_decode_table_plane_1
                [(ppu_chr_rom [tile * 16 + y + 8]) & 0x0F];
            

            *(UINT32 *) (&ppu_chr_rom_cache [((tile * 8 + y) * 8)]) =
                pixels0_3;
            *(UINT32 *) (&ppu_chr_rom_cache [((tile * 8 + y) * 8) + 4]) =
                pixels4_7;
        }
    }
}


void ppu_cache_all_vram (void)
{
    int tile, num_tiles;

    num_tiles = sizeof(ppu_pattern_vram) / (8 * 2);

    for (tile = 0; tile < num_tiles; tile++)
    {
        int y;

        for (y = 0; y < 8; y++)
        {
            UINT32 pixels0_3, pixels4_7;

            pixels0_3 = tile_decode_table_plane_0
                [(ppu_pattern_vram [tile * 16 + y]) >> 4];
            pixels4_7 = tile_decode_table_plane_0
                [(ppu_pattern_vram [tile * 16 + y]) & 0x0F];

            pixels0_3 |= tile_decode_table_plane_1
                [(ppu_pattern_vram [tile * 16 + y + 8]) >> 4];
            pixels4_7 |= tile_decode_table_plane_1
                [(ppu_pattern_vram [tile * 16 + y + 8]) & 0x0F];
            

            *(UINT32 *) (&ppu_pattern_vram_cache [((tile * 8 + y) * 8)]) =
                pixels0_3;
            *(UINT32 *) (&ppu_pattern_vram_cache [((tile * 8 + y) * 8) + 4]) =
                pixels4_7;
        }
    }
}


void ppu_set_ram_1k_pattern_vram_block (UINT16 block_address, int vram_block)
{
    ppu_vram_block [block_address >> 10] = vram_block;

    ppu_vram_block_read_address [block_address >> 10] =
        ppu_pattern_vram + (vram_block << 10);
    ppu_vram_block_write_address [block_address >> 10] =
        ppu_pattern_vram + (vram_block << 10);

    ppu_vram_block_cache_address [block_address >> 10] =
        ppu_pattern_vram_cache + ((vram_block << 10) / 2 * 8);
}


void ppu_set_ram_1k_pattern_vrom_block (UINT16 block_address, int vrom_block)
{
    if (vrom_block >= (ppu_chr_rom_pages * 8))
    {
        vrom_block &= ppu_chr_rom_page_overflow_mask;
    }

    ppu_vram_block [block_address >> 10] =
        FIRST_VROM_BLOCK + vrom_block;

    ppu_vram_block_read_address [block_address >> 10] =
        ppu_chr_rom + (vrom_block << 10);
    ppu_vram_block_write_address [block_address >> 10] =
        ppu_vram_dummy_write;

    ppu_vram_block_cache_address [block_address >> 10] =
        ppu_chr_rom_cache + ((vrom_block << 10) / 2 * 8);
}


void ppu_set_ram_8k_pattern_vram (void)
{
    ppu_set_ram_1k_pattern_vram_block (0x0000, 0);
    ppu_set_ram_1k_pattern_vram_block (0x0400, 1);
    ppu_set_ram_1k_pattern_vram_block (0x0800, 2);
    ppu_set_ram_1k_pattern_vram_block (0x0C00, 3);
    ppu_set_ram_1k_pattern_vram_block (0x1000, 4);
    ppu_set_ram_1k_pattern_vram_block (0x1400, 5);
    ppu_set_ram_1k_pattern_vram_block (0x1800, 6);
    ppu_set_ram_1k_pattern_vram_block (0x1C00, 7);
}


int ppu_init (void)
{
    int i;

    /* calculate the tile decoding lookup tables */
    for (i = 0; i < 16; i++)
    {
        int pixels0 = 0, pixels1 = 0;

#ifdef LSB_FIRST
        if (i & 8)
        {
            pixels0 |= (0xFC | 1);
            pixels1 |= (0xFC | 2);
        }
        if (i & 4)
        {
            pixels0 |= (0xFC | 1) << 8;
            pixels1 |= (0xFC | 2) << 8;
        }
        if (i & 2)
        {
            pixels0 |= (0xFC | 1) << 16;
            pixels1 |= (0xFC | 2) << 16;
        }
        if (i & 1)
        {
            pixels0 |= (0xFC | 1) << 24;
            pixels1 |= (0xFC | 2) << 24;
        }
#else
        if (i & 8)
        {
            pixels0 |= (0xFC | 1) << 24;
            pixels1 |= (0xFC | 2) << 24;
        }
        if (i & 4)
        {
            pixels0 |= (0xFC | 1) << 16;
            pixels1 |= (0xFC | 2) << 16;
        }
        if (i & 2)
        {
            pixels0 |= (0xFC | 1) << 8;
            pixels1 |= (0xFC | 2) << 8;
        }
        if (i & 1)
        {
            pixels0 |= (0xFC | 1);
            pixels1 |= (0xFC | 2);
        }
#endif
        tile_decode_table_plane_0 [i] = pixels0;
        tile_decode_table_plane_1 [i] = pixels1;
    }

    ppu_cache_chr_rom_pages ();

    ppu_reset ();


    return (0);
}


void clear_vram_set (int vram_block)
{
    ppu_vram_set_begin [vram_block] = -2;
    ppu_vram_set_end [vram_block] = -2;
}


void recache_vram_set (int vram_block)
{
    int tile, begin, end;

    begin = ppu_vram_set_begin [vram_block] + (vram_block * 0x400) / (8 * 2);
    end = ppu_vram_set_end [vram_block] + (vram_block * 0x400) / (8 * 2);

    for (tile = begin; tile <= end; tile++)
    {
        int y;

        for (y = 0; y < 8; y++)
        {
            UINT32 pixels0_3, pixels4_7;

            pixels0_3 = tile_decode_table_plane_0
                [(ppu_pattern_vram [tile * 16 + y]) >> 4];
            pixels4_7 = tile_decode_table_plane_0
                [(ppu_pattern_vram [tile * 16 + y]) & 0x0F];

            pixels0_3 |= tile_decode_table_plane_1
                [(ppu_pattern_vram [tile * 16 + y + 8]) >> 4];
            pixels4_7 |= tile_decode_table_plane_1
                [(ppu_pattern_vram [tile * 16 + y + 8]) & 0x0F];
            

            *(UINT32 *) (&ppu_pattern_vram_cache [((tile * 8 + y) * 8)]) =
                pixels0_3;
            *(UINT32 *) (&ppu_pattern_vram_cache [((tile * 8 + y) * 8) + 4]) =
                pixels4_7;
        }
    }
}


void recache_vram_sets (void)
{
    int i;

    for (i = 0; i < FIRST_VROM_BLOCK; i++)
    {
        if ((ppu_vram_set_end [i] + 1) > 0)
        {
            recache_vram_set (i);
            clear_vram_set (i);
        }
    }
}


void ppu_exit (void)
{
    FILE * dump_file;


#ifdef DEBUG

    dump_file = fopen ("ppudump.ram", "wb");


    if (dump_file)
    {
        fwrite (ppu_name_table_vram, 1, sizeof (ppu_name_table_vram),
            dump_file);
        fwrite (ppu_pattern_vram, 1, sizeof (ppu_pattern_vram), dump_file);

        fclose (dump_file);
    }


    dump_file = fopen ("ppudump.spr", "wb");


    if (dump_file)
    {
        fwrite (ppu_spr_ram, 1, sizeof (ppu_spr_ram), dump_file);

        fclose (dump_file);
    }

#endif
}


int get_ppu_mirroring (void)
{
 return ppu_mirroring;
}

void set_name_table_address (int table, UINT8 *address)
{
    name_tables_read[table] = address;
    name_tables_write[table] = address;
}

void set_name_table_address_rom (int table, UINT8 *address)
{
    name_tables_read[table] = address;
    name_tables_write[table] = ppu_vram_dummy_write;
}

void set_ppu_mirroring_one_screen (void)
{
    set_name_table_address(0, one_screen_base_address);
    set_name_table_address(1, one_screen_base_address);
    set_name_table_address(2, one_screen_base_address);
    set_name_table_address(3, one_screen_base_address);
}

void set_ppu_mirroring (int mirroring)
{
    ppu_mirroring = mirroring;

    switch (ppu_mirroring)
    {
     case MIRRORING_ONE_SCREEN:
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_ONE_SCREEN_2000:
        one_screen_base_address = ppu_name_table_vram;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_ONE_SCREEN_2400:
        one_screen_base_address = ppu_name_table_vram + 0x400;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_ONE_SCREEN_2800:
        one_screen_base_address = ppu_name_table_vram + 0x800;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_ONE_SCREEN_2C00:
        one_screen_base_address = ppu_name_table_vram + 0xC00;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_VERTICAL:
        set_name_table_address(0, ppu_name_table_vram);
        set_name_table_address(1, ppu_name_table_vram + 0x400);
        set_name_table_address(2, ppu_name_table_vram);
        set_name_table_address(3, ppu_name_table_vram + 0x400);

        break;

     case MIRRORING_HORIZONTAL:
        set_name_table_address(0, ppu_name_table_vram);
        set_name_table_address(1, ppu_name_table_vram);
        set_name_table_address(2, ppu_name_table_vram + 0x400);
        set_name_table_address(3, ppu_name_table_vram + 0x400);

        break;

     case MIRRORING_FOUR_SCREEN:
        set_name_table_address(0, ppu_name_table_vram);
        set_name_table_address(1, ppu_name_table_vram + 0x400);
        set_name_table_address(2, ppu_name_table_vram + 0x800);
        set_name_table_address(3, ppu_name_table_vram + 0xC00);

        break;
    }

}


void invert_ppu_mirroring (void)   /* '/' key. */
{
    switch (ppu_mirroring)
    {
        case MIRRORING_HORIZONTAL:

            set_ppu_mirroring (MIRRORING_VERTICAL);


            break;


        case MIRRORING_VERTICAL:

            set_ppu_mirroring (MIRRORING_HORIZONTAL);


            break;
    }
}

static int old_sprites_enabled = FALSE;
static INT8 sprite_list_needs_recache;

void ppu_reset (void)
{
    int i;

    memset (ppu_pattern_vram, NULL, sizeof (ppu_pattern_vram));
    memset (ppu_name_table_vram, NULL, sizeof (ppu_name_table_vram));
    memset (ppu_spr_ram, NULL, sizeof (ppu_spr_ram));


    ppu_cache_all_vram ();

    for (i = 0; i < FIRST_VROM_BLOCK; i++)
    {
        clear_vram_set (i);
    }


    vram_address = 0;
    address_temp = 0;

    x_offset = 0;

    spr_ram_address = 0;

    address_write = FALSE;

    address_increment = 1;

    want_vblank_nmi = TRUE;

    vblank_occured = FALSE;


    sprite_height = 8;
    sprite_list_needs_recache = TRUE;

    buffered_vram_read = 0;

    background_enabled = FALSE;
    sprites_enabled = FALSE;

    background_tileset = 0;

    set_ppu_mirroring(ppu_mirroring);

    sprite_tileset = 0;

    ppu_clear ();
}


UINT8 ppu_vram_read()
{
    UINT16 address = vram_address & 0x3FFF;
    UINT8 temp = buffered_vram_read;

    /* VRAM Read I/O. */

    if (address >= 0x2000)
    {
        /* name tables */
        {
            buffered_vram_read = name_tables_read[(address >> 10) & 3]
             [address & 0x3FF];
        }
    }
    else
    /* pattern tables */
    {
        /* If the mapper's behavior is affected by PPU bus activity, *
         *  it will need to install a handler in the function        *
         *  pointer mmc_check_latches.                               */

        /* This code is currently configured to report accesses to   *
         *  0FD0-0FEF and 1FD0-1FEF only, for MMC2 and MMC4.         */

        /* If a mapper needs to watch other PPU address ranges,      *
         *  this code will need to be changed.                       */
        if (mmc_check_latches)
        {
            if ((address & 0xFFF) >= 0xFD0 && (address & 0xFFF) <= 0xFEF)
            {
                mmc_check_latches(address);
            }
        }

        buffered_vram_read =
            ppu_vram_block_read_address [address >> 10] [address & 0x3FF];
    }

    vram_address += address_increment;


    return (temp);

}


void ppu_vram_write(UINT8 value)
{
    UINT16 address = vram_address & 0x3FFF;

    /* VRAM Write I/O. */

    if (address >= 0x2000)
    {
        if (address >= 0x3F00)
        /* palettes */
        {
            if ((address & 0x03) == 0)
            {
                ppu_background_palette [(address & 0x0F)] = (value & 0x3F) + 1;
                ppu_sprite_palette [(address & 0x0F)] = (value & 0x3F) + 1;
            }
            else
            {
                ppu_palette [address & 0x1F] = (value & 0x3F) + 1;
            }
        }
        else
        /* name tables */
        {
            name_tables_write[(address >> 10) & 3]
             [address & 0x3FF] = value;
        }
    }
    else
    /* pattern tables */
    {
        if (mmc_check_latches)
        {
            if ((address & 0xFFF) >= 0xFD0 && (address & 0xFFF) <= 0xFEF)
            {
                mmc_check_latches(address);
            }
        }

        if (ppu_vram_block [address >> 10] < FIRST_VROM_BLOCK)
        {
            int vram_block = ppu_vram_block [address >> 10];
            int this_tile = (address & 0x3FF) / 16;

            if (ppu_vram_set_end [vram_block] != this_tile)
            {
                if (ppu_vram_set_end [vram_block] == this_tile - 1)
                {
                    ppu_vram_set_end [vram_block] ++;
                }
                else
                {
                    recache_vram_set (vram_block);

                    ppu_vram_set_end [vram_block] =
                        ppu_vram_set_begin [vram_block] = this_tile;
                }
            }
        }
            ppu_vram_block_write_address [address >> 10] [address & 0x3FF] = value;
    }

    vram_address += address_increment;


}


UINT8 ppu_read (UINT16 address)
{
    int data = 0;


    if (address == 0x4014)
    {
        return (0);
    }


    /* Handle register mirroring. */

    switch (address & 7)
    {
        case 0x2002 & 7:

            /* PPU status register. */

            if (vblank_occured)
            {
                data |= VBLANK_FLAG_BIT;
                vblank_occured = FALSE;
            }

            if (!vram_writable)
            {
                if (background_enabled || sprites_enabled)
                {
                    data |= VRAM_WRITE_FLAG_BIT;
                }
            }

            if (hit_first_sprite)
            {
                data |= SPRITE_0_COLLISION_BIT;
            }
            /*
            else
            {
                data |= SPRITE_0_COLLISION_BIT;
            }
            */


            address_write = FALSE;


            return (data);


            break;


        case 0x2004 & 7:

            /* Sprite RAM I/O. */

            data = ppu_spr_ram [spr_ram_address++];


            break;


        case 0x2007 & 7:

            /* VRAM Read I/O. */

            return ppu_vram_read();


            break;


        default:


            break;
    }

    return (0);
}


void ppu_write (UINT16 address, UINT8 value)
{

    if (address == 0x4014)
    {
        /* Sprite RAM DMA. */

        do_spr_ram_dma (value);

        return;
    }


    /* Handle register mirroring. */

    switch (address & 7)
    {
        case 0x2000 & 7:

            /* Control register #1. */

            {
                int new_sprite_height = ((value & SPRITE_SIZE_BIT) ? 16 : 8);

                if (sprite_height != new_sprite_height)
                {
                    sprite_height = new_sprite_height;
                    sprite_list_needs_recache = TRUE;
                }
            }


            want_vblank_nmi = (value & VBLANK_NMI_FLAG_BIT);

            address_increment =
                ((value & ADDRESS_INCREMENT_BIT) ? 32 : 1);


            background_tileset =
                ((value & BACKGROUND_TILESET_BIT) ? 0x1000 : 0x0000);

            sprite_tileset =
                ((value & SPRITE_TILESET_BIT) ? 0x1000 : 0x0000);


            address_temp = (address_temp & ~(3 << 10)) | ((value & 3) << 10);

            break;


        case 0x2001 & 7:

            /* Control register #2. */

            background_enabled =
                (value & BACKGROUND_ENABLE_BIT);

            sprites_enabled =
                (value & SPRITES_ENABLE_BIT);

            background_clip_enabled =
                !(value & BACKGROUND_CLIP_LEFT_EDGE_BIT);

            ppu_clip_background = background_clip_enabled;
                                  

            sprites_clip_enabled =
                !(value & SPRITES_CLIP_LEFT_EDGE_BIT);

            break;


        case 0x2003 & 7:

            /* Sprite RAM address. */

            spr_ram_address = value;


            break;


        case 0x2004 & 7:

            /* Sprite RAM I/O. */

            if (ppu_spr_ram [spr_ram_address] != value)
            {
                ppu_spr_ram [spr_ram_address] = value;
                sprite_list_needs_recache = TRUE;
            }
            spr_ram_address++;


            break;


        case 0x2005 & 7:

            /* Horizontal / Vertical offset. */

            if (! address_write)
            {
                /* Horizontal offset. */

                address_write = TRUE;

                address_temp = (address_temp & ~0x1F) | ((value >> 3) & 0x1F);
                x_offset = value & 7;

            }
            else
            {
                /* Vertical offset. */

                address_write = FALSE;

                address_temp = (address_temp & ~(0x1F << 5)) | (((value >> 3) & 0x1F) << 5);
                address_temp = (address_temp & ~(7 << 12)) | ((value & 7) << 12);

            }

            break;

        case 0x2006 & 7:

            /* VRAM address. */

            if (! address_write)
            {
                address_write = TRUE;

                address_temp = (address_temp & 0xFF) | ((value & 0x3F) << 8);

            }
            else
            {
                address_write = FALSE;

                address_temp = (address_temp & ~0xFF) | value;

                vram_address = address_temp;
            }


            break;


        case 0x2007 & 7:

            /* VRAM Write I/O. */

            ppu_vram_write(value);


            break;



        default:


            break;
    }
}


static void do_spr_ram_dma(UINT8 page)
{
    /* Sprite RAM DMA. */

    int index;
    unsigned address = page * 0x100;

    for (index = 0; index < 256; index ++)
    {
        int value = cpu_read (address + index);

        if (ppu_spr_ram [spr_ram_address] != value)
        {
            ppu_spr_ram [spr_ram_address] = value;
            sprite_list_needs_recache = TRUE;
        }

        spr_ram_address++;
    }
}


static void vram_address_start_new_frame(void)
{
    if (background_enabled || sprites_enabled)
    {
        vram_address = address_temp;
    }
}


void ppu_start_line(void)
{
    if (background_enabled || sprites_enabled)
    {
        vram_address = (vram_address & (~0x1F & ~(1 << 10)))
         | (address_temp & (0x1F | (1 << 10)));
    }
}

void ppu_end_line(void)
{
    if (background_enabled || sprites_enabled)
    {
        vram_address += 0x1000;

        if ((vram_address & (7 << 12)) == 0)
        {
            vram_address += 32;

            switch (vram_address & (0x1F << 5))
            {
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


void ppu_clear (void)
{
    vblank_occured = FALSE;
    vram_writable = FALSE;

    first_line_this_frame = TRUE;
    vram_address_start_new_frame();

    hit_first_sprite = FALSE;
}


void ppu_start_render (void)
{
    clear_to_color (video_buffer, ppu_background_palette [0]);
}



static UINT8 sprites_on_line [240] [8];
static UINT8 sprite_count_on_line [240];
static INT8 sprite_overflow_on_line [240];


void recache_sprite_list (void)
{
    int line, sprite;

    FILE *spr_dmp = fopen("spr.dmp", "wb");

    for (line = 0; line < 240; line++)
    {
        sprite_overflow_on_line [line] = FALSE;
        sprite_count_on_line [line] = 0;
    }

    for (sprite = 0; sprite < 64; sprite++)
    {
        int first_y, last_y;

        first_y = ppu_spr_ram [(sprite * 4) + 0] + 1;
        last_y = first_y + sprite_height - 1;

        /* vertical clipping */
        if (last_y >= 239) last_y = 239;

        for (line = first_y; line <= last_y; line++)
        {
            if (sprite_count_on_line [line] == 8)
            {
                sprite_overflow_on_line [line] = TRUE;
                continue;
            }

            sprites_on_line [line] [8 - 1 - sprite_count_on_line [line]++] = sprite;
        }
    }

    sprite_list_needs_recache = FALSE;

    if (spr_dmp)
    {
        fwrite(ppu_spr_ram, 1, sizeof(ppu_spr_ram), spr_dmp);
        fwrite(sprite_count_on_line, 1, sizeof(sprite_count_on_line), spr_dmp);
        fwrite(sprite_overflow_on_line, 1, sizeof(sprite_overflow_on_line), spr_dmp);
        fwrite(sprites_on_line, 1, sizeof(sprites_on_line), spr_dmp);
        fclose (spr_dmp);
    }
}


void ppu_sprite_priority_0_check (int line)
{
    int first_y, last_y, y;
    int address;

    if (hit_first_sprite) return;

    if (!background_enabled || !sprites_enabled) return;

    first_y = ppu_spr_ram [0] + 2;
    last_y = first_y + sprite_height - 1;

    if (line < first_y || line > last_y) return;

    if (ppu_spr_ram [2] & 0x80) y = last_y - line;
    else y = line - first_y;

    address = ppu_spr_ram [1];

    if (sprite_height == 8)
    {
        address = address * 16 + sprite_tileset + y;

    }
    else /* sprite_height == 16 */
    {
        if (address & 1) address = (address - 1) * 16 + 0x1000 + y;
        else address = address * 16 + y;

        if (y & 8) address += 8;
    }

    if (vram_read (address + y) | vram_read (address + y + 8))
    {
        hit_first_sprite = TRUE;
    }
}


void ppu_stub_render_line (int line)
{
    recache_vram_sets ();
    ppu_sprite_priority_0_check(line);
}


/* VRAM address bit layout          */
/* -YYY VHyy yyyx xxxx              */
/* x = x tile offset in name table  */
/* y = y tile offset in name table  */
/* H = horizontal name table        */
/* V = vertical name table          */
/* Y = y line offset in tile        */


INT8 background_pixels [8 + 256 + 8];

void ppu_render_background (int line)
{
    int attribute_address, attribute_byte = 0;
    int name_table;
    UINT8 *name_table_address;

    int x, sub_x;
    int y, sub_y;

    name_table = (vram_address >> 10) & 3;
    name_table_address = name_tables_read[name_table];

    y = (vram_address >> 5) & 0x1F;
    sub_y = (vram_address >> 12) & 7;

    attribute_address =
     /* Y position */
     ((y >> 2) * 8) +
     /* X position */
     ((vram_address & 0x1F) >> 2);

    if (vram_address & 3)
    /* fetch and shift first attribute byte */
    {
        attribute_byte =
            name_table_address [0x3C0 + attribute_address] >>
            ( ( (y & 2) * 2 + (vram_address & 2)));
    }


    /* Draw the background. */
    for (x = 0; x < (256 / 8) + 1; x ++)
    {
        int attribute;
        int tile;
        UINT8 *cache_address;


        if (!(vram_address & 3))
        /* fetch and shift attribute byte */
        {
            attribute_byte =
            name_table_address [0x3C0 + attribute_address] >>
                ( (y & 2) * 2);
        }

        tile = name_table_address [vram_address & 0x3FF];

        tile = ((tile * 16) + background_tileset);
    
    
        if (mmc_check_latches)
        {
            if ((tile & 0xFFF) >= 0xFD0 && (tile & 0xFFF) <= 0xFEF)
            {
                mmc_check_latches(tile);
            }
        }

        cache_address = ppu_vram_block_cache_address [tile >> 10] +
            ((tile & 0x3FF) / 2 * 8) + sub_y * 8;


        attribute = ((attribute_byte & 3) << 2) | 3;

        for (sub_x = 0; sub_x < 8; sub_x ++)
        {
            UINT8 color = cache_address[sub_x] & attribute;

            if (color == 0) continue;

            background_pixels [8 + ((x * 8) + sub_x - x_offset)] = color;

            color = ppu_background_palette [color];


            PPU_PUTPIXEL (video_buffer,
                ((x * 8) + sub_x - x_offset), line, color);
        }

        ++vram_address;
        /* next name byte */
        if (!(vram_address & 1))
        /* new attribute */
        {
            if (!(vram_address & 2))
            /* new attribute byte */
            {
                ++attribute_address;

                if ((vram_address & 0x1F) == 0)
                /* horizontal name table toggle */
                {
                    name_table ^= 1;
                    name_table_address = name_tables_read[name_table];

                    /* handle address wrap */
                    vram_address = (vram_address - (1 << 5)) ^ (1 << 10);
                    attribute_address -= (1 << 3);
                }
            }
            else
            /* same attribute byte */
            {
                attribute_byte >>= 2;
            }
        }
    }
}


static void ppu_render_sprites (int line);

void ppu_render_line (int line)
{
    int i;

    if (!background_enabled && !sprites_enabled)
    {
        return;
    }

    if (sprites_enabled)
    {
        for (i = 0; i < 256; i++)
        {
            background_pixels [8 + i] = 0;
        }
    }

    recache_vram_sets ();

    if (background_enabled)
    {
        ppu_render_background (line);
    }

    if (sprites_enabled)
    {
        ppu_render_sprites (line);
    }
}

void ppu_vblank (void)
{
    vblank_occured = TRUE;

    vram_writable = TRUE;


    cpu_execute (1);


    if (want_vblank_nmi)
    {
        cpu_interrupt (CPU_INTERRUPT_NMI);
    }
}


void ppu_end_render (void)
{
    if (input_enable_zapper)
    {
        input_update_zapper ();
    }


    video_blit ();

    ppu_vblank ();
}


/* ----- Sprite rendering routines. ----- */


#define SPRITE_PRIORITY_BIT     0x20


#define SPRITE_H_FLIP_BIT       0x40

#define SPRITE_V_FLIP_BIT       0x80


static INLINE void ppu_render_sprite (int sprite, int line)
{
    int x, y, sub_x, sub_y;

    int first_x, last_x, last_y;

    int tile, color, attribute;

    int address, priority;

    int flip_h, flip_v;

    UINT8 *cache_address;


    /* Offset. */

    sprite *= 4;


    y = line - (ppu_spr_ram [sprite] + 1);

    x = ppu_spr_ram [sprite + 3];


    /* perform horizontal clipping */

    if (x <= 256 - 8)
    /* sprite not partially off right edge */
    {
        last_x = 7;

        if (sprites_clip_enabled && x < 8)
        /* is sprite clipped on left-edge? */
        {
         if (x == 0)
         /* sprite fully clipped? */
         {
          return;
         }

         /* sprite partially clipped */
         first_x = 8 - x;

        }
        else
        /* sprite not clipped on left edge */
        {
            first_x = 0;
        }
    }
    else
    /* sprite clipped on right edge */
    {
        first_x = 0;
        last_x = 256 - x - 1;
    }


    tile = ppu_spr_ram [sprite + 1];


    if (sprite_height == 8)
    {
        /* Draw 8x8 sprites. */

        address = ((tile * 16) + sprite_tileset);
    
    }
    else
    {
        /* Draw 8x16 sprites. */

        if (! (tile & 1))
        {
            address = (tile * 16);
        }
        else
        {
            address = (((tile - 1) * 16) + 0x1000);
        }

    }

    if (mmc_check_latches)
    {
        int address2 = address + y;

        if (y >= 8) address2 += 8;

        if (((address2 & 0xfff) >= 0xfd0) &&
            ((address2 & 0xfff) <= 0xfef))
        {
            mmc_check_latches (address2);
        }
    }

    cache_address = ppu_vram_block_cache_address [address >> 10] +
        ((address & 0x3FF) / 2 * 8);

    attribute = ((ppu_spr_ram [sprite + 2] & 3) << 2) | 3;


    /* Flipping. */

    flip_h = (ppu_spr_ram [sprite + 2] & SPRITE_H_FLIP_BIT);

    flip_v = (ppu_spr_ram [sprite + 2] & SPRITE_V_FLIP_BIT);


    priority = (ppu_spr_ram [sprite + 2] & SPRITE_PRIORITY_BIT);

    if (priority)
    /* low priority, plot under background */
    {
        for (sub_x = first_x; sub_x <= last_x; sub_x ++)
        {
            /* Transparency. */
            if (background_pixels [8 + (x + sub_x)])
            {
                continue;
            }

            if (flip_h)
            {
                if (flip_v)
                {
                    color = cache_address
                        [( (sprite_height - 1 - y) * 8) + (7 - sub_x)];
                }
                else
                {
                    color = cache_address
                        [(y * 8) + (7 - sub_x)];
                }
            }
            else
            {
                if (flip_v)
                {
                    color = cache_address
                        [( (sprite_height - 1 - y) * 8) + sub_x];
                }
                else
                {
                    color = cache_address
                        [(y * 8) + sub_x];
                }
            }

            /* Transparency. */
            if ((color &= attribute) == 0)
            {
                continue;
            }

            color = ppu_sprite_palette [color];


            PPU_PUTPIXEL (video_buffer,
                (x + sub_x), line, color);

        }
    }
    else
    /* high priority, plot over background */
    {
        for (sub_x = first_x; sub_x <= last_x; sub_x ++)
        {
            if (flip_h)
            {
                if (flip_v)
                {
                    color = cache_address
                        [( (sprite_height - 1 - y) * 8) + (7 - sub_x)];
                }
                else
                {
                    color = cache_address
                        [(y * 8) + (7 - sub_x)];
                }
            }
            else
            {
                if (flip_v)
                {
                    color = cache_address
                        [( (sprite_height - 1 - y) * 8) + sub_x];
                }
                else
                {
                    color = cache_address
                        [(y * 8) + sub_x];
                }
            }

            /* Transparency. */
            if ((color &= attribute) == 0)
            {
                continue;
            }


            color = ppu_sprite_palette [color];


            PPU_PUTPIXEL (video_buffer,
                (x + sub_x), line, color);

        }
    }
}


static void ppu_render_sprites (int line)
{
    int i, priority;


    if (! sprites_enabled)
    {
        return;
    }


    if (sprite_list_needs_recache)
    {
        recache_sprite_list ();
    }

    if (sprite_count_on_line [line] == 0) return;

    if (sprites_on_line [line] [sprite_count_on_line [line] - 1] == 0)
    /* sprite 0 collision detection */
    {
        if (background_enabled)
        {
            ppu_sprite_priority_0_check (line);
        }
    }

    for (i = 8 - sprite_count_on_line [line]; i < 8; i++)
    {
        int sprite = sprites_on_line [line] [i];

        ppu_render_sprite (sprite, line);
    }
}
