  

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

video.c: Implementation of the video interface.

Copyright (c) 2003, Randy McDowell.
Copyright (c) 2003, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <stdlib.h>

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


static BITMAP * screen_buffer = NIL;

static BITMAP * status_buffer = NIL;


static BITMAP * mouse_sprite_remove_buffer = NIL;


static BITMAP * stretch_buffer = NIL;


UINT8 video_messages [MAX_MESSAGES] [(MAX_MESSAGE_LENGTH + 1)];


volatile int video_message_duration = 0;


static void video_message_timer (void)
{
    if (video_message_duration > 0)
    {
        video_message_duration -= 1000;
    }
}

END_OF_STATIC_FUNCTION (video_message_timer);


int video_display_status = FALSE;

int video_enable_vsync = FALSE;


int video_force_window = FALSE;


int video_driver = GFX_AUTODETECT;


static int screen_height = 0;

static int screen_width = 0;


static int color_depth = 0;


static int stretch_width = 0;

static int stretch_height = 0;


static int first_blit_line = 0;

static int last_blit_line = 0;


static int image_offset_x = 0;

static int image_offset_y = 0;


static int zoom_factor_x = 0;

static int zoom_factor_y = 0;


static int blit_x_offset = 0;

static int blit_y_offset = 0;


static int light_adjustment = 0;


static int blitter_type = 0;

static int filter_list = 0;


static int selected_blitter = 0;


#define VIDEO_COLOR_BLACK   palette_color [0]

#define VIDEO_COLOR_WHITE   palette_color [33]


static int preserve_video_buffer = FALSE;

static int preserve_palette = FALSE;


static PALETTE internal_palette;


static RGB * last_palette = NIL;


int video_init (void)
{
    int driver;


    LOCK_VARIABLE (video_message_duration);


    LOCK_FUNCTION (video_message_timer);


    install_int_ex (video_message_timer, BPS_TO_TIMER (1));


    screen_width = get_config_int ("video", "screen_width", 320);

    screen_height = get_config_int ("video", "screen_height", 240);


    color_depth = get_config_int ("video", "color_depth", 8);


    video_force_window = get_config_int ("video", "force_window", FALSE);


    blitter_type = get_config_int ("video", "blitter_type", VIDEO_BLITTER_AUTOMATIC);

    filter_list = get_config_int ("video", "filter_list", 0);


    stretch_width = get_config_int ("video", "stretch_width", 512);

    stretch_height = get_config_int ("video", "stretch_height", 480);


    first_blit_line = get_config_int ("video", "first_blit_line", 0);

    last_blit_line = get_config_int ("video", "last_blit_line", 239);

    if (last_blit_line < first_blit_line) last_blit_line = first_blit_line;


    image_offset_x = get_config_int ("video", "image_offset_x", 0);

    image_offset_y = get_config_int ("video", "image_offset_y", 0);


    zoom_factor_x = get_config_int ("video", "zoom_factor_x", 256);

    zoom_factor_y = get_config_int ("video", "zoom_factor_y", 240);


    light_adjustment = get_config_int ("video", "light_adjustment", 0);


    video_display_status = get_config_int ("video", "display_status", FALSE);

    video_enable_vsync = get_config_int ("video", "enable_vsync", FALSE);


    if (video_driver == GFX_AUTODETECT)
    {
        driver = (video_force_window ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT);
    }
    else
    {
        driver = video_driver;
    }


    if ((color_depth != 8) && (color_depth != 15) && (color_depth != 16) && (color_depth != 32))
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


    set_mouse_sprite (DATA_TO_BITMAP (ARROW_SPRITE));


    show_mouse (screen);

    scare_mouse ();


    if (preserve_palette)
    {
        set_palette (internal_palette);
    }
    else
    {
        video_set_palette (DATA_TO_RGB (DEFAULT_PALETTE));
    }


    font = DATA_TO_FONT (SMALL_FONT);


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
    remove_int (video_message_timer);


    set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);


    destroy_bitmap (status_buffer);

    destroy_bitmap (screen_buffer);


    destroy_bitmap (mouse_sprite_remove_buffer);


    if (stretch_buffer)
    {
        destroy_bitmap (stretch_buffer);


        stretch_buffer = NIL;
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


    set_config_int ("video", "image_offset_x", image_offset_x);

    set_config_int ("video", "image_offset_y", image_offset_y);


    set_config_int ("video", "zoom_factor_x", zoom_factor_x);

    set_config_int ("video", "zoom_factor_y", zoom_factor_y);


    set_config_int ("video", "light_adjustment", light_adjustment);


    set_config_int ("video", "display_status", video_display_status);

    set_config_int ("video", "enable_vsync", video_enable_vsync);
}


