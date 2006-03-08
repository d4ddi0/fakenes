/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   video.c: Implementation of the video interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "common.h"
#include "cpu.h"
#include "data.h"
#include "debug.h"
#include "gui.h"
#include "input.h"
#include "log.h"
#include "ppu.h"
#include "rom.h"
#include "timing.h"
#include "types.h"
#include "video.h"


static BITMAP * screen_buffer = NIL;

static BITMAP * page_buffer = NIL;


static BITMAP * status_buffer = NIL;


static BITMAP * mouse_sprite_remove_buffer = NIL;


static BITMAP * stretch_buffer = NIL;


#define MAX_MESSAGES    10


static USTRING video_messages[MAX_MESSAGES];


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


int video_force_fullscreen = FALSE;


int video_driver = 0;


BITMAP * base_video_buffer = NIL;

BITMAP * video_buffer = NIL;


FONT * small_font = NIL;


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


RGB * video_palette = NIL;

static int video_palette_id = -1;


static int using_custom_font = FALSE;


#include "blit/2xscl.h"

#include "blit/des.h"

#include "blit/hq.h"

#include "blit/interp.h"


int video_init (void)
{
    int driver;


    const UINT8 * font_file;


    LOCK_VARIABLE (video_message_duration);


    LOCK_FUNCTION (video_message_timer);


    install_int_ex (video_message_timer, BPS_TO_TIMER (1));


    video_driver = get_config_id ("video", "driver", GFX_AUTODETECT);


    screen_width = get_config_int ("video", "screen_width", 640);

    screen_height = get_config_int ("video", "screen_height", 480);


    color_depth = get_config_int ("video", "color_depth", -1);


    video_force_fullscreen = get_config_int ("video", "force_fullscreen", FALSE);


    blitter_type = get_config_int ("video", "blitter_type", VIDEO_BLITTER_STRETCHED);

    filter_list = get_config_int ("video", "filter_list", 0);


    stretch_width = get_config_int ("video", "stretch_width", 512);

    stretch_height = get_config_int ("video", "stretch_height", 480);


    first_blit_line = get_config_int ("video", "first_blit_line", 0);

    last_blit_line = get_config_int ("video", "last_blit_line", 239);

    if (last_blit_line < first_blit_line)
    {
        last_blit_line = first_blit_line;
    }


    image_offset_x = get_config_int ("video", "image_offset_x", 0);

    image_offset_y = get_config_int ("video", "image_offset_y", 0);


    zoom_factor_x = get_config_int ("video", "zoom_factor_x", 256);

    zoom_factor_y = get_config_int ("video", "zoom_factor_y", 240);


    light_adjustment = get_config_int ("video", "light_adjustment", 0);


    video_display_status = get_config_int ("video", "display_status", FALSE);

    video_enable_vsync = get_config_int ("video", "enable_vsync", FALSE);


    if (video_driver == GFX_AUTODETECT)
    {
        if (video_force_fullscreen)
        {
            driver = GFX_AUTODETECT_FULLSCREEN;
        }
        else
        {
            int depth;

            depth = desktop_color_depth ();

            /* Attempt to detect a windowed environment.  This has a side
               effect of changing the default color depth to that of the
               desktop. */

            if (depth > 0)
            {
                driver = GFX_AUTODETECT_WINDOWED;

                if (color_depth == -1)
                {
                    color_depth = depth;
                }
            }
            else
            {
                driver = GFX_AUTODETECT;
            }
        }
    }
    else
    {
        driver = video_driver;
    }


    if (color_depth == -1)
    {
        /* No windowed environment present to autodetect a color depth from;
           default to 256 colors. */

        color_depth = 8;
    }

    if ((color_depth != 8) && (color_depth != 15) && (color_depth != 16) && (color_depth != 24) && (color_depth != 32))
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

    clear (screen_buffer);


    if (gfx_capabilities & GFX_HW_VRAM_BLIT)
    {
       /* VRAM->VRAM blits are hardware accelerated (note: This call doesn't
          have to succeed). */

       page_buffer = create_video_bitmap (SCREEN_W, SCREEN_H);
    }
    else
    {
       /* No hardware acceleration available. */

       page_buffer = NIL;
    }


    status_buffer = create_sub_bitmap (screen_buffer, 0, (SCREEN_H - 128), 72, 128);


    mouse_sprite_remove_buffer = create_bitmap_ex (8, 16, 16);


    /* Heh, redundant variable assignment. :( */

    video_set_blitter (blitter_type);

    video_set_filter_list (filter_list);


    if (! preserve_video_buffer)
    {
        base_video_buffer = create_bitmap_ex (8, ((8 + 256) + 8), ((16 + 240) + 16));
    
        video_buffer = create_sub_bitmap (base_video_buffer, 8, 16, 256, 240);
    

        clear (base_video_buffer);
    }


    if (preserve_palette)
    {
        video_set_palette (NIL);
    }
    else
    {
        video_set_palette (DATA_TO_RGB (MODERN_NTSC_PALETTE));

        video_set_palette_id (DATA_INDEX (MODERN_NTSC_PALETTE));
    }


    small_font = DATA_TO_FONT (SMALL_FONT);


    font_file = get_config_string ("gui", "font", "");

    if ((strlen (font_file) > 1) && (exists (font_file)))
    {
        font = load_font (font_file, NIL, NIL);

        if (! font)
        {
            using_custom_font = FALSE;


            font = DATA_TO_FONT (SMALL_FONT_CLEAN);
        }
        else
        {
            using_custom_font = TRUE;
        }
    }
    else
    {
        /* Reset just in case. */

        using_custom_font = FALSE;


        font = DATA_TO_FONT (SMALL_FONT_CLEAN);
    }


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
            show_mouse (screen);
        }
    }


    return (result);
}


