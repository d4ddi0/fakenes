

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


void gui_spawn_file_menu_snapshot (void);


void gui_spawn_machine_menu_status (void);


void gui_spawn_machine_state_menu_save (void);

void gui_spawn_machine_state_menu_restore (void);


void gui_spawn_options_video_layers_menu_sprites_a (void);

void gui_spawn_options_video_layers_menu_sprites_b (void);

void gui_spawn_options_video_layers_menu_background (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __GUI_H__ */
