

/*

FakeNES - A portable, open-source NES emulator.

video.c: Implementation of the video interface.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include "cpu.h"

#include "video.h"


#include "data.h"

#include "misc.h"


#include "timing.h"


int video_status_display = FALSE;


static int screen_height = 0;

static int screen_width = 0;


static int force_window = FALSE;


static int scaled_mode = FALSE;


static int scale_width = 0;

static int scale_height = 0;


static int blit_x_offset = 0;

static int blit_y_offset = 0;


int video_init (void)
{
    int driver;


    set_color_depth (8);


    screen_width =
        get_config_int ("video", "screen_width", 256);

    screen_height =
        get_config_int ("video", "screen_height", 240);


    force_window =
        get_config_int ("video", "force_window", FALSE);


    scaled_mode =
        get_config_int ("video", "scaled_mode", FALSE);


    scale_width =
        get_config_int ("video", "scale_width", 512);

    scale_height =
        get_config_int ("video", "scale_height", 480);


    driver = (force_window ?
        GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT);


    if (set_gfx_mode (driver,
        screen_width, screen_height, 0, 0) != 0)
    {
        return (1);
    }


    if (scaled_mode)
    {
        blit_x_offset = ((SCREEN_W / 2) - (scale_width / 2));
    
        blit_y_offset = ((SCREEN_H / 2) - (scale_height / 2));
    }
    else
    {
        blit_x_offset = ((SCREEN_W / 2) - (256 / 2));
    
        blit_y_offset = ((SCREEN_H / 2) - (240 / 2));
    }


    base_video_buffer =
        create_bitmap (((8 + 256) + 8), ((16 + 240) + 16));

    video_buffer =
        create_sub_bitmap (base_video_buffer, 8, 16, 256, 240);


    clear (base_video_buffer);


    set_mouse_sprite (DATA_ARROW_SPRITE);


    show_mouse (screen);

    scare_mouse ();


    set_palette (DATA_NES_PALETTE);


    text_mode (-1);

    font = DATA_SMALL_FONT;


    set_display_switch_mode (SWITCH_PAUSE);


    return (0);
}


void video_exit (void)
{
    set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);


    destroy_bitmap (video_buffer);

    destroy_bitmap (base_video_buffer);


    set_config_int ("video", "screen_width", screen_width);

    set_config_int ("video", "screen_height", screen_height);


    set_config_int ("video", "force_window", force_window);


    set_config_int ("video", "scaled_mode", scaled_mode);


    set_config_int ("video", "scale_width", scale_width);

    set_config_int ("video", "scale_height", scale_height);
}


void video_blit (void)
{
    if (video_status_display)
    {
        textprintf (video_buffer,
            font, 16, 216, 33, "PC: $%04X", cpu_get_pc ());


        textprintf (video_buffer,
            font, 16, 184, 33, "%02d FPS", timing_fps);
    
        textprintf (video_buffer,
            font, 16, 200, 33, "%02d Hz", timing_hertz);
    }


    acquire_bitmap (screen);


    if (scaled_mode)
    {
        stretch_blit (video_buffer, screen, 0, 0, 256, 240,
            blit_x_offset, blit_y_offset, scale_width, scale_height);
    }
    else
    {
        blit (video_buffer, screen,
            0, 0, blit_x_offset, blit_y_offset, 256, 240);
    }


    release_bitmap (screen);
}


void video_clear (void)
{
    clear (base_video_buffer);
}
