

/*

FakeNES - A portable, open-source NES emulator.

video.c: Implementation of the video interface.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include "cpu.h"

#include "input.h"

#include "ppu.h"

#include "rom.h"

#include "video.h"


#include "data.h"

#include "misc.h"


#include "timing.h"


int video_display_status = FALSE;

int video_enable_vsync = FALSE;


static int screen_height = 0;

static int screen_width = 0;


static int force_window = FALSE;


static int scaled_mode = FALSE;


static int scale_width = 0;

static int scale_height = 0;


static int zoom_factor = 0;


static int blit_x_offset = 0;

static int blit_y_offset = 0;


#define VIDEO_COLOR_WHITE   palette_color [33]


int video_init (void)
{
    int driver;


    screen_width = get_config_int ("video", "screen_width", 320);

    screen_height = get_config_int ("video", "screen_height", 240);


    force_window = get_config_int ("video", "force_window", FALSE);


    scaled_mode = get_config_int ("video", "scaled_mode", FALSE);


    scale_width = get_config_int ("video", "scale_width", 512);

    scale_height = get_config_int ("video", "scale_height", 480);


    zoom_factor = get_config_int ("video", "zoom_factor", 256);


    video_display_status = get_config_int ("video", "display_status", FALSE);

    video_enable_vsync = get_config_int ("video", "enable_vsync", FALSE);


    driver = (force_window ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT);


    set_color_depth (8);

    if (set_gfx_mode (driver, screen_width, screen_height, 0, 0) != 0)
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


    base_video_buffer = create_bitmap (((8 + 256) + 8), ((16 + 240) + 16));

    video_buffer = create_sub_bitmap (base_video_buffer, 8, 16, 256, 240);


    clear (base_video_buffer);


    set_mouse_sprite (DATA_ARROW_SPRITE);


    show_mouse (screen);

    scare_mouse ();


    set_palette (DATA_DEFAULT_PALETTE);


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


    set_config_int ("video", "zoom_factor", zoom_factor);


    set_config_int ("video", "display_status", video_display_status);

    set_config_int ("video", "enable_vsync", video_enable_vsync);
}


void video_blit (void)
{
    if ((video_display_status) && (rom_is_loaded))
    {
        textout (video_buffer, font,
            "Video:", 16, 120, VIDEO_COLOR_WHITE);

        textout (video_buffer, font,
            "Audio:", 16, 152, VIDEO_COLOR_WHITE);


        textout (video_buffer, font,
            "Core:", 16, 184, VIDEO_COLOR_WHITE);


        textprintf (video_buffer, font, 20, 136,
            VIDEO_COLOR_WHITE, "%02d FPS", timing_fps);

        textprintf (video_buffer, font, 20, 168,
            VIDEO_COLOR_WHITE, "%02d FPS", timing_audio_fps);


        textprintf (video_buffer, font, 20, 200,
            VIDEO_COLOR_WHITE, "%02d Hz", timing_hertz);

        textprintf (video_buffer, font, 20, 216,
            VIDEO_COLOR_WHITE, "PC: $%04X", * cpu_active_pc);
    }


    if ((input_enable_zapper) && (rom_is_loaded))
    {
        if ((mouse_x < 256) && (mouse_y < 240))
        {
            masked_blit (DATA_GUN_SPRITE, base_video_buffer,
                0, 0, (mouse_x + 1), (mouse_y + 9), 16, 16);
        }
    }


    acquire_screen ();


    if (video_enable_vsync)
    {
        vsync ();
    }


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


    release_screen ();
}


void video_zoom (int factor)
{
    if (scaled_mode)
    {
        scale_width += factor;

        scale_height += factor;
    }
    else
    {
        scaled_mode = TRUE;


        scale_width = (256 + factor);

        scale_height = (240 + factor);
    }


    if ((scale_width != 256) && (scale_height != 240))
    {
        blit_x_offset = ((SCREEN_W / 2) - (scale_width / 2));
    
        blit_y_offset = ((SCREEN_H / 2) - (scale_height / 2));
    }
    else
    {
        scaled_mode = FALSE;


        blit_x_offset = ((SCREEN_W / 2) - (256 / 2));
    
        blit_y_offset = ((SCREEN_H / 2) - (240 / 2));
    }
}


void video_zoom_in (void)
{
    video_zoom (+zoom_factor);
}


void video_zoom_out (void)
{
    video_zoom (-zoom_factor);


    vsync ();

    clear (screen);
}
