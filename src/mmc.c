

/*

FakeNES - A portable, Open Source NES emulator.

mmc.c: Implementation of the MMC emulation.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <stdio.h>


#include "cpu.h"

#include "gui.h"

#include "mmc.h"

#include "ppu.h"

#include "rom.h"


#include "misc.h"


#include "timing.h"


#include "mmc/mmc1.h"

#include "mmc/mmc2.h"

#include "mmc/mmc3.h"

#include "mmc/mmc4.h"


#include "mmc/unrom.h"

#include "mmc/cnrom.h"

#include "mmc/aorom.h"

#include "mmc/gnrom.h"


#include "mmc/bandai.h"

#include "mmc/dreams.h"

#include "mmc/nina.h"

#include "mmc/sunsoft.h"


#define MMC_FIRST_LIST_ITEM(id)     \
    if (mmc_ ##id.number == rom -> mapper_number)       \
        rom -> current_mmc = &mmc_ ##id

#define MMC_NEXT_LIST_ITEM(id)      \
    else if (mmc_ ##id.number == rom -> mapper_number)  \
        rom -> current_mmc = &mmc_ ##id

#define MMC_LAST_LIST_ITEM()        \
    else rom -> current_mmc = NULL


static int none_init (void);

static void none_reset (void);


const MMC mmc_none =
{
    0, "No mapper",

    none_init, none_reset
};


void none_reset (void)
{
    /* Do nothing. */
}


int none_init (void)
{
    /* Select first 32k page. */

    cpu_set_read_address_32k_rom_block (0x8000, 0);
            

    if (ROM_CHR_ROM_PAGES > 0)
    {
        int index;

        /* Select first 8k page. */

        for (index = 0; index < 8; index ++)
        {
            ppu_set_ram_1k_pattern_vrom_block ((index << 10), index);
        }
    }
    else
    {
        /* No VROM is present. */

        ppu_set_ram_8k_pattern_vram ();
    }


    return (0);
}


void mmc_request (const ROM * rom)
{
    MMC_FIRST_LIST_ITEM (none);     /* No mapper. */


    /* Nintendo MMCs. */

    MMC_NEXT_LIST_ITEM (mmc1);      /* MMC1. */

    MMC_NEXT_LIST_ITEM (mmc2);      /* MMC2. */

    MMC_NEXT_LIST_ITEM (mmc3);      /* MMC3. */

    MMC_NEXT_LIST_ITEM (mmc4);      /* MMC4. */


    /* Other MMCs. */

    MMC_NEXT_LIST_ITEM (unrom);     /* UNROM. */
                          
    MMC_NEXT_LIST_ITEM (cnrom);     /* CNROM. */

    MMC_NEXT_LIST_ITEM (aorom);     /* AOROM. */

    MMC_NEXT_LIST_ITEM (gnrom);     /* GNROM. */


    MMC_NEXT_LIST_ITEM (bandai);    /* Bandai. */

    MMC_NEXT_LIST_ITEM (dreams);    /* Color Dreams. */

    MMC_NEXT_LIST_ITEM (nina);      /* NINA-001. */

    MMC_NEXT_LIST_ITEM (sunsoft);   /* Sunsoft. */


    MMC_LAST_LIST_ITEM ();          /* Unsupported mapper. */
}


int mmc_init (void)
{
    int index;


    for (index = 0x8000; index < (64 << 10); index += (8 << 10))
    {
        cpu_set_write_address_8k (index, dummy_read);
    }


    mmc_scanline_start = NULL;

    mmc_scanline_end = NULL;


    mmc_check_latches = NULL;


    if (ROM_CURRENT_MMC == NULL)
    {
        return (1);
    }


    if (! gui_is_active)
    {
        printf ("Using memory mapper #%u (%s) (%d PRG, %d CHR).\n\n", ROM_MAPPER_NUMBER,
            ROM_CURRENT_MMC -> name, ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    return (ROM_CURRENT_MMC -> init ());
}


void mmc_reset (void)
{
    ROM_CURRENT_MMC -> reset ();
}
