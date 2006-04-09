/* FakeNES - A free, portable, Open Source NES emulator.

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

static BITMAP *screen_buffer = NULL;
static BITMAP *page_buffer = NULL;

static BITMAP *status_buffer = NULL;
static BITMAP *mouse_sprite_remove_buffer = NULL;

#define MAX_MESSAGES    10

static USTRING video_messages[MAX_MESSAGES];

volatile int video_message_duration = 0;

static void video_message_timer (void)
{
   if (video_message_duration > 0)
      video_message_duration -= 1000;
}

END_OF_STATIC_FUNCTION (video_message_timer);

BOOL video_display_status = FALSE;
BOOL video_enable_page_buffer = FALSE;
BOOL video_enable_vsync = FALSE;
BOOL video_force_fullscreen = FALSE;

int video_driver = 0;

BITMAP *base_video_buffer = NULL;
BITMAP *video_buffer = NULL;

FONT *small_font = NULL;

static int screen_width  = 640;
static int screen_height = 480;
static int color_depth   = -1;
static int brightness    = 0;
                         
static LIST filter_list = 0;

#define VIDEO_COLOR_BLACK   palette_color[0]
#define VIDEO_COLOR_WHITE   palette_color[33]

static BOOL preserve_video_buffer = FALSE;
static BOOL preserve_palette = FALSE;

static PALETTE internal_palette;
RGB * video_palette = NULL;
static int video_palette_id = -1;

static BOOL using_custom_font = FALSE;

LIST video_edge_clipping = 0;

/* Blitter API. */
#include "blit/shared.h"

/* Blit buffers. */
static void *blit_buffer_in  = NULL;
static void *blit_buffer_out = NULL;

/* Blitter variables. */
static int blitter_id         = VIDEO_BLITTER_STRETCHED;
static const BLITTER *blitter = NULL;   /* Blitter interface. */
static int blit_x_offset      = 0;      
static int blit_y_offset      = 0;      
static int stretch_width      = 512;
static int stretch_height     = 480;

/* Blitters. */
#include "blit/2xscl.h"
#include "blit/des.h"
#include "blit/hq.h"
#include "blit/interp.h"
#include "blit/ntsc.h"
#include "blit/std.h"

static void switch_out_callback (void)
{
   if (!gui_is_active)
      audio_suspend ();
}

static void switch_in_callback (void)
{
   if (!gui_is_active)
      audio_resume ();
}

