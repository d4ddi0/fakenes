/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   gui.c: Implementation of the object-based GUI.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "cpu.h"
#include "data.h"
#include "debug.h"
#include "genie.h"
#include "gui.h"
#include "input.h"
#include "log.h"
#include "mmc.h"
#include "netplay.h"
#include "papu.h"
#include "ppu.h"
#include "rom.h"
#include "timing.h"
#include "types.h"
#include "version.h"
#include "video.h"

static int dialog_x = 0;
static int dialog_y = 0;
static BOOL restart_dialog = FALSE;
 
GUI_THEME gui_theme;
ENUM gui_theme_id = -1;
const GUI_THEME *last_theme = NULL;

RGB *gui_image_palette = NULL;
static BITMAP *gui_mouse_sprite = NULL;
static BITMAP *background_image = NULL;

/* Keep these in order! */
#include "gui/themes.h"
#include "gui/objects.h"
#include "gui/menus.h"
#include "gui/dialogs.h"

BOOL gui_needs_restart = FALSE;
BOOL gui_is_active = FALSE;
static BOOL want_exit = FALSE;

static USTRING message_buffer;

static PALETTE custom_palette;

/* Time to rest in milliseconds (used by gui_heartbeat()). */
#define REST_TIME 50

static int state_index = 0;   /* For states. */
static int replay_index = 0;  /* For replays. */

static INLINE MENU *load_menu (const MENU *menu)
{
   MENU *new_menu;
   int size = 0;
   int index = 0;

   RT_ASSERT(menu);

   while (menu[index].text || menu[index].proc)
   {
       size += sizeof (MENU);

       index++;
   }

   /* Once more for the end marker. */
   size += sizeof (MENU);

   if (!(new_menu = malloc (size)))
   {
      WARN("Failed to allocate menu structure");

      return (NULL);
   }

   memcpy (new_menu, menu, size);

   /* Reset counter. */
   index = 0;

   while (new_menu[index].text || new_menu[index].proc)
   {
      MENU *item = &new_menu[index];

      /* Import menu by reference. */
      if (item->child)
         item->child = *(MENU **)item->child;

      index++;
   }

   return (new_menu);
}

static INLINE DIALOG *load_dialog (const DIALOG *dialog)
{
   DIALOG *new_dialog;
   int size = 0;
   int index = 0;
   int width, height;

   RT_ASSERT(dialog);

   while (dialog[index].proc)
   {
      size += sizeof (DIALOG);

      index++;
   }

   /* Once more for the end marker. */
   size += sizeof (DIALOG);

   if (!(new_dialog = malloc (size)))
   {
       WARN("Failed to allocate dialog structure");

       return (NULL);
   }

   memcpy (new_dialog, dialog, size);

   /* Font scaling parameters. */
   width = text_length (font, "X");
   height = text_height (font);

   /* Reset counter. */
   index = 0;

   while (new_dialog[index].proc)
   {
      DIALOG *object = &new_dialog[index];

      /* Import menu by reference. */
      if (object->proc == d_menu_proc)
         object->dp = *(MENU **)object->dp;

      /* Dialog to font scaling. */
      if (font != small_font)
      {
         switch (index)
         {
            case 0: /* sl_frame. */
            {
               object->w = ROUND(((object->w / 5.0f) * width));
               object->h = (ROUND(((object->h / 6.0f) * height)) - height);

               break;
            }
         
            case 1: /* sl_x_button. */
            {
               object->x = ROUND(((object->x / 5.0f) * width));
     
               break;
            } 
    
            default:
            {
               object->x = ROUND(((object->x / 5.0f) * width));
               object->y = (ROUND(((object->y / 6.0f) * height)) - height);
               object->w = ROUND(((object->w / 5.0f) * width));
               object->h = ROUND(((object->h / 6.0f) * height));
    
               break;
            }
         }

      }

      index++;
    }

    return (new_dialog);
}

static INLINE void unload_menu (MENU *menu)
{
   RT_ASSERT(menu);

   free (menu);
}

static INLINE void unload_dialog (DIALOG *dialog)
{
   RT_ASSERT(dialog);

   free (dialog);
}

/* Define helper macros. */
#define MENU_FROM_BASE(name)    (name = load_menu (name ##_base))
#define DIALOG_FROM_BASE(name)  (name = load_dialog (name ##_base))

static INLINE void load_menus (void)
{
   MENU_FROM_BASE(main_state_select_menu);
   MENU_FROM_BASE(main_state_autosave_menu);
   MENU_FROM_BASE(main_state_menu);
   MENU_FROM_BASE(main_replay_select_menu);
   MENU_FROM_BASE(main_replay_record_menu);
   MENU_FROM_BASE(main_replay_play_menu);
   MENU_FROM_BASE(main_replay_menu);
   MENU_FROM_BASE(main_menu);
   MENU_FROM_BASE(netplay_protocol_menu);
   MENU_FROM_BASE(netplay_server_menu);
   MENU_FROM_BASE(netplay_client_menu);
   MENU_FROM_BASE(netplay_menu);
   MENU_FROM_BASE(options_gui_theme_menu);
   MENU_FROM_BASE(options_gui_menu);
   MENU_FROM_BASE(options_system_menu);
   MENU_FROM_BASE(options_audio_subsystem_menu);
   MENU_FROM_BASE(options_audio_mixing_channels_menu);
   MENU_FROM_BASE(options_audio_mixing_frequency_menu);
   MENU_FROM_BASE(options_audio_mixing_quality_menu);
   MENU_FROM_BASE(options_audio_mixing_anti_aliasing_menu);
   MENU_FROM_BASE(options_audio_mixing_menu);
   MENU_FROM_BASE(options_audio_effects_menu);
   MENU_FROM_BASE(options_audio_filters_menu);
   MENU_FROM_BASE(options_audio_channels_menu);
   MENU_FROM_BASE(options_audio_advanced_menu);
   MENU_FROM_BASE(options_audio_record_menu);
   MENU_FROM_BASE(options_audio_menu);
   MENU_FROM_BASE(options_video_driver_dos_menu);
   MENU_FROM_BASE(options_video_driver_windows_menu);
   MENU_FROM_BASE(options_video_driver_linux_menu);
   MENU_FROM_BASE(options_video_driver_unix_menu);
   MENU_FROM_BASE(options_video_driver_menu);
   MENU_FROM_BASE(options_video_resolution_proportionate_menu);
   MENU_FROM_BASE(options_video_resolution_extended_menu);
   MENU_FROM_BASE(options_video_resolution_menu);
   MENU_FROM_BASE(options_video_colors_menu);
   MENU_FROM_BASE(options_video_blitter_menu);
   MENU_FROM_BASE(options_video_filters_menu);
   MENU_FROM_BASE(options_video_layers_menu);
   MENU_FROM_BASE(options_video_palette_menu);
   MENU_FROM_BASE(options_video_advanced_menu);
   MENU_FROM_BASE(options_video_menu);
   MENU_FROM_BASE(options_input_menu);
   MENU_FROM_BASE(options_menu);
   MENU_FROM_BASE(help_menu);
   MENU_FROM_BASE(top_menu);
}

static INLINE void load_dialogs (void)
{
   DIALOG_FROM_BASE(main_dialog);
   DIALOG_FROM_BASE(main_state_save_dialog);
   DIALOG_FROM_BASE(main_replay_record_start_dialog);
   DIALOG_FROM_BASE(main_messages_dialog);
   DIALOG_FROM_BASE(options_input_dialog);
   DIALOG_FROM_BASE(options_patches_add_dialog);
   DIALOG_FROM_BASE(options_patches_dialog);
   DIALOG_FROM_BASE(netplay_client_connect_dialog);
   DIALOG_FROM_BASE(help_shortcuts_dialog);
   DIALOG_FROM_BASE(help_about_dialog);
}

/* Undefine helper macros. */
#undef MENU_FROM_BASE
#undef DIALOG_FROM_BASE

static INLINE void unload_menus (void)
{
   unload_menu (main_state_select_menu);
   unload_menu (main_state_autosave_menu);
   unload_menu (main_state_menu);
   unload_menu (main_replay_select_menu);
   unload_menu (main_replay_record_menu);
   unload_menu (main_replay_play_menu);
   unload_menu (main_replay_menu);
   unload_menu (main_menu);
   unload_menu (netplay_protocol_menu);
   unload_menu (netplay_server_menu);
   unload_menu (netplay_client_menu);
   unload_menu (netplay_menu);
   unload_menu (options_gui_theme_menu);
   unload_menu (options_gui_menu);
   unload_menu (options_system_menu);
   unload_menu (options_audio_subsystem_menu);
   unload_menu (options_audio_mixing_channels_menu);
   unload_menu (options_audio_mixing_frequency_menu);
   unload_menu (options_audio_mixing_quality_menu);
   unload_menu (options_audio_mixing_anti_aliasing_menu);
   unload_menu (options_audio_mixing_menu);
   unload_menu (options_audio_effects_menu);
   unload_menu (options_audio_filters_menu);
   unload_menu (options_audio_channels_menu);
   unload_menu (options_audio_advanced_menu);
   unload_menu (options_audio_record_menu);
   unload_menu (options_audio_menu);
   unload_menu (options_video_driver_dos_menu);
   unload_menu (options_video_driver_windows_menu);
   unload_menu (options_video_driver_linux_menu);
   unload_menu (options_video_driver_unix_menu);
   unload_menu (options_video_driver_menu);
   unload_menu (options_video_resolution_proportionate_menu);
   unload_menu (options_video_resolution_extended_menu);
   unload_menu (options_video_resolution_menu);
   unload_menu (options_video_colors_menu);
   unload_menu (options_video_blitter_menu);
   unload_menu (options_video_filters_menu);
   unload_menu (options_video_layers_menu);
   unload_menu (options_video_palette_menu);
   unload_menu (options_video_advanced_menu);
   unload_menu (options_video_menu);
   unload_menu (options_input_menu);
   unload_menu (options_menu);
   unload_menu (help_menu);
   unload_menu (top_menu);
}

static INLINE void unload_dialogs (void)
{
   unload_dialog (main_dialog);
   unload_dialog (main_state_save_dialog);
   unload_dialog (main_replay_record_start_dialog);
   unload_dialog (main_messages_dialog);
   unload_dialog (options_input_dialog);
   unload_dialog (options_patches_add_dialog);
   unload_dialog (options_patches_dialog);
   unload_dialog (netplay_client_connect_dialog);
   unload_dialog (help_shortcuts_dialog);
   unload_dialog (help_about_dialog);
}

static INLINE void pack_color (GUI_COLOR *color)
{
    int r, g, b;

    RT_ASSERT(color);

    r = (color->r * 255);
    g = (color->g * 255);
    b = (color->b * 255);

    color->packed = video_create_color (r, g, b);
}

void gui_set_theme (const GUI_THEME *theme)
{
   int index;

   RT_ASSERT(theme);

   last_theme = theme;

   memcpy (&gui_theme, theme, sizeof (GUI_THEME));

   video_set_palette (NULL);

   for (index = 0; index < GUI_TOTAL_COLORS; index++)
      pack_color (&gui_theme[index]);

   gui_bg_color = GUI_FILL_COLOR;
   gui_fg_color = GUI_TEXT_COLOR;
   gui_mg_color = GUI_DISABLED_COLOR;
}

static INLINE void update_colors (void)
{
   /* This function simply re-sets the last (current) theme to make sure all
      colors are correctly packed. */

   if (last_theme)
      gui_set_theme (last_theme);
}

static INLINE void redraw (void)
{
   /* This function redraws the current dialog. */

   broadcast_dialog_message (MSG_DRAW, 0);
}

static INLINE void draw_message (int color)
{
   /* This function draws the message currently present in message_buffer,
      either directly to the screen or by sending it to the video code. */

   if (gui_is_active)
   {
      BITMAP *bmp;
      int x1, y1, x2, y2;

      bmp = gui_get_screen ();

      x1 = 16;
      y1 = (((bmp->h - 16) - text_height (font)) - 8);
      x2 = (bmp->w - 16);
      y2 = (bmp->h - 16);
   
      vline (bmp, (x2 + 1), (y1 + 1), (y2 + 1), GUI_SHADOW_COLOR);
      hline (bmp, (x1 + 1), (y2 + 1), (x2 + 1), GUI_SHADOW_COLOR);
   
      rectfill (bmp, x1, y1, x2, y2, GUI_FILL_COLOR);
      rect (bmp, x1, y1, x2, y2, GUI_BORDER_COLOR);
   
      textout_centre_ex (bmp, font, message_buffer, (bmp->w / 2), ((bmp->h
         - 19) - text_height (font)), 0, -1);
      textout_centre_ex (bmp, font, message_buffer, ((bmp->w / 2) - 1),
         (((bmp->h - 19) - text_height (font)) - 1), color, -1);

      /* refresh (); */

      log_printf ("GUI: %s\n", message_buffer);
   }
   else
   {
      video_message (message_buffer);
      video_message_duration = 3000;
   }
}

void gui_message (int color, const UCHAR *message, ...)
{
   va_list format;

   RT_ASSERT(message);

   va_start (format, message);
   uvszprintf (message_buffer, USTRING_SIZE, message, format);
   va_end (format);

   draw_message (color);
}

static INLINE void message_local (const UCHAR *message, ...)
{
   /* This is identical to gui_message(), except that it always uses the
      GUI text color. */

   va_list format;

   RT_ASSERT(message);

   va_start (format, message);
   uvszprintf (message_buffer, USTRING_SIZE, message, format);
   va_end (format);

   draw_message (GUI_TEXT_COLOR);
}

