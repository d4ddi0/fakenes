/* FakeNES - A free, portable, Open Source NES emulator.

   input.h: Declarations for the input abstraction.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern LIST input_mode;
extern USTRING input_chat_text;

extern int input_autosave_interval;
extern BOOL input_enable_zapper;
extern int input_zapper_x_offset;
extern int input_zapper_y_offset;
extern BOOL input_zapper_trigger;
extern BOOL input_zapper_on_screen;

extern void input_load_config (void);
extern void input_save_config (void);
extern int input_init (void);
extern void input_exit (void);
extern void input_reset (void);
extern UINT8 input_read (UINT16);
extern void input_write (UINT16, UINT8);
extern void input_process (void);
extern void input_update_zapper (void);
extern void input_update_zapper_offsets (void);
extern void input_handle_keypress (int, int);
extern ENUM input_get_player_device (ENUM);
extern void input_set_player_device (ENUM, ENUM);
extern void input_map_player_button (ENUM, ENUM);
extern int input_get_player_button_param (ENUM, ENUM, ENUM);
extern void input_set_player_button_param (ENUM, ENUM, ENUM, int);
extern BOOL input_get_button_state (ENUM, ENUM);
extern void input_save_state (PACKFILE *, int);
extern void input_load_state (PACKFILE *, int);

enum
{
   INPUT_MODE_PLAY          = (1 << 0),
   INPUT_MODE_REPLAY        = (1 << 1),
   INPUT_MODE_REPLAY_RECORD = (1 << 2),
   INPUT_MODE_REPLAY_PLAY   = (1 << 3),
   INPUT_MODE_CHAT          = (1 << 4),
   INPUT_MODE_NSF           = (1 << 5),
};

enum
{
   INPUT_DEVICE_NONE = 0,
   INPUT_DEVICE_KEYS_1,
   INPUT_DEVICE_KEYS_2,
   INPUT_DEVICE_JOYSTICK_1,
   INPUT_DEVICE_JOYSTICK_2,
   INPUT_DEVICE_JOYSTICK_3,
   INPUT_DEVICE_JOYSTICK_4,
   INPUT_DEVICE_MOUSE,
   INPUT_DEVICES
};

enum
{
   INPUT_PLAYER_1 = 0,
   INPUT_PLAYER_2,
   INPUT_PLAYER_3,
   INPUT_PLAYER_4,
   INPUT_PLAYERS
};

/* Keep these in order! */
enum
{
   INPUT_BUTTON_A = 0,
   INPUT_BUTTON_B,
   INPUT_BUTTON_SELECT,
   INPUT_BUTTON_START,
   INPUT_BUTTON_UP,
   INPUT_BUTTON_DOWN,
   INPUT_BUTTON_LEFT,
   INPUT_BUTTON_RIGHT,
   INPUT_BUTTONS
};

enum
{
   INPUT_PLAYER_BUTTON_PARAM_AUTO,
   INPUT_PLAYER_BUTTON_PARAM_TURBO
};

#ifdef __cplusplus
}
#endif
#endif   /* !INPUT_H_INCLUDED */
