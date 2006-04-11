/* FakeNES - A free, portable, Open Source NES emulator.

   input.c: Implementation of the input abstraction.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "common.h"
#include "gui.h"
#include "input.h"
#include "ppu.h"
#include "rom.h"
#include "save.h"
#include "timing.h"
#include "types.h"
#include "video.h"


int input_enable_zapper = FALSE;


int input_mode = 0;


USTRING input_chat_text;


static int wait_frames = 0;


static int buttons [4] [8];


enum
{
    INPUT_PLAYER_1, INPUT_PLAYER_2,

    INPUT_PLAYER_3, INPUT_PLAYER_4
};


static UINT8 key1_defaults [25] = { "24 26 64 67 84 85 82 83\0" };

static UINT8 key2_defaults [25] = { "38 40 44 46 45 39 41 43\0" };


static UINT8 key1_buffer [100];
                
static UINT8 key2_buffer [100];


static int key1_scancodes [8];

static int key2_scancodes [8];


static UINT8 joy1_defaults [8] = { "1 0 2 3\0" };

static UINT8 joy2_defaults [8] = { "1 0 2 3\0" };


static UINT8 joy1_buffer [8];

static UINT8 joy2_buffer [8];


static int joy1_buttons [4];

static int joy2_buttons [4];


static int last_write = 0;


static int current_read_p1 = 0;

static int current_read_p2 = 0;


static int input_devices [4];


static int zapper_mask = 0;


void input_update_zapper_offsets (void)
{
    int mouse_needs_range_fixup = FALSE;

    input_zapper_x_offset = mouse_x;

    input_zapper_y_offset = mouse_y;

    input_zapper_trigger = (mouse_b & 0x01);


    /* Perform bounds checking */
    if (input_zapper_x_offset >= 256)
    {
        mouse_needs_range_fixup = TRUE;
        input_zapper_x_offset = 255;
    }

    if (input_zapper_y_offset >= 240)
    {
        mouse_needs_range_fixup = TRUE;
        input_zapper_y_offset = 239;
    }

    if (mouse_needs_range_fixup)
    {
        position_mouse (input_zapper_x_offset, input_zapper_y_offset);
    }


    if ((input_zapper_x_offset < 256) && (input_zapper_y_offset < 240))
    {
        input_zapper_on_screen = TRUE;
    }


    if (! input_zapper_on_screen)
    {
        input_update_zapper ();
    }
}


void input_update_zapper (void)
{
    int pixel;


    zapper_mask = 0x08;


    if (input_zapper_trigger)
    {
        /* Left button. */

        zapper_mask |= 0x10;
    }


    if (input_zapper_on_screen)
    {
        pixel = (_getpixel (video_buffer, input_zapper_x_offset, input_zapper_y_offset) - 1);


        if ((pixel == 32) || (pixel == 48))
        {
            zapper_mask &= ~0x08;
        }
    }
}


static INLINE void load_keyboard_layouts (void)
{
    memset (key1_buffer, NIL, sizeof (key1_buffer));

    memset (key2_buffer, NIL, sizeof (key2_buffer));


    sprintf (key1_buffer, "%s", get_config_string ("input", "key1_scancodes", key1_defaults));

    sprintf (key2_buffer, "%s", get_config_string ("input", "key2_scancodes", key2_defaults));


    if (sscanf (key1_buffer, "%d%d%d%d%d%d%d%d",
        &key1_scancodes [0], &key1_scancodes [1], &key1_scancodes [2],
        &key1_scancodes [3], &key1_scancodes [4], &key1_scancodes [5],
        &key1_scancodes [6], &key1_scancodes [7]) < 8)
    {
        sscanf (key1_defaults, "%d%d%d%d%d%d%d%d",
        &key1_scancodes [0], &key1_scancodes [1], &key1_scancodes [2],
        &key1_scancodes [3], &key1_scancodes [4], &key1_scancodes [5],
        &key1_scancodes [6], &key1_scancodes [7]);
    }


    if (sscanf (key2_buffer, "%d%d%d%d%d%d%d%d",
        &key2_scancodes [0], &key2_scancodes [1], &key2_scancodes [2],
        &key2_scancodes [3], &key2_scancodes [4], &key2_scancodes [5],
        &key2_scancodes [6], &key2_scancodes [7]) < 8)
    {
        sscanf (key2_defaults, "%d%d%d%d%d%d%d%d",
        &key2_scancodes [0], &key2_scancodes [1], &key2_scancodes [2],
        &key2_scancodes [3], &key2_scancodes [4], &key2_scancodes [5],
        &key2_scancodes [6], &key2_scancodes [7]);
    }
}