static INLINE void shadow_textout (BITMAP * bitmap, FONT * font, const UINT8 * text, int x, int y, int color)
{
    /* This is a pain to do for printf, so we just do that manually. */

    textout_ex (bitmap, font, text, (x + 1), (y + 1), VIDEO_COLOR_BLACK, -1);

    textout_ex (bitmap, font, text, x, y, color, -1);
}


/* Todo: Find a better way to do all this. */

static INLINE void display_status (BITMAP * bitmap, int color)
{
    shadow_textout (bitmap, font, "Video:", 16, (bitmap -> h - 114), color);

    shadow_textout (bitmap, font, "Audio:", 16, (bitmap -> h - 82), color);


    shadow_textout (bitmap, font, "Core:", 16, (bitmap -> h - 50), color);


    textprintf_ex (bitmap, font, (20 + 1), ((bitmap -> h - 100) + 1), VIDEO_COLOR_BLACK, -1, "%02d FPS", timing_fps);

    textprintf_ex (bitmap, font, 20, (bitmap -> h - 100), color, -1, "%02d FPS", timing_fps);


    if (audio_enable_output)
    {
        textprintf_ex (bitmap, font, (20 + 1), ((bitmap -> h - 68) + 1), VIDEO_COLOR_BLACK, -1, "%02d FPS", timing_audio_fps);

        textprintf_ex (bitmap, font, 20, (bitmap -> h - 68), color, -1, "%02d FPS", timing_audio_fps);
    }
    else
    {
        shadow_textout (bitmap, font, "Disabled", 20, (bitmap -> h - 68), color);
    }


    textprintf_ex (bitmap, font, (20 + 1), ((bitmap -> h - 36) + 1), VIDEO_COLOR_BLACK, -1, "%02d Hz", timing_hertz);

    textprintf_ex (bitmap, font, 20, (bitmap -> h - 36), color, -1, "%02d Hz", timing_hertz);


    textprintf_ex (bitmap, font, (20 + 1), ((bitmap -> h - 22) + 1), VIDEO_COLOR_BLACK, -1, "PC: $%04X", * cpu_active_pc);

    textprintf_ex (bitmap, font, 20, (bitmap -> h - 22), color, -1, "PC: $%04X", * cpu_active_pc);
}


#define FAST_GETPIXEL(bitmap, x, y)         bitmap -> line [y] [x]

#define FAST_PUTPIXEL(bitmap, x, y, color)  (bitmap -> line [y] [x] = color)


#define BLITTER_SIZE_CHECK()                                                                                      \
    if ((target -> w < (source -> w * 2)) || (target -> h < (source -> h * 2)))                                   \
    {                                                                                                             \
        /* Center error message on target. */                                                                     \
                                                                                                                  \
        y += ((source -> h / 2) - (((text_height (font) * 2) + (text_height (font) / 2)) / 2));                   \
                                                                                                                  \
                                                                                                                  \
        textout_ex (target, font, "Target dimensions are not large enough.", x, y, VIDEO_COLOR_WHITE, -1);        \
                                                                                                                  \
                                                                                                                  \
        textprintf_ex (target, font, x, ((y + text_height (font)) + (text_height (font) /  2)),                   \
            VIDEO_COLOR_WHITE, -1, "At least %dx%d pixels are required.", (source -> w * 2), (source -> h * 2));  \
                                                                                                                  \
                                                                                                                  \
        return;                                                                                                   \
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


    BLITTER_SIZE_CHECK ();


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


    BLITTER_SIZE_CHECK ();


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);
        

            if (x_offset == 0)
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
	

            if (y_offset == 0)
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