static INLINE void refresh (void)
{
   BITMAP *bmp;

   /* If the GUI is not being drawn directly to the screen, this function
      stretch_blit()'s the GUI bitmap to the screen with vsync(). */

   bmp = gui_get_screen ();

   if (bmp == screen)
      return;

   acquire_screen ();
   vsync ();
   stretch_blit (bmp, screen, 0, 0, bmp->w, bmp->h, 0, 0, SCREEN_W,
      SCREEN_H);
   release_screen ();
}

void gui_heartbeat (void)
{
   /* Called by both run_dialog() and the custom, non-blocking menu objects
      at a rate of (1000 / REST_TIME) milliseconds, or close enough. */

   refresh ();

   rest (REST_TIME);
}

static INLINE int run_dialog (DIALOG *dialog)
{
   DIALOG_PLAYER *player;
   int index;

   /* Similar to Allegro's do_dialog(), but is built to be non-blocking with
      minimal CPU usage and automatic screen refresh for when the GUI is not
      being drawn directly to the screen. */

   RT_ASSERT(dialog);

   player = init_dialog (dialog, -1);

   while (update_dialog (player))
      gui_heartbeat ();

   index = player->obj;

   shutdown_dialog (player);

   return (index);
}

static INLINE int show_dialog (DIALOG *dialog)
{
   BITMAP *bmp;
   BITMAP *saved;
   int position;
   UINT16 x = x, y = y; /* Kill warnings. */
   BOOL moved = FALSE;
   int index = 0;

   RT_ASSERT(dialog);

   bmp = gui_get_screen ();

   saved = create_bitmap (bmp->w, bmp->h);
   if (!saved)
      WARN("Failed to create temporary background buffer; crash imminent");

   scare_mouse ();
   blit (bmp, saved, 0, 0, 0, 0, bmp->w, bmp->h);
   unscare_mouse ();

   position = get_config_hex ("dialogs", dialog[0].dp2, -1);

   if (position == -1)
   {
      centre_dialog (dialog);
   }
   else
   {
      x = (position >> 16);
      y = (position & 0x0000ffff);

      position_dialog (dialog, x, y);
   }

   dialog[0].dp3 = DATA_TO_FONT(LARGE_FONT);

   while (dialog[index].d1 != SL_FRAME_END)
   {
      /* Update colors. */

      DIALOG *object = &dialog[index];

      object->fg = GUI_TEXT_COLOR;
      object->bg = gui_bg_color;

      index++;
   }

   next:
   {
      index = run_dialog (dialog);
   
      scare_mouse ();
      blit (saved, bmp, 0, 0, 0, 0, saved->w, saved->h);
      unscare_mouse ();
   
      if (restart_dialog)
      {
         restart_dialog = FALSE;

         x = dialog_x;
         y = dialog_y;
   
         position_dialog (dialog, x, y);
   
         moved = TRUE;
   
         goto next;
      }
   }

   if (moved)
      set_config_hex ("dialogs", dialog[0].dp2, ((x << 16) | y));

   destroy_bitmap (saved);

   return (index);
}

static INLINE void update_menus (void)
{
#ifndef USE_OPENAL
   DISABLE_MENU_ITEM(options_audio_subsystem_menu_openal);
#endif

   SET_MENU_ITEM_ENABLED(options_audio_mixing_quality_menu_interpolation,
      (audio_subsystem != AUDIO_SUBSYSTEM_OPENAL));

   if (!audio_pseudo_stereo)
   {
      papu_swap_channels = FALSE;
      papu_spatial_stereo = FALSE;

      DISABLE_MENU_ITEM(options_audio_mixing_channels_menu_swap_channels);
      DISABLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_1);
      DISABLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_2);
      DISABLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_3);
   }
   else
   {
      ENABLE_MENU_ITEM(options_audio_mixing_channels_menu_swap_channels);
      ENABLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_1);
      ENABLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_2);
      ENABLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_3);
   }

   TOGGLE_MENU_ITEM(main_state_select_menu_0, (state_index == 0));
   TOGGLE_MENU_ITEM(main_state_select_menu_1, (state_index == 1));
   TOGGLE_MENU_ITEM(main_state_select_menu_2, (state_index == 2));
   TOGGLE_MENU_ITEM(main_state_select_menu_3, (state_index == 3));
   TOGGLE_MENU_ITEM(main_state_select_menu_4, (state_index == 4));
   TOGGLE_MENU_ITEM(main_state_select_menu_5, (state_index == 5));
   TOGGLE_MENU_ITEM(main_state_select_menu_6, (state_index == 6));
   TOGGLE_MENU_ITEM(main_state_select_menu_7, (state_index == 7));
   TOGGLE_MENU_ITEM(main_state_select_menu_8, (state_index == 8));
   TOGGLE_MENU_ITEM(main_state_select_menu_9, (state_index == 9));

   TOGGLE_MENU_ITEM(main_state_autosave_menu_disabled,   (input_autosave_interval == 0));
   TOGGLE_MENU_ITEM(main_state_autosave_menu_10_seconds, (input_autosave_interval == 10));
   TOGGLE_MENU_ITEM(main_state_autosave_menu_30_seconds, (input_autosave_interval == 30));
   TOGGLE_MENU_ITEM(main_state_autosave_menu_60_seconds, (input_autosave_interval == 60));

   TOGGLE_MENU_ITEM(main_replay_select_menu_0, (replay_index == 0));
   TOGGLE_MENU_ITEM(main_replay_select_menu_1, (replay_index == 1));
   TOGGLE_MENU_ITEM(main_replay_select_menu_2, (replay_index == 2));
   TOGGLE_MENU_ITEM(main_replay_select_menu_3, (replay_index == 3));
   TOGGLE_MENU_ITEM(main_replay_select_menu_4, (replay_index == 4));

   TOGGLE_MENU_ITEM(options_menu_status, video_display_status);

   TOGGLE_MENU_ITEM(options_gui_theme_menu_classic,         (last_theme == &classic_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_stainless_steel, (last_theme == &stainless_steel_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_zero_4,          (last_theme == &zero_4_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_panta,           (last_theme == &panta_theme));

   TOGGLE_MENU_ITEM(options_system_menu_ntsc_60_hz, (machine_type == MACHINE_TYPE_NTSC));
   TOGGLE_MENU_ITEM(options_system_menu_pal_50_hz,  (machine_type == MACHINE_TYPE_PAL));

   TOGGLE_MENU_ITEM(options_audio_menu_enabled, audio_enable_output);

   TOGGLE_MENU_ITEM(options_audio_subsystem_menu_none,   (audio_subsystem == AUDIO_SUBSYSTEM_NONE));
   TOGGLE_MENU_ITEM(options_audio_subsystem_menu_allegro,(audio_subsystem == AUDIO_SUBSYSTEM_ALLEGRO));
   TOGGLE_MENU_ITEM(options_audio_subsystem_menu_openal, (audio_subsystem == AUDIO_SUBSYSTEM_OPENAL));

   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_8000_hz,  (audio_sample_rate == 8000));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_11025_hz, (audio_sample_rate == 11025));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_16000_hz, (audio_sample_rate == 16000));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_22050_hz, (audio_sample_rate == 22050));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_32000_hz, (audio_sample_rate == 32000));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_44100_hz, (audio_sample_rate == 44100));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_48000_hz, (audio_sample_rate == 48000));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_80200_hz, (audio_sample_rate == 80200));
   TOGGLE_MENU_ITEM(options_audio_mixing_frequency_menu_96000_hz, (audio_sample_rate == 96000));

   TOGGLE_MENU_ITEM(options_audio_mixing_channels_menu_mono,                 !audio_pseudo_stereo);
   TOGGLE_MENU_ITEM(options_audio_mixing_channels_menu_stereo_mix,           (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_4));
   TOGGLE_MENU_ITEM(options_audio_mixing_channels_menu_pseudo_stereo_mode_1, (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_1));
   TOGGLE_MENU_ITEM(options_audio_mixing_channels_menu_pseudo_stereo_mode_2, (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_2));
   TOGGLE_MENU_ITEM(options_audio_mixing_channels_menu_stereo,               (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_3));
   TOGGLE_MENU_ITEM(options_audio_mixing_channels_menu_swap_channels,        papu_swap_channels);

   TOGGLE_MENU_ITEM(options_audio_mixing_quality_menu_low_8_bit,     (audio_sample_size == 8));
   TOGGLE_MENU_ITEM(options_audio_mixing_quality_menu_high_16_bit,   (audio_sample_size == 16));
   TOGGLE_MENU_ITEM(options_audio_mixing_quality_menu_interpolation, audio_interpolation);
   TOGGLE_MENU_ITEM(options_audio_mixing_quality_menu_dithering,     papu_dithering);

   TOGGLE_MENU_ITEM(options_audio_mixing_anti_aliasing_menu_disabled,     (papu_interpolate == 0));
   TOGGLE_MENU_ITEM(options_audio_mixing_anti_aliasing_menu_bilinear_2x,  (papu_interpolate == 1));
   TOGGLE_MENU_ITEM(options_audio_mixing_anti_aliasing_menu_bilinear_4x,  (papu_interpolate == 2));
   TOGGLE_MENU_ITEM(options_audio_mixing_anti_aliasing_menu_bilinear_8x,  (papu_interpolate == 3));
   TOGGLE_MENU_ITEM(options_audio_mixing_anti_aliasing_menu_bilinear_16x, (papu_interpolate == 4));

   TOGGLE_MENU_ITEM(options_audio_effects_menu_linear_echo,           papu_linear_echo);
   TOGGLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_1, (papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_1));
   TOGGLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_2, (papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_2));
   TOGGLE_MENU_ITEM(options_audio_effects_menu_spatial_stereo_mode_3, (papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_3));

   TOGGLE_MENU_ITEM(options_audio_filters_menu_low_pass_mode_1, (papu_get_filter_list () & PAPU_FILTER_LOW_PASS_MODE_1));
   TOGGLE_MENU_ITEM(options_audio_filters_menu_low_pass_mode_2, (papu_get_filter_list () & PAPU_FILTER_LOW_PASS_MODE_2));
   TOGGLE_MENU_ITEM(options_audio_filters_menu_low_pass_mode_3, (papu_get_filter_list () & PAPU_FILTER_LOW_PASS_MODE_3));
   TOGGLE_MENU_ITEM(options_audio_filters_menu_high_pass,       (papu_get_filter_list () & PAPU_FILTER_HIGH_PASS));

   TOGGLE_MENU_ITEM(options_audio_channels_menu_square_wave_a, papu_enable_square_1);
   TOGGLE_MENU_ITEM(options_audio_channels_menu_square_wave_b, papu_enable_square_2);
   TOGGLE_MENU_ITEM(options_audio_channels_menu_triangle_wave, papu_enable_triangle);
   TOGGLE_MENU_ITEM(options_audio_channels_menu_white_noise,   papu_enable_noise);
   TOGGLE_MENU_ITEM(options_audio_channels_menu_digital,       papu_enable_dmc);
   TOGGLE_MENU_ITEM(options_audio_channels_menu_extended,      papu_enable_exsound);

   TOGGLE_MENU_ITEM(options_audio_advanced_menu_ideal_triangle, papu_ideal_triangle);
   TOGGLE_MENU_ITEM(options_audio_advanced_menu_hard_sync,      audio_hard_sync);

#ifdef ALLEGRO_DOS

   TOGGLE_MENU_ITEM(options_video_driver_dos_menu_vga,           (gfx_driver->id == GFX_VGA));
   TOGGLE_MENU_ITEM(options_video_driver_dos_menu_vga_modex,     (gfx_driver->id == GFX_MODEX));
   TOGGLE_MENU_ITEM(options_video_driver_dos_menu_vesa,          (gfx_driver->id == GFX_VESA1));
   TOGGLE_MENU_ITEM(options_video_driver_dos_menu_vesa_2_banked, (gfx_driver->id == GFX_VESA2B));
   TOGGLE_MENU_ITEM(options_video_driver_dos_menu_vesa_2_linear, (gfx_driver->id == GFX_VESA2L));
   TOGGLE_MENU_ITEM(options_video_driver_dos_menu_vesa_3,        (gfx_driver->id == GFX_VESA3));
   TOGGLE_MENU_ITEM(options_video_driver_dos_menu_vesa_vbe_af,   (gfx_driver->id == GFX_VBEAF));

#endif   /* ALLEGRO_DOS */

#ifdef ALLEGRO_WINDOWS

   TOGGLE_MENU_ITEM(options_video_driver_windows_menu_directx,         (gfx_driver->id == GFX_DIRECTX));
   TOGGLE_MENU_ITEM(options_video_driver_windows_menu_directx_window,  (gfx_driver->id == GFX_DIRECTX_WIN));
   TOGGLE_MENU_ITEM(options_video_driver_windows_menu_directx_overlay, (gfx_driver->id == GFX_DIRECTX_OVL));
   TOGGLE_MENU_ITEM(options_video_driver_windows_menu_gdi,             (gfx_driver->id == GFX_GDI));

#endif   /* ALLEGRO_WINDOWS */

#ifdef ALLEGRO_LINUX

   TOGGLE_MENU_ITEM(options_video_driver_linux_menu_vga,         (gfx_driver->id == GFX_VGA));
   TOGGLE_MENU_ITEM(options_video_driver_linux_menu_vga_modex,   (gfx_driver->id == GFX_MODEX));
   TOGGLE_MENU_ITEM(options_video_driver_linux_menu_vesa_vbe_af, (gfx_driver->id == GFX_VBEAF));
#ifdef GFX_FBCON
   TOGGLE_MENU_ITEM(options_video_driver_linux_menu_framebuffer, (gfx_driver->id == GFX_FBCON));
#endif
#ifdef GFX_SVGALIB
   TOGGLE_MENU_ITEM(options_video_driver_linux_menu_svgalib,     (gfx_driver->id == GFX_SVGALIB));
