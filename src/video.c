  

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

#include "gui.h"

#include "input.h"

#include "ppu.h"

#include "rom.h"

#include "video.h"


#include "data.h"

#include "misc.h"


#include "timing.h"


static BITMAP * screen_buffer = NULL;

static BITMAP * overlay_buffer = NULL;


int video_display_status = FALSE;

int video_enable_vsync = FALSE;


static int screen_height = 0;

static int screen_width = 0;


static int force_window = FALSE;


static int stretch_width = 0;

static int stretch_height = 0;


static int zoom_factor_x = 0;

static int zoom_factor_y = 0;


static int blit_x_offset = 0;

static int blit_y_offset = 0;


static int light_level = 0;


static int blitter_type = 0;


#define VIDEO_COLOR_BLACK   palette_color [0]

#define VIDEO_COLOR_WHITE   palette_color [33]


static int preserve_video_buffer = FALSE;


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


    screen_buffer = create_bitmap (SCREEN_W, SCREEN_H);

    clear (screen_buffer);


    overlay_buffer = create_bitmap (256, 240);


    /* Heh, recursive variable assignation. :( */

    video_set_blitter (blitter_type);


    if (! preserve_video_buffer)
    {
        base_video_buffer = create_bitmap (((8 + 256) + 8), ((16 + 240) + 16));
    
        video_buffer = create_sub_bitmap (base_video_buffer, 8, 16, 256, 240);
    

        clear (base_video_buffer);
    }

    
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


    destroy_bitmap (screen_buffer);

    destroy_bitmap (overlay_buffer);


    if (! preserve_video_buffer)
    {
        destroy_bitmap (video_buffer);
    
        destroy_bitmap (base_video_buffer);
    }


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


/* Todo: Find a better way to do this. */

static INLINE void display_status (BITMAP * bitmap, int color)
{
    textout (bitmap, font, "Video:", 16, (bitmap -> h - 114), color);

    textout (bitmap, font, "Audio:", 16, (bitmap -> h - 82), color);


    textout (bitmap, font, "Core:", 16, (bitmap -> h - 50), color);


    textprintf (bitmap, font, 20, (bitmap -> h - 100), color, "%02d FPS", timing_fps);

    textprintf (bitmap, font, 20, (bitmap -> h - 68), color, "%02d FPS", timing_audio_fps);


    textprintf (bitmap, font, 20, (bitmap -> h - 36), color, "%02d Hz", timing_hertz);

    textprintf (bitmap, font, 20, (bitmap -> h - 22), color, "PC: $%04X", * cpu_active_pc);
}


static INLINE void blit_2xsoe (BITMAP * source, BITMAP * target, int x, int y)
{
    int x_base;

    int y_base;


    int x_offset;

    int y_offset;


    int center_pixel;


    int north_pixel;

    int south_pixel;


    int east_pixel;

    int west_pixel;


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = getpixel (source, x_offset, y_offset);


            north_pixel = getpixel (source, x_offset, (y_offset - 1));

            east_pixel = getpixel (source, (x_offset - 1), y_offset);


            south_pixel = getpixel (source, x_offset, (y_offset + 1));

            west_pixel = getpixel (source, (x_offset + 1), y_offset);


            if (north_pixel != west_pixel)
            {
                putpixel (target, (x_base + 1), y_base, center_pixel);
            }
            else
            {
                putpixel (target, (x_base + 1), y_base, west_pixel);
            }    


            if (north_pixel != east_pixel)
            {
                putpixel (target, x_base, y_base, center_pixel);
            }
            else
            {
                putpixel (target, x_base, y_base, east_pixel);
            }    


            if (south_pixel != west_pixel)
            {
                putpixel (target, (x_base + 1), (y_base + 1), center_pixel);
            }
            else
            {
                putpixel (target, (x_base + 1), (y_base + 1), west_pixel);
            }    


            if (south_pixel != east_pixel)
            {
                putpixel (target, x_base, (y_base + 1), center_pixel);
            }
            else
            {
                putpixel (target, x_base, (y_base + 1), east_pixel);
            }    
        }
    }
}


void video_blit (BITMAP * bitmap)
{
    BITMAP * source_buffer;


    if ((input_enable_zapper) && (! gui_is_active))
    {
        if (! (mouse_x < 256))
        {
            position_mouse (255, mouse_y);
        }


        if (! (mouse_y < 239))
        {
            position_mouse (mouse_x, 239);
        }


        blit (video_buffer, overlay_buffer, 0, 0, 0, 0, 256, 240);

        masked_blit (DATA_GUN_SPRITE, overlay_buffer, 0, 0, (mouse_x - 7), (mouse_y - 7), 16, 16);


        source_buffer = overlay_buffer;
    }
    else
    {
        source_buffer = video_buffer;
    }


    switch (blitter_type)
    {
        case VIDEO_BLITTER_NORMAL:

            blit (source_buffer, screen_buffer, 0, 0, blit_x_offset, blit_y_offset, 256, 240);


            break;


        case VIDEO_BLITTER_STRETCHED:

            stretch_blit (source_buffer, screen_buffer, 0, 0, 256, 240,
                blit_x_offset, blit_y_offset, stretch_width, stretch_height);


            break;


        case VIDEO_BLITTER_2XSOE:

            blit_2xsoe (source_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;
    }


    if ((video_display_status) && (! gui_is_active))
    {
        display_status (screen_buffer, VIDEO_COLOR_WHITE);
    }


    acquire_bitmap (bitmap);

    blit (screen_buffer, bitmap, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    release_bitmap (bitmap);


    clear (screen_buffer);
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
    if (blitter_type == VIDEO_BLITTER_2XSOE)
    {
        return;
    }


    if (! (blitter_type == VIDEO_BLITTER_STRETCHED))
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

            blit_x_offset = ((SCREEN_W / 2) - (256 / 2));
        
            blit_y_offset = ((SCREEN_H / 2) - (240 / 2));


            break;


        case VIDEO_BLITTER_STRETCHED:

            blit_x_offset = ((SCREEN_W / 2) - (stretch_width / 2));
        
            blit_y_offset = ((SCREEN_H / 2) - (stretch_height / 2));


            break;


        case VIDEO_BLITTER_2XSOE:

            blit_x_offset = ((SCREEN_W / 2) - 256);

            blit_y_offset = ((SCREEN_H / 2) - 240);


            break;


        default:

            break;
    }
}


int video_get_blitter (void)
{
    return (blitter_type);
}


void video_set_resolution (int width, int height)
{
    int old_width;

    int old_height;


    if ((width == SCREEN_W) && (height == SCREEN_H))
    {
        return;
    }


    old_width = screen_width;

    old_height = screen_height;


    screen_width = width;

    screen_height = height;


    preserve_video_buffer = TRUE;


    video_exit ();


    if (video_init != 0)
    {
        screen_width = old_width;

        screen_height = old_height;


        video_init ();
    }


    set_palette (internal_palette);


    preserve_video_buffer = FALSE;


    if (gui_is_active)
    {
        unscare_mouse ();
    }
}
