  

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


#include "audio.h"

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

static BITMAP * status_buffer = NULL;


static BITMAP * mouse_sprite_remove_buffer = NULL;


static BITMAP * stretch_buffer = NULL;


int video_display_status = FALSE;

int video_enable_vsync = FALSE;


int video_force_window = FALSE;


static int screen_height = 0;

static int screen_width = 0;


static int color_depth = 0;


static int stretch_width = 0;

static int stretch_height = 0;


static int first_blit_line = 0;

static int last_blit_line = 0;


static int zoom_factor_x = 0;

static int zoom_factor_y = 0;


static int blit_x_offset = 0;

static int blit_y_offset = 0;


static int light_adjustment = 0;


static int blitter_type = 0;

static int filter_list = 0;


#define VIDEO_COLOR_WHITE   palette_color [33]

#define VIDEO_COLOR_RED     palette_color [6]


static int preserve_video_buffer = FALSE;

static int preserve_palette = FALSE;


static PALETTE internal_palette;


int video_init (void)
{
    int driver;


    screen_width = get_config_int ("video", "screen_width", 320);

    screen_height = get_config_int ("video", "screen_height", 240);


    color_depth = get_config_int ("video", "color_depth", 8);


    video_force_window = get_config_int ("video", "force_window", FALSE);


    blitter_type = get_config_int ("video", "blitter_type", VIDEO_BLITTER_NORMAL);

    filter_list = get_config_int ("video", "filter_list", 0);


    stretch_width = get_config_int ("video", "stretch_width", 512);

    stretch_height = get_config_int ("video", "stretch_height", 480);


    first_blit_line = get_config_int ("video", "first_blit_line", 0);

    last_blit_line = get_config_int ("video", "last_blit_line", 239);


    zoom_factor_x = get_config_int ("video", "zoom_factor_x", 256);

    zoom_factor_y = get_config_int ("video", "zoom_factor_y", 240);


    light_adjustment = get_config_int ("video", "light_adjustment", 0);


    video_display_status = get_config_int ("video", "display_status", FALSE);

    video_enable_vsync = get_config_int ("video", "enable_vsync", FALSE);


    driver = (video_force_window ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT);


    if ((color_depth != 8) && (color_depth != 15) && (color_depth != 16))
    {
        return (1);
    }


    set_color_depth (color_depth);

    if (set_gfx_mode (driver, screen_width, screen_height, 0, 0) != 0)
    {
        return (2);
    }


    if (color_depth != 8)
    {
        set_color_conversion (COLORCONV_TOTAL);
    }


    screen_buffer = create_bitmap (SCREEN_W, SCREEN_H);

    status_buffer = create_sub_bitmap (screen_buffer, 0, (SCREEN_H - 128), 72, 128);

    clear (screen_buffer);


    mouse_sprite_remove_buffer = create_bitmap_ex (8, 16, 16);



    /* Heh, recursive variable assignation. :( */

    video_set_blitter (blitter_type);

    video_set_filter_list (filter_list);


    if (! preserve_video_buffer)
    {
        base_video_buffer = create_bitmap_ex (8, ((8 + 256) + 8), ((16 + 240) + 16));
    
        video_buffer = create_sub_bitmap (base_video_buffer, 8, 16, 256, 240);
    

        clear (base_video_buffer);
    }


    set_mouse_sprite (DATA_ARROW_SPRITE);


    show_mouse (screen);

    scare_mouse ();


    if (preserve_palette)
    {
        set_palette (internal_palette);
    }
    else
    {
        video_set_palette (DATA_DEFAULT_PALETTE);
    }


    text_mode (-1);

    font = DATA_SMALL_FONT;


    set_display_switch_mode (SWITCH_PAUSE);


    return (0);
}


int video_reinit (void)
{
    int result;


    preserve_video_buffer = TRUE;

    preserve_palette = TRUE;


    video_exit ();


    result = video_init ();

    if (result == 0)
    {
        preserve_video_buffer = FALSE;
    
        preserve_palette = FALSE;
    
    
        if (gui_is_active)
        {
            unscare_mouse ();
        }
    }


    return (result);
}


