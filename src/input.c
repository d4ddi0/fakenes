

/*

FakeNES - A portable, Open Source NES emulator.

input.c: Implementation of the input emulation.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "gui.h"

#include "input.h"

#include "ppu.h"

#include "rom.h"

#include "video.h"


#include "misc.h"


#include "timing.h"


int input_enable_zapper = FALSE;


static int buttons [4] [8];


enum
{
    INPUT_PLAYER_1, INPUT_PLAYER_2,

    INPUT_PLAYER_3, INPUT_PLAYER_4
};


static UINT8 key1_defaults [25] = { "24 26 64 67 84 85 82 83\0" };

static UINT8 key2_defaults [25] = { "38 40 44 46 45 39 41 43\0" };


static UINT8 key1_buffer [50];
                
static UINT8 key2_buffer [50];


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


enum
{
    INPUT_DEVICE_NONE,


    INPUT_DEVICE_KEYBOARD_1, INPUT_DEVICE_KEYBOARD_2,

    INPUT_DEVICE_JOYSTICK_1, INPUT_DEVICE_JOYSTICK_2
};


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
    memset (key1_buffer, NULL, sizeof (key1_buffer));

    memset (key2_buffer, NULL, sizeof (key2_buffer));


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
    memset (joy1_buffer, NULL, sizeof (joy1_buffer));

    memset (joy2_buffer, NULL, sizeof (joy2_buffer));


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


int input_init (void)
{
    install_keyboard ();

    install_mouse ();


    install_joystick (JOY_TYPE_AUTODETECT);


    input_devices [0] = get_config_int ("input", "player_1_device", INPUT_DEVICE_KEYBOARD_1);

    input_devices [1] = get_config_int ("input", "player_2_device", INPUT_DEVICE_KEYBOARD_2);


    input_devices [2] = get_config_int ("input", "player_3_device", INPUT_DEVICE_NONE);

    input_devices [3] = get_config_int ("input", "player_4_device", INPUT_DEVICE_NONE);


    input_enable_zapper = get_config_int ("input", "enable_zapper", FALSE);


    load_keyboard_layouts ();

    load_joystick_layouts ();


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


    set_config_string ("input", "key1_scancodes", key1_buffer);

    set_config_string ("input", "key2_scancodes", key2_buffer);


    set_config_string ("input", "joy1_buttons", joy1_buffer);

    set_config_string ("input", "joy2_buttons", joy2_buffer);


    set_config_int ("input", "enable_zapper", input_enable_zapper);
}


void input_reset (void)
{
    int index, player;


    for (player = 0; player < 4; player ++)
    {
        for (index = 0; index < 8; index ++)
        {
            buttons [player] [index] = 0x01;
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

                return (buttons [INPUT_PLAYER_3] [index]);
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

                return (buttons [INPUT_PLAYER_1] [current_read_p1 ++]);
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

                return ((buttons [INPUT_PLAYER_4] [index] | zapper_mask));
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

                return ((buttons [INPUT_PLAYER_2] [current_read_p2 ++] | zapper_mask));
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
    ((joy [device].button [index].b) ? 0x41 : 0)


#define JOYSTICK_LEFT(device)  \
    ((joy [device].stick [0].axis [0].d1) ? 0x41 : 0)

#define JOYSTICK_RIGHT(device)   \
    ((joy [device].stick [0].axis [0].d2) ? 0x41 : 0)


#define JOYSTICK_UP(device)  \
    ((joy [device].stick [0].axis [1].d1) ? 0x41 : 0)

#define JOYSTICK_DOWN(device)    \
    ((joy [device].stick [0].axis [1].d2) ? 0x41 : 0)


static INLINE void do_keyboard_1 (int player)
{
    int index;


    for (index = 0; index < 8; index ++)
    {
        buttons [player] [index] = (key [key1_scancodes [index]] ? 0x41 : 0);
    }
}


static INLINE void do_keyboard_2 (int player)
{
    int index;


    for (index = 0; index < 8; index ++)
    {
        buttons [player] [index] = (key [key2_scancodes [index]] ? 0x41 : 0);
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
        buttons [player] [2] = (key [key1_scancodes [2]] ? 0x41 : 0);

        buttons [player] [3] = (key [key1_scancodes [3]] ? 0x41 : 0);
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
        buttons [player] [2] = (key [key2_scancodes [2]] ? 0x41 : 0);

        buttons [player] [3] = (key [key2_scancodes [3]] ? 0x41 : 0);
    }


    buttons [player] [4] = JOYSTICK_UP (1);

    buttons [player] [5] = JOYSTICK_DOWN (1);


    buttons [player] [6] = JOYSTICK_LEFT (1);

    buttons [player] [7] = JOYSTICK_RIGHT (1);
}


int input_process (void)
{
    int player;


    int want_exit = FALSE;

    int want_poll = TRUE;


    while (keypressed ())
    {
        int index;


        index = readkey ();


        switch ((index >> 8))
        {
            case KEY_ESC:

                suspend_timing ();


                show:

                  want_exit = show_gui (FALSE);


                if (gui_needs_restart)
                {
                    /* Ugh. */

                    goto show;
                }


                resume_timing ();


                break;


            case KEY_EQUALS:

                if (frame_skip_max < 60)
                {
                    frame_skip_max ++;
                }


                break;


           case KEY_MINUS:

                if (frame_skip_max > 1)
                {
                    frame_skip_max --;
                }


                break;


            case KEY_F5:

                ppu_invert_mirroring ();


                break;


            case KEY_F6:

                input_enable_zapper = (! input_enable_zapper);


                break;


            default:

                break;
        }


        video_handle_keypress (index);


        gui_handle_keypress (index);
    }


    for (player = 0; player < 4; player ++)
    {
        switch (input_devices [player])
        {
            case INPUT_DEVICE_KEYBOARD_1:

                do_keyboard_1 (player);


                break;


            case INPUT_DEVICE_KEYBOARD_2:

                do_keyboard_2 (player);


                break;


            case INPUT_DEVICE_JOYSTICK_1:

                if (want_poll)
                {
                    poll_joystick ();


                    want_poll = FALSE;
                }


                do_joystick_1 (player);


                break;


            case INPUT_DEVICE_JOYSTICK_2:

                if (want_poll)
                {
                    poll_joystick ();


                    want_poll = FALSE;
                }


                do_joystick_2 (player);


                break;
        }
    }


    return (want_exit);
}


void input_save_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    pack_putc (last_write, file_chunk);


    pack_putc (current_read_p1, file_chunk);

    pack_putc (current_read_p2, file_chunk);


    pack_fclose_chunk (file_chunk);
}


void input_load_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    last_write = pack_getc (file_chunk);


    current_read_p1 = pack_getc (file_chunk);

    current_read_p2 = pack_getc (file_chunk);


    pack_fclose_chunk (file_chunk);
}
