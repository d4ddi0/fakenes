

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

#include "mmc.h"

#include "ppu.h"

#include "video.h"


#include "misc.h"


/* VRAM and sprite RAM. */

static UINT8 ppu_vram [12 * 1024];

static UINT8 ppu_spr_ram [256];

static UINT8 ppu_palette [32];
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


static int vram_address = 0;

static UINT8 spr_ram_address = 0;


static int address_write = 0;
static int address_temp = 0;
static int x_offset = 0;
static int address_increment = 1;


static int want_vblank_nmi = TRUE;

static int vblank_occured = FALSE;
static int vram_writable = FALSE;
static int mirror_mask = 0xFFF;
static int sprite_height = 8;


static int background_enabled = FALSE;
static int sprites_enabled = FALSE;
static int first_line_this_frame = TRUE;
static int background_tileset = 0;
static int buffered_vram_read = 0;
static int sprite_tileset = 0;

static UINT8 *name_tables[4];

static INLINE UINT8 vram_read (UINT16 address)
{
    if (address < 0x2000)
    {
        if (mmc_no_vrom)
        {
            return (ppu_vram [address]);
        }
        else
        {
            return (READ_VROM (address));
        }
    }
    else
    {
        return (ppu_vram [address]);
    }
}


int ppu_init (void)
{
    ppu_reset ();


    return (0);
}


