

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under Clarified Artistic License.

papu.h: Declarations for the APU abstraction.

Copyright (c) 2005, Randy McDowell.
Copyright (c) 2005, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef PAPU_H_INCLUDED

#define PAPU_H_INCLUDED


#include <allegro.h>


#include "misc.h"


#define PAPU_EXSOUND_VRC6   1

#define PAPU_EXSOUND_MMC5   8


#define PAPU_FILTER_LOW_PASS_MODE_1     1

#define PAPU_FILTER_LOW_PASS_MODE_2     2

#define PAPU_FILTER_LOW_PASS_MODE_3     4

#define PAPU_FILTER_HIGH_PASS           8


int papu_enable_square_1;

int papu_enable_square_2;


int papu_enable_triangle;

int papu_enable_noise;


int papu_enable_dmc;


int papu_enable_exsound;


int papu_swap_channels;


int papu_ideal_triangle;


int papu_linear_echo;


int papu_spatial_stereo;


enum
{
    PAPU_SPATIAL_STEREO_MODE_1 = 1,

    PAPU_SPATIAL_STEREO_MODE_2,

    PAPU_SPATIAL_STEREO_MODE_3,

};


int papu_dithering;


int papu_is_recording;


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


void papu_save_state (PACKFILE *, int);

void papu_load_state (PACKFILE *, int);


void papu_set_filter_list (int);

int papu_get_filter_list (void);


#endif /* ! PPU_H_INCLUDED */
