

/*

FakeNES - A portable, open-source NES emulator.

input.h: Declarations for the input emulation.

Copyright (c) 2002, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __INPUT_H__
#define __INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


int input_enable_zapper;


int input_zapper_x_offset;

int input_zapper_y_offset;


int input_zapper_trigger;


int input_zapper_on_screen;


int input_init (void);

void input_exit (void);


void input_reset (void);


UINT8 input_read (UINT16);

void input_write (UINT16, UINT8);


int input_process (void);


void input_update_zapper (void);

void input_update_zapper_offsets (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __INPUT_H__ */