#define FAST_PUTPIXEL32(bitmap, x, y, color)    (((UINT32 *) bitmap -> line [y]) [x] = color)


static INLINE int mix (int color_a, int color_b)
{
    int r;

    int g;

    int b;


    /* 0 - 63 --> 0 - 127. */

    r = ((internal_palette [color_a].r * 3) + internal_palette [color_b].r);

    g = ((internal_palette [color_a].g * 3) + internal_palette [color_b].g);

    b = ((internal_palette [color_a].b * 3) + internal_palette [color_b].b);


    if (color_depth == 32)
    {
        return (makecol32 (r, g, b));
    }
    else if (color_depth == 16)
    {
        return (makecol16 (r, g, b));
    }
    else if (color_depth == 15)
    {
        return (makecol15 (r, g, b));
    }
    else
    {
        return (makecol (r, g, b));
    }
}


static INLINE void blit_interpolated (BITMAP * source, BITMAP * target, int x, int y)
{
    int x_base;

    int y_base;


    int x_offset;

    int y_offset;


    int center_pixel;


    int east_pixel;

    int south_pixel;


    int south_east_pixel;


    BLITTER_SIZE_CHECK ();


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


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


            if (((x_offset + 1) >= source -> w) || ((y_offset + 1) >= source -> h))
            {
                south_east_pixel = center_pixel;
            }
            else
            {
                south_east_pixel = FAST_GETPIXEL (source, (x_offset + 1), (y_offset + 1));
            }


            FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [center_pixel]);


            FAST_PUTPIXEL16 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));

            FAST_PUTPIXEL16 (target, x_base, (y_base + 1), mix (center_pixel, south_pixel));


            FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), mix (center_pixel, south_east_pixel));
        }
    }
}


static INLINE void blit_interpolated_32 (BITMAP * source, BITMAP * target, int x, int y)
{
    int x_base;

    int y_base;


    int x_offset;

    int y_offset;


    int center_pixel;


    int east_pixel;

    int south_pixel;


    int south_east_pixel;


    BLITTER_SIZE_CHECK ();


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


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


            if (((x_offset + 1) >= source -> w) || ((y_offset + 1) >= source -> h))
            {
                south_east_pixel = center_pixel;
            }
            else
            {
                south_east_pixel = FAST_GETPIXEL (source, (x_offset + 1), (y_offset + 1));
            }


            FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [center_pixel]);


            FAST_PUTPIXEL32 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));

            FAST_PUTPIXEL32 (target, x_base, (y_base + 1), mix (center_pixel, south_pixel));


            FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), mix (center_pixel, south_east_pixel));
        }
    }
}


static INLINE void blit_super_2xsoe (BITMAP * source, BITMAP * target, int x, int y)
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


    BLITTER_SIZE_CHECK ();


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        x_offset = 0;

        x_base = x;


        center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


        FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [center_pixel]);

        FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [center_pixel]);


        if (source -> w < 2)
        {
            FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [center_pixel]);

            FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);


            continue;
        }


        east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


        if (y_offset > 0)
        {
            north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


            if (north_pixel != east_pixel)
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [center_pixel]);
        }


        if ((y_offset + 1) < source -> h)
        {
            south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


            if (south_pixel != east_pixel)
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
        }


        for (x_offset = 1; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            west_pixel = center_pixel;

            center_pixel = east_pixel;


            if (y_offset > 0)
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


                if (north_pixel != west_pixel)
                {
                    FAST_PUTPIXEL16 (target, x_base, y_base, mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [west_pixel]);
                }    
            }
            else
            {
                north_pixel = -1;


                FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [center_pixel]);
            }


            if ((y_offset + 1) < source -> h)
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


                if (south_pixel != west_pixel)
                {
                    FAST_PUTPIXEL16 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [west_pixel]);
                }    
            }
            else
            {
                south_pixel = -1;


                FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [center_pixel]);
            }


            if ((x_offset + 1) < source -> w)
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


                if ((north_pixel < 0) || (north_pixel != east_pixel))
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [east_pixel]);
                }    


                if ((south_pixel < 0) || (south_pixel != east_pixel))
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
                }    
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [center_pixel]);

                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
            }
        }
    }
}


