

/*

FakeNES - A portable, Open Source NES emulator.

mmc.h: Declarations for the MMC emulation.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __MMC_H__
#define __MMC_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <allegro.h>


#include "misc.h"


int mmc_init (void);


void mmc_reset (void);


void mmc_request (ROM * rom);


typedef struct _MMC
{
    int number;

    const UINT8 * name;


    int (* init) (void);

    void (* reset) (void);


    const UINT8 * id;


    void (* save_state) (PACKFILE *, int);

    void (* load_state) (PACKFILE *, int);

} MMC;


int (* mmc_hblank_start) (int);

int (* mmc_scanline_end) (int);


void (* mmc_check_latches) (UINT16);


int mmc_get_name_table_count(void);

int mmc_uses_pattern_vram(void);


void mmc_save_state (PACKFILE *, int);

void mmc_load_state (PACKFILE *, int);


#ifdef __cplusplus
}
#endif

#endif /* ! __MMC_H__ */
