

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


#include "mmc/dreams.h"

#include "mmc/nina.h"

#include "mmc/sunsoft.h"


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

void none_reset (void)
{
}

AL_CONST MMC mmc_none =
{
 "No mapper",
 none_init,
 none_reset
};

void mmc_request (ROM *rom)
{
    switch (rom -> mapper_number)
    {
        /* No banking hardware. */

        case 0: rom -> current_mmc = &mmc_none; break;


        /* MMC1. */

        case 1: rom -> current_mmc = &mmc_mmc1; break;


        /* UNROM. */

        case 2: rom -> current_mmc = &mmc_unrom; break;


        /* CNROM. */

        case 3: rom -> current_mmc = &mmc_cnrom; break;


        /* MMC3. */

        case 4: rom -> current_mmc = &mmc_mmc3; break;


        /* AOROM. */

        case 7: rom -> current_mmc = &mmc_aorom; break;


        /* MMC2. */

        case 9: rom -> current_mmc = &mmc_mmc2; break;


        /* MMC4. */

        case 10: rom -> current_mmc = &mmc_mmc4; break;


        /* Color Dreams. */

        case 11: rom -> current_mmc = &mmc_dreams; break;


        /* Nina-1. */

        case 34: rom -> current_mmc = &mmc_nina; break;


        /* GNROM */
	
        case 66: rom -> current_mmc = &mmc_gnrom; break;


        /* Sunsoft. */

        case 68: rom -> current_mmc = &mmc_sunsoft; break;


        /* Unsupported mapper. */

        default: rom -> current_mmc = NULL; break;

    }
}


int mmc_init (void)
{
    int index;


    for (index = 0x8000; index < (64 << 10); index += (8 << 10))
    {
        cpu_set_write_address_8k (index, dummy_read);
    }


    mmc_scanline_start = NULL;

    mmc_check_latches = NULL;


    if (ROM_CURRENT_MMC == NULL) return (1);

    if (! gui_is_active)
    {
        printf ("Using memory mapper #%u (%s) "
            "(%d PRG, %d CHR).\n\n",
            ROM_MAPPER_NUMBER, ROM_CURRENT_MMC -> name,
            ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }

    return ROM_CURRENT_MMC -> init();
}


void mmc_reset (void)
{
    ROM_CURRENT_MMC -> reset();
}
