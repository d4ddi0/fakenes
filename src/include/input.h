

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under Clarified Artistic License.

input.h: Declarations for the input abstraction.

Copyright (c) 2003, Randy McDowell.
Copyright (c) 2003, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __INPUT_H__
#define __INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <allegro.h>


#include "misc.h"


int input_enable_zapper;


int input_zapper_x_offset;

int input_zapper_y_offset;


int input_zapper_trigger;


int input_zapper_on_screen;


int input_autosave_interval;

int input_autosave_triggered;


int input_init (void);

void input_exit (void);


void input_reset (void);


UINT8 input_read (UINT16);

void input_write (UINT16, UINT8);


int input_process (void);


void input_update_zapper (void);

void input_update_zapper_offsets (void);


void input_save_state (PACKFILE *, int);

void input_load_state (PACKFILE *, int);


#ifdef __cplusplus
}
#endif

#endif /* ! __INPUT_H__ */