void video_exit (void)
{
    set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);


    destroy_bitmap (status_buffer);

    destroy_bitmap (screen_buffer);


    destroy_bitmap (mouse_sprite_remove_buffer);


    if (stretch_buffer)
    {
        destroy_bitmap (stretch_buffer);


        stretch_buffer = NULL;
    }


    if (! preserve_video_buffer)
    {
        destroy_bitmap (video_buffer);
    
        destroy_bitmap (base_video_buffer);
    }


    set_config_int ("video", "screen_width", screen_width);

    set_config_int ("video", "screen_height", screen_height);


    set_config_int ("video", "color_depth", color_depth);


    set_config_int ("video", "force_window", video_force_window);


    set_config_int ("video", "blitter_type", blitter_type);


    set_config_int ("video", "filter_list", filter_list);


    set_config_int ("video", "stretch_width", stretch_width);

    set_config_int ("video", "stretch_height", stretch_height);


    set_config_int ("video", "first_blit_line", first_blit_line);

    set_config_int ("video", "last_blit_line", last_blit_line);


    set_config_int ("video", "zoom_factor_x", zoom_factor_x);

    set_config_int ("video", "zoom_factor_y", zoom_factor_y);


    set_config_int ("video", "light_adjustment", light_adjustment);


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


    if (audio_enable_output)
    {
        textprintf (bitmap, font, 20, (bitmap -> h - 68), color, "%02d FPS", timing_audio_fps);
    }
    else
    {
        textout (bitmap, font, "Disabled", 20, (bitmap -> h - 68), color);
    }


    textprintf (bitmap, font, 20, (bitmap -> h - 36), color, "%02d Hz", timing_hertz);

    textprintf (bitmap, font, 20, (bitmap -> h - 22), color, "PC: $%04X", * cpu_active_pc);
}


#define FAST_GETPIXEL(bitmap, x, y)         bitmap -> line [y] [x]

#define FAST_PUTPIXEL(bitmap, x, y, color)  (bitmap -> line [y] [x] = color)


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


    if ((target -> w < (source -> w * 2)) || (target -> h < (source -> h * 2)))
    {
        /* Center error message on target. */

        y += ((source -> h / 2) - (((text_height (font) * 2) + (text_height (font) / 2)) / 2));


        textout (target, font, "Target dimensions are not large enough.", x, y, VIDEO_COLOR_RED);


        textprintf (target, font, x, ((y + text_height (font)) + (text_height (font) /  2)),
            VIDEO_COLOR_RED, "At least %dx%d pixels are required.", (source -> w * 2), (source -> h * 2));


        return;
    }


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        /* source grid: A B C  output grid: E1 E2
         *              D E F               E3 E4
         *              G H I
         */

        /* Handle first pixel on line. */

        x_offset = 0;

        x_base = x;


        center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


        /* A,D,G = invalid, E1,E3 = E */
        FAST_PUTPIXEL (target, x_base, y_base, center_pixel);

        FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);


        /* if C,F,I == invalid, E2,E4 = E */
        if (source -> w < 2)
        {
            FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);

            FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);


            continue;
        }


        east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


        /* if B = invalid, E2 = E */
        /* else E2 = B == F ? F : E; */
        if (y_offset > 0)
        {
            north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


            if (north_pixel != east_pixel)
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, east_pixel);
            }    
        }
        else
        {
            FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
        }


        /* if H = invalid, E4 = E */
        /* else E4 = F == H ? F : E; */
        if ((y_offset + 1) < source -> h)
        {
            south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


            if (south_pixel != east_pixel)
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), east_pixel);
            }    
        }
        else
        {
            FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
        }


        for (x_offset = 1; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            west_pixel = center_pixel;

            center_pixel = east_pixel;


            /* D = valid */
            /* if B = invalid, E1 = E */
            /* else E1 = B == D ? D : E; */
            if (y_offset > 0)
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


                if (north_pixel != west_pixel)
                {
                    FAST_PUTPIXEL (target, x_base, y_base, center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, x_base, y_base, west_pixel);
                }    
            }
            else
            {
                north_pixel = -1;


                FAST_PUTPIXEL (target, x_base, y_base, center_pixel);
            }


            /* if H = invalid, E1 = E */
            /* else E1 = D == H ? D : E; */
            if ((y_offset + 1) < source -> h)
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


                if (south_pixel != west_pixel)
                {
                    FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, x_base, (y_base + 1), west_pixel);
                }    
            }
            else
            {
                south_pixel = -1;


                FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);
            }


            /* if F = invalid, E2,E4 = E */
            /* else */
            /* if B = invalid, E2 = E */
            /* else E2 = B == F ? F : E; */
            /* if H = invalid, E4 = E */
            /* else E4 = F == H ? F : E; */
            if ((x_offset + 1) < source -> w)
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


                if ((north_pixel < 0) || (north_pixel != east_pixel))
                {
                    FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, (x_base + 1), y_base, east_pixel);
                }    


                if ((south_pixel < 0) || (south_pixel != east_pixel))
                {
                    FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), east_pixel);
                }    
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);

                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
            }
        }
    }
}