void video_exit (void)
{
    remove_int (video_message_timer);


    set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);


    destroy_bitmap (status_buffer);


    if (page_buffer)
    {
        destroy_bitmap (page_buffer);
    }

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


    if (using_custom_font)
    {
        destroy_font (font);
    }


    set_config_id ("video", "driver", video_driver);


    set_config_int ("video", "screen_width", screen_width);

    set_config_int ("video", "screen_height", screen_height);


    set_config_int ("video", "color_depth", color_depth);


    set_config_int ("video", "force_fullscreen", video_force_fullscreen);


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
    shadow_textout (bitmap, small_font, "Video:", 16, (bitmap -> h - 114), color);

    shadow_textout (bitmap, small_font, "Audio:", 16, (bitmap -> h - 82), color);


    shadow_textout (bitmap, small_font, "Core:", 16, (bitmap -> h - 50), color);


    textprintf_ex (bitmap, small_font, (20 + 1), ((bitmap -> h - 100) + 1), VIDEO_COLOR_BLACK, -1, "%02d FPS", timing_fps);

    textprintf_ex (bitmap, small_font, 20, (bitmap -> h - 100), color, -1, "%02d FPS", timing_fps);


    if (audio_enable_output)
    {
        textprintf_ex (bitmap, small_font, (20 + 1), ((bitmap -> h - 68) + 1), VIDEO_COLOR_BLACK, -1, "%02d FPS", timing_audio_fps);

        textprintf_ex (bitmap, small_font, 20, (bitmap -> h - 68), color, -1, "%02d FPS", timing_audio_fps);
    }
    else
    {
        shadow_textout (bitmap, small_font, "Disabled", 20, (bitmap -> h - 68), color);
    }


    textprintf_ex (bitmap, small_font, (20 + 1), ((bitmap -> h - 36) + 1), VIDEO_COLOR_BLACK, -1, "%02d Hz", timing_hertz);

    textprintf_ex (bitmap, small_font, 20, (bitmap -> h - 36), color, -1, "%02d Hz", timing_hertz);


    textprintf_ex (bitmap, small_font, (20 + 1), ((bitmap -> h - 22) + 1), VIDEO_COLOR_BLACK, -1, "PC: $%04X", * cpu_active_pc);

    textprintf_ex (bitmap, small_font, 20, (bitmap -> h - 22), color, -1, "PC: $%04X", * cpu_active_pc);
}


static INLINE int select_blitter (void)
{
    if ((SCREEN_W >= 1024) && (SCREEN_H >= 960))
    {
        return (VIDEO_BLITTER_HQ4X);
    }
    if ((SCREEN_W >= 768) && (SCREEN_H >= 720))
    {
        return (VIDEO_BLITTER_HQ3X);
    }
    if ((SCREEN_W >= 512) && (SCREEN_H >= 480))
    {
        return (VIDEO_BLITTER_HQ2X);
    }
    else
    {
        return (VIDEO_BLITTER_DES);
    }
}


