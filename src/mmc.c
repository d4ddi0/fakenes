/* FakeNES - A free, portable, Open Source NES emulator.

   mmc.c: Implementation of the MMC emulation.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "apu.h"
#include "common.h"
#include "cpu.h"
#include "debug.h"
#include "gui.h"
#include "machine.h"
#include "mmc.h"
#include "ppu.h"
#include "rom.h"
#include "timing.h"
#include "types.h"

void (*mmc_scanline_start) (const int);
void (*mmc_hblank_start) (const int);
void (*mmc_hblank_prefetch_start) (const int);
BOOL (*mmc_virtual_scanline_start) (const int);
BOOL (*mmc_virtual_hblank_start) (const int);
BOOL (*mmc_virtual_hblank_prefetch_start) (const int);
void (*mmc_predict_asynchronous_irqs) (const cpu_time_t cycles);
void (*mmc_check_vram_banking) (void);
void (*mmc_check_address_lines) (const UINT16);

static int mmc_name_table_count;
static int mmc_pattern_vram_in_use;
static int mmc_fixed_mirroring;

int mmc_get_name_table_count(void)
{
    return mmc_name_table_count;
}

int mmc_uses_pattern_vram(void)
{
    return mmc_pattern_vram_in_use;
}

int mmc_controls_mirroring(void)
{
   return !mmc_fixed_mirroring;
}

#include "mmc/none.h"
#include "mmc/mmc1.h"
#include "mmc/mmc3.h"
#include "mmc/mmc2and4.h"
#include "mmc/mmc5.h"
#include "mmc/unrom.h"
#include "mmc/cnrom.h"
#include "mmc/aorom.h"
#include "mmc/gnrom.h"
#include "mmc/bandai.h"
#include "mmc/dreams.h"
#include "mmc/nina.h"
#include "mmc/sunsoft4.h"
#include "mmc/vrc6.h"
#include "mmc/ffe_f3.h"

static const MMC *current_mmc = NULL;

#define MMC_FIRST_LIST_ITEM(id)     \
    if (mmc_ ##id.number == mapper_number)       \
        current_mmc = &mmc_ ##id

#define MMC_NEXT_LIST_ITEM(id)      \
    else if (mmc_ ##id.number == mapper_number)  \
        current_mmc = &mmc_ ##id

#define MMC_LAST_LIST_ITEM()        \
    else current_mmc = NIL

void mmc_request (const int mapper_number)
{
    MMC_FIRST_LIST_ITEM (none);     /* No mapper. */

    /* Nintendo MMCs. */
    MMC_NEXT_LIST_ITEM (mmc1);      /* MMC1. */
    MMC_NEXT_LIST_ITEM (mmc2);      /* MMC2. */
    MMC_NEXT_LIST_ITEM (mmc3);      /* MMC3. */
    MMC_NEXT_LIST_ITEM (mmc4);      /* MMC4. */
    MMC_NEXT_LIST_ITEM (mmc5);      /* MMC5. */

    /* Other MMCs. */
    MMC_NEXT_LIST_ITEM (unrom);     /* UNROM. */
    MMC_NEXT_LIST_ITEM (cnrom);     /* CNROM. */
    MMC_NEXT_LIST_ITEM (aorom);     /* AOROM. */
    MMC_NEXT_LIST_ITEM (gnrom);     /* GNROM. */
    MMC_NEXT_LIST_ITEM (bandai);    /* Bandai. */
    MMC_NEXT_LIST_ITEM (dreams);    /* Color Dreams. */
    MMC_NEXT_LIST_ITEM (nina);      /* NINA-001. */
    MMC_NEXT_LIST_ITEM (sunsoft4);  /* Sunsoft mapper #4. */
    MMC_NEXT_LIST_ITEM (vrc6);      /* VRC6. */
    MMC_NEXT_LIST_ITEM (vrc6v);     /* VRC6. */
    MMC_NEXT_LIST_ITEM (ffe_f3);    /* FFE F3xxx. */

    MMC_LAST_LIST_ITEM ();          /* Unsupported mapper. */
}

void mmc_force (const MMC *mmc)
{
   /* Like mmc_request(), but forces a mapper without requiring a mapper number. */

   current_mmc = mmc;
}

int mmc_init (void)
{
    int index;

    for (index = 0x8000; index < (64 << 10); index += (8 << 10))
    {
        cpu_set_write_address_8k (index, dummy_write);
    }

    mmc_scanline_start = NULL;
    mmc_hblank_start = NULL;
    mmc_hblank_prefetch_start = NULL;
    mmc_virtual_scanline_start = NULL;
    mmc_virtual_hblank_start = NULL;
    mmc_virtual_hblank_prefetch_start = NULL;
    mmc_predict_asynchronous_irqs = NULL;
    mmc_check_vram_banking = NULL;
    mmc_check_address_lines = NULL;

    mmc_name_table_count =
        (global_rom.control_byte_1 & ROM_CTRL_FOUR_SCREEN) ? 4 : 2;

    mmc_pattern_vram_in_use = (ROM_CHR_ROM_PAGES == 0);

    mmc_fixed_mirroring = TRUE;

    if (mmc_pattern_vram_in_use)
    {
        /* No VROM is present. */
        ppu_set_8k_pattern_table_vram ();
    }
    else
    {
        int index;

        /* Select first 8k page. */
        for (index = 0; index < 8; index ++)
        {
            ppu_set_1k_pattern_table_vrom_page ((index << 10), index);
        }
    }

    if (!current_mmc)
    {
        return (1);
    }

    log_printf ("Using memory mapper #%u (%s) (%d PRG, %d CHR).\n\n", current_mmc -> number,
            current_mmc -> name, ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);

    return (current_mmc -> init ());
}

void mmc_reset (void)
{
    current_mmc -> reset ();
}

void mmc_save_state (PACKFILE * file, const int version)
{
    if (current_mmc -> save_state)
    {
        pack_fwrite (current_mmc -> id, 8, file);

        current_mmc -> save_state (file, version);
    }
}

void mmc_load_state (PACKFILE * file, const int version)
{
    UINT8 signature [8];

    if (current_mmc -> load_state)
    {
        pack_fread (signature, 8, file);

        current_mmc -> load_state (file, version);
    }
}

void mmc_save_state_prg (PACKFILE * file, int version)
{
    if (current_mmc -> save_state_prg)
    {
        pack_fwrite (current_mmc -> id, 8, file);

        current_mmc -> save_state_prg (file, version);
    }
}

void mmc_load_state_prg (PACKFILE * file, const int version)
{
    UINT8 signature [8];

    if (current_mmc -> load_state_prg)
    {
        pack_fread (signature, 8, file);

        current_mmc -> load_state_prg (file, version);
    }
}

void mmc_save_state_chr (PACKFILE * file, const int version)
{
    if (current_mmc -> save_state_chr)
    {
        pack_fwrite (current_mmc -> id, 8, file);

        current_mmc -> save_state_chr (file, version);
    }
}

void mmc_load_state_chr (PACKFILE * file, const int version)
{
    UINT8 signature [8];

    if (current_mmc -> load_state_chr)
    {
        pack_fread (signature, 8, file);

        current_mmc -> load_state_chr (file, version);
    }
}