static INLINE void load_joystick_layouts (void)
{
    memset (joy1_buffer, NIL, sizeof (joy1_buffer));

    memset (joy2_buffer, NIL, sizeof (joy2_buffer));


    sprintf (joy1_buffer, "%s", get_config_string ("input", "joy1_buttons", joy1_defaults));

    sprintf (joy2_buffer, "%s", get_config_string ("input", "joy2_buttons", joy2_defaults));


    if (sscanf (joy1_buffer, "%d%d%d%d", &joy1_buttons [0],
        &joy1_buttons [1], &joy1_buttons [2], &joy1_buttons [3]) < 4)
    {
        sscanf (joy1_defaults, "%d%d%d%d", &joy1_buttons [0],
        &joy1_buttons [1], &joy1_buttons [2], &joy1_buttons [3]);
    }


    if (sscanf (joy2_buffer, "%d%d%d%d", &joy2_buttons [0],
        &joy2_buttons [1], &joy2_buttons [2], &joy2_buttons [3]) < 4)
    {
        sscanf (joy2_defaults, "%d%d%d%d", &joy2_buttons [0],
        &joy2_buttons [1], &joy2_buttons [2], &joy2_buttons [3]);
    }
}


int input_autosave_interval = 0;

int input_autosave_triggered = FALSE;


int input_init (void)
{
    install_keyboard ();

    install_mouse ();


    if (load_joystick_data (NULL) != 0)
    {
        /* load_joystick_data() failed; reinitialize joystick system. */

        install_joystick (JOY_TYPE_AUTODETECT);
    }


    input_devices [0] = get_config_int ("input", "player_1_device", INPUT_DEVICE_KEYBOARD_1);

    input_devices [1] = get_config_int ("input", "player_2_device", INPUT_DEVICE_KEYBOARD_2);


    input_devices [2] = get_config_int ("input", "player_3_device", INPUT_DEVICE_NONE);

    input_devices [3] = get_config_int ("input", "player_4_device", INPUT_DEVICE_NONE);


    input_enable_zapper = get_config_int ("input", "enable_zapper", FALSE);


    input_autosave_interval = get_config_int ("timing", "autosave_interval", 0);


    load_keyboard_layouts ();

    load_joystick_layouts ();


    USTRING_CLEAR(input_chat_text);


    input_mode = 0;


    input_mode |= INPUT_MODE_PLAY;


    return (0);
}


void input_exit (void)
{
    remove_keyboard ();

    remove_mouse ();


    remove_joystick ();


    set_config_int ("input", "player_1_device", input_devices [0]);

    set_config_int ("input", "player_2_device", input_devices [1]);


    set_config_int ("input", "player_3_device", input_devices [2]);

    set_config_int ("input", "player_4_device", input_devices [3]);


    sprintf (key1_buffer, "%d %d %d %d %d %d %d %d", key1_scancodes [0], key1_scancodes [1],
        key1_scancodes [2], key1_scancodes [3], key1_scancodes [4],
        key1_scancodes [5], key1_scancodes [6], key1_scancodes [7]);

    sprintf (key2_buffer, "%d %d %d %d %d %d %d %d", key2_scancodes [0], key2_scancodes [1],
        key2_scancodes [2], key2_scancodes [3], key2_scancodes [4],
        key2_scancodes [5], key2_scancodes [6], key2_scancodes [7]);


    set_config_string ("input", "key1_scancodes", key1_buffer);

    set_config_string ("input", "key2_scancodes", key2_buffer);


    sprintf (joy1_buffer, "%d %d %d %d", joy1_buttons [0], joy1_buttons [1],
        joy1_buttons [2], joy1_buttons [3]);

    sprintf (joy2_buffer, "%d %d %d %d", joy2_buttons [0], joy2_buttons [1],
        joy2_buttons [2], joy2_buttons [3]);


    set_config_string ("input", "joy1_buttons", joy1_buffer);

    set_config_string ("input", "joy2_buttons", joy2_buffer);


    set_config_int ("input", "enable_zapper", input_enable_zapper);


    set_config_int ("timing", "autosave_interval", input_autosave_interval);
}