static INLINE void color_deemphasis_overlay (void)
{
    int y;


    int width;

    int height;


    switch (blitter_type)
    {
        case VIDEO_BLITTER_NORMAL:

        case VIDEO_BLITTER_DES:

            width = 256;

            height = 1;


            break;


        case VIDEO_BLITTER_INTERPOLATED_2X:

        case VIDEO_BLITTER_2XSCL:

        case VIDEO_BLITTER_DESII:

        case VIDEO_BLITTER_SUPER_2XSCL:

        case VIDEO_BLITTER_ULTRA_2XSCL:

        case VIDEO_BLITTER_HQ2X:

            width = 512;

            height = 2;


            break;


        case VIDEO_BLITTER_INTERPOLATED_3X:

        case VIDEO_BLITTER_HQ3X:

            width = 768;

            height = 3;


            break;


        case VIDEO_BLITTER_HQ4X:

            width = 1024;

            height = 4;


            break;


        default:

            return;
    }


    set_multiply_blender (0, 0, 0, 255);


    drawing_mode (DRAW_MODE_TRANS, NULL, 0, 0);


    for (y = 0; y < PPU_DISPLAY_LINES; y ++)
    {
        int bits;


        int red;

        int green;

        int blue;


        int line;


        bits = ((ppu_register_2001_cache [y] >> 5) & 0x07);

        if (bits == 0)
        {
            continue;
        }


        red = ((bits & 0x06) ? 191 : 255);

        green = ((bits & 0x05) ? 191 : 255);

        blue = ((bits & 0x03) ? 191 : 255);


        for (line = 0; line < height; line ++)
        {
            hline (screen_buffer, blit_x_offset, (blit_y_offset + ((y * height) + line)), (blit_x_offset + (width - 1)), makecol (red, green, blue));
        }
    }


    solid_mode ();
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


        case VIDEO_BLITTER_DES:

            blit_des (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_INTERPOLATED_2X:

            blit_interpolated_2x (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_2XSCL:

            blit_2xscl (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_DESII:

            blit_desii (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_SUPER_2XSCL:

            blit_super_2xscl (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_ULTRA_2XSCL:

            blit_ultra_2xscl (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_HQ2X:

            blit_hq2x (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_INTERPOLATED_3X:

            blit_interpolated_3x (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_HQ3X:

            blit_hq3x (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


            break;


        case VIDEO_BLITTER_HQ4X:

            blit_hq4x (video_buffer, screen_buffer, blit_x_offset, blit_y_offset);


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
    }


    color_deemphasis_overlay ();


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


    if (page_buffer)
    {
        /* Reduce screen tearing by blitting to VRAM first, then doing a
           VRAM to VRAM blit to the visible portion of the screen, since
           such blits are much faster.  Of course, we could just do page
           flipping, but this way we keep things simple and compatible. */

        acquire_bitmap (page_buffer);

        blit (screen_buffer, page_buffer, 0, 0, image_offset_x, image_offset_y, SCREEN_W, SCREEN_H);

        release_bitmap (page_buffer);


        if (video_enable_vsync)
        {
            vsync ();
        }

        blit (page_buffer, bitmap, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    }
    else
    {
        acquire_bitmap (bitmap);

        if (video_enable_vsync)
        {
            vsync ();
        }

        blit (screen_buffer, bitmap, 0, 0, image_offset_x, image_offset_y, SCREEN_W, SCREEN_H);

        release_bitmap (bitmap);
    }


    if (((video_message_duration > 0) || (input_mode & INPUT_MODE_CHAT)) && (! gui_is_active))
    {
        erase_messages ();
    }


    clear (status_buffer);


    /* clear (screen_buffer); */
}


void video_zoom (int x_factor, int y_factor)
{
    if ((blitter_type != VIDEO_BLITTER_NORMAL) &&
        (blitter_type != VIDEO_BLITTER_STRETCHED))
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


            video_set_palette (video_palette);


            break;


        case KEY_F11:

            if (light_adjustment < 63)
            {
                light_adjustment ++;
            }


            video_set_palette (video_palette);


            break;


        default:

            break;
    }
}


#define NES_PALETTE_SIZE                64


#define NES_PALETTE_START               1

#define NES_PALETTE_END                 (NES_PALETTE_START + NES_PALETTE_SIZE)


#define GUI_GRADIENT_PALETTE_SIZE       64


#define GUI_GRADIENT_PALETTE_START      (NES_PALETTE_END + 1)

#define GUI_GRADIENT_PALETTE_END        (GUI_GRADIENT_PALETTE_START + GUI_GRADIENT_PALETTE_SIZE)


#define GUI_COLORS_PALETTE_SIZE         GUI_TOTAL_COLORS


#define GUI_COLORS_PALETTE_START        (GUI_GRADIENT_PALETTE_END + 1)

#define GUI_COLORS_PALETTE_END          (GUI_COLORS_PALETTE_START + GUI_COLORS_PALETTE_SIZE)


#define GUI_IMAGE_PALETTE_SIZE          112


#define GUI_IMAGE_PALETTE_START         (256 - GUI_IMAGE_PALETTE_SIZE)

#define GUI_IMAGE_PALETTE_END           (GUI_IMAGE_PALETTE_START + GUI_IMAGE_PALETTE_SIZE)


static UINT16 solid_map [64] [64] [64];


static COLOR_MAP half_transparency_map;


void video_set_palette (RGB * palette)
{
    int index;


    int r;

    int g;

    int b;


    if (palette)
    {
        video_palette = palette;
    
    
        memcpy (internal_palette, palette, sizeof (internal_palette));
    }


    for (index = NES_PALETTE_START; index < NES_PALETTE_END; index ++)
    {
        internal_palette [index].r = fix ((internal_palette [index].r + light_adjustment), 0, 63);

        internal_palette [index].g = fix ((internal_palette [index].g + light_adjustment), 0, 63);

        internal_palette [index].b = fix ((internal_palette [index].b + light_adjustment), 0, 63);
    }


    if (gui_is_active)
    {
        video_create_gui_gradient (&gui_theme [0], &gui_theme [1], GUI_GRADIENT_PALETTE_SIZE);
    
    
        for (index = GUI_GRADIENT_PALETTE_START; index < GUI_GRADIENT_PALETTE_END; index ++)
        {
            GUI_COLOR color;
    
    
            video_create_gui_gradient (&color, NIL, NIL);
    
    
            internal_palette [index].r = (color.r * 63);
    
            internal_palette [index].g = (color.g * 63);
    
            internal_palette [index].b = (color.b * 63);
        }
    
    
        for (index = GUI_COLORS_PALETTE_START; index < GUI_COLORS_PALETTE_END; index ++)
        {
            int color;
    
    
            color = (index - GUI_COLORS_PALETTE_START);
    
    
            internal_palette [index].r = (gui_theme [color].r * 63);
    
            internal_palette [index].g = (gui_theme [color].g * 63);
    
            internal_palette [index].b = (gui_theme [color].b * 63);
        }
    
    
        for (index = GUI_IMAGE_PALETTE_START; index < GUI_IMAGE_PALETTE_END; index ++)
        {
            internal_palette [index].r = gui_image_palette [index].r;
    
            internal_palette [index].g = gui_image_palette [index].g;
    
            internal_palette [index].b = gui_image_palette [index].b;
        }
    }


    set_palette (internal_palette);


    if (color_depth < 24)
    {
        for (r = 0; r < 64; r ++)
        {
            for (g = 0; g < 64; g ++)
            {
                for (b = 0; b < 64; b ++)
                {
                    solid_map [r] [g] [b] = makecol (((r / 63.0) * 255), ((g / 63.0) * 255), ((b / 63.0) * 255));
                }
            }
        }
    }


    set_trans_blender (0, 0, 0, 127);


    create_blender_table (&half_transparency_map, internal_palette, NIL);


    color_map = &half_transparency_map;
}


void video_set_palette_id (int id)
{
   video_palette_id = id;
}


int video_get_palette_id (void)
{
   return (video_palette_id);
}


int video_create_color (int r, int g, int b)
{
    switch (color_depth)
    {
        case 8:

        case 15:

        case 16:

            return (solid_map [(r / 4)] [(g / 4)] [(b / 4)]);


        case 24:

            return (makecol24 (r, g, b));


        case 32:

            return (makecol32 (r, g, b));


        default:

            return (0);
    }
}


static int dither_table [4] [4] =
{
    {  0,  2,  0, -2 },
    {  2,  0, -2,  0 },
    {  0, -2,  0,  2 },
    { -2,  0,  2,  0 }
};


/*
static int dither_table [4] [4] =
{
    { -8,  0, -6,  2 },
    {  4, -4,  6, -2 },
    { -5,  3, -7,  1 },
    {  7, -1,  5, -3 }
};
*/


int video_create_color_dither (int r, int g, int b, int x, int y)
{
    if (color_depth < 24)
    {
        x &= 3;
    
        y &= 3;
    
    
        r = fix ((r + dither_table [y] [x]), 0, 255);
    
        g = fix ((g + dither_table [y] [x]), 0, 255);
    
        b = fix ((b + dither_table [y] [x]), 0, 255);
    }


    return (video_create_color (r, g, b));
}


#define GRADIENT_SHIFTS         16


#define GRADIENT_MULTIPLIER     (255 << GRADIENT_SHIFTS)


static int gradient_start [3];

static int gradient_end [3];


static float gradient_delta [3];


static int gradient_slice;


static int gradient_last_x;


int video_create_gradient (int start, int end, int slices, int x, int y)
{
    if (slices)
    {
        gradient_start [0] = (getr (start) << GRADIENT_SHIFTS);

        gradient_start [1] = (getg (start) << GRADIENT_SHIFTS);

        gradient_start [2] = (getb (start) << GRADIENT_SHIFTS);


        gradient_end [0] = (getr (end) << GRADIENT_SHIFTS);

        gradient_end [1] = (getg (end) << GRADIENT_SHIFTS);

        gradient_end [2] = (getb (end) << GRADIENT_SHIFTS);


        gradient_delta [0] = ((gradient_end [0] - gradient_start [0]) / slices);

        gradient_delta [1] = ((gradient_end [1] - gradient_start [1]) / slices);

        gradient_delta [2] = ((gradient_end [2] - gradient_start [2]) / slices);


        gradient_slice = 0;


        gradient_last_x = -1;


        return (NIL);
    }
    else
    {
        int red;

        int green;

        int blue;


        red = (gradient_start [0] + (gradient_delta [0] * gradient_slice));

        green = (gradient_start [1] + (gradient_delta [1] * gradient_slice));

        blue = (gradient_start [2] + (gradient_delta [2] * gradient_slice));


        red >>= GRADIENT_SHIFTS;

        green >>= GRADIENT_SHIFTS;

        blue >>= GRADIENT_SHIFTS;


        if (gradient_last_x != x)
        {
            gradient_last_x = x;


            gradient_slice ++;
        }


        return (video_create_color_dither (red, green, blue, x, y));
    }
}
 

void video_create_gui_gradient (GUI_COLOR * start, GUI_COLOR * end, int slices)
{
    if (slices)
    {
        gradient_start [0] = (start -> r * GRADIENT_MULTIPLIER);

        gradient_start [1] = (start -> g * GRADIENT_MULTIPLIER);

        gradient_start [2] = (start -> b * GRADIENT_MULTIPLIER);


        gradient_end [0] = (end -> r * GRADIENT_MULTIPLIER);

        gradient_end [1] = (end -> g * GRADIENT_MULTIPLIER);

        gradient_end [2] = (end -> b * GRADIENT_MULTIPLIER);


        gradient_delta [0] = ((gradient_end [0] - gradient_start [0]) / slices);

        gradient_delta [1] = ((gradient_end [1] - gradient_start [1]) / slices);

        gradient_delta [2] = ((gradient_end [2] - gradient_start [2]) / slices);


        gradient_slice = 0;
    }
    else
    {
        start -> r = ((gradient_start [0] + (gradient_delta [0] * gradient_slice)) / GRADIENT_MULTIPLIER);

        start -> g = ((gradient_start [1] + (gradient_delta [1] * gradient_slice)) / GRADIENT_MULTIPLIER);

        start -> b = ((gradient_start [2] + (gradient_delta [2] * gradient_slice)) / GRADIENT_MULTIPLIER);


        gradient_slice ++;
    }
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

        case VIDEO_BLITTER_DES:

            blit_x_offset = ((SCREEN_W / 2) - (256 / 2));
        
            blit_y_offset = ((SCREEN_H / 2) - (240 / 2));


            break;


        case VIDEO_BLITTER_INTERPOLATED_2X:

        case VIDEO_BLITTER_DESII:

        case VIDEO_BLITTER_2XSCL:

        case VIDEO_BLITTER_SUPER_2XSCL:

        case VIDEO_BLITTER_ULTRA_2XSCL:

        case VIDEO_BLITTER_HQ2X:

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


        case VIDEO_BLITTER_INTERPOLATED_3X:

        case VIDEO_BLITTER_HQ3X:

            if (! ((SCREEN_W < 768) || (SCREEN_H < 720)))
            {
                blit_x_offset = ((SCREEN_W / 2) - 384);
    
                blit_y_offset = ((SCREEN_H / 2) - 360);
            }
            else
            {
                blit_x_offset = ((SCREEN_W / 2) - (384 / 2));
            
                blit_y_offset = ((SCREEN_H / 2) - (360 / 2));
            }


            break;


        case VIDEO_BLITTER_HQ4X:

            if (! ((SCREEN_W < 1024) || (SCREEN_H < 960)))
            {
                blit_x_offset = ((SCREEN_W / 2) - 512);
    
                blit_y_offset = ((SCREEN_H / 2) - 480);
            }
            else
            {
                blit_x_offset = ((SCREEN_W / 2) - (512 / 2));
            
                blit_y_offset = ((SCREEN_H / 2) - (480 / 2));
            }


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
        show_mouse (screen);
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
        show_mouse (screen);
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
        show_mouse (screen);
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
            hline (screen_buffer, blit_x_offset, y, screen_buffer -> w, 0);
        }
    }
    else if (filter_list & VIDEO_FILTER_SCANLINES_MEDIUM)
    {
        set_trans_blender (0, 0, 0, 127);


        drawing_mode (DRAW_MODE_TRANS, NIL, 0, 0);


        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, blit_x_offset, y, screen_buffer -> w, makecol (0, 0, 0));
        }


        solid_mode ();
    }
    else if (filter_list & VIDEO_FILTER_SCANLINES_LOW)
    {
        set_trans_blender (0, 0, 0, 63);


        drawing_mode (DRAW_MODE_TRANS, NIL, 0, 0);


        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, blit_x_offset, y, screen_buffer -> w, makecol (0, 0, 0));
        }


        solid_mode ();
    }
}


void video_message (const UCHAR *message, ...)
{
    va_list format;


    int index;


    USTRING buffer;


    va_start (format, message);

    uvszprintf (buffer, USTRING_SIZE, message, format);

    va_end (format);


    for (index = 0; index < (MAX_MESSAGES - 1); index ++)
    {
        ustrzcpy (&video_messages [index] [0], USTRING_SIZE, &video_messages [(index + 1)] [0]);
    }


    ustrzcpy (&video_messages [(MAX_MESSAGES - 1)] [0], USTRING_SIZE, buffer);


    log_printf ("%s\n", buffer);
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


    BOOL box = (input_mode & INPUT_MODE_CHAT);


    height = get_messages_height ();


    height_text = text_height (font);


    gray = video_create_color (127, 127, 127);

    silver = video_create_color (191, 191, 191);


    x = 0;

    y = ((SCREEN_H - (((height_text + 6) + height) + 3)) - 1);


    x2 = (SCREEN_W - 1);

    y2 = (SCREEN_H - 1);


    if (box)
    {
        drawing_mode (DRAW_MODE_TRANS, NIL, 0, 0);
    
        rectfill (screen_buffer, x, y, x2, y2, gray);
    
    
        solid_mode ();
    
        rect (screen_buffer, x, y, x2, y2, VIDEO_COLOR_WHITE);
    }


    x = 3;

    y ++;


    x2 = ((SCREEN_W - 1) - 1);


    if (box)
    {
        hline (screen_buffer, x, y, x2, VIDEO_COLOR_BLACK);
    }


    x = 0;

    y = ((SCREEN_H - (height_text + 5)) - 1);


    if (box)
    {
        hline (screen_buffer, x, y, x2, VIDEO_COLOR_WHITE);
    }


    x = 3;

    y ++;


    if (box)
    {
        hline (screen_buffer, x, y, x2, VIDEO_COLOR_BLACK);
    }


    x = 4;

    y = ((SCREEN_H - ((height_text + 6) + height)) - 1);


    x2 = ((SCREEN_W - 4) - 1);


    for (index = 0; index < MAX_MESSAGES; index ++)
    {
        UINT8 * token;


        USTRING buffer;


        memcpy (buffer, &video_messages [index] [0], USTRING_SIZE);


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


        memcpy (&video_messages [index] [0], buffer, USTRING_SIZE);


        x = 4;

        y += (height_text + 1);
    }


    y = ((SCREEN_H - (height_text + 2)) - 1);


    if (box)
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