void ppu_exit (void)
{
    FILE * dump_file;


#ifdef DEBUG

    dump_file = fopen ("ppudump.ram", "wb");


    if (dump_file)
    {
        fwrite (ppu_vram, 1, sizeof (ppu_vram), dump_file);

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

void set_name_table_address(int table, UINT8 *address)
{
    name_tables[table] = address;
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
        one_screen_base_address = ppu_vram + 0x2000;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_ONE_SCREEN_2400:
        one_screen_base_address = ppu_vram + 0x2400;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_ONE_SCREEN_2800:
        one_screen_base_address = ppu_vram + 0x2800;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_ONE_SCREEN_2C00:
        one_screen_base_address = ppu_vram + 0x2C00;
        set_ppu_mirroring_one_screen();

        break;

     case MIRRORING_VERTICAL:
        set_name_table_address(0, ppu_vram + 0x2000);
        set_name_table_address(1, ppu_vram + 0x2400);
        set_name_table_address(2, ppu_vram + 0x2000);
        set_name_table_address(3, ppu_vram + 0x2400);

        break;

     case MIRRORING_HORIZONTAL:
        set_name_table_address(0, ppu_vram + 0x2000);
        set_name_table_address(1, ppu_vram + 0x2000);
        set_name_table_address(2, ppu_vram + 0x2400);
        set_name_table_address(3, ppu_vram + 0x2400);

        break;

     case MIRRORING_FOUR_SCREEN:
        set_name_table_address(0, ppu_vram + 0x2000);
        set_name_table_address(1, ppu_vram + 0x2400);
        set_name_table_address(2, ppu_vram + 0x2800);
        set_name_table_address(3, ppu_vram + 0x2C00);

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


void ppu_reset (void)
{
    memset (ppu_vram, NULL, sizeof (ppu_vram));

    memset (ppu_spr_ram, NULL, sizeof (ppu_spr_ram));


    vram_address = 0;
    address_temp = 0;

    x_offset = 0;

    spr_ram_address = 0;

    address_write = FALSE;

    address_increment = 1;

    want_vblank_nmi = TRUE;

    vblank_occured = FALSE;


    sprite_height = 8;


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
            buffered_vram_read = name_tables[(address >> 10) & 3]
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

        if (mmc_no_vrom)
        {
            buffered_vram_read = ppu_vram [address];
        }
        else
        {
            buffered_vram_read = READ_VROM (address);
        }
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
            name_tables[(address >> 10) & 3]
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

        ppu_vram [address] = value;
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
            else
            {
                data |= SPRITE_0_COLLISION_BIT;
            }

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

            want_vblank_nmi = (value & VBLANK_NMI_FLAG_BIT);

            sprite_height = ((value & SPRITE_SIZE_BIT) ? 16 : 8);


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


            break;


        case 0x2003 & 7:

            /* Sprite RAM address. */

            spr_ram_address = value;


            break;


        case 0x2004 & 7:

            /* Sprite RAM I/O. */

            ppu_spr_ram [spr_ram_address++] = value;


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
        ppu_spr_ram [spr_ram_address++] = cpu_read (address + index);
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
}


/* VRAM address bit layout          */
/* -YYY VHyy yyyx xxxx              */
/* x = x tile offset in name table  */
/* y = y tile offset in name table  */
/* H = horizontal name table        */
/* V = vertical name table          */
/* Y = y line offset in tile        */

void ppu_render_line (int line)
{
    hline (video_buffer, 0, line, 255, ppu_background_palette[0]);

    if (background_enabled)
    {
        int attribute_address, attribute_byte = 0;
        int name_table;
        UINT8 *name_table_address;

        int x, sub_x;
        int y, sub_y;

        name_table = (vram_address >> 10) & 3;
        name_table_address = name_tables[name_table];

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
            int tile, color;
            int pattern1, pattern2;


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

            pattern1 = vram_read (tile + sub_y);
            pattern2 = vram_read ((tile + 8) + sub_y);


            if ((pattern1 | pattern2) != 0)
            {
                attribute = (attribute_byte & 3) << 2;

                for (sub_x = 0; sub_x < 8; sub_x ++)
                {
                    color = ((pattern2 & 128) >> 6);
                    color += ((pattern1 & 128) >> 7);

                    if (color != 0)
                    {
                        color = ppu_background_palette [attribute + color];
        
        
                        _putpixel (video_buffer,
                            ((x * 8) + sub_x - x_offset), line, color);
                    }


                    pattern1 <<= 1;
                    pattern2 <<= 1;
                }
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
                        name_table_address = name_tables[name_table];

                        /* handle address wrap */
                        vram_address = (vram_address - (1 << 5)) ^ (1 << 10);
                        attribute_address = (attribute_address - (1 << 3));
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


static void ppu_render_sprites (void);


void ppu_render (void)
{
    ppu_render_sprites ();


    video_blit ();

    ppu_vblank ();
}


/* ----- Sprite rendering routines. ----- */


#define SPRITE_PRIORITY_BIT     0x20


#define SPRITE_H_FLIP_BIT       0x40

#define SPRITE_V_FLIP_BIT       0x80


static INLINE void ppu_fetch_tile (int address, UINT8 * buffer)
{
    int x, y, pixel;

    int plane1, plane2;


    if (mmc_check_latches)
    {
        if (((address & 0xfff) >= 0xfd0) &&
            ((address & 0xfff) <= 0xfef))
        {
            mmc_check_latches (address);
        }
    }


    for (y = 0; y < 8; y ++)
    {
        /* Retrieve the two bit planes. */

        plane1 = vram_read (address + y);
    
        plane2 = vram_read ((address + 8) + y);


        /* Render a single line. */

        for (x = 0; x < 8; x ++)
        {
            /* Merge the two bit planes. */

            pixel = ((plane1 & 0x80) >> 7);

            pixel += ((plane2 & 0x80) >> 6);


            buffer [(y * 8) + x] = pixel;


            plane1 <<= 1;

            plane2 <<= 1;
        }
    }
}


static UINT8 sprite_buffer [8 * 16];


static INLINE void ppu_render_sprite (int sprite)
{
    int x, y, sub_x, sub_y;

    int tile, color, attribute;

    int address1, address2;

    int flip_h, flip_v;


    /* Offset. */

    sprite *= 4;


    y = (ppu_spr_ram [sprite] + 1);

    x = ppu_spr_ram [sprite + 3];


    /*
    if ((y >= 240) && (y <= (256 - sprite_height)))
    {
        return;
    }
    */


    tile = ppu_spr_ram [sprite + 1];


    if (sprite_height == 8)
    {
        /* Draw 8x8 sprites. */

        address1 = ((tile * 16) + sprite_tileset);
    
        ppu_fetch_tile (address1, sprite_buffer);
    }
    else
    {
        /* Draw 8x16 sprites. */

        if (! (tile & 1))
        {
            address1 = (tile * 16);

            address2 = ((tile + 1) * 16);
        }
        else
        {
            address1 = (((tile - 1) * 16) + 0x1000);

            address2 = ((tile * 16) + 0x1000);
        }


        ppu_fetch_tile (address1, sprite_buffer);

        ppu_fetch_tile (address2, (sprite_buffer + 0x40));
    }


    attribute = ((ppu_spr_ram [sprite + 2] & 0x03) << 2);


    /* Flipping. */

    flip_h = (ppu_spr_ram [sprite + 2] & SPRITE_H_FLIP_BIT);

    flip_v = (ppu_spr_ram [sprite + 2] & SPRITE_V_FLIP_BIT);


    for (sub_y = 0; sub_y < sprite_height; sub_y ++)
    {
        for (sub_x = 0; sub_x < 8; sub_x ++)
        {
            /* Clipping. */

            if (((x + sub_x) >= 256) || ((y + sub_y) >= 240))
            {
                continue;
            }


            color = sprite_buffer [(sub_y * 8) + sub_x];


            /* Transparency. */

            if (color == 0)
            {
                continue;
            }


            color |= attribute;

            color = ppu_sprite_palette [color];


            if (flip_h)
            {
                if (flip_v)
                {
                    _putpixel (video_buffer, ((x + 7) - sub_x),
                        ((y + (sprite_height - 1)) - sub_y), color);
                }
                else
                {
                    _putpixel (video_buffer,
                        ((x + 7) - sub_x), (y + sub_y), color);
                }
            }
            else
            {
                if (flip_v)
                {
                    _putpixel (video_buffer, (x + sub_x),
                        ((y + (sprite_height - 1)) - sub_y), color);
                }
                else
                {
                    _putpixel (video_buffer,
                        (x + sub_x), (y + sub_y), color);
                }
            }
        }
    }
}


static void ppu_render_sprites (void)
{
    int sprite, priority;


    if (! sprites_enabled)
    {
        return;
    }


    for (sprite = 63; sprite >= 0; sprite --)
    {
        priority = (ppu_spr_ram
            [(sprite * 4) + 2] & SPRITE_PRIORITY_BIT);


        if (! priority)
        {
            ppu_render_sprite (sprite);
        }
    }
}