#endif

#endif   /* ALLEGRO_LINUX */

#ifdef ALLEGRO_UNIX

   TOGGLE_MENU_ITEM(options_video_driver_unix_menu_x_windows,      (gfx_driver->id == GFX_XWINDOWS));
   TOGGLE_MENU_ITEM(options_video_driver_unix_menu_x_windows_full, (gfx_driver->id == GFX_XWINDOWS_FULLSCREEN));
   TOGGLE_MENU_ITEM(options_video_driver_unix_menu_x_dga,          (gfx_driver->id == GFX_XDGA));
   TOGGLE_MENU_ITEM(options_video_driver_unix_menu_x_dga_full,     (gfx_driver->id == GFX_XDGA_FULLSCREEN));
   TOGGLE_MENU_ITEM(options_video_driver_unix_menu_x_dga_2,        (gfx_driver->id == GFX_XDGA2));

#endif   /* ALLEGRO_UNIX */

   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_256_224,   ((SCREEN_W == 256)  && (SCREEN_H == 224)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_256_240,   ((SCREEN_W == 256)  && (SCREEN_H == 240)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_512_448,   ((SCREEN_W == 512)  && (SCREEN_H == 448)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_512_480,   ((SCREEN_W == 512)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_768_672,   ((SCREEN_W == 768)  && (SCREEN_H == 672)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_768_720,   ((SCREEN_W == 768)  && (SCREEN_H == 720)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_1024_896,  ((SCREEN_W == 1024) && (SCREEN_H == 896)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_1024_960,  ((SCREEN_W == 1024) && (SCREEN_H == 960)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_1280_1120, ((SCREEN_W == 1280) && (SCREEN_H == 1120)));
   TOGGLE_MENU_ITEM(options_video_resolution_proportionate_menu_1280_1200, ((SCREEN_W == 1280) && (SCREEN_H == 1200)));

   TOGGLE_MENU_ITEM(options_video_resolution_menu_320_240,   ((SCREEN_W == 320)  && (SCREEN_H == 240)));
   TOGGLE_MENU_ITEM(options_video_resolution_menu_640_480,   ((SCREEN_W == 640)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(options_video_resolution_menu_800_600,   ((SCREEN_W == 800)  && (SCREEN_H == 600)));
   TOGGLE_MENU_ITEM(options_video_resolution_menu_1024_768,  ((SCREEN_W == 1024) && (SCREEN_H == 768)));
   TOGGLE_MENU_ITEM(options_video_resolution_menu_1152_864,  ((SCREEN_W == 1152) && (SCREEN_H == 864)));
   TOGGLE_MENU_ITEM(options_video_resolution_menu_1280_1024, ((SCREEN_W == 1280) && (SCREEN_H == 1024)));
   TOGGLE_MENU_ITEM(options_video_resolution_menu_1600_1200, ((SCREEN_W == 1600) && (SCREEN_H == 1200)));

   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_400_300,  ((SCREEN_W == 400)  && (SCREEN_H == 300)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_480_360,  ((SCREEN_W == 480)  && (SCREEN_H == 360)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_512_384,  ((SCREEN_W == 512)  && (SCREEN_H == 384)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_640_400,  ((SCREEN_W == 640)  && (SCREEN_H == 400)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_720_480,  ((SCREEN_W == 720)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_720_576,  ((SCREEN_W == 720)  && (SCREEN_H == 576)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_848_480,  ((SCREEN_W == 848)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_1280_720, ((SCREEN_W == 1280) && (SCREEN_H == 720)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_1280_960, ((SCREEN_W == 1280) && (SCREEN_H == 960)));
   TOGGLE_MENU_ITEM(options_video_resolution_extended_menu_1360_768, ((SCREEN_W == 1360) && (SCREEN_H == 768)));

   TOGGLE_MENU_ITEM(options_video_colors_menu_paletted_8_bit,    (video_get_color_depth () == 8));
   TOGGLE_MENU_ITEM(options_video_colors_menu_true_color_15_bit, (video_get_color_depth () == 15));
   TOGGLE_MENU_ITEM(options_video_colors_menu_true_color_16_bit, (video_get_color_depth () == 16));
   TOGGLE_MENU_ITEM(options_video_colors_menu_true_color_32_bit, (video_get_color_depth () == 32));

   TOGGLE_MENU_ITEM(options_video_blitter_menu_automatic,       (video_get_blitter () == VIDEO_BLITTER_AUTOMATIC));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_normal,          (video_get_blitter () == VIDEO_BLITTER_NORMAL));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_stretched,       (video_get_blitter () == VIDEO_BLITTER_STRETCHED));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_interpolated_2x, (video_get_blitter () == VIDEO_BLITTER_INTERPOLATED_2X));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_interpolated_3x, (video_get_blitter () == VIDEO_BLITTER_INTERPOLATED_3X));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_2xsoe,           (video_get_blitter () == VIDEO_BLITTER_2XSOE));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_2xscl,           (video_get_blitter () == VIDEO_BLITTER_2XSCL));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_super_2xsoe,     (video_get_blitter () == VIDEO_BLITTER_SUPER_2XSOE));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_super_2xscl,     (video_get_blitter () == VIDEO_BLITTER_SUPER_2XSCL));
   TOGGLE_MENU_ITEM(options_video_blitter_menu_ultra_2xscl,     (video_get_blitter () == VIDEO_BLITTER_ULTRA_2XSCL));

   TOGGLE_MENU_ITEM(options_video_filters_menu_scanlines_25_percent,  (video_get_filter_list () & VIDEO_FILTER_SCANLINES_LOW));
   TOGGLE_MENU_ITEM(options_video_filters_menu_scanlines_50_percent,  (video_get_filter_list () & VIDEO_FILTER_SCANLINES_MEDIUM));
   TOGGLE_MENU_ITEM(options_video_filters_menu_scanlines_100_percent, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_HIGH));

   TOGGLE_MENU_ITEM(options_video_menu_vsync, video_enable_vsync);

   TOGGLE_MENU_ITEM(options_video_palette_menu_ntsc_color,     (video_get_palette_id () == DATA_INDEX(DEFAULT_PALETTE)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_ntsc_grayscale, (video_get_palette_id () == DATA_INDEX(GRAYSCALE_PALETTE)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_gnuboy,         (video_get_palette_id () == DATA_INDEX(GNUBOY_PALETTE)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_nester,         (video_get_palette_id () == DATA_INDEX(NESTER_PALETTE)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_nesticle,       (video_get_palette_id () == DATA_INDEX(NESTICLE_PALETTE)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_modern_ntsc,    (video_get_palette_id () == DATA_INDEX(MODERN_NTSC_PALETTE)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_modern_pal,     (video_get_palette_id () == DATA_INDEX(MODERN_PAL_PALETTE)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_ega_mode_1,     (video_get_palette_id () == DATA_INDEX(EGA_PALETTE_1)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_ega_mode_2,     (video_get_palette_id () == DATA_INDEX(EGA_PALETTE_2)));
   TOGGLE_MENU_ITEM(options_video_palette_menu_custom,         (video_get_palette_id () == -1));

   TOGGLE_MENU_ITEM(options_video_advanced_menu_force_window, video_force_window);

   TOGGLE_MENU_ITEM(options_video_layers_menu_sprites_a,  ppu_enable_sprite_layer_a);
   TOGGLE_MENU_ITEM(options_video_layers_menu_sprites_b,  ppu_enable_sprite_layer_b);
   TOGGLE_MENU_ITEM(options_video_layers_menu_background, ppu_enable_background_layer);

   TOGGLE_MENU_ITEM(options_input_menu_enable_zapper, input_enable_zapper);

   TOGGLE_MENU_ITEM(netplay_protocol_menu_tcpip, (netplay_protocol == NETPLAY_PROTOCOL_TCPIP));
   TOGGLE_MENU_ITEM(netplay_protocol_menu_spx,   (netplay_protocol == NETPLAY_PROTOCOL_SPX));
}

static INLINE void draw_background (void)
{
   BITMAP *bmp;

   if (rom_is_loaded)
      return;

   bmp = gui_get_screen ();

   rectfill (bmp, 0, 0, bmp->w, bmp->h, GUI_BACKGROUND_COLOR);

   if (background_image)
   {
      stretch_blit (background_image, bmp, 0, 0, background_image->w,
         background_image->h, 0, 0, bmp->w, bmp->h);
   }
}

int gui_init (void)
{
   /* Set up replacement objects. */
   gui_menu_draw_menu = sl_draw_menu;
   gui_menu_draw_menu_item = sl_draw_menu_item;

   /* Set up menus & dialogs. */
   load_menus ();
   load_dialogs ();

#ifdef ALLEGRO_DOS

   DISABLE_SUBMENU(options_video_driver_windows_menu);
   DISABLE_SUBMENU(options_video_driver_linux_menu);
   DISABLE_SUBMENU(options_video_driver_unix_menu);
   DISABLE_MENU_ITEM(options_video_advanced_menu_force_window);
   DISABLE_SUBMENU(netplay_menu);

#endif   /* ALLEGRO_DOS */

#ifdef ALLEGRO_WINDOWS

   DISABLE_SUBMENU(options_video_driver_dos_menu);
   DISABLE_SUBMENU(options_video_driver_linux_menu);
   DISABLE_SUBMENU(options_video_driver_unix_menu);

#endif   /* ALLEGRO_WINDOWS */

#ifdef ALLEGRO_UNIX

   DISABLE_SUBMENU(options_video_driver_dos_menu);
   DISABLE_SUBMENU(options_video_driver_windows_menu);
 
#ifdef ALLEGRO_LINUX

#ifndef GFX_FBCON
   DISABLE_MENU_ITEM(options_video_driver_linux_menu_framebuffer);
#endif
#ifndef GFX_SVGALIB
   DISABLE_MENU_ITEM(options_video_driver_linux_menu_svgalib);
#endif

#else /* ALLEGRO_LINUX */

   DISABLE_SUBMENU(options_video_driver_linux_menu);

#endif   /* !ALLEGRO_LINUX */

#endif   /* ALLEGRO_UNIX */

   /* Select default palette. */
   CHECK_MENU_ITEM(options_video_palette_menu_modern_ntsc);

   /* Load configuration */
   gui_theme_id = get_config_int ("gui", "theme", 0);

   /* Cheap hack to fix palette. */
   gui_is_active = TRUE;
   set_theme ();
   gui_is_active = FALSE;

   return (0);
}

void gui_exit (void)
{
   unload_menus ();
   unload_dialogs ();
}

static INLINE void cycle_video (void);

int show_gui (BOOL first_run)
{
   STRING save_path;
   STRING ip_address;

   gui_needs_restart = FALSE;
   gui_is_active = TRUE;

   audio_suspend ();

   want_exit = FALSE;

   /* Set up menus. */
   update_menus ();

   if (!rom_is_loaded)
   {
      DISABLE_MENU_ITEM(main_menu_resume);
      DISABLE_MENU_ITEM(main_menu_reset);
      DISABLE_MENU_ITEM(main_menu_snapshot);
      DISABLE_SUBMENU(main_state_menu);
      DISABLE_SUBMENU(main_replay_menu);
      DISABLE_MENU_ITEM(options_menu_patches);
   }

   cycle_video ();

   if (first_run)
   {
      alert ("FakeNES version " VERSION_STRING " " ALLEGRO_PLATFORM_STR, "",
         "Get the latest from http://fakenes.sourceforge.net/.", "&OK",
            NULL, 'o', 0);
   }

   run_dialog (main_dialog);

   /* Save configuration. */
   memset (save_path, 0, sizeof (save_path));
   strncpy (save_path, get_config_string ("gui", "save_path", "./"),
      sizeof (save_path) - 1);
   set_config_string ("gui", "save_path", save_path);

   memset (ip_address, 0, sizeof (ip_address));
   strncpy (ip_address, get_config_string ("netplay", "ip_address",
      "0.0.0.0"), sizeof (ip_address) - 1);
   set_config_string ("netplay", "ip_address", ip_address);

   set_config_int ("gui", "theme", gui_theme_id);

   /* Shut down GUI. */
   gui_is_active = FALSE;

   cycle_video ();

   audio_resume ();

   return (want_exit);
}

void gui_handle_keypress (int c)
{
   switch ((c >> 8))
   {
      case KEY_F1:
      {
         /* Save snapshot. */
         main_menu_snapshot ();

         break;
      }

      case KEY_F2:
      {
         /* Toggle status display. */
         options_menu_status ();

         break;
      }

      case KEY_F3:
      {
         /* Quick save state. */
         main_state_menu_save ();

         break;
      }

      case KEY_F4:
      {
         /* Quick load state. */

         if (!(input_mode & INPUT_MODE_REPLAY))
            main_state_menu_restore ();

         break;
      }

      case KEY_F7:
      {
         /* Toggle sprites. */

         options_video_layers_menu_sprites_a ();
         options_video_layers_menu_sprites_b ();

         break;
      }

      case KEY_F8:
      {
         /* Toggle background. */
         options_video_layers_menu_background ();

         break;
      }

      case KEY_F9:
      {
         /* Toggle half speed mode. */

         timing_half_speed = !timing_half_speed;

         if (!gui_is_active)
         {
            suspend_timing ();
            resume_timing ();
         }

         audio_exit ();
         audio_init ();

         papu_reinit ();

         break;
      }

      case KEY_F12:
      {
         /* Start/stop replay recording. */

         if (!(input_mode & INPUT_MODE_REPLAY_PLAY))
         {
            if (input_mode & INPUT_MODE_REPLAY_RECORD)
               main_replay_record_menu_stop ();
            else
               main_replay_record_menu_start ();

            break;
         }
      }

      case KEY_0:
      case KEY_1:
      case KEY_2:
      case KEY_3:
      case KEY_4:
      case KEY_5:
      case KEY_6:
      case KEY_7:
      case KEY_8:
      case KEY_9:
      {
         /* Select state slot. */

         if (!(input_mode & INPUT_MODE_CHAT))
         {
            state_index = ((c >> 8) - KEY_0);
    
            message_local ("Machine state slot set to %d.",
               state_index);
         }

         break;
      }

      default:
         break;
   }
}

void gui_stop_replay (void)
{
   main_replay_play_menu_stop ();
}

static BOOL client_connected = FALSE;

/* TODO: Change this to a normal function, remove it from the main dialog,
   and place a call to it in gui_heartbeat(). */
static int netplay_handler (int message, DIALOG *dialog, int key)
{
   if (netplay_server_active && !client_connected)
   {
      client_connected = netplay_poll_server ();

      if (client_connected)
      {
         message_local ("Accepted connection from client.");

         if (main_menu_load_rom () == D_CLOSE)
            return (D_CLOSE);
         else
            message_local ("NetPlay session canceled.");
      }
   }

   return (D_O_K);
}

/* --- Utility functions. --- */

/* Max in-file title length for save states and replays. */
#define SAVE_TITLE_SIZE    16

/* Text that appears in "unused" menu slots for states and replays. */
#define UNUSED_SLOT_TEXT   "Empty"

static INLINE BOOL fnss_save (PACKFILE *file, const UCHAR *title)
{
   UINT16 version;

   /* Core FNSS (FakeNES save state) saving code.  Returns TRUE if the save
      suceeded or FALSE if the save failed (which can't happen). */

   RT_ASSERT(file);
   RT_ASSERT(title);

   /* Set version. */
   version = 0x0100;

   /* Write signature. */
   pack_fwrite ("FNSS", 4, file);
   
   /* Write version number. */
   pack_iputw (version, file);
   
   /* Write title. */
   pack_fwrite (title, SAVE_TITLE_SIZE, file);
   
   /* Write CRC32s. */
   pack_iputl (global_rom.trainer_crc32, file);
   pack_iputl (global_rom.prg_rom_crc32, file);
   pack_iputl (global_rom.chr_rom_crc32, file);
   
   /* Write CPU chunk. */
   pack_fwrite ("CPU\0", 4, file);
   cpu_save_state (file, version);
   
   /* Write PPU chunk. */
   pack_fwrite ("PPU\0", 4, file);
   ppu_save_state (file, version);
   
   /* Write PAPU chunk. */
   pack_fwrite ("PAPU", 4, file);
   papu_save_state (file, version);
   
   /* Write MMC chunk. */
   pack_fwrite ("MMC\0", 4, file);
   mmc_save_state (file, version);
   
   /* Write CTRL chunk. */
   pack_fwrite ("CTRL", 4, file);
   input_save_state (file, version);

   /* Return success. */
   return (TRUE);
}

static INLINE BOOL fnss_load (PACKFILE *file)
{
   /* Core FNSS (FakeNES save state) loading code.  Returns TRUE if the load
      suceeded or FALSE if the load failed. */

   UINT8 signature[4];
   UINT16 version;
   UCHAR title[SAVE_TITLE_SIZE];
   UINT32 trainer_crc;
   UINT32 prg_rom_crc;
   UINT32 chr_rom_crc;

   RT_ASSERT(file);

   /* Fetch signature. */
   pack_fread (signature, 4, file);

   /* Verify signature. */
   if (strncmp (signature, "FNSS", 4))
   {
      gui_message (GUI_ERROR_COLOR, "Machine state file is invalid.");
    
      return (FALSE);
   }

   /* Fetch version number. */
   version = pack_igetw (file);

   /* Verify version number. */
   if (version > 0x0100)
   {
      gui_message (GUI_ERROR_COLOR, "Machine state file is of a future "
         "version.");
    
      return (FALSE);
   }

   /* Fetch save title. */
   pack_fread (title, SAVE_TITLE_SIZE, file);

   /* Fetch CRC32s. */
   trainer_crc = pack_igetl (file);
   prg_rom_crc = pack_igetl (file);
   chr_rom_crc = pack_igetl (file);

   /* Verify CRC32s. */
   if ((trainer_crc != global_rom.trainer_crc32) ||
       (prg_rom_crc != global_rom.prg_rom_crc32) ||
       (chr_rom_crc != global_rom.chr_rom_crc32))
   {
      gui_message (GUI_ERROR_COLOR, "Machine state file is for a different "
         "ROM.");
    
      return (FALSE);
   }

   /* Reset the virtual machine to it's initial state. */
   machine_reset ();
    
   /* We ignore signatures for now, this will be used in the future to load
      chunks in any order. */

   /* Load CPU chunk. */
   pack_fread (signature, 4, file);
   cpu_load_state (file, version);

   /* Load PPU chunk. */
   pack_fread (signature, 4, file);
   ppu_load_state (file, version);

   /* Load PAPU chunk. */
   pack_fread (signature, 4, file);
   papu_load_state (file, version);

   /* Load MMC chunk. */
   pack_fread (signature, 4, file);
   mmc_load_state (file, version);

   /* Load CTRL chunk. */
   pack_fread (signature, 4, file);
   input_load_state (file, version);

   /* Return success. */
   return (TRUE);
}

static INLINE UCHAR *get_save_filename (UCHAR *filename, const UCHAR *ext,
   int size)
{
   /* THis function builds a path and filename suitable for the storage of
      save data, using the name of the ROM combined with the extension
      'ext', and stores up to 'size' Unicode characters in 'filename'. */

   USTRING path;

   RT_ASSERT(filename);
   RT_ASSERT(ext);

   USTRING_CLEAR(path);
   ustrncat (path, get_config_string ("gui", "save_path", "./"), (sizeof
      (path) - 1));
   put_backslash (path);
   ustrncat (path, get_filename (global_rom.filename), (sizeof (path) - 1));
   
   replace_extension (path, path, ext, sizeof (path));

   /* Copy to output. */
   USTRING_CLEAR_SIZE(path, size);
   ustrncat (filename, path, (size - 1));

   return (filename);
}

static INLINE UCHAR *get_state_filename (UCHAR *filename, int index, int
   size)
{

   /* This function generates the path and filename for the state file
      associated with the state slot 'index'.  State files are stored in
      the save path, and have a .fn# extension. */

   USTRING ext;

   RT_ASSERT(filename);

   /* Build extension. */
   uszprintf (ext, sizeof (ext), "fn%d", index);

   /* Generate filename. */
   get_save_filename (filename, ext, size);

   return (filename);
}

static INLINE UCHAR *get_replay_filename (UCHAR *filename, int index, int
   size)
{
   /* This function generates the path and filename for the replay file
      associated with the replay slot 'index'.  Replay files are stored in
      the save path, and have a .fr# extension. */

   USTRING ext;

   RT_ASSERT(filename);

   /* Build extension. */
   uszprintf (ext, sizeof (ext), "fr%d", index);

   /* Generate filename. */
   get_save_filename (filename, ext, size);

   return (filename);
}

static INLINE UCHAR *get_save_title (const UCHAR *filename, UCHAR *title,
   int size)
{
   /* This function retrives the save title from the state or replay file
      specified in 'filename'.  Returns either the retrieved title,
      "Untitled" if the retrieved title was zero-length, OR
      UNUSED_SLOT_TEXT if the file could not be opened. */

   PACKFILE *file;
   USTRING save_title;

   RT_ASSERT(filename);
   RT_ASSERT(title);

   USTRING_CLEAR(save_title);

   file = pack_fopen (filename, "r");

   if (file)
   {
      UINT8 signature[4];
      UINT16 version;

      /* Probably don't need to verify these... */
      pack_fread (signature, 4, file);
      version = pack_igetw (file);

      pack_fread (save_title, SAVE_TITLE_SIZE, file);

      pack_fclose (file);

      if (ustrlen (save_title) == 0)
         ustrncat (save_title, "Untitled", (sizeof (save_title) - 1));
   }
   else
   {
      ustrncat (save_title, UNUSED_SLOT_TEXT, (sizeof (save_title) - 1));
   }

   /* Copy to output. */
   ustrzncpy (title, size, save_title, sizeof (save_title));

   return (title);
}

static INLINE UCHAR *fix_save_title (UCHAR *title, int size)
{
   /* This function compares 'title' against UNUSED_SLOT_TEXT, and if they
      are found to be the same, it replaces it with "Untitled" instead.

      Without this function, we might end up with untitled state and replay
      files with UNUSED_SLOT_TEXT as their title (lifted directly from their
      cooresponding menu slot), and that would be icky.

      If 'title' is found to not be equal to UNUSED_SLOT_TEXT, then it will
      remain unchanged, but will still be returned. */

   RT_ASSERT(title);

   if (ustrncmp (title, UNUSED_SLOT_TEXT, size) == 0)
   {
      USTRING_CLEAR_SIZE(title, size);
      ustrncat (title, "Untitled", (size - 1));
   }

   return (title);
}

static INLINE void set_autosave (int interval)
{
   /* This function simply sets the state autosave interval to 'interval'
      seconds (in game speed, not real world speed. */

   input_autosave_interval = interval;
   update_menus ();

   if (interval <= 0)
      message_local ("Autosave disabled.");
   else
      message_local ("Autosave interval set to %d seconds.", interval);
}

static INLINE void cycle_audio (void)
{
   /* This function cycles (removes and reinstalls) the audio subsystem so
      that any major parameter changes can take effect. */

   audio_exit ();
   audio_init ();

   papu_reinit ();
}

static INLINE void cycle_video (void)
{
   BITMAP *bmp;

   /* This function fixes any problems with the GUI after a video change
      has taken effect (such as changing the screen color depth).  It also
      gets called each time you enter and exit the GUI. */

   bmp = gui_get_screen ();

   clear_bitmap (bmp);
   video_blit (bmp);

   if (gui_is_active)
   {
      update_colors ();

      draw_background ();

      redraw ();

      if (gui_mouse_sprite)
         set_mouse_sprite (gui_mouse_sprite);
      set_mouse_sprite_focus (8, 8);
      show_mouse (bmp);

      message_local ("%dx%d %d-bit, %s.", bmp->w, bmp->h, bitmap_color_depth
         (bmp), gfx_driver->name);
   }
   else
   {
      show_mouse (NULL);
      set_mouse_sprite_focus (0, 0);
   }

   refresh ();
}

static INLINE const UCHAR *get_enabled_text (BOOL value)
{
   /* This simple function returns either "enabled" or "disabled", depending
      on the value of the boolean parameter 'value'. */

   return ((value ? "enabled" : "disabled"));
}

static INLINE const UCHAR *get_enabled_text_ex (BOOL value, const UCHAR
   *enabled_text)
{
   /* Identical to the above function, except that it returns 'enabled_text'
      instead of "enabled". */

   RT_ASSERT(enabled_text);

   return ((value ? enabled_text : "disabled"));
}

/* --- Menu handlers. --- */

/* Number of replay slots available in the menu. */
#define REPLAY_SLOTS    5

#define REPLAY_SELECT_MENU_HANDLER(index) \
   static int main_replay_select_menu_##index (void)  \
   {  \
      replay_index = index;   \
      update_menus ();  \
      message_local ("Replay slot set to %d.", index);   \
      return (D_O_K);   \
   }

REPLAY_SELECT_MENU_HANDLER(0)
REPLAY_SELECT_MENU_HANDLER(1)
REPLAY_SELECT_MENU_HANDLER(2)
REPLAY_SELECT_MENU_HANDLER(3)
REPLAY_SELECT_MENU_HANDLER(4)

#undef REPLAY_SELECT_MENU_HANDLER

static USTRING replay_titles[REPLAY_SLOTS];
static USTRING replay_menu_texts[REPLAY_SLOTS];

static int main_replay_menu_select (void)
{
   int index;

   for (index = 0; index < REPLAY_SLOTS; index++)
   {
      UCHAR *title;
      UCHAR *text;
      USTRING filename;

      title = replay_titles[index];
      text = replay_menu_texts[index];

      /* Generate filename. */
      get_replay_filename (filename, index, sizeof (filename));

      /* Retrieve title. */
      get_save_title (filename, title, USTRING_SIZE);

      /* Build menu text. */
      uszprintf (text, USTRING_SIZE, "&%d: %s", index, title);

      /* Update menu. */
      main_replay_select_menu[index].text = text;
   }

   return (D_O_K);
}

static int main_replay_record_menu_start (void)
{
   USTRING title;
   USTRING filename;
   PACKFILE *file;

   /* Duplicate title. */
   ustrncpy (title, replay_titles[replay_index], sizeof (title));

   /* Patch up duplicate. */
   fix_save_title (title, sizeof (title));

   if (gui_is_active)
   {
      /* Allow user to customize title before save. */

      main_replay_record_start_dialog[4].d1 = (SAVE_TITLE_SIZE - 1);
      main_replay_record_start_dialog[4].dp = title;

      if (show_dialog (main_replay_record_start_dialog) != 5)
         return (D_O_K);
   }

   /* Generate filename. */
   get_replay_filename (filename, replay_index, sizeof (filename));

   file = pack_fopen (filename, "w");

   if (file)
   {
      PACKFILE *chunk;

      /* Save state. */
      fnss_save (file, title);

      /* Open REPL chunk. */
      pack_fwrite ("REPL", 4, file);
      chunk = pack_fopen_chunk (file, FALSE);

      replay_file = file;
      replay_file_chunk = chunk;

      DISABLE_MENU_ITEM(main_replay_record_menu_start);
      ENABLE_MENU_ITEM(main_replay_record_menu_stop);
      DISABLE_MENU_ITEM(main_menu_load_rom);
      DISABLE_SUBMENU(main_replay_select_menu);
      DISABLE_SUBMENU(main_replay_play_menu);
      DISABLE_SUBMENU(main_state_autosave_menu);
      DISABLE_SUBMENU(netplay_menu);

      /* Enter replay recording mode. */
      input_mode |= INPUT_MODE_REPLAY;
      input_mode |= INPUT_MODE_REPLAY_RECORD;
    
      message_local ("Replay recording session started.");
    
      /* Update save state titles. */
      main_replay_menu_select ();

      return (D_CLOSE);
   }
   else
   {
      gui_message (GUI_ERROR_COLOR, "Failed to open new machine state "
         "file.");
   }

   return (D_O_K);
}

static int main_replay_record_menu_stop (void)
{
   pack_fclose_chunk (replay_file_chunk);
   pack_fclose (replay_file);

   /* Exit replay recording mode. */
   input_mode &= ~INPUT_MODE_REPLAY;
   input_mode &= ~INPUT_MODE_REPLAY_RECORD;

   ENABLE_MENU_ITEM(main_replay_record_menu_start);
   DISABLE_MENU_ITEM(main_replay_record_menu_stop);
   ENABLE_MENU_ITEM(main_menu_load_rom);
   ENABLE_SUBMENU(main_replay_select_menu);
   ENABLE_SUBMENU(main_replay_play_menu);
   ENABLE_SUBMENU(main_state_autosave_menu);
   ENABLE_SUBMENU(netplay_menu);

   message_local ("Replay recording session stopped.");

   return (D_O_K);
}

static int main_replay_play_menu_start (void)
{
   USTRING filename;
   PACKFILE *file;
   UINT8 signature[4];

   /* Generate filename. */
   get_replay_filename (filename, replay_index, sizeof (filename));

   file = pack_fopen (filename, "r");
   if (!file)
   {
      gui_message (GUI_ERROR_COLOR, "Machine state file does not exist.");

      return (D_O_K);
   }

   /* Load state. */
   if (!fnss_load (file))
   {
      /* Load failed. */
      pack_fclose (file);

      return (D_O_K);
   }

   /* Load suceeded. */

   /* Open REPL chunk. */
   pack_fread (signature, 4, file);

   /* Verify REPL chunk. */
   if (strncmp (signature, "REPL", 4))
   {
      gui_message (GUI_ERROR_COLOR, "Machine state file is missing replay "
         "chunk.");
    
      pack_fclose (file);

      return (D_O_K);
   }

   replay_file_chunk = pack_fopen_chunk (file, FALSE);

   replay_file = file;

   DISABLE_MENU_ITEM(main_replay_play_menu_start);
   ENABLE_MENU_ITEM(main_replay_play_menu_stop);
   DISABLE_MENU_ITEM(main_menu_load_rom);
   DISABLE_SUBMENU(main_replay_select_menu);
   DISABLE_SUBMENU(main_replay_record_menu);
   DISABLE_MENU_ITEM(main_state_menu_restore);
   DISABLE_SUBMENU(netplay_menu);

   /* Enter replay playback mode. */
   input_mode &= ~INPUT_MODE_PLAY;
   input_mode |= INPUT_MODE_REPLAY;
   input_mode |= INPUT_MODE_REPLAY_PLAY;

   message_local ("Replay playback started.");

   return (D_CLOSE);
}

static int main_replay_play_menu_stop (void)
{
   pack_fclose_chunk (replay_file_chunk);
   pack_fclose (replay_file);

   /* Exit replay playback mode. */
   if (!(input_mode & INPUT_MODE_CHAT))
      input_mode |= INPUT_MODE_PLAY;
   input_mode &= ~INPUT_MODE_REPLAY;
   input_mode &= ~INPUT_MODE_REPLAY_PLAY;

   ENABLE_MENU_ITEM(main_replay_play_menu_start);
   DISABLE_MENU_ITEM(main_replay_play_menu_stop);
   ENABLE_MENU_ITEM(main_menu_load_rom);
   ENABLE_SUBMENU(main_replay_select_menu);
   ENABLE_SUBMENU(main_replay_record_menu);
   ENABLE_MENU_ITEM(main_state_menu_restore);
   ENABLE_SUBMENU(netplay_menu);

   if (gui_is_active)
      message_local ("Replay playback stopped.");
   else
      message_local ("Replay playback finished.");

   return (D_O_K);
}

static int main_menu_load_rom (void)
{
   USTRING path;
   BITMAP *bmp;
   int w, h;
   int result;
   USTRING scratch;

   /* Retrive path from configuration file. */
   USTRING_CLEAR(path);
   ustrncat (path, get_config_string ("gui", "load_rom_path", "/"), (sizeof
      (path) - 1));

   /* Get drawing surface. */
   bmp = gui_get_screen ();

   /* Calculate file selector dimensions. */
   w = ROUND((bmp->w * 0.80f));
   h = ROUND((bmp->h * 0.67f));

#ifdef USE_ZLIB
   result = file_select_ex ("iNES ROMs (*.NES, *.GZ, *.ZIP)", path,
      "NES;nes;GZ;gz;ZIP;zip", sizeof (path), w, h);
#else
   result = file_select_ex ("iNES ROMs (*.NES)", path, "NES;nes", sizeof
      (path), w, h);
#endif

   /* Update path. */
   set_config_string ("gui", "load_rom_path", replace_filename (scratch,
      path, "", sizeof (scratch)));

   if (result != 0)
   {
      /* Dialog was OK'ed. */

      ROM rom;

      if (load_rom (path, &rom) != 0)
      {
         gui_message (GUI_ERROR_COLOR, "Failed to load ROM!");

         /* Shut down netplay engine. */
         if (netplay_server_active)
            netplay_close_server ();
         if (netplay_client_active)
            netplay_close_client ();

         return (D_O_K);
      }
      else
      {
         if (rom_is_loaded)
         {
            /* Close currently open ROM and save data. */

            /* Save SRAM. */
            if (global_rom.sram_flag)
               sram_save (global_rom.filename);

            /* Save patch table. */
            patches_save (global_rom.filename);

            free_rom (&global_rom);
         }

         memcpy (&global_rom, &rom, sizeof (ROM));

         /* Update state titles. */
         main_state_menu_select ();
         /* Update replay titles. */
         main_replay_menu_select ();

         rom_is_loaded = TRUE;

         /* Initialize machine. */
         machine_init ();

         if (!netplay_server_active && !netplay_client_active)
         {
            ENABLE_MENU_ITEM(main_menu_resume);
            ENABLE_MENU_ITEM(main_menu_reset);
            ENABLE_SUBMENU(main_state_menu);
            ENABLE_SUBMENU(main_replay_menu);
            ENABLE_MENU_ITEM(main_menu_snapshot);
            ENABLE_MENU_ITEM(options_menu_patches);
         }

         /* Update window title. */
         uszprintf (scratch, sizeof (scratch), "FakeNES - %s", get_filename
            (global_rom.filename));
         set_window_title (scratch);

         return (D_CLOSE);
      }
   }

   /* Dialog was cancelled. */
   return (D_O_K);
}

static int main_menu_resume (void)
{
    return (D_CLOSE);
}

static int main_menu_reset (void)
{
    machine_reset ();

    return (D_CLOSE);
}

#define STATE_SLOTS  10

#define STATE_SELECT_MENU_HANDLER(index)  \
   static int main_state_select_menu_##index (void)   \
   {  \
      state_index = index;  \
      update_menus ();  \
      message_local ("Machine state slot set to %d.", index);  \
      return (D_O_K);   \
   }

STATE_SELECT_MENU_HANDLER(0);
STATE_SELECT_MENU_HANDLER(1);
STATE_SELECT_MENU_HANDLER(2);
STATE_SELECT_MENU_HANDLER(3);
STATE_SELECT_MENU_HANDLER(4);
STATE_SELECT_MENU_HANDLER(5);
STATE_SELECT_MENU_HANDLER(6);
STATE_SELECT_MENU_HANDLER(7);
STATE_SELECT_MENU_HANDLER(8);
STATE_SELECT_MENU_HANDLER(9);

#undef STATE_MENU_HANDLER

static USTRING state_titles[STATE_SLOTS];
static USTRING state_menu_texts[STATE_SLOTS];

static int main_state_menu_select (void)
{
   int index;

   for (index = 0; index < STATE_SLOTS; index++)
   {
      UCHAR *title;
      UCHAR *text;
      USTRING filename;

      title = state_titles[index];
      text = state_menu_texts[index];

      /* Generate filename. */
      get_state_filename (filename, index, sizeof (filename));

      /* Retrieve title. */
      get_save_title (filename, title, USTRING_SIZE);

      /* Build menu text. */
      uszprintf (text, USTRING_SIZE, "&%d: %s", index, title);

      /* Update menu. */
      main_state_select_menu[index].text = text;
   }

   return (D_O_K);
}

static int main_state_menu_save (void)
{
   USTRING title;
   USTRING filename;
   PACKFILE *file;

   /* Duplicate title. */
   ustrncpy (title, state_titles[state_index], sizeof (title));

   /* Patch up duplicate. */
   fix_save_title (title, sizeof (title));

   if (gui_is_active)
   {
      /* Allow user to customize title before save. */

      main_state_save_dialog[4].d1 = (SAVE_TITLE_SIZE - 1);
      main_state_save_dialog[4].dp = title;

      if (show_dialog (main_state_save_dialog) != 5)
         return (D_O_K);
   }

   /* Generate filename. */
   get_state_filename (filename, state_index, sizeof (filename));

   file = pack_fopen (filename, "w");

   if (file)
   {
      /* Save state. */
      fnss_save (file, title);

      pack_fclose (file);

      /* Update state titles. */
      main_state_menu_select ();

      if (!input_autosave_triggered)
         message_local ("Machine state saved in slot %d.", state_index);

      return (D_CLOSE);
   }
   else
   {
      gui_message (GUI_ERROR_COLOR, "Failed to open new machine state "
         "file.");
   }

   return (D_O_K);
}

static int main_state_menu_restore (void)
{
   USTRING filename;
   PACKFILE *file;

   /* Generate filename. */
   get_state_filename (filename, state_index, sizeof (filename));

   file = pack_fopen (filename, "r");
   if (!file)
   {
      gui_message (GUI_ERROR_COLOR, "Machine state file does not exist.");

      return (D_O_K);
   }

   /* Load state. */
   if (!fnss_load (file))
   {
      /* Load failed. */
      pack_fclose (file);

      return (D_O_K);
   }

   /* Load suceeded. */
   pack_fclose (file);

   message_local ("Machine state loaded from slot %d.", state_index);

   return (D_CLOSE);
}

static int main_state_autosave_menu_disabled (void)
{
   set_autosave (0);

   return (D_O_K);
}

static int main_state_autosave_menu_10_seconds (void)
{
   set_autosave (10);

   return (D_O_K);
}

static int main_state_autosave_menu_30_seconds (void)
{
   set_autosave (30);

   return (D_O_K);
}

static int main_state_autosave_menu_60_seconds (void)
{
   set_autosave (60);

   return (D_O_K);
}

static int main_menu_snapshot (void)
{
   int index;

   for (index = 0; index < 9999; index++)
   {
      USTRING filename;

      uszprintf (filename, sizeof (filename), "snap%04d.pcx", index);

      if (exists (filename))
         continue;

      save_bitmap (filename, video_buffer, video_palette);

      message_local ("Snapshot saved to %s.", filename);

      return (D_O_K);
   }

   gui_message (GUI_ERROR_COLOR, "Couldn't find a suitable image "
      "filename.");

   return (D_O_K);
}

static int main_menu_messages (void)
{
    PACKFILE * file;


    UINT8 * buffer;

   /* TODO: REWRITE ALL OF THIS. */
#if 0 // needs to be converted to use the new logfile code
    file = pack_fopen (logfile, "r");

    if (! file)
    {
        buffer = malloc (strlen ("Failed to open (or bi-open) log file."));

        if (! buffer)
        {
            /* Yuck. */

            return (D_O_K);
        }


        sprintf (buffer, "Failed to open (or bi-open) log file.");
    }
    else
    {
        int size;


        size = file_size (logfile);


        buffer = malloc (size);

        if (! buffer)
        {
            /* Log file has a maximum size of 64kB. */

            /* Pretty sad if malloc failed to allocate that. */

            return (D_O_K);
        }


        pack_fread (buffer, size, file);
    }


    pack_fclose (file);


    main_messages_dialog [2].dp = buffer;


    show_dialog (main_messages_dialog);
#endif

    return (D_O_K);
}

static int main_menu_exit (void)
{
   want_exit = TRUE;

   return (D_CLOSE);
}

static int options_menu_status (void)
{
   video_display_status = (! video_display_status);
   update_menus ();

   return (D_O_K);
}

#define SET_THEME(name) \
   set_##name##_theme ();  \
   gui_needs_restart = TRUE;

static int options_gui_theme_menu_classic (void)
{
   SET_THEME(classic);

   return (D_CLOSE);
}

static int options_gui_theme_menu_stainless_steel (void)
{
   SET_THEME(stainless_steel);

   return (D_CLOSE);
}

static int options_gui_theme_menu_zero_4 (void)
{
   SET_THEME(zero_4);

   return (D_CLOSE);
}


static int options_gui_theme_menu_panta (void)
{
   SET_THEME(panta);

   return (D_CLOSE);
}

#undef SET_THEME

static int options_system_menu_ntsc_60_hz (void)
{
    machine_type = MACHINE_TYPE_NTSC;
    update_menus ();

    cycle_audio ();

    message_local ("System type set to NTSC (60 Hz).");

    return (D_O_K);
}

static int options_system_menu_pal_50_hz (void)
{
   machine_type = MACHINE_TYPE_PAL;
   update_menus ();

   cycle_audio ();

   message_local ("System type set to PAL (50 Hz).");

   return (D_O_K);
}

static int options_audio_menu_enabled (void)
{
   audio_enable_output = !audio_enable_output;
   update_menus ();

   cycle_audio ();

   message_local ("Audio rendering and output %s.", get_enabled_text
      (audio_enable_output));

   return (D_O_K);
}

static int options_audio_subsystem_menu_none (void)
{
   audio_subsystem = AUDIO_SUBSYSTEM_NONE;
   update_menus ();

   cycle_audio ();

   message_local ("Audio subsystem set to NONE.");

   return (D_O_K);
}

static int options_audio_subsystem_menu_allegro (void)
{
   audio_subsystem = AUDIO_SUBSYSTEM_ALLEGRO;
   update_menus ();

   cycle_audio ();

   message_local ("Audio subsystem set to Allegro.");

   return (D_O_K);
}

static int options_audio_subsystem_menu_openal (void)
{
   audio_subsystem = AUDIO_SUBSYSTEM_OPENAL;
   update_menus ();

   cycle_audio ();

   message_local ("Audio subsystem set to OpenAL.");

   return (D_O_K);
}

#define MIXING_FREQUENCY_MENU_HANDLER(freq)  \
   static int options_audio_mixing_frequency_menu_##freq##_hz (void) \
   {  \
      audio_sample_rate = freq;  \
      update_menus ();  \
      cycle_audio ();   \
      message_local ("Audio mixing frequency set to %d Hz.", freq);  \
      return (D_O_K);   \
   }

MIXING_FREQUENCY_MENU_HANDLER(8000)
MIXING_FREQUENCY_MENU_HANDLER(11025)
MIXING_FREQUENCY_MENU_HANDLER(16000)
MIXING_FREQUENCY_MENU_HANDLER(22050)
MIXING_FREQUENCY_MENU_HANDLER(32000)
MIXING_FREQUENCY_MENU_HANDLER(44100)
MIXING_FREQUENCY_MENU_HANDLER(48000)
MIXING_FREQUENCY_MENU_HANDLER(80200)
MIXING_FREQUENCY_MENU_HANDLER(96000)

static int options_audio_mixing_channels_menu_mono (void)
{
   audio_pseudo_stereo = FALSE;
   update_menus ();

   cycle_audio ();

   gui_message (GUI_TEXT_COLOR, "Audio channels set to mono.");

   return (D_O_K);
}

static int options_audio_mixing_channels_menu_stereo_mix (void)
{
   audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_4;
   update_menus ();
    
   cycle_audio ();
   
   message_local ("Audio channels set to mono with stereo mixing.");

   return (D_O_K);
}

static int options_audio_mixing_channels_menu_pseudo_stereo_mode_1 (void)
{
   audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_1;
   update_menus ();
    
   cycle_audio ();

   message_local ("Audio channels set to pseudo stereo (mode 1).");

   return (D_O_K);
}

static int options_audio_mixing_channels_menu_pseudo_stereo_mode_2 (void)
{
   audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_2;
   update_menus ();

   cycle_audio ();

   message_local ("Audio channels set to pseudo stereo (mode 2).");

   return (D_O_K);
}

static int options_audio_mixing_channels_menu_stereo (void)
{
   audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_3;
   update_menus ();

   cycle_audio ();

   message_local ("Audio channels set to stereo.");

   return (D_O_K);
}

static int options_audio_mixing_channels_menu_swap_channels (void)
{
   papu_swap_channels = !papu_swap_channels;
   update_menus ();

   message_local ("Audio stereo channel swapping %s.", get_enabled_text
      (papu_swap_channels));

   return (D_O_K);
}

static int options_audio_mixing_quality_menu_low_8_bit (void)
{
   audio_sample_size = 8;
   update_menus ();

   cycle_audio ();

   message_local ("Audio mixing quality set to low (8-bit).");

   return (D_O_K);
}

static int options_audio_mixing_quality_menu_high_16_bit (void)
{
   audio_sample_size = 16;
   update_menus ();

   cycle_audio ();

   message_local ("Audio mixing quality set to high (16-bit).");

   return (D_O_K);
}

static int options_audio_mixing_quality_menu_interpolation (void)
{
   audio_interpolation = !audio_interpolation;
   update_menus ();

   cycle_audio ();

   message_local ("Audio interpolation %s.", get_enabled_text
      (audio_interpolation));

   return (D_O_K);
}

static int options_audio_mixing_quality_menu_dithering (void)
{
   papu_dithering = !papu_dithering;
   update_menus ();

   message_local ("Audio dithering %s.", get_enabled_text (papu_dithering));

   return (D_O_K);
}

static int options_audio_mixing_anti_aliasing_menu_disabled (void)
{
   papu_interpolate = 0;
   update_menus ();

   papu_reinit ();

   message_local ("Audio anti-aliasing disabled.");

   return (D_O_K);
}

static int options_audio_mixing_anti_aliasing_menu_bilinear_2x (void)
{
   papu_interpolate = 1;
   update_menus ();

   papu_reinit ();

   message_local ("Audio anti-aliasing method set to bilinear 2X.");

   return (D_O_K);
}

static int options_audio_mixing_anti_aliasing_menu_bilinear_4x (void)
{
   papu_interpolate = 2;
   update_menus ();

   papu_reinit ();

   message_local ("Audio anti-aliasing method set to bilinear 4X.");

   return (D_O_K);
}

static int options_audio_mixing_anti_aliasing_menu_bilinear_8x (void)
{
   papu_interpolate = 3;
   update_menus ();

   papu_reinit ();

   message_local ("Audio anti-aliasing method set to bilinear 8X.");

   return (D_O_K);
}

static int options_audio_mixing_anti_aliasing_menu_bilinear_16x (void)
{
   papu_interpolate = 4;
   update_menus ();

   papu_reinit ();

   message_local ("Audio anti-aliasing method set to bilinear 16X.");

   return (D_O_K);
}

static int options_audio_effects_menu_linear_echo (void)
{
   papu_linear_echo = !papu_linear_echo;
   update_menus ();

   papu_reinit ();

   message_local ("Audio linear echo effect %s.", get_enabled_text
      (papu_linear_echo));

   return (D_O_K);
}

static int options_audio_effects_menu_spatial_stereo_mode_1 (void)
{
   papu_spatial_stereo = ((papu_spatial_stereo ==
      PAPU_SPATIAL_STEREO_MODE_1) ? FALSE : PAPU_SPATIAL_STEREO_MODE_1);
   update_menus ();

   papu_reinit ();

   message_local ("Audio spatial stereo effect %s.", (papu_spatial_stereo ?
      "enabled (mode 1)" : "disabled"));

   return (D_O_K);
}

static int options_audio_effects_menu_spatial_stereo_mode_2 (void)
{
   papu_spatial_stereo = ((papu_spatial_stereo ==
      PAPU_SPATIAL_STEREO_MODE_2) ? FALSE : PAPU_SPATIAL_STEREO_MODE_2);
   update_menus ();

   papu_reinit ();

   message_local ("Audio spatial stereo effect %s.", (papu_spatial_stereo ?
      "enabled (mode 2)" : "disabled"));

   return (D_O_K);
}

static int options_audio_effects_menu_spatial_stereo_mode_3 (void)
{
   papu_spatial_stereo = ((papu_spatial_stereo ==
      PAPU_SPATIAL_STEREO_MODE_3) ? FALSE : PAPU_SPATIAL_STEREO_MODE_3);
   update_menus ();

   papu_reinit ();

   message_local ("Audio spatial stereo effect %s.", (papu_spatial_stereo ?
      "enabled (mode 3)" : "disabled"));

   return (D_O_K);
}

static int options_audio_filters_menu_low_pass_mode_1 (void)
{
   LIST filters;

   filters = papu_get_filter_list ();

   if (filters & PAPU_FILTER_LOW_PASS_MODE_1)
   {
      papu_set_filter_list ((filters & ~PAPU_FILTER_LOW_PASS_MODE_1));
   }
   else
   {
      filters &= ~PAPU_FILTER_LOW_PASS_MODE_2;
      filters &= ~PAPU_FILTER_LOW_PASS_MODE_3;
      papu_set_filter_list ((filters | PAPU_FILTER_LOW_PASS_MODE_1));
   }

   update_menus ();

   message_local ("Low pass audio filter %s.", get_enabled_text_ex ((filters
      & PAPU_FILTER_LOW_PASS_MODE_1), "enabled (mode 1)"));

   return (D_O_K);
}

static int options_audio_filters_menu_low_pass_mode_2 (void)
{
   LIST filters;

   filters = papu_get_filter_list ();

   if (filters & PAPU_FILTER_LOW_PASS_MODE_2)
   {
      papu_set_filter_list ((filters & ~PAPU_FILTER_LOW_PASS_MODE_2));
   }
   else
   {
      filters &= ~PAPU_FILTER_LOW_PASS_MODE_1;
      filters &= ~PAPU_FILTER_LOW_PASS_MODE_3;
      papu_set_filter_list ((filters | PAPU_FILTER_LOW_PASS_MODE_2));
   }

   update_menus ();

   message_local ("Low pass audio filter %s.", get_enabled_text_ex ((filters
      & PAPU_FILTER_LOW_PASS_MODE_2), "enabled (mode 2)"));

   return (D_O_K);
}

static int options_audio_filters_menu_low_pass_mode_3 (void)
{
   LIST filters;

   filters = papu_get_filter_list ();

   if (filters & PAPU_FILTER_LOW_PASS_MODE_3)
   {
      papu_set_filter_list ((filters & ~PAPU_FILTER_LOW_PASS_MODE_3));
   }
   else
   {
      filters &= ~PAPU_FILTER_LOW_PASS_MODE_1;
      filters &= ~PAPU_FILTER_LOW_PASS_MODE_2;
      papu_set_filter_list ((filters | PAPU_FILTER_LOW_PASS_MODE_3));
   }

   update_menus ();

   message_local ("Low pass audio filter %s.", get_enabled_text_ex ((filters
      & PAPU_FILTER_LOW_PASS_MODE_3), "enabled (mode 3)"));

   return (D_O_K);
}

static int options_audio_filters_menu_high_pass (void)
{
   LIST filters;

   filters = papu_get_filter_list ();

   if (filters & PAPU_FILTER_HIGH_PASS)
      papu_set_filter_list ((filters & ~PAPU_FILTER_HIGH_PASS));
   else
      papu_set_filter_list ((filters | PAPU_FILTER_HIGH_PASS));

   update_menus ();

   message_local ("Toggled high pass audio filter.");

   return (D_O_K);
}

static int options_audio_channels_menu_square_wave_a (void)
{
   papu_enable_square_1 = !papu_enable_square_1;
   update_menus ();

   papu_update ();

   message_local ("Audio square wave channel A %s.", get_enabled_text
      (papu_enable_square_1));

   return (D_O_K);
}

static int options_audio_channels_menu_square_wave_b (void)
{
   papu_enable_square_2 = !papu_enable_square_2;
   update_menus ();

   papu_update ();

   message_local ("Audio square wave channel B %s.", get_enabled_text
      (papu_enable_square_2));

   return (D_O_K);
}

static int options_audio_channels_menu_triangle_wave (void)
{
   papu_enable_triangle = !papu_enable_triangle;
   update_menus ();

   papu_update ();

   message_local ("Audio triangle wave channel %s.", get_enabled_text
      (papu_enable_triangle));

   return (D_O_K);
}

static int options_audio_channels_menu_white_noise (void)
{
   papu_enable_noise = !papu_enable_noise;
   papu_update ();

   update_menus ();

   message_local ("Audio white noise channel %s.", get_enabled_text
      (papu_enable_noise));

   return (D_O_K);
}

static int options_audio_channels_menu_digital (void)
{
   papu_enable_dmc = !papu_enable_dmc;
   papu_update ();

   update_menus ();

   message_local ("Audio digital channel %s.", get_enabled_text
      (papu_enable_dmc));

   return (D_O_K);
}

static int options_audio_channels_menu_extended (void)
{
   papu_enable_exsound = !papu_enable_exsound;
   papu_update ();

   update_menus ();

   message_local ("Audio extended channels %s.", get_enabled_text
      (papu_enable_exsound));

   return (D_O_K);
}

static int options_audio_advanced_menu_ideal_triangle (void)
{
   papu_ideal_triangle = !papu_ideal_triangle;
   update_menus ();

   papu_update ();

   message_local ("Audio ideal triangle emulation %s.", get_enabled_text
      (papu_ideal_triangle));

   return (D_O_K);
}

static int options_audio_advanced_menu_hard_sync (void)
{
   audio_hard_sync = !audio_hard_sync;
   update_menus ();

   message_local ("Audio hard synchronization %s.", get_enabled_text
      (audio_hard_sync));

   return (D_O_K);
}

static int options_audio_record_menu_start (void)
{
   if (papu_start_record () == 0)
   {
      DISABLE_MENU_ITEM(options_audio_record_menu_start);
      ENABLE_MENU_ITEM(options_audio_record_menu_stop);
   }

   message_local ("Audio recording session started.");

   return (D_O_K);
}

static int options_audio_record_menu_stop (void)
{
   papu_stop_record ();

   ENABLE_MENU_ITEM(options_audio_record_menu_start);
   DISABLE_MENU_ITEM(options_audio_record_menu_stop);

   message_local ("Audio recording session stopped.");

   return (D_O_K);
}

#define DRIVER_MENU_HANDLER(system, driver, id) \
   static int options_video_driver_##system##_menu_##driver (void)   \
   {  \
      video_set_driver (id);  \
      gui_needs_restart = TRUE;  \
      return (D_CLOSE); \
   }

#ifdef ALLEGRO_DOS

DRIVER_MENU_HANDLER(dos, vga,           GFX_VGA)
DRIVER_MENU_HANDLER(dos, vga_mode_x,    GFX_MODEX)
DRIVER_MENU_HANDLER(dos, vesa,          GFX_VESA1)
DRIVER_MENU_HANDLER(dos, vesa_2_banked, GFX_VESA2B)
DRIVER_MENU_HANDLER(dos, vesa_2_linear, GFX_VESA2L)
DRIVER_MENU_HANDLER(dos, vesa_3,        GFX_VESA3)
DRIVER_MENU_HANDLER(dos, vesa_vbe_af,   GFX_VBEAF)

#endif   /* ALLEGRO_DOS */

#ifdef ALLEGRO_WINDOWS

DRIVER_MENU_HANDLER(windows, directx,         GFX_DIRECTX)
DRIVER_MENU_HANDLER(windows, directx_window,  GFX_DIRECTX_WIN)
DRIVER_MENU_HANDLER(windows, directx_overlay, GFX_DIRECTX_OVL)
DRIVER_MENU_HANDLER(windows, gdi,             GFX_GDI)

#endif   /* ALLEGRO_WINDOWS */

#ifdef ALLEGRO_LINUX

DRIVER_MENU_HANDLER(linux, vga,         GFX_VGA)
DRIVER_MENU_HANDLER(linux, vga_mode_x,  GFX_MODEX)
DRIVER_MENU_HANDLER(linux, vesa_vbe_af, GFX_VBEAF)
#ifdef GFX_FBCON
DRIVER_MENU_HANDLER(linux, framebuffer, GFX_FBCON)
#else
DRIVER_MENU_HANDLER(linux, framebuffer, NULL)
#endif
#ifdef GFX_SVGALIB
DRIVER_MENU_HANDLER(linux, svgalib,     GFX_SVGALIB)
#else
DRIVER_MENU_HANDLER(linux, svgalib,     NULL)
#endif

#endif   /* ALLEGRO_LINUX */

#ifdef ALLEGRO_UNIX

DRIVER_MENU_HANDLER(unix, x_windows,      GFX_XWINDOWS)
DRIVER_MENU_HANDLER(unix, x_windows_full, GFX_XWINDOWS_FULLSCREEN)
DRIVER_MENU_HANDLER(unix, x_dga,          GFX_XDGA)
DRIVER_MENU_HANDLER(unix, x_dga_full,     GFX_XDGA_FULLSCREEN)
DRIVER_MENU_HANDLER(unix, x_dga_2,        GFX_XDGA2)

#endif   /* ALLEGRO_UNIX */

#undef DRIVER_MENU_HANDLER

static int options_video_driver_menu_automatic (void)
{
   video_set_driver (GFX_AUTODETECT);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

#define RESOLUTION_MENU_HANDLER(width, height)  \
   static int options_video_resolution_menu_##width##_##height (void)   \
   {  \
      video_set_resolution (width, height);  \
      gui_needs_restart = TRUE;  \
      return (D_CLOSE); \
   }

#define RESOLUTION_MENU_HANDLER_EX(type, width, height)  \
   static int options_video_resolution_##type##_menu_##width##_##height (void)   \
   {  \
      video_set_resolution (width, height);  \
      gui_needs_restart = TRUE;  \
      return (D_CLOSE); \
   }

RESOLUTION_MENU_HANDLER_EX(proportionate, 256,  224)
RESOLUTION_MENU_HANDLER_EX(proportionate, 256,  240)
RESOLUTION_MENU_HANDLER_EX(proportionate, 512,  448)
RESOLUTION_MENU_HANDLER_EX(proportionate, 512,  480)
RESOLUTION_MENU_HANDLER_EX(proportionate, 768,  672)
RESOLUTION_MENU_HANDLER_EX(proportionate, 768,  720)
RESOLUTION_MENU_HANDLER_EX(proportionate, 1024, 896)
RESOLUTION_MENU_HANDLER_EX(proportionate, 1024, 960)
RESOLUTION_MENU_HANDLER_EX(proportionate, 1280, 1120)
RESOLUTION_MENU_HANDLER_EX(proportionate, 1280, 1200)

RESOLUTION_MENU_HANDLER(320,  240)
RESOLUTION_MENU_HANDLER(640,  480)
RESOLUTION_MENU_HANDLER(800,  600)
RESOLUTION_MENU_HANDLER(1024, 768)
RESOLUTION_MENU_HANDLER(1152, 864)
RESOLUTION_MENU_HANDLER(1280, 1024)
RESOLUTION_MENU_HANDLER(1600, 1200)

RESOLUTION_MENU_HANDLER_EX(extended, 400,  300)
RESOLUTION_MENU_HANDLER_EX(extended, 480,  360)
RESOLUTION_MENU_HANDLER_EX(extended, 512,  384)
RESOLUTION_MENU_HANDLER_EX(extended, 640,  400)
RESOLUTION_MENU_HANDLER_EX(extended, 720,  480)
RESOLUTION_MENU_HANDLER_EX(extended, 720,  576)
RESOLUTION_MENU_HANDLER_EX(extended, 848,  480)
RESOLUTION_MENU_HANDLER_EX(extended, 1280, 720)
RESOLUTION_MENU_HANDLER_EX(extended, 1280, 960)
RESOLUTION_MENU_HANDLER_EX(extended, 1360, 768)

#undef RESOLUTION_MENU_HANDLER
#undef RESOLUTION_MENU_HANDLER_EX

static int options_video_colors_menu_paletted_8_bit (void)
{
   video_set_color_depth (8);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int options_video_colors_menu_true_color_15_bit (void)
{
   video_set_color_depth (15);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int options_video_colors_menu_true_color_16_bit (void)
{
   video_set_color_depth (16);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int options_video_colors_menu_true_color_32_bit (void)
{
   video_set_color_depth (32);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

#define BLITTER_MENU_HANDLER(name, caption, id) \
   static int options_video_blitter_menu_##name (void)   \
   {  \
      video_set_blitter (id); \
      update_menus ();  \
      cycle_video ();   \
      message_local ("Video blitter set to %s.", caption);  \
      return (D_O_K);   \
   }

BLITTER_MENU_HANDLER(automatic,       "automatic",          VIDEO_BLITTER_AUTOMATIC)
BLITTER_MENU_HANDLER(normal,          "normal",             VIDEO_BLITTER_NORMAL)
BLITTER_MENU_HANDLER(stretched,       "stretched",          VIDEO_BLITTER_STRETCHED)
BLITTER_MENU_HANDLER(interpolated_2x, "interpolated (2x)",  VIDEO_BLITTER_INTERPOLATED_2X)
BLITTER_MENU_HANDLER(interpolated_3x, "interpolated (3x)",  VIDEO_BLITTER_INTERPOLATED_2X)
BLITTER_MENU_HANDLER(2xsoe,           "2xSOE engine",       VIDEO_BLITTER_2XSOE)
BLITTER_MENU_HANDLER(2xscl,           "2xSCL engine",       VIDEO_BLITTER_2XSCL)
BLITTER_MENU_HANDLER(super_2xsoe,     "super 2xSOE engine", VIDEO_BLITTER_SUPER_2XSOE)
BLITTER_MENU_HANDLER(super_2xscl,     "super 2xSCL engine", VIDEO_BLITTER_SUPER_2XSCL)
BLITTER_MENU_HANDLER(ultra_2xscl,     "ultra 2xSCL engine", VIDEO_BLITTER_ULTRA_2XSCL)

#undef BLITTER_MENU_HANDLER

static int options_video_filters_menu_scanlines_25_percent (void)
{
   LIST filters;

   filters = video_get_filter_list ();

   if (filters & VIDEO_FILTER_SCANLINES_LOW)
   {
      video_set_filter_list ((filters & ~VIDEO_FILTER_SCANLINES_LOW));
   }
   else
   {
      filters &= ~VIDEO_FILTER_SCANLINES_HIGH;
      filters &= ~VIDEO_FILTER_SCANLINES_MEDIUM;
      video_set_filter_list ((filters | VIDEO_FILTER_SCANLINES_LOW));
   }

   update_menus ();

   cycle_video ();

   message_local ("Scanlines video filter %s.", get_enabled_text_ex
      ((filters & VIDEO_FILTER_SCANLINES_LOW), "enabled (25%)"));

   return (D_O_K);
}

static int options_video_filters_menu_scanlines_50_percent (void)
{
   LIST filters;

   filters = video_get_filter_list ();

   if (filters & VIDEO_FILTER_SCANLINES_MEDIUM)
   {
      video_set_filter_list ((filters & ~VIDEO_FILTER_SCANLINES_MEDIUM));
   }
   else
   {
      filters &= ~VIDEO_FILTER_SCANLINES_HIGH;
      filters &= ~VIDEO_FILTER_SCANLINES_LOW;
      video_set_filter_list ((filters | VIDEO_FILTER_SCANLINES_MEDIUM));
   }

   update_menus ();

   cycle_video ();

   message_local ("Scanlines video filter %s.", get_enabled_text_ex
      ((filters & VIDEO_FILTER_SCANLINES_MEDIUM), "enabled (50%)"));

   return (D_O_K);
}

static int options_video_filters_menu_scanlines_100_percent (void)
{
   LIST filters;

   filters = video_get_filter_list ();

   if (filters & VIDEO_FILTER_SCANLINES_HIGH)
   {
      video_set_filter_list ((filters & ~VIDEO_FILTER_SCANLINES_HIGH));
   }
   else
   {
      filters &= ~VIDEO_FILTER_SCANLINES_MEDIUM;
      filters &= ~VIDEO_FILTER_SCANLINES_LOW;
      video_set_filter_list ((filters | VIDEO_FILTER_SCANLINES_HIGH));
   }

   update_menus ();

   cycle_video ();

   message_local ("Scanlines video filter %s.", get_enabled_text_ex
      ((filters & VIDEO_FILTER_SCANLINES_HIGH), "enabled (100%)"));

   return (D_O_K);
}

static int options_video_menu_vsync (void)
{
   video_enable_vsync = !video_enable_vsync;
   update_menus ();

   message_local ("VSync %s.", get_enabled_text (video_enable_vsync));

   return (D_O_K);
}

static int options_video_layers_menu_sprites_a (void)
{
   ppu_enable_sprite_layer_a = !ppu_enable_sprite_layer_a;
   update_menus ();

   message_local ("Video sprites layer A %s.", get_enabled_text
      (ppu_enable_sprite_layer_a));

   return (D_O_K);
}

static int options_video_layers_menu_sprites_b (void)
{
   ppu_enable_sprite_layer_b = !ppu_enable_sprite_layer_b;
   update_menus ();

   message_local ("Video sprites layer B %s.", get_enabled_text
      (ppu_enable_sprite_layer_b));

   return (D_O_K);
}


static int options_video_layers_menu_background (void)
{
   ppu_enable_background_layer = !ppu_enable_background_layer;
   update_menus ();

   message_local ("Video background layer %s.", get_enabled_text
      (ppu_enable_background_layer));

   return (D_O_K);
}

#define PALETTE_MENU_HANDLER(name, caption, id) \
   static int options_video_palette_menu_##name (void)   \
   {  \
      video_set_palette (DATA_TO_RGB(id));  \
      video_set_palette_id (DATA_INDEX(id)); \
      update_menus ();  \
      cycle_video ();   \
      message_local ("Video palette set to %s.", caption);  \
      return (D_O_K);   \
   }

PALETTE_MENU_HANDLER(ntsc_color,     "NTSC color",     DEFAULT_PALETTE)
PALETTE_MENU_HANDLER(ntsc_grayscale, "NTSC grayscale", GRAYSCALE_PALETTE)
PALETTE_MENU_HANDLER(gnuboy,         "gnuboy",         GNUBOY_PALETTE)
PALETTE_MENU_HANDLER(nester,         "NESter",         NESTER_PALETTE)
PALETTE_MENU_HANDLER(nesticle,       "NESticle",       NESTICLE_PALETTE)
PALETTE_MENU_HANDLER(modern_ntsc,    "modern NTSC",    MODERN_NTSC_PALETTE)
PALETTE_MENU_HANDLER(modern_pal,     "modern PAL",     MODERN_PAL_PALETTE)
PALETTE_MENU_HANDLER(ega_mode_1,     "EGA (mode 1)",   EGA_PALETTE_1)
PALETTE_MENU_HANDLER(ega_mode_2,     "EGA (mode 2)",   EGA_PALETTE_2)

#undef PALETTE_MENU_HANDLER

static int options_video_palette_menu_custom (void)
{
   PACKFILE *file;
   int index;

   file = pack_fopen ("fakenes.pal", "r");

   if (!file)
   {
      gui_message (GUI_ERROR_COLOR, "Error opening FAKENES.PAL!");

      return (D_O_K);
   }

   memset (custom_palette, 0, sizeof (PALETTE));

   for (index = 1; index <= 64; index++)
   {
      custom_palette[index].r = ROUND((pack_getc (file) / 4.0f));
      custom_palette[index].g = ROUND((pack_getc (file) / 4.0f));
      custom_palette[index].b = ROUND((pack_getc (file) / 4.0f));
   }

   pack_fclose (file);

   video_set_palette (((RGB *)custom_palette));
   video_set_palette_id (-1);

   update_menus ();

   cycle_video ();

   message_local ("Video palette set to custom.");
    
   return (D_O_K);
}

static int options_video_advanced_menu_force_window (void)
{
   video_force_window = !video_force_window;
   video_reinit ();

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int options_input_menu_enable_zapper (void)
{
   input_enable_zapper = !input_enable_zapper;
   update_menus ();

   message_local ("Zapper emulation %s.", get_enabled_text
      (input_enable_zapper));

   return (D_O_K);
}


static int options_input_menu_configure (void)
{
   show_dialog (options_input_dialog);

   return (D_O_K);
}

static int help_menu_shortcuts (void)
{
   show_dialog (help_shortcuts_dialog);

   return (D_O_K);
}

static int help_menu_about (void)
{
   show_dialog (help_about_dialog);

   return (D_O_K);
}


/* ---- Dialog handlers. ---- */


static int options_patches_dialog_list (DIALOG *dialog)
{
   CPU_PATCH *patch;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
      return (D_O_K);

   patch = &cpu_patch_info[dialog->d1];

   if (patch -> enabled)
      options_patches_dialog[5].flags |= D_SELECTED;
   else
      options_patches_dialog[5].flags &= ~D_SELECTED;

   scare_mouse ();
   object_message (&options_patches_dialog[5], MSG_DRAW, 0);
   unscare_mouse ();

   return (D_O_K);
}

static int options_patches_dialog_add (DIALOG *dialog)
{

   UINT8 buffer[16];
   UINT8 buffer2[11];
   CPU_PATCH *patch;
   UINT8 value;

   if (cpu_patch_count >= MAX_PATCHES)
   {
      alert ("- Error -", NULL, "The patch list is already full.", "&OK",
         NULL, 'o', 0);

      return (D_O_K);
   }

   memset (buffer, 0, sizeof (buffer));
   options_patches_add_dialog[4].d1 = (sizeof (buffer) - 1);
   options_patches_add_dialog[4].dp = buffer;

   memset (buffer2, 0, sizeof (buffer2));
   options_patches_add_dialog[7].d1 = (sizeof (buffer2) - 1);
   options_patches_add_dialog[7].dp = buffer2;

   if (show_dialog (options_patches_add_dialog) != 8)
      return (D_O_K);

   patch = &cpu_patch_info[cpu_patch_count];

   if (genie_decode (buffer2, &patch->address, &patch->value,
      &patch->match_value) != 0)
   {
      alert ("- Error -", NULL, "You must enter a valid Game Genie (or "
         "NESticle raw) code.", "&OK", NULL, 'o', 0);


      return (D_O_K);
   }

   if (patch->title)
      sprintf (patch->title, "%s", buffer);

   patch->enabled = TRUE;

   cpu_patch_count++;

   value = cpu_read (patch->address);

   if (value == patch->match_value)
   {
      /* Enable patch. */
      patch->active = TRUE;

      cpu_patch_table[patch->address] = (patch->value - value);
   }

   return (D_REDRAW);
}

static int options_patches_dialog_remove (DIALOG *dialog)
{
   int start;
   CPU_PATCH *src;
   int index;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
      return (D_O_K);

   start = options_patches_dialog[2].d1;
   src = &cpu_patch_info[start];

   /* Disable patch. */
   if (src->active)
   {
      if (alert ("Confirmation", NULL, "Really deactivate and remove this "
         "patch?", "&OK", "&Cancel", 'o', 'c') == 2)
         return (D_O_K);

      cpu_patch_table[src->address] = 0;
   }

   for (index = (start + 1); index < cpu_patch_count; index++)
   {
      CPU_PATCH *dest;

      src = &cpu_patch_info[index];
      dest = &cpu_patch_info[(index - 1)];

      memcpy (dest, src, sizeof (CPU_PATCH));
   }

   src = &cpu_patch_info[(cpu_patch_count - 1)];

   memset (src, 0, sizeof (CPU_PATCH));
   sprintf (src->title, "?");

   cpu_patch_count--;

   if (cpu_patch_count == 0)
      options_patches_dialog[5].flags &= ~D_SELECTED;

   return (D_REDRAW);
}

static int options_patches_dialog_enabled (DIALOG * dialog)
{
   CPU_PATCH *patch;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
   {
      dialog->flags &= ~D_SELECTED;

      return (D_O_K);
   }

   patch = &cpu_patch_info[options_patches_dialog[2].d1];

   patch->enabled = (dialog->flags & D_SELECTED);

   /* Toggle patch. */
   if (!patch->enabled && patch->active)
   {
      patch->active = FALSE;

      cpu_patch_table[patch->address] = 0;
   }
   else if (patch->enabled && !patch->active)
   {
      UINT8 value;

      value = cpu_read (patch->address);
    
      if (value == patch->match_value)
      {
         /* Enable patch. */
         patch->active = TRUE;
    
         cpu_patch_table[patch->address] = (patch->value - value);
      }
   }

   scare_mouse ();
   object_message (&options_patches_dialog[2], MSG_DRAW, 0);
   unscare_mouse ();

   return (D_O_K);
}

static STRING options_patches_dialog_list_texts[MAX_PATCHES];

static char *options_patches_dialog_list_filler (int index, int *list_size)
{
   RT_ASSERT(list_size);

   if (index >= 0)
   {
      CPU_PATCH *patch;
      CHAR *text;

      patch = &cpu_patch_info[index];

      text = options_patches_dialog_list_texts[index];

      STRING_CLEAR(text);
      snprintf (text, STRING_SIZE, "$%04x -$%02x +$%02x %s ",
         patch->address, patch->match_value, patch->value, (patch->active ?
            "Active" : " Idle "));

      if (patch->title)
         strncat (text, patch->title, STRING_SIZE);
      else
         strncat (text, "No title", STRING_SIZE);

      return (text);
   }
   else
   {
      *list_size = cpu_patch_count;

      return (NULL);
   }
}

static int selected_player = -1;
static int selected_player_device = 0;

static int options_input_dialog_player_select (DIALOG * dialog)
{
   RT_ASSERT(dialog);

   selected_player = (dialog->d2 - 1);
   selected_player_device = input_get_player_device (selected_player);

   options_input_dialog[8].flags &= ~D_SELECTED;
   options_input_dialog[9].flags &= ~D_SELECTED;
   options_input_dialog[10].flags &= ~D_SELECTED;
   options_input_dialog[11].flags &= ~D_SELECTED;
   options_input_dialog[(7 + selected_player_device)].flags |= D_SELECTED;

   scare_mouse ();
   object_message (&options_input_dialog[8], MSG_DRAW, 0);
   object_message (&options_input_dialog[9], MSG_DRAW, 0);
   object_message (&options_input_dialog[10], MSG_DRAW, 0);
   object_message (&options_input_dialog[11], MSG_DRAW, 0);
   unscare_mouse ();

   return (D_O_K);
}

static int options_input_dialog_device_select (DIALOG *dialog)
{
   RT_ASSERT(dialog);

   if (selected_player < 0)
   {
      alert ("- Error -", "", "Please select a player to modify first.",
         "&OK", NULL, 'o', 0);

      return (D_O_K);
   }

   selected_player_device = dialog->d2;

   input_set_player_device (selected_player, selected_player_device);

   return (D_O_K);
}

static int options_input_dialog_set_buttons (DIALOG *dialog)
{
   int button;
   int index;

   RT_ASSERT(dialog);

   if (selected_player < 0)
   {
      alert ("- Error -", "", "Please select a player to modify first.",
         "&OK", NULL, 'o', 0);

      return (D_O_K);
   }

   button = dialog->d2;

   if ((selected_player_device == INPUT_DEVICE_JOYSTICK_1) ||
       (selected_player_device == INPUT_DEVICE_JOYSTICK_2))
   {
      if ((button == INPUT_DEVICE_BUTTON_UP) ||
          (button == INPUT_DEVICE_BUTTON_DOWN) ||
          (button == INPUT_DEVICE_BUTTON_LEFT) ||
          (button == INPUT_DEVICE_BUTTON_RIGHT))
      {
         alert ("- Error -", "", "Unable to set direction buttons for "
            "joystick devices.", "&OK", NULL, 'o', 0);
    
         return (D_O_K);
      }
   }

   switch (selected_player_device)
   {
      case INPUT_DEVICE_KEYBOARD_1:
      case INPUT_DEVICE_KEYBOARD_2:
      {
         message_local ("Press any key.");
    
         clear_keybuf ();

         while (!keypressed ())
         {
            gui_heartbeat ();
         }
    
         index = (readkey () >> 8);
    
         input_map_device_button (selected_player_device, button, index);

         message_local ("Button mapped to scancode %d.", index);

         break;
      }

      case INPUT_DEVICE_JOYSTICK_1:
      {
         message_local ("Press any button on joystick #1.");
    
         clear_keybuf ();
    
         for (;;)
         {
            poll_joystick ();
    
            for (index = 0; index < joy[0].num_buttons; index++)
            {
               if (joy[0].button[index].b)
               {
                  input_map_device_button (selected_player_device, button,
                     index);

                  message_local ("Button mapped to joystick #1 button %d.",
                     index);
    
                  return (D_O_K);
               }
    
               if (keypressed ())
               {
                  if ((readkey () >> 8) == KEY_ESC)
                  { 
                     gui_message (GUI_ERROR_COLOR, "Button mapping "
                        "canceled.");
    
                     return (D_O_K);
                  }
               }
            }

            gui_heartbeat ();
         }

         break;
      }

      case INPUT_DEVICE_JOYSTICK_2:
      {
         message_local ("Press any button on joystick #2.");
    
         clear_keybuf ();

         for (;;)
         {
            poll_joystick ();
    
            for (index = 0; index < joy[1].num_buttons; index++)
            {
               if (joy[1].button[index].b)
               {
                  input_map_device_button (selected_player_device, button,
                     index);
                
                  message_local ("Button mapped to joystick #1 button %d.",
                     index);
    
                  return (D_O_K);
               }
            }
    
            if (keypressed ())
            {
               if ((readkey () >> 8) == KEY_ESC)
               {
                  gui_message (GUI_ERROR_COLOR, "Button mapping canceled.");
    
                  return (D_O_K);
               }
            }

            gui_heartbeat ();
         }

         break;
      }

      default:
         WARN_GENERIC();
   }

   return (D_O_K);
}

static int options_menu_patches (void)
{
   if (show_dialog (options_patches_dialog) == 6)
      patches_save (global_rom.filename);

   return (D_O_K);
}

static int netplay_protocol_menu_tcpip (void)
{
   netplay_protocol = NETPLAY_PROTOCOL_TCPIP;
   update_menus ();

   message_local ("Netplay protocol set to TCP/IP.");

   return (D_O_K);
}

static int netplay_protocol_menu_spx (void)
{
   netplay_protocol = NETPLAY_PROTOCOL_SPX;
   update_menus ();

   message_local ("Netplay protocol set to SPX.");

   return (D_O_K);
}

static int netplay_server_menu_start (void)
{
   if (netplay_open_server () != 0)
      gui_message (GUI_ERROR_COLOR, "Failed to start the netplay server!");

   /* TODO: FIX MENU STUFF */
   DISABLE_MENU (top_menu, 0);
   DISABLE_MENU (top_menu, 1);
   DISABLE_MENU (top_menu, 3);
   DISABLE_MENU (netplay_menu, 0);
   DISABLE_MENU (netplay_menu, 4);
   DISABLE_MENU (netplay_server_menu, 0);
   ENABLE_MENU (netplay_server_menu, 2);

   message_local ("Started NetPlay server, awaiting client.");

   return (D_REDRAW);
}

static int netplay_server_menu_stop (void)
{
   netplay_close_server ();

   /* TODO: FIX MENU STUFF */
   DISABLE_MENU (netplay_server_menu, 2);
   ENABLE_MENU (top_menu, 0);
   ENABLE_MENU (top_menu, 1);
   ENABLE_MENU (top_menu, 3);
   ENABLE_MENU (netplay_menu, 0);
   ENABLE_MENU (netplay_menu, 4);
   ENABLE_MENU (netplay_server_menu, 0);

   message_local ("Stopped NetPlay server.");

   return (D_REDRAW);
}

static int netplay_client_menu_connect (void)
{
   if (netplay_protocol == NETPLAY_PROTOCOL_TCPIP)
   {
      CHAR buffer[16];
    
      memset (buffer, 0, sizeof (buffer));
      strncat (buffer, get_config_string ("netplay", "ip_address",
         "0.0.0.0"), (sizeof (buffer) - 1));

      netplay_client_connect_dialog[4].d1 = (sizeof (buffer) - 1);
      netplay_client_connect_dialog[4].dp = buffer;
    
      if (show_dialog (netplay_client_connect_dialog) != 5)
         return (D_O_K);

      if (netplay_open_client (buffer) != 0)
      {
         gui_message (GUI_ERROR_COLOR, "Failed to connect to the server!");

         return (D_O_K);
      }

      set_config_string ("netplay", "ip_address", buffer);
   }
   else
   {
      if (netplay_open_client (NULL) != 0)
      {
         gui_message (GUI_ERROR_COLOR, "Failed to connect to the server!");

         return (D_O_K);
      }
   }

   /* TODO:FIX MENU STUFF */
   DISABLE_MENU (top_menu, 0);
   DISABLE_MENU (top_menu, 1);
   DISABLE_MENU (top_menu, 3);
   DISABLE_MENU (netplay_menu, 0);
   DISABLE_MENU (netplay_menu, 2);
   DISABLE_MENU (netplay_client_menu, 0);
   ENABLE_MENU (netplay_client_menu, 2);

   message_local ("NetPlay client connected to the server.");

   return (D_O_K);
}


static int netplay_client_menu_disconnect (void)
{
   netplay_close_client ();

   /* TODO: FIX MENU STUFF */
   DISABLE_MENU (netplay_client_menu, 2);
   ENABLE_MENU (top_menu, 0);
   ENABLE_MENU (top_menu, 1);
   ENABLE_MENU (top_menu, 3);
   ENABLE_MENU (netplay_menu, 0);
   ENABLE_MENU (netplay_menu, 2);
   ENABLE_MENU (netplay_client_menu, 0);

   message_local ("NetPlay client disconnected from the server.");

   return (D_O_K);
}
