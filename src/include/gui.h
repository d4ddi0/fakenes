

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

gui.c: Declarations for the object-based GUI.

Copyright (c) 2004, Randy McDowell.
Copyright (c) 2004, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef GUI_H_INCLUDED

#define GUI_H_INCLUDED


#include <allegro.h>


#include "misc.h"


int show_gui (int);


void gui_message (int, AL_CONST UINT8 *, ...);


int gui_needs_restart;

int gui_is_active;


void gui_handle_keypress (int);


void gui_stop_replay (void);


#endif /* ! GUI_H_INCLUDED */