void input_reset (void)
{
    int index, player;


    for (player = 0; player < 4; player ++)
    {
        for (index = 0; index < 8; index ++)
        {
            buttons [player] [index] = 1;
        }
    }


    last_write = 0;


    current_read_p1 = 0;

    current_read_p2 = 0;
}


UINT8 input_read (UINT16 address)               
{
    int index;


    if (! input_enable_zapper)
    {
        zapper_mask = 0x00;
    }


    switch (address)
    {
        case 0x4016:

            /* 1st and 3rd players. */

            if (current_read_p1 == 19)
            {
                /* Signature. */

                current_read_p1 ++;

                return (0x01);
            }
            else if ((current_read_p1 > 7) && (current_read_p1 < 16))
            {
                /* Player 3 button status. */

                index = (current_read_p1 - 8);


                current_read_p1 ++;

                return (buttons [INPUT_PLAYER_3] [index] | 0x40);
            }
            else if ((current_read_p1 > 15) && (current_read_p1 < 23))
            {
                /* Ignored. */

                current_read_p1 ++;

                return (0);
            }
            else if (current_read_p1 == 23)
            {
                /* Strobe flip-flop. */

                current_read_p1 = 0;

                return (0);
            }
            else
            {
                /* Player 1 button status. */

                return (buttons [INPUT_PLAYER_1] [current_read_p1 ++] | 0x40);
            }


            break;


        case 0x4017:

            /* 2nd and 4th players. */

            if (current_read_p2 == 18)
            {
                /* Signature. */

                current_read_p2 ++;

                return ((0x01 | zapper_mask));
            }
            else if ((current_read_p2 > 7) && (current_read_p2 < 16))
            {
                /* Player 4 button status. */

                index = (current_read_p2 - 8);


                current_read_p2 ++;

                return (buttons [INPUT_PLAYER_4] [index] | zapper_mask | 0x40);
            }
            else if ((current_read_p2 > 15) && (current_read_p2 < 23))
            {
                /* Ignored. */

                current_read_p2 ++;

                return (zapper_mask);
            }
            else if (current_read_p2 == 23)
            {
                /* Strobe flip-flop. */

                current_read_p2 = 0;

                return (zapper_mask);
            }
            else
            {
                /* Player 2 button status. */

                return (buttons [INPUT_PLAYER_2] [current_read_p2 ++] | zapper_mask | 0x40);
            }


            break;


        default:

            return (0);


            break;
   }
}


void input_write (UINT16 address, UINT8 value)
{
    switch (address)
    {
        case 0x4016:

             /* 1st and 3rd players. */

            if ((! (value & 0x01)) && (last_write & 0x01))
            {
                /* Full strobe. */

                current_read_p1 = 0;
            
                current_read_p2 = 0;
            }


            last_write = value;


            break;


        case 0x4017:

            /* 2nd and 4th players. */


            break;


        default:


            break;
   }
}


#define JOYSTICK_BUTTON(device, index)  \
    ((joy [device].button [index].b) ? 1 : 0)


#define JOYSTICK_LEFT(device)  \
    ((joy [device].stick [0].axis [0].d1) ? 1 : 0)

#define JOYSTICK_RIGHT(device)   \
    ((joy [device].stick [0].axis [0].d2) ? 1 : 0)


#define JOYSTICK_UP(device)  \
    ((joy [device].stick [0].axis [1].d1) ? 1 : 0)

#define JOYSTICK_DOWN(device)    \
    ((joy [device].stick [0].axis [1].d2) ? 1 : 0)


static INLINE void do_keyboard_1 (int player)
{
    int index;


    for (index = 0; index < 8; index ++)
    {
        buttons [player] [index] = (key [key1_scancodes [index]] ? 1 : 0);
    }
}


static INLINE void do_keyboard_2 (int player)
{
    int index;


    for (index = 0; index < 8; index ++)
    {
        buttons [player] [index] = (key [key2_scancodes [index]] ? 1 : 0);
    }
}


