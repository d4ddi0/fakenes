

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


int mmc_init (void)
{
    int index;


    for (index = 0x8000; index < (64 << 10); index += (8 << 10))
    {
        cpu_set_write_address_8k (index, dummy_read);
    }


    mmc_scanline_start = NULL;

    mmc_check_latches = NULL;


    switch (ROM_MAPPER_NUMBER)
    {
        /* No banking hardware. */

        case 0:

            /* Select first 32k page. */

            cpu_set_read_address_32k_rom_block (0x8000, 0);
            

            if (ROM_CHR_ROM_PAGES > 0)
            {
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


            break;


        /* MMC1. */

        case 1: return (mmc1_init ()); break;


        /* UNROM. */

        case 2: return (unrom_init ()); break;


        /* CNROM. */

        case 3: return (cnrom_init ()); break;


        /* MMC3. */

        case 4: return (mmc3_init ()); break;


        /* AOROM. */

        case 7: return (aorom_init ()); break;


        /* MMC2. */

        case 9: return (mmc2_init ()); break;


        /* MMC4. */

        case 10: return (mmc4_init ()); break;


        /* Color Dreams. */

        case 11: return (dreams_init ()); break;


        /* Nina-1. */

        case 34: return (nina_init ()); break;


        /* GNROM */
	
        case 66: return (gnrom_init ()); break;


        /* Sunsoft. */

        case 68: return (sunsoft_init ()); break;


        default:

            return (1);


            break;
    }
}


void mmc_reset (void)
{
    switch (ROM_MAPPER_NUMBER)
    {
        /* MMC1. */

        case 1: mmc1_reset (); break;


        /* UNROM. */

        case 2: unrom_reset (); break;


        /* CNROM. */

        case 3: cnrom_reset (); break;


        /* MMC3. */

        case 4: mmc3_reset (); break;


        /* AOROM. */

        case 7: aorom_reset (); break;


        /* MMC2. */

        case 9: mmc2_reset (); break;


        /* MMC4. */

        case 10: mmc4_reset (); break;


        /* Color Dreams. */

        case 11: dreams_reset (); break;


        /* Nina-1. */

        case 34: nina_reset (); break;


        /* GNROM */
	
        case 66: gnrom_reset (); break;


        /* Sunsoft. */

        case 68: sunsoft_reset (); break;


        default:

            break;
    }
}
