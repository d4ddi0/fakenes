

/*

FakeNES - A portable, Open Source NES emulator.

gui.c: Declarations for the object-based GUI.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __GUI_H__
#define __GUI_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <allegro.h>


#include "misc.h"


int show_gui (int);


void gui_message (int, AL_CONST UINT8 *, ...);


int gui_needs_restart;

int gui_is_active;


void gui_handle_keypress (int);


#ifdef __cplusplus
}
#endif

#endif /* ! __GUI_H__ */