static INLINE void do_joystick_1 (int player)
{
    buttons [player] [0] = JOYSTICK_BUTTON (0, joy1_buttons [0]);

    buttons [player] [1] = JOYSTICK_BUTTON (0, joy1_buttons [1]);


    if (joy [0].num_buttons >= 4)
    {
        buttons [player] [2] = JOYSTICK_BUTTON (0, joy1_buttons [2]);

        buttons [player] [3] = JOYSTICK_BUTTON (0, joy1_buttons [3]);
    }
    else
    {
        buttons [player] [2] = (key [key1_scancodes [2]] ? 1 : 0);

        buttons [player] [3] = (key [key1_scancodes [3]] ? 1 : 0);
    }


    buttons [player] [4] = JOYSTICK_UP (0);

    buttons [player] [5] = JOYSTICK_DOWN (0);


    buttons [player] [6] = JOYSTICK_LEFT (0);

    buttons [player] [7] = JOYSTICK_RIGHT (0);
}


static INLINE void do_joystick_2 (int player)
{
    buttons [player] [0] = JOYSTICK_BUTTON (1, joy1_buttons [0]);

    buttons [player] [1] = JOYSTICK_BUTTON (1, joy1_buttons [1]);


    if (joy [0].num_buttons >= 4)
    {
        buttons [player] [2] = JOYSTICK_BUTTON (1, joy1_buttons [2]);

        buttons [player] [3] = JOYSTICK_BUTTON (1, joy1_buttons [3]);
    }
    else
    {
        buttons [player] [2] = (key [key2_scancodes [2]] ? 1 : 0);

        buttons [player] [3] = (key [key2_scancodes [3]] ? 1 : 0);
    }


    buttons [player] [4] = JOYSTICK_UP (1);

    buttons [player] [5] = JOYSTICK_DOWN (1);


    buttons [player] [6] = JOYSTICK_LEFT (1);

    buttons [player] [7] = JOYSTICK_RIGHT (1);
}


/* TODO: Move into configurable variables. */

#define MOUSE_SENSETIVITY_X   (SCREEN_W / 128)

#define MOUSE_SENSETIVITY_Y   (SCREEN_H / 128)


static INLINE void do_mouse (int player)
{
    int mickey_x;

    int mickey_y;


    buttons [player] [0] = (mouse_b & 2); /* A. */

    buttons [player] [1] = (mouse_b & 1); /* B. */


    /* TODO: Support for mice with more than 2 buttons. */

    buttons [player] [2] = (key [key1_scancodes [2]] ? 1 : 0);

    buttons [player] [3] = (key [key1_scancodes [3]] ? 1 : 0);


    get_mouse_mickeys (&mickey_x, &mickey_y);


    buttons [player] [4] = ((mickey_y < -MOUSE_SENSETIVITY_Y) ? 1 : 0);

    buttons [player] [5] = ((mickey_y > +MOUSE_SENSETIVITY_Y) ? 1 : 0);


    buttons [player] [6] = ((mickey_x < -MOUSE_SENSETIVITY_X) ? 1 : 0);

    buttons [player] [7] = ((mickey_x > +MOUSE_SENSETIVITY_X) ? 1 : 0);
}