static INLINE void blit_super_2xsoe_32 (BITMAP * source, BITMAP * target, int x, int y)
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


    BLITTER_SIZE_CHECK ();


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        x_offset = 0;

        x_base = x;


        center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


        FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [center_pixel]);

        FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [center_pixel]);


        if (source -> w < 2)
        {
            FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [center_pixel]);

            FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);


            continue;
        }


        east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


        if (y_offset > 0)
        {
            north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


            if (north_pixel != east_pixel)
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [center_pixel]);
        }


        if ((y_offset + 1) < source -> h)
        {
            south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


            if (south_pixel != east_pixel)
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
        }


        for (x_offset = 1; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            west_pixel = center_pixel;

            center_pixel = east_pixel;


            if (y_offset > 0)
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


                if (north_pixel != west_pixel)
                {
                    FAST_PUTPIXEL32 (target, x_base, y_base, mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [west_pixel]);
                }    
            }
            else
            {
                north_pixel = -1;


                FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [center_pixel]);
            }


            if ((y_offset + 1) < source -> h)
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


                if (south_pixel != west_pixel)
                {
                    FAST_PUTPIXEL32 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [west_pixel]);
                }    
            }
            else
            {
                south_pixel = -1;


                FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [center_pixel]);
            }


            if ((x_offset + 1) < source -> w)
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


                if ((north_pixel < 0) || (north_pixel != east_pixel))
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [east_pixel]);
                }    


                if ((south_pixel < 0) || (south_pixel != east_pixel))
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
                }    
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [center_pixel]);

                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
            }
        }
    }
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


    BLITTER_SIZE_CHECK ();


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


            if (x_offset == 0)
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
	

            if (y_offset == 0)
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


static INLINE void blit_super_2xscl_32 (BITMAP * source, BITMAP * target, int x, int y)
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


    BLITTER_SIZE_CHECK ();


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
                FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, x_base, y_base, mix (center_pixel, west_pixel));
            }


            if ((north_pixel == east_pixel) && (north_pixel != west_pixel) && (east_pixel != south_pixel))
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }


            if ((west_pixel == south_pixel) && (west_pixel != north_pixel) && (south_pixel != east_pixel))
            {
                FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
            }


            if ((south_pixel == east_pixel) && (west_pixel != south_pixel) && (north_pixel != east_pixel))
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }                
        }
    }
}


static INLINE int select_blitter (void)
{
    if ((SCREEN_W >= 512) && (SCREEN_H >= 480))
    {
        if (color_depth != 8)
        {
            return (VIDEO_BLITTER_SUPER_2XSCL);
        }
        else
        {
            return (VIDEO_BLITTER_2XSCL);
        }
    }
    else
    {
        return (VIDEO_BLITTER_NORMAL);
    }
}


static void draw_messages (void);

static void erase_messages (void);


