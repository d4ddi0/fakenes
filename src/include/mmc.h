

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


#include "misc.h"


UINT8 * mmc_rom_banks [4];


int mmc_init (void);


void mmc_reset (void);


void (* mmc_write) (UINT16, UINT8);


int (* mmc_scanline_start) (int);

void (* mmc_check_latches) (UINT16);


#define READ_ROM(offset)   \
    (cpu_block_2k_read_address [(offset >> 11)] [offset & 0x7ff])


#ifdef __cplusplus
}
#endif

#endif /* ! __MMC_H__ */