static INLINE void blit_2xscl (BITMAP * source, BITMAP * target, int x, int y)
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


    if ((target -> w < (source -> w * 2)) || (target -> h < (source -> h * 2)))
    {
        /* Center error message on target. */

        y += ((source -> h / 2) - (((text_height (font) * 2) + (text_height (font) / 2)) / 2));


        textout (target, font, "Target dimensions are not large enough.", x, y, VIDEO_COLOR_RED);


        textprintf (target, font, x, ((y + text_height (font)) + (text_height (font) /  2)),
            VIDEO_COLOR_RED, "At least %dx%d pixels are required.", (source -> w * 2), (source -> h * 2));


        return;
    }


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);
        

            if (x_base == 0)
            {
                west_pixel = center_pixel;
            }
            else
            {
                west_pixel = FAST_GETPIXEL (source, (x_offset - 1), y_offset);
            }
        

            if ((x_offset + 1) >= source -> w)
            {
                east_pixel = center_pixel;
            }
            else
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);
            }
	

            if ((y_offset + 1) >= source -> h)
            {
                south_pixel = center_pixel;
            }
            else
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));
            }
	

            if (y_base == 0)
            {
                north_pixel = center_pixel;
            }
            else
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));
            }


            if ((west_pixel == north_pixel) && (north_pixel != east_pixel) && (west_pixel != south_pixel))
            {
                FAST_PUTPIXEL (target, x_base, y_base, west_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, x_base, y_base, center_pixel);
            }


            if ((north_pixel == east_pixel) && (north_pixel != west_pixel) && (east_pixel != south_pixel))
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, east_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
            }


            if ((west_pixel == south_pixel) && (west_pixel != north_pixel) && (south_pixel != east_pixel))
            {
                FAST_PUTPIXEL (target, x_base, (y_base + 1), west_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);
            }


            if ((south_pixel == east_pixel) && (west_pixel != south_pixel) && (north_pixel != east_pixel))
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), east_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
            }
        }
    }
}


#define FAST_PUTPIXEL16(bitmap, x, y, color)    (((UINT16 *) bitmap -> line [y]) [x] = color)


static INLINE int mix (int color_a, int color_b)
{
    int r;

    int g;

    int b;


    /* 0 - 63 --> 0 - 127. */

    r = (internal_palette [color_a].r * 2);

    g = (internal_palette [color_a].g * 2);

    b = (internal_palette [color_a].b * 2);


    r += (internal_palette [color_b].r * 2);

    g += (internal_palette [color_b].g * 2);

    b += (internal_palette [color_b].b * 2);


    return (makecol (r, g, b));
}


