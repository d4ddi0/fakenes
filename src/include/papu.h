

/*

FakeNES - A portable, Open Source NES emulator.

papu.h: Declarations for the APU interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __PAPU_H__
#define __PAPU_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


#define PAPU_EXSOUND_VRC6   1

#define PAPU_EXSOUND_MMC5   8


int papu_filter_type;


int papu_enable_square_1;

int papu_enable_square_2;


int papu_enable_triangle;

int papu_enable_noise;


int papu_enable_dmc;


int papu_enable_exsound;


int papu_ideal_triangle;


int papu_linear_echo;

int papu_surround_sound;


int papu_dithering;


int papu_init (void);

int papu_reinit (void);

void papu_exit (void);


void papu_reset (void);


UINT8 papu_read (UINT16);

void papu_write (UINT16, UINT8);

void papu_exwrite (UINT16, UINT8);


void papu_set_exsound (int);

void papu_clear_exsound (void);


int papu_start_record (void);

void papu_stop_record (void);


void papu_process (void);

void papu_update (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __PAPU_H__ */