void input_process (void)
{
   int player;
   int want_poll = TRUE;

   if (!(input_mode & INPUT_MODE_REPLAY_RECORD))
   {
      /* Timed autosave code.  Only executes when a replay isn't currently
         recording, but is allowed to execute while a replay is playing. */

      if (input_autosave_interval > 0)
      {
         static int frames = 0;

         if (++frames == (input_autosave_interval * timing_get_speed ()))
         {
            /* Set trigger flag. */
            input_autosave_triggered = TRUE;
     
            /* Simulate keypress. */
            gui_handle_keypress (0, KEY_F3);

            /* Clear trigger flag. */
            input_autosave_triggered = FALSE;
     
            /* Reset frame counter. */
            frames = 0;
         }
      }
   }

   if (input_mode & INPUT_MODE_REPLAY_PLAY)
   {
       /* Replay playback code.  Reads data from the replay file and feeds
          it directly into the player button states. */

      for (player = 0; player < 4; player++)
      {
         BOOL eof;
         UINT8 data;
         int button;

         eof = get_replay_data (&data);

         for (button = 0; button < 8; button++)
             buttons[player][button] = ((data & (1 << button)) ? 1 : 0);

         if (eof)
         {
            /* End of replay reached. */
            gui_stop_replay ();
            return;
         }
      }
   }

   if (!(input_mode & INPUT_MODE_PLAY))
   {
      /* The remaining code should only execute in the normal gameplay
         mode - bail out. */
      return;
   }

   if (wait_frames > 0)
      wait_frames--;
   if (wait_frames > 0)
      return;

   for (player = 0; player < 4; player++)
   {
      switch (input_devices[player])
      {
         case INPUT_DEVICE_KEYBOARD_1:
         {
            do_keyboard_1 (player);

            break;
         }
 
         case INPUT_DEVICE_KEYBOARD_2:
         {
            do_keyboard_2 (player);
 
            break;
         }
 
         case INPUT_DEVICE_JOYSTICK_1:
         {
            if (want_poll)
            {
               /* Some joystick devices require polling. */
               poll_joystick ();

               want_poll = FALSE;
            }
 
            do_joystick_1 (player);
 
            break;
         }
 
         case INPUT_DEVICE_JOYSTICK_2:
         {
            if (want_poll)
            {
               poll_joystick ();
 
               want_poll = FALSE;
            }
 
            do_joystick_2 (player);

            break;
         }

         case INPUT_DEVICE_MOUSE:
         {
            poll_mouse ();
 
            do_mouse (player);
 
            break;
         }
      }
 
      if (buttons[player][4] && buttons[player][5])
      {
         /* Prevent up and down from being pressed at the same time */
          buttons[player][4] =
          buttons[player][5] = 0;
      }
 
      if (buttons[player][6] && buttons[player][7])
      {
         /* Prevent left and right from being pressed at the same time */
         buttons[player][6] =
         buttons[player][7] = 0;
      }
 
      if (input_mode & INPUT_MODE_REPLAY_RECORD)
      {
         /* Send player button states to the replay file. */
         UINT8 data = 0;
         int button;
 
         for (button = 0; button < 8; button++)
         {
            if (buttons[player][button])
               data |= (1 << button);
         }
 
         save_replay_data (data);
      }
   }
}


void input_handle_keypress (int c, int scancode)
{
   if (!(input_mode & INPUT_MODE_CHAT))
      return;

   /* TODO: Make these Unicode calls protect against buffer overflow. */

   switch (scancode)
   {
      case KEY_BACKSPACE:
      {
         if (ustrlen (input_chat_text) > 0)
         {
            /* Remove the last character from the buffer. */
            uremove (input_chat_text, (ustrlen (input_chat_text) - 1));
         }

         break;
      }

      case KEY_ENTER:
      {
         if (ustrlen (input_chat_text) > 0)
         {
            video_message (input_chat_text);
            video_message_duration = 5000;

            /* Clear buffer. */
            USTRING_CLEAR(input_chat_text);
         }

         input_mode &= ~INPUT_MODE_CHAT;

         if (!(input_mode & INPUT_MODE_REPLAY_PLAY))
            input_mode |= INPUT_MODE_PLAY;

         wait_frames = (timing_get_speed () / 2);

         return;
      }

      default:
      {
         /* Not sure if this is correct, but it's better than a crash! */

         if (ustrsizez (input_chat_text) < ((sizeof (input_chat_text) -
            uwidth_max (U_CURRENT)) - 1))
         {
            /* Add character to the end of the buffer. */
            uinsert (input_chat_text, ustrlen (input_chat_text), c);
         }

         break;
      }
   }
}


int input_get_player_device (int player)
{
    return (input_devices [player]);
}


void input_set_player_device (int player, int device)
{
    input_devices [player] = device;
}


void input_map_device_button (int device, int button, int value)
{
    switch (device)
    {
        case INPUT_DEVICE_KEYBOARD_1:

            key1_scancodes [button] = value;


            break;


        case INPUT_DEVICE_KEYBOARD_2:

            key2_scancodes [button] = value;


            break;


        case INPUT_DEVICE_JOYSTICK_1:

            joy1_buttons [button] = value;


            break;


        case INPUT_DEVICE_JOYSTICK_2:

            joy2_buttons [button] = value;


            break;


        default:

            break;
    }
}


void input_save_state (PACKFILE * file, int version)
{
    pack_putc (last_write, file);


    pack_putc (current_read_p1, file);

    pack_putc (current_read_p2, file);
}


void input_load_state (PACKFILE * file, int version)
{
    last_write = pack_getc (file);


    current_read_p1 = pack_getc (file);

    current_read_p2 = pack_getc (file);
}