static int flash_tick = 0;


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

        masked_blit (DATA_TO_BITMAP (GUN_SPRITE), video_buffer, 0, 0, (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 16, 16);
    }


    if (blitter_type == VIDEO_BLITTER_AUTOMATIC)
    {
        if (! (selected_blitter > 0))
        {
            /* Force resync. */

            video_set_blitter (blitter_type);
        }


        blitter_type = selected_blitter;
    }


    switch (blitter_type)
    {
        case VIDEO_BLITTER_NORMAL:

            blit (video_buffer, screen_buffer, 0, first_blit_line, blit_x_offset, blit_y_offset, 256, (last_blit_line - first_blit_line + 1));


            break;


        case VIDEO_BLITTER_STRETCHED:

            /* See comment in video_set_blitter(). */

            if (color_depth != 8)
            {
                blit (video_buffer, stretch_buffer, 0, 0, 0, 0, 256, 240);

                stretch_blit (stretch_buffer, screen_buffer, 0, first_blit_line, 256, (last_blit_line - first_blit_line + 1),
                    blit_x_offset, blit_y_offset, stretch_width, stretch_height);
            }
            else
            {
                stretch_blit (video_buffer, screen_buffer, 0, first_blit_line, 256, (last_blit_line - first_blit_line + 1),
                    blit_x_offset, blit_y_offset, stretch_width, stretch_height);
            }


            break;


        case VIDEO_BLITTER_INTERPOLATED:

            if (color_depth == 32)
            {
                blit_interpolated_32 (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);
            }
            else
            {
                blit_interpolated (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);
            }


            break;


        case VIDEO_BLITTER_2XSOE:

            blit_2xsoe (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_2XSCL:

            blit_2xscl (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_SUPER_2XSOE:

            if (color_depth == 32)
            {
                blit_super_2xsoe_32 (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);
            }
            else
            {
                blit_super_2xsoe (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);
            }


            break;


        case VIDEO_BLITTER_SUPER_2XSCL:

            if (color_depth == 32)
            {
                blit_super_2xscl_32 (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);
            }
            else
            {
                blit_super_2xscl (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);
            }


            break;
    }


    if (selected_blitter > 0)
    {
        blitter_type = VIDEO_BLITTER_AUTOMATIC;
    }


    if ((input_enable_zapper) && (! gui_is_active))
    {
        blit (mouse_sprite_remove_buffer, video_buffer, 0, 0, (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 16, 16);
    }


    video_filter ();


    if ((video_display_status) && (! gui_is_active))
    {
        display_status (screen_buffer, VIDEO_COLOR_WHITE);
    }


    if (((video_message_duration > 0) || (input_mode & INPUT_MODE_CHAT)) && (! gui_is_active))
    {
        draw_messages ();
    }


    acquire_bitmap (bitmap);

    blit (screen_buffer, bitmap, 0, 0, image_offset_x, image_offset_y, SCREEN_W, SCREEN_H);

    release_bitmap (bitmap);


    if (((video_message_duration > 0) || (input_mode & INPUT_MODE_CHAT)) && (! gui_is_active))
    {
        erase_messages ();
    }


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
    if (((blitter_type == VIDEO_BLITTER_2XSOE) || (blitter_type == VIDEO_BLITTER_SUPER_2XSOE)) ||
        ((blitter_type == VIDEO_BLITTER_2XSCL) || (blitter_type == VIDEO_BLITTER_SUPER_2XSCL)) ||
         (blitter_type == VIDEO_BLITTER_INTERPOLATED))
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


void video_handle_keypress (int index)
{
    if (! (input_mode & INPUT_MODE_CHAT))
    {
        switch ((index >> 8))
        {
            case KEY_EQUALS:
    
                video_zoom (zoom_factor_x, zoom_factor_y);
    
    
                break;
    
    
            case KEY_MINUS:
    
                video_zoom (-zoom_factor_x, -zoom_factor_y);
    
    
                break;


            default:

                break;
        }
    }


    switch ((index >> 8))
    {
        case KEY_F10:

            if (light_adjustment > -63)
            {
                light_adjustment --;
            }


            video_set_palette (last_palette);


            break;


        case KEY_F11:

            if (light_adjustment < 63)
            {
                light_adjustment ++;
            }


            video_set_palette (last_palette);


            break;


        default:

            break;
    }
}


#define NES_PALETTE_START       1

#define NES_PALETTE_END         (NES_PALETTE_START + 64)


#define GUI_PALETTE_START       128

#define GUI_PALETTE_END         (GUI_PALETTE_START + 64)


static COLOR_MAP half_transparency_map;


void video_set_palette (RGB * palette)
{
    int index;


    last_palette = palette;


    memcpy (internal_palette, palette, sizeof (internal_palette));


    for (index = NES_PALETTE_START; index < NES_PALETTE_END; index ++)
    {
        internal_palette [index].r = fix ((internal_palette [index].r + light_adjustment), 0, 63);

        internal_palette [index].g = fix ((internal_palette [index].g + light_adjustment), 0, 63);

        internal_palette [index].b = fix ((internal_palette [index].b + light_adjustment), 0, 63);
    }


    for (index = GUI_PALETTE_START; index < GUI_PALETTE_END; index ++)
    {
        internal_palette [index].r = (index - GUI_PALETTE_START);

        internal_palette [index].g = (index - GUI_PALETTE_START);

        internal_palette [index].b = (index - GUI_PALETTE_START);
    }


    set_palette (internal_palette);


    set_trans_blender (0, 0, 0, 127);


    create_blender_table (&half_transparency_map, internal_palette, NIL);


    color_map = &half_transparency_map;
}


void video_set_blitter (int blitter)
{
    blitter_type = blitter;


    if (blitter == VIDEO_BLITTER_AUTOMATIC)
    {
        selected_blitter = select_blitter ();


        blitter = selected_blitter;
    }
    else
    {
        /* Clear automatic blitter. */

        selected_blitter = 0;
    }


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


        case VIDEO_BLITTER_INTERPOLATED:

        case VIDEO_BLITTER_2XSOE:

        case VIDEO_BLITTER_2XSCL:

        case VIDEO_BLITTER_SUPER_2XSOE:

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


    /* Sync. */

    if (selected_blitter > 0)
    {
        selected_blitter = 0;
    }


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


    /* Sync. */

    if (selected_blitter > 0)
    {
        selected_blitter = 0;
    }


    if (gui_is_active)
    {
        unscare_mouse ();
    }
}


void video_set_driver (int driver)
{
    int old_driver;


    if (gfx_driver -> id == driver)
    {
        return;
    }


    old_driver = gfx_driver -> id;


    video_driver = driver;


    preserve_video_buffer = TRUE;

    preserve_palette = TRUE;


    video_exit ();


    if (video_init () != 0)
    {
        video_driver = old_driver;


        video_init ();
    }


    preserve_video_buffer = FALSE;

    preserve_palette = FALSE;


    /* Sync. */

    if (selected_blitter > 0)
    {
        selected_blitter = 0;
    }


    if (gui_is_active)
    {
        unscare_mouse ();
    }
}


void video_set_filter_list (int filters)
{
    filter_list = filters;


    if (filter_list & VIDEO_FILTER_SCANLINES_MEDIUM)
    {
        set_trans_blender (0, 0, 0, 127);
    }
    else if (filter_list & VIDEO_FILTER_SCANLINES_LOW)
    {
        set_trans_blender (0, 0, 0, 63);
    }


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
            hline (screen_buffer, blit_x_offset, y, screen_buffer -> w, 0);
        }
    }
    else if (filter_list & VIDEO_FILTER_SCANLINES_MEDIUM)
    {
        drawing_mode (DRAW_MODE_TRANS, NIL, 0, 0);


        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, blit_x_offset, y, screen_buffer -> w, 0);
        }


        solid_mode ();
    }
    else if (filter_list & VIDEO_FILTER_SCANLINES_LOW)
    {
        drawing_mode (DRAW_MODE_TRANS, NIL, 0, 0);


        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, blit_x_offset, y, screen_buffer -> w, 0);
        }


        solid_mode ();
    }
}


