  

/*

FakeNES - A portable, Open Source NES emulator.

video.c: Implementation of the video interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <string.h>


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


static int stretched_mode = FALSE;


static int stretch_width = 0;

static int stretch_height = 0;


static int zoom_factor_x = 0;

static int zoom_factor_y = 0;


static int blit_x_offset = 0;

static int blit_y_offset = 0;


static int light_level = 0;


static int blitter_type = 0;


#define VIDEO_COLOR_WHITE   palette_color [33]


int video_init (void)
{
    int driver;


    screen_width = get_config_int ("video", "screen_width", 320);

    screen_height = get_config_int ("video", "screen_height", 240);


    force_window = get_config_int ("video", "force_window", FALSE);


    blitter_type = get_config_int ("video", "blitter_type", VIDEO_BLITTER_NORMAL);


    stretch_width = get_config_int ("video", "stretch_width", 512);

    stretch_height = get_config_int ("video", "stretch_height", 480);


    zoom_factor_x = get_config_int ("video", "zoom_factor_x", 256);

    zoom_factor_y = get_config_int ("video", "zoom_factor_y", 240);


    light_level = get_config_int ("video", "light_level", 0);


    video_display_status = get_config_int ("video", "display_status", FALSE);

    video_enable_vsync = get_config_int ("video", "enable_vsync", FALSE);


    driver = (force_window ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT);


    set_color_depth (8);

    if (set_gfx_mode (driver, screen_width, screen_height, 0, 0) != 0)
    {
        return (1);
    }


    /* Heh, recursive variable assignation. :( */

    video_set_blitter (blitter_type);


    base_video_buffer = create_bitmap (((8 + 256) + 8), ((16 + 240) + 16));

    video_buffer = create_sub_bitmap (base_video_buffer, 8, 16, 256, 240);


    clear (base_video_buffer);


    set_mouse_sprite (DATA_ARROW_SPRITE);


    show_mouse (screen);

    scare_mouse ();


    video_set_palette (DATA_DEFAULT_PALETTE);


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


    set_config_int ("video", "blitter_type", blitter_type);


    set_config_int ("video", "stretch_width", stretch_width);

    set_config_int ("video", "stretch_height", stretch_height);


    set_config_int ("video", "zoom_factor_x", zoom_factor_x);

    set_config_int ("video", "zoom_factor_y", zoom_factor_y);


    set_config_int ("video", "light_level", light_level);


    set_config_int ("video", "display_status", video_display_status);

    set_config_int ("video", "enable_vsync", video_enable_vsync);
}


void video_blit (void)
{
    if ((video_display_status) && (rom_is_loaded))
    {
        textout (video_buffer, font, "Video:", 16, 120, VIDEO_COLOR_WHITE);

        textout (video_buffer, font, "Audio:", 16, 152, VIDEO_COLOR_WHITE);


        textout (video_buffer, font, "Core:", 16, 184, VIDEO_COLOR_WHITE);


        textprintf (video_buffer, font, 20, 136, VIDEO_COLOR_WHITE, "%02d FPS", timing_fps);

        textprintf (video_buffer, font, 20, 168, VIDEO_COLOR_WHITE, "%02d FPS", timing_audio_fps);


        textprintf (video_buffer, font, 20, 200, VIDEO_COLOR_WHITE, "%02d Hz", timing_hertz);

        textprintf (video_buffer, font, 20, 216, VIDEO_COLOR_WHITE, "PC: $%04X", * cpu_active_pc);
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


    if (stretched_mode)
    {
        stretch_blit (video_buffer, screen, 0, 0, 256, 240,
            blit_x_offset, blit_y_offset, stretch_width, stretch_height);
    }
    else
    {
        blit (video_buffer, screen,
            0, 0, blit_x_offset, blit_y_offset, 256, 240);
    }


    release_screen ();
}


static INLINE int fix (int value, int base, int limit)
{
    if (value < base)
    {
        value = base;
    }


    if (value > limit)
    {
        value = limit;
    }


    return (value);
}


void video_zoom (int x_factor, int y_factor)
{
    if (! stretched_mode)
    {
        stretch_width = 256;

        stretch_height = 240;
    }


    stretch_width = fix ((stretch_width + x_factor), 256, (256 * 8));

    stretch_height = fix ((stretch_height + y_factor), 240, (240 * 8));


    if ((stretch_width == 256) && (stretch_height == 240))
    {
        video_set_blitter (VIDEO_BLITTER_NORMAL);
    }
    else
    {
        video_set_blitter (VIDEO_BLITTER_STRETCHED);
    }
}


void video_zoom_in (void)
{
    video_zoom (zoom_factor_x, zoom_factor_y);
}


void video_zoom_out (void)
{
    video_zoom (-zoom_factor_x, -zoom_factor_y);


    /* Nasty hack. */

    vsync ();

    clear (screen);
}


static PALETTE internal_palette;


void video_set_palette (RGB * palette)
{
    int index;


    memcpy (internal_palette, palette, sizeof (internal_palette));


    for (index = 1; index < 65; index ++)
    {
        internal_palette [index].r = fix ((internal_palette [index].r + light_level), 0, 63);

        internal_palette [index].g = fix ((internal_palette [index].g + light_level), 0, 63);

        internal_palette [index].b = fix ((internal_palette [index].b + light_level), 0, 63);
    }


    set_palette (internal_palette);
}


void video_set_blitter (int blitter)
{
    blitter_type = blitter;


    switch (blitter)
    {
        case VIDEO_BLITTER_NORMAL:

            stretched_mode = FALSE;


            blit_x_offset = ((SCREEN_W / 2) - (256 / 2));
        
            blit_y_offset = ((SCREEN_H / 2) - (240 / 2));


            break;


        case VIDEO_BLITTER_STRETCHED:

            stretched_mode = TRUE;


            blit_x_offset = ((SCREEN_W / 2) - (stretch_width / 2));
        
            blit_y_offset = ((SCREEN_H / 2) - (stretch_height / 2));


            break;


        default:

            break;
    }
}


int video_get_blitter (void)
{
    return (blitter_type);
}