int video_init (void)
{
   int driver;
   const CHAR *font_file;

   /* Install message timer. */

   LOCK_VARIABLE (video_message_duration);
   LOCK_FUNCTION (video_message_timer);
   install_int_ex (video_message_timer, BPS_TO_TIMER(1));

   /* Load configuration. */

   video_driver             = get_config_id  ("video", "driver",             video_driver);
   screen_width             = get_config_int ("video", "screen_width",       screen_width);
   screen_height            = get_config_int ("video", "screen_height",      screen_height);
   color_depth              = get_config_int ("video", "color_depth",        color_depth);
   video_force_fullscreen   = get_config_int ("video", "force_fullscreen",   video_force_fullscreen);
   blitter_id               = get_config_int ("video", "blitter",            blitter_id);
   filter_list              = get_config_int ("video", "filter_list",        filter_list);
   stretch_width            = get_config_int ("video", "stretch_width",      stretch_width);
   stretch_height           = get_config_int ("video", "stretch_height",     stretch_height);
   brightness               = get_config_int ("video", "brightness",         brightness);
   video_display_status     = get_config_int ("video", "display_status",     video_display_status);
   video_enable_page_buffer = get_config_int ("video", "enable_page_buffer", video_enable_page_buffer);
   video_enable_vsync       = get_config_int ("video", "enable_vsync",       video_enable_vsync);
   video_edge_clipping      = get_config_int ("video", "edge_clipping",      video_edge_clipping);

   /* Determine which driver to use. */

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
               color_depth = depth;
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

   /* Set color depth. */

   if (color_depth == -1)
   {
      /* No windowed environment present to autodetect a color depth from;
         default to 256 colors. */

      color_depth = 8;
   }

   if ((color_depth != 8)  &&
       (color_depth != 15) &&
       (color_depth != 16) &&
       (color_depth != 24) &&
       (color_depth != 32))
   {   
      WARN("Invalid color depth");
      return (1);
   }

   set_color_depth (color_depth);

   /* Enter graphics mode. */

   if (set_gfx_mode (driver, screen_width, screen_height, 0, 0) != 0)
   {
      WARN("set_gfx_mode() failed");
      return (2);
   }

   if (color_depth != 8)
      set_color_conversion (COLORCONV_TOTAL);

   /* Create screen buffer. */

   screen_buffer = create_bitmap (SCREEN_W, SCREEN_H);
   if (!screen_buffer)
   {
      WARN("Couldn't create screen buffer");
      return (3);
   }

   clear_bitmap (screen_buffer);
        
   /* Create page buffer. */

   if (video_enable_page_buffer)
   {
      page_buffer = create_video_bitmap (SCREEN_W, SCREEN_H);
      if (!page_buffer)
      {
         WARN("Failed to create page buffer");
         video_enable_page_buffer = FALSE;
      }
   }
   else
   {
      page_buffer = NULL;
   }

   /* Create status buffer. */
   status_buffer = create_sub_bitmap (screen_buffer, 0, (screen_buffer->h -
      128), 72, 128);
   if (!status_buffer)
   {
      WARN("Failed to create status buffer");
      return (4);
   }
  
   if (!preserve_video_buffer)
   {
      /* Create video buffer. */

      base_video_buffer = create_bitmap_ex (8, ((8 + 256) + 8), ((16 + 240)
         + 16));
      video_buffer = create_sub_bitmap (base_video_buffer, 8, 16, 256, 240);

      if (!base_video_buffer || !video_buffer)
      {
         WARN("Couldn't create video buffer");
         return (5);
      }

      clear_bitmap (base_video_buffer);
   }

   /* TODO: Is this really neccessary?  Maybe we can remove this stuff when
      the PPU gets a new internal buffer format. */
   mouse_sprite_remove_buffer = create_bitmap_ex (8, 16, 16);
   if (!mouse_sprite_remove_buffer)
   {
      WARN_GENERIC();
      return (6);
   }

   /* Set up palette. */

   if (preserve_palette)
   {
      /* Use existing palette. */
      video_set_palette (NULL);
   }
   else
   {
      /* Set default palette. */
      video_set_palette    (DATA_TO_RGB(MODERN_NTSC_PALETTE));
      video_set_palette_id (DATA_INDEX(MODERN_NTSC_PALETTE));
   }

   /* Set up blitter & filters. */

   video_set_blitter (blitter_id);
   video_set_filter_list (filter_list);

   /* Set up fonts. */

   small_font = DATA_TO_FONT(SMALL_FONT);

   font_file = get_config_string ("gui", "font", "");

   if ((strlen (font_file) > 1) && (exists (font_file)))
   {
      font = load_font (font_file, NULL, NULL);

      if (font)
      {
         using_custom_font = TRUE;
      }
      else
      {
         WARN("Font load failed");

         font = DATA_TO_FONT(SMALL_FONT_CLEAN);
         using_custom_font = FALSE;
      }
   }
   else
   {
      /* Reset just in case. */

      font = DATA_TO_FONT (SMALL_FONT_CLEAN);
      using_custom_font = FALSE;
   }
      
   if (is_windowed_mode ())
   {
      set_display_switch_mode (SWITCH_BACKGROUND);
   }
   else
   {
      set_display_switch_mode (SWITCH_AMNESIA);

      /* Install callbacks. */
      set_display_switch_callback (SWITCH_IN,  switch_in_callback);
      set_display_switch_callback (SWITCH_OUT, switch_out_callback);
   }

   /* Return success. */
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
         show_mouse (screen);
   }

   return (result);
}

void video_exit (void)
{
   if (!is_windowed_mode ())
   {
      /* Remove callbacks. */
      remove_display_switch_callback (switch_in_callback);
      remove_display_switch_callback (switch_out_callback);
   }

   /* Remove message timer. */
   remove_int (video_message_timer);

   /* Return to text mode. */
   set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);

   if (using_custom_font)
   {
      /* Destroy font. */
      destroy_font (font);
      using_custom_font = FALSE;
   }

   if (blitter)
   {
      /* Deinitializer blitter. */

      if (blitter->deinit)
         blitter->deinit ();

      blitter = NULL;
   }

   /* Destroy buffers. */

   if (mouse_sprite_remove_buffer)
      destroy_bitmap (mouse_sprite_remove_buffer);

   if (!preserve_video_buffer)
   {
      if (video_buffer)
         destroy_bitmap (video_buffer);
      if (base_video_buffer)
         destroy_bitmap (base_video_buffer);
   }

   if (status_buffer)
      destroy_bitmap (status_buffer);

   if (page_buffer)
      destroy_bitmap (page_buffer);
   if (screen_buffer)
      destroy_bitmap (screen_buffer);

   /* Save configuration. */

   set_config_id  ("video", "driver",             video_driver);
   set_config_int ("video", "screen_width",       screen_width);
   set_config_int ("video", "screen_height",      screen_height);
   set_config_int ("video", "color_depth",        color_depth);
   set_config_int ("video", "force_fullscreen",   video_force_fullscreen);
   set_config_int ("video", "blitter",            blitter_id);
   set_config_int ("video", "filter_list",        filter_list);
   set_config_int ("video", "stretch_width",      stretch_width);
   set_config_int ("video", "stretch_height",     stretch_height);
   set_config_int ("video", "brightness",         brightness);
   set_config_int ("video", "display_status",     video_display_status);
   set_config_int ("video", "enable_page_buffer", video_enable_page_buffer);
   set_config_int ("video", "enable_vsync",       video_enable_vsync);
   set_config_int ("video", "edge_clipping",      video_edge_clipping);
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