void video_message (const UINT8 * message, ...)
{
    va_list format;


    int index;


    UINT8 buffer [256];


    va_start (format, message);

    vsprintf (buffer, message, format);

    va_end (format);


    for (index = 0; index < (MAX_MESSAGES - 1); index ++)
    {
        strncpy (&video_messages [index] [0], &video_messages [(index + 1)] [0], MAX_MESSAGE_LENGTH);
    }


    strncpy (&video_messages [(MAX_MESSAGES - 1)] [0], buffer, MAX_MESSAGE_LENGTH);


    if (video_messages [(MAX_MESSAGES - 1)] [(MAX_MESSAGE_LENGTH - 1)])
    {
        video_messages [(MAX_MESSAGES - 1)] [(MAX_MESSAGE_LENGTH - 1)] = NIL;
    }


    if (log_file)
    {
        fprintf (log_file, "%s\n", buffer);
    }
}


static INLINE int get_messages_height (void)
{
    int index;


    int height = 0;


    for (index = 0; index < MAX_MESSAGES; index ++)
    {
        int length;


        length = text_length (font, &video_messages [index] [0]);


        if (length > (SCREEN_W - 8))
        {
            height += ((text_height (font) + 1) * 2);
        }
        else
        {
            height += (text_height (font) + 1);
        }
    }


    return (height);
}