static INLINE void blit_super_2xscl (BITMAP * source, BITMAP * target, int x, int y)
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


    if ((target -> w < (source -> w * 2)) || (target -> h < (source -> h * 2)))
    {
        /* Center error message on target. */

        y += ((source -> h / 2) - (((text_height (font) * 2) + (text_height (font) / 2)) / 2));


        textout (target, font, "Target dimensions are not large enough.", x, y, VIDEO_COLOR_RED);


        textprintf (target, font, x, ((y + text_height (font)) + (text_height (font) /  2)),
            VIDEO_COLOR_RED, "At least %dx%d pixels are required.", (source -> w * 2), (source -> h * 2));


        return;
    }


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


            if (x_base == 0)
            {
                west_pixel = center_pixel;
            }
            else
            {
                west_pixel = FAST_GETPIXEL (source, (x_offset - 1), y_offset);
            }
        

            if ((x_offset + 1) >= source -> w)
            {
                east_pixel = center_pixel;
            }
            else
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);
            }
	

            if ((y_offset + 1) >= source -> h)
            {
                south_pixel = center_pixel;
            }
            else
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));
            }
	

            if (y_base == 0)
            {
                north_pixel = center_pixel;
            }
            else
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));
            }


            if ((west_pixel == north_pixel) && (north_pixel != east_pixel) && (west_pixel != south_pixel))
            {
                FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, x_base, y_base, mix (center_pixel, west_pixel));
            }


            if ((north_pixel == east_pixel) && (north_pixel != west_pixel) && (east_pixel != south_pixel))
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }


            if ((west_pixel == south_pixel) && (west_pixel != north_pixel) && (south_pixel != east_pixel))
            {
                FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
            }


            if ((south_pixel == east_pixel) && (west_pixel != south_pixel) && (north_pixel != east_pixel))
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }
        }
    }
}


