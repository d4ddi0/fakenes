

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under Clarified Artistic License.

input.h: Declarations for the input abstraction.

Copyright (c) 2003, Randy McDowell.
Copyright (c) 2003, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef INPUT_H_INCLUDED

#define INPUT_H_INCLUDED


#include <allegro.h>


#include "misc.h"


int input_enable_zapper;


int input_zapper_x_offset;

int input_zapper_y_offset;


int input_zapper_trigger;


int input_zapper_on_screen;


int input_autosave_interval;

int input_autosave_triggered;


int input_mode;


#define INPUT_MODE_PLAY             1


#define INPUT_MODE_REPLAY           (1 << 1)


#define INPUT_MODE_REPLAY_RECORD    (1 << 2)

#define INPUT_MODE_REPLAY_PLAY      (1 << 3)


#define INPUT_MODE_CHAT             (1 << 4)


UINT8 input_chat_name [256];

UINT8 input_chat_text [256];


int input_chat_offset;


int input_init (void);

void input_exit (void);


void input_reset (void);


UINT8 input_read (UINT16);

void input_write (UINT16, UINT8);


int input_process (void);


void input_update_zapper (void);

void input_update_zapper_offsets (void);


int input_get_player_device (int);

void input_set_player_device (int, int);


enum
{
    INPUT_DEVICE_NONE,


    INPUT_DEVICE_KEYBOARD_1, INPUT_DEVICE_KEYBOARD_2,

    INPUT_DEVICE_JOYSTICK_1, INPUT_DEVICE_JOYSTICK_2
};


void input_map_device_button (int, int, int);


enum
{
    INPUT_DEVICE_BUTTON_A,

    INPUT_DEVICE_BUTTON_B,


    INPUT_DEVICE_BUTTON_START,

    INPUT_DEVICE_BUTTON_SELECT,


    INPUT_DEVICE_BUTTON_UP,

    INPUT_DEVICE_BUTTON_DOWN,

    INPUT_DEVICE_BUTTON_LEFT,

    INPUT_DEVICE_BUTTON_RIGHT
};


PACKFILE * replay_file;

PACKFILE * replay_file_chunk;


void input_save_state (PACKFILE *, int);

void input_load_state (PACKFILE *, int);


#endif /* ! INPUT_H_INCLUDED */