static void draw_messages (void)
{
    int index;


    int x;

    int y;


    int x2;

    int y2;


    int height;


    int height_text;


    int gray;

    int silver;


    height = get_messages_height ();


    height_text = text_height (font);


    gray = makecol (127, 127, 127);

    silver = makecol (191, 191, 191);


    x = 0;

    y = ((SCREEN_H - (((height_text + 6) + height) + 3)) - 1);


    x2 = (SCREEN_W - 1);

    y2 = (SCREEN_H - 1);


    drawing_mode (DRAW_MODE_TRANS, NIL, 0, 0);

    rectfill (screen_buffer, x, y, x2, y2, gray);


    solid_mode ();

    rect (screen_buffer, x, y, x2, y2, VIDEO_COLOR_WHITE);


    x = 3;

    y ++;


    x2 = ((SCREEN_W - 1) - 1);


    hline (screen_buffer, x, y, x2, VIDEO_COLOR_BLACK);


    x = 0;

    y = ((SCREEN_H - (height_text + 5)) - 1);


    hline (screen_buffer, x, y, x2, VIDEO_COLOR_WHITE);


    x = 3;

    y ++;


    hline (screen_buffer, x, y, x2, VIDEO_COLOR_BLACK);


    x = 4;

    y = ((SCREEN_H - ((height_text + 6) + height)) - 1);


    x2 = ((SCREEN_W - 4) - 1);


    for (index = 0; index < MAX_MESSAGES; index ++)
    {
        UINT8 * token;


        UINT8 buffer [(MAX_MESSAGE_LENGTH + 1)];


        memcpy (buffer, &video_messages [index] [0], MAX_MESSAGE_LENGTH);


        for (token = strtok (&video_messages [index] [0], " "); token; token = strtok (NIL, " "))
        {
            int length;


            length = text_length (font, token);


            if ((x + length) > x2)
            {
                x = 9;

                y += (height_text + 1);
            }

            
            if (index == (MAX_MESSAGES - 1))
            {
                shadow_textout (screen_buffer, font, token, x, y, VIDEO_COLOR_WHITE);
            }
            else
            {
                shadow_textout (screen_buffer, font, token, x, y, silver);
            }


            x += (length + (text_length (font, " ") + 1));
        }


        memcpy (&video_messages [index] [0], buffer, MAX_MESSAGE_LENGTH);


        x = 4;

        y += (height_text + 1);
    }


    y = ((SCREEN_H - (height_text + 2)) - 1);


    if (input_mode & INPUT_MODE_CHAT)
    {
        int length;


        length = text_length (font, &input_chat_text [input_chat_offset]);


        shadow_textout (screen_buffer, font, &input_chat_text [input_chat_offset], x, y, VIDEO_COLOR_WHITE);


        x += (length + 1);

        y += (height_text - 1);


        hline (screen_buffer, (x + 1), (y + 1), ((x + 5) + 1), VIDEO_COLOR_BLACK);

        hline (screen_buffer, x, y, (x + 5), VIDEO_COLOR_WHITE);
    }
}


static void erase_messages (void)
{
    int x;

    int y;


    int x2;

    int y2;


    int height;


    int height_text;


    height = get_messages_height ();


    height_text = text_height (font);


    x = 0;

    y = ((SCREEN_H - (((height_text + 6) + height) + 3)) - 1);


    x2 = (SCREEN_W - 1);

    y2 = (SCREEN_H - 1);


    rectfill (screen_buffer, x, y, x2, y2, VIDEO_COLOR_BLACK);
}