void video_blit (BITMAP * bitmap)
{
    BITMAP * source_buffer;


    if (! rom_is_loaded)
    {
        return;
    }


    if ((input_enable_zapper) && (! gui_is_active))
    {
        blit (video_buffer, mouse_sprite_remove_buffer, (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 0, 0, 16, 16);

        masked_blit (DATA_GUN_SPRITE, video_buffer, 0, 0, (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 16, 16);
    }


    switch (blitter_type)
    {
        case VIDEO_BLITTER_NORMAL:

            blit (video_buffer, screen_buffer, 0, first_blit_line, blit_x_offset, blit_y_offset, 256, (last_blit_line + 1));


            break;


        case VIDEO_BLITTER_STRETCHED:

            /* See comment in video_set_blitter(). */

            if (color_depth != 8)
            {
                blit (video_buffer, stretch_buffer, 0, 0, 0, 0, 256, 240);

                stretch_blit (stretch_buffer, screen_buffer, 0, first_blit_line, 256, (last_blit_line + 1),
                    blit_x_offset, blit_y_offset, stretch_width, stretch_height);
            }
            else
            {
                stretch_blit (video_buffer, screen_buffer, 0, first_blit_line, 256, (last_blit_line + 1),
                    blit_x_offset, blit_y_offset, stretch_width, stretch_height);
            }


            break;


        case VIDEO_BLITTER_2XSOE:

            blit_2xsoe (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_2XSCL:

            blit_2xscl (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_SUPER_2XSCL:

            blit_super_2xscl (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;
    }


    if ((input_enable_zapper) && (! gui_is_active))
    {
        blit (mouse_sprite_remove_buffer, video_buffer, 0, 0, (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 16, 16);
    }


    video_filter ();


    if ((video_display_status) && (! gui_is_active))
    {
        display_status (status_buffer, VIDEO_COLOR_WHITE);
    }


    acquire_bitmap (bitmap);

    blit (screen_buffer, bitmap, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    release_bitmap (bitmap);


    clear (status_buffer);

    /* clear (screen_buffer); */
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
    if ((blitter_type == VIDEO_BLITTER_2XSOE) ||
        (blitter_type == VIDEO_BLITTER_2XSCL))
    {
        return;
    }


    if (! (blitter_type == VIDEO_BLITTER_STRETCHED))
    {
        stretch_width = 256;

        stretch_height = 240;
    }


    stretch_width = fix ((stretch_width + x_factor), 256, (256 * 12));

    stretch_height = fix ((stretch_height + y_factor), 240, (240 * 12));


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


#define NES_PALETTE_START       1

#define NES_PALETTE_END         (NES_PALETTE_START + 64)


#define GUI_PALETTE_START       128

#define GUI_PALETTE_END         (GUI_PALETTE_START + 64)


void video_set_palette (RGB * palette)
{
    int index;


    memcpy (internal_palette, palette, sizeof (internal_palette));


    for (index = NES_PALETTE_START; index < NES_PALETTE_END; index ++)
    {
        internal_palette [index].r = fix ((internal_palette [index].r + light_adjustment), 0, 63);

        internal_palette [index].g = fix ((internal_palette [index].g + light_adjustment), 0, 63);

        internal_palette [index].b = fix ((internal_palette [index].b + light_adjustment), 0, 63);
    }


    for (index = GUI_PALETTE_START; index < (GUI_PALETTE_START + GUI_PALETTE_END); index ++)
    {
        internal_palette [index].r = fix (((index - GUI_PALETTE_START) + light_adjustment), 0, 63);

        internal_palette [index].g = fix (((index - GUI_PALETTE_START) + light_adjustment), 0, 63);

        internal_palette [index].b = fix (((index - GUI_PALETTE_START) + light_adjustment), 0, 63);
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


            /* Yuck, no color conversion in stretch_blit! */

            if (color_depth != 8)
            {
                if (stretch_buffer)
                {
                    destroy_bitmap (stretch_buffer);
                }


                stretch_buffer = create_bitmap (256, 240);
            }


            break;


        case VIDEO_BLITTER_2XSOE:

        case VIDEO_BLITTER_2XSCL:

        case VIDEO_BLITTER_SUPER_2XSCL:

            if (! ((SCREEN_W < 512) || (SCREEN_H < 480)))
            {
                blit_x_offset = ((SCREEN_W / 2) - 256);
    
                blit_y_offset = ((SCREEN_H / 2) - 240);
            }
            else
            {
                blit_x_offset = ((SCREEN_W / 2) - (256 / 2));
            
                blit_y_offset = ((SCREEN_H / 2) - (240 / 2));
            }


            break;


        default:

            break;
    }


    clear (screen_buffer);
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

    preserve_palette = TRUE;


    video_exit ();


    if (video_init () != 0)
    {
        set_config_int ("video", "screen_width", old_width);

        set_config_int ("video", "screen_height", old_height);


        video_init ();
    }


    preserve_video_buffer = FALSE;

    preserve_palette = FALSE;


    if (gui_is_active)
    {
        unscare_mouse ();
    }
}


int video_get_color_depth (void)
{
    return (color_depth);
}


void video_set_color_depth (int depth)
{
    int old_depth;


    if (color_depth == depth)
    {
        return;
    }


    old_depth = color_depth;


    color_depth = depth;


    preserve_video_buffer = TRUE;

    preserve_palette = TRUE;


    video_exit ();


    if (video_init () != 0)
    {
        set_config_int ("video", "color_depth", old_depth);


        video_init ();
    }


    preserve_video_buffer = FALSE;

    preserve_palette = FALSE;


    if (gui_is_active)
    {
        unscare_mouse ();
    }
}


void video_set_filter_list (int filters)
{
    filter_list = filters;


    clear (screen_buffer);
}


int video_get_filter_list (void)
{
    return (filter_list);
}


void video_filter (void)
{
    int y;


    if (filter_list & VIDEO_FILTER_SCANLINES_HIGH)
    {
        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, 0, y, screen_buffer -> w, 0);
        }
    }
    else if (filter_list & VIDEO_FILTER_SCANLINES_MEDIUM)
    {
        set_trans_blender (0, 0, 0, 127);

        drawing_mode (DRAW_MODE_TRANS, NULL, 0, 0);


        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, 0, y, screen_buffer -> w, 0);
        }


        solid_mode ();
    }
    else if (filter_list & VIDEO_FILTER_SCANLINES_LOW)
    {
        set_trans_blender (0, 0, 0, 63);

        drawing_mode (DRAW_MODE_TRANS, NULL, 0, 0);


        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, 0, y, screen_buffer -> w, 0);
        }


        solid_mode ();
    }
}
