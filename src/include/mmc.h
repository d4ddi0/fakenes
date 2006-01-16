/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   mmc.h: Declarations for the MMC emulations.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef MMC_H_INCLUDED
#define MMC_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif


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



int mmc_init (void);


void mmc_reset (void);


void mmc_request (ROM *);


int (* mmc_hblank_start) (int);

int (* mmc_scanline_end) (int);


void (* mmc_check_latches) (UINT16);


int mmc_get_name_table_count (void);

int mmc_uses_pattern_vram (void);


void mmc_save_state (PACKFILE *, int);

void mmc_load_state (PACKFILE *, int);


#ifdef __cplusplus
}
#endif
#endif   /* !MMC_H_INCLUDED */