static void draw_messages (void);

static void erase_messages (void);


static int flash_tick = 0;


void video_blit (BITMAP *bitmap)
{
   RT_ASSERT(bitmap);

   if (!rom_is_loaded)
      return;

   if (video_edge_clipping)
   {
      int w, h;

      /* Calculate sizes. */
      w = (video_buffer->w - 1);
      h = (video_buffer->h - 1);

      if (video_edge_clipping & VIDEO_EDGE_CLIPPING_HORIZONTAL)
      {
         /* Left edge. */
         rectfill (video_buffer, 0, 0, 12, h, 0);
   
         /* Right edge. */
         rectfill (video_buffer, (w - 12), 0, w, h, 0);
      }

      if (video_edge_clipping & VIDEO_EDGE_CLIPPING_VERTICAL)
      {
         /* Top edge. */
         rectfill (video_buffer, 0, 0, w, 12, 0);
   
         /* Bottom edge. */
         rectfill (video_buffer, 0, (h - 12), w, h, 0);
      }
   }

   if (input_enable_zapper && !gui_is_active)
   {
      /* Draw Zapper sprite. */

      blit (video_buffer, mouse_sprite_remove_buffer,
         (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 0, 0, 16,
            16);

      masked_blit (DATA_TO_BITMAP(GUN_SPRITE), video_buffer, 0, 0,
         (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 16, 16);
   }

   /* Perform blitting operation. */
   blitter->blit (video_buffer, screen_buffer, blit_x_offset,
      blit_y_offset);

   if (input_enable_zapper && !gui_is_active)
   {
      /* Undraw Zapper sprite. */

      blit (mouse_sprite_remove_buffer, video_buffer, 0, 0,
         (input_zapper_x_offset - 7), (input_zapper_y_offset - 7), 16, 16);
   }

   /* Apply filters. */
   video_filter ();

   if (video_display_status && !gui_is_active)
      display_status (screen_buffer, VIDEO_COLOR_WHITE);

   if (((video_message_duration > 0) ||
        (input_mode & INPUT_MODE_CHAT)) && !gui_is_active)
   {
      /* Draw messages. */
      draw_messages ();
   }

    /* Send screen buffer to screen. */

   if (page_buffer && is_video_bitmap (bitmap))
   {
      /* Reduce screen tearing by blitting to VRAM first, then doing a
         VRAM to VRAM blit to the visible portion of the screen, since
         such blits are much faster.  Of course, we could just do page
         flipping, but this way we keep things simple and compatible. */

      acquire_bitmap (page_buffer);
      blit (screen_buffer, page_buffer, 0, 0, 0, 0, screen_buffer->w,
         screen_buffer->h);
      release_bitmap (page_buffer);

      acquire_bitmap (bitmap);
      if (video_enable_vsync)
         vsync ();
      blit (page_buffer, bitmap, 0, 0, 0, 0, page_buffer->w,
         page_buffer->h);
      release_bitmap (bitmap);
   }
   else
   {
      acquire_bitmap (bitmap);
      if (video_enable_vsync)
         vsync ();
      blit (screen_buffer, bitmap, 0, 0, 0, 0, screen_buffer->w,
         screen_buffer->h);
      release_bitmap (bitmap);
   }

   if (((video_message_duration > 0) ||
        (input_mode & INPUT_MODE_CHAT)) && !gui_is_active)
   {
      /* Undraw messages. */
      erase_messages ();
   }

   /* Clear status buffer. */
   clear (status_buffer);
}


void video_handle_keypress (int index)
{
    if (! (input_mode & INPUT_MODE_CHAT))
    {
        switch ((index >> 8))
        {
            default:

                break;
        }
    }


    switch ((index >> 8))
    {
        case KEY_F10:

            brightness -= 5;

            if (brightness < -100)
            {
                brightness = -100;
            }


            video_set_palette (video_palette);


            break;


        case KEY_F11:

            brightness += 5;

            if (brightness > 100)
            {
               brightness = 100;
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


    int adjust;


    if (palette)
    {
        video_palette = palette;
    
    
        memcpy (internal_palette, palette, sizeof (internal_palette));
    }


    adjust = ROUND(((brightness / 100.0f) * 63.0f));

    for (index = NES_PALETTE_START; index < NES_PALETTE_END; index ++)
    {
        internal_palette [index].r = fix ((internal_palette [index].r + adjust), 0, 63);

        internal_palette [index].g = fix ((internal_palette [index].g + adjust), 0, 63);

        internal_palette [index].b = fix ((internal_palette [index].b + adjust), 0, 63);
    }


    if (gui_is_active)
    {
        video_create_gui_gradient (&gui_theme [0], &gui_theme [1], GUI_GRADIENT_PALETTE_SIZE);
    
    
        for (index = GUI_GRADIENT_PALETTE_START; index < GUI_GRADIENT_PALETTE_END; index ++)
        {
            GUI_COLOR color;
    
    
            video_create_gui_gradient (&color, NULL, NULL);
    
    
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


    create_blender_table (&half_transparency_map, internal_palette, NULL);


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


        return (NULL);
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

static INLINE int get_automatic_blitter (void)
{
   if ((SCREEN_W >= 1024) && (SCREEN_H >= 960))
      return (VIDEO_BLITTER_HQ4X);
   if ((SCREEN_W >= 768) && (SCREEN_H >= 720))
      return (VIDEO_BLITTER_HQ3X);
   if ((SCREEN_W >= 512) && (SCREEN_H >= 480))
      return (VIDEO_BLITTER_HQ2X);
   else
      return (VIDEO_BLITTER_DES);
}

#define BLITTER_SWITCH(id, name) \
   case id :   \
   {  \
      blitter = & blitter_##name ;  \
      break;   \
   }

void video_set_blitter (ENUM id)
{
   int selected_blitter;

   if (blitter)
   {
      if (blitter->deinit)
      {
         /* Deinitialize blitter. */
         blitter->deinit ();
      }
   }

   blitter_id = id;

   if (blitter_id == VIDEO_BLITTER_AUTOMATIC)
      selected_blitter = get_automatic_blitter ();
   else
      selected_blitter = blitter_id;

   switch (selected_blitter)
   {
      BLITTER_SWITCH(VIDEO_BLITTER_NORMAL,          normal)
      BLITTER_SWITCH(VIDEO_BLITTER_DES,             des)
      BLITTER_SWITCH(VIDEO_BLITTER_INTERPOLATED_2X, interpolated_2x)
      BLITTER_SWITCH(VIDEO_BLITTER_DESII,           desii)
      BLITTER_SWITCH(VIDEO_BLITTER_2XSCL,           2xscl)
      BLITTER_SWITCH(VIDEO_BLITTER_SUPER_2XSCL,     super_2xscl)
      BLITTER_SWITCH(VIDEO_BLITTER_ULTRA_2XSCL,     ultra_2xscl)
      BLITTER_SWITCH(VIDEO_BLITTER_HQ2X,            hq2x)
      BLITTER_SWITCH(VIDEO_BLITTER_NES_NTSC,        nes_ntsc)
      BLITTER_SWITCH(VIDEO_BLITTER_INTERPOLATED_3X, interpolated_3x)
      BLITTER_SWITCH(VIDEO_BLITTER_HQ3X,            hq3x)
      BLITTER_SWITCH(VIDEO_BLITTER_HQ4X,            hq4x)
      BLITTER_SWITCH(VIDEO_BLITTER_STRETCHED,       stretched)

      default:
         WARN_GENERIC();
   }

   if (blitter->init)
   {
      /* Initialize blitter. */
      blitter->init (video_buffer, screen_buffer);
   }

   clear (screen_buffer);
}

#undef BLITTER_SWITCH

ENUM video_get_blitter (void)
{
   return (blitter_id);
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


    if (gui_is_active)
    {
        show_mouse (screen);
    }
}


void video_set_filter_list (LIST filters)
{
    filter_list = filters;


    clear (screen_buffer);
}


LIST video_get_filter_list (void)
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

    if (filter_list & VIDEO_FILTER_SCANLINES_MEDIUM)
    {
        set_trans_blender (0, 0, 0, 127);


        drawing_mode (DRAW_MODE_TRANS, NULL, 0, 0);


        for (y = 0; y < screen_buffer -> h; y += 2)
        {
            hline (screen_buffer, blit_x_offset, y, screen_buffer -> w, makecol (0, 0, 0));
        }


        solid_mode ();
    }

    if (filter_list & VIDEO_FILTER_SCANLINES_LOW)
    {
        set_trans_blender (0, 0, 0, 63);


        drawing_mode (DRAW_MODE_TRANS, NULL, 0, 0);


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
        drawing_mode (DRAW_MODE_TRANS, NULL, 0, 0);
    
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


        for (token = strtok (&video_messages [index] [0], " "); token; token = strtok (NULL, " "))
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
            else if (box)
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
