/* FakeNES - A free, portable, Open Source NES emulator.

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
#include "cheats.h"
#include "common.h"
#include "cpu.h"
#include "data.h"
#include "debug.h"
#include "dsp.h"
#include "gui.h"
#include "input.h"
#include "log.h"
#include "mmc.h"
#include "netplay.h"
#include "ppu.h"
#include "rom.h"
#include "save.h"
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

static int save_state_index = 0; /* For save states. */
static int replay_index = 0;     /* For replays. */

/* Text that appears in "unused" menu slots for recent items. */
#define UNUSED_SLOT_TEXT   "Empty"

/* Number of slots available in each of the associated menus. */
#define OPEN_RECENT_SLOTS  10
#define REPLAY_SLOTS       10
#define SAVE_STATE_SLOTS   10

static USTRING open_recent_filenames[OPEN_RECENT_SLOTS];
static USTRING open_recent_menu_texts[OPEN_RECENT_SLOTS];
static USTRING replay_titles[REPLAY_SLOTS];
static USTRING replay_menu_texts[REPLAY_SLOTS];
static USTRING save_state_titles[SAVE_STATE_SLOTS];
static USTRING save_state_menu_texts[SAVE_STATE_SLOTS];

static BOOL lock_recent = FALSE;

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
   MENU_FROM_BASE(main_open_recent_menu);
   MENU_FROM_BASE(main_replay_select_menu);
   MENU_FROM_BASE(main_replay_record_menu);
   MENU_FROM_BASE(main_replay_play_menu);
   MENU_FROM_BASE(main_replay_menu);
   MENU_FROM_BASE(main_menu);
   MENU_FROM_BASE(machine_save_state_select_menu);
   MENU_FROM_BASE(machine_save_state_autosave_menu);
   MENU_FROM_BASE(machine_save_state_menu);
   MENU_FROM_BASE(machine_region_menu);
   MENU_FROM_BASE(machine_menu);
   MENU_FROM_BASE(audio_subsystem_menu);
   MENU_FROM_BASE(audio_mixing_channels_menu);
   MENU_FROM_BASE(audio_mixing_frequency_menu);
   MENU_FROM_BASE(audio_mixing_quality_menu);
   MENU_FROM_BASE(audio_mixing_menu);
   MENU_FROM_BASE(audio_effects_menu);
   MENU_FROM_BASE(audio_filters_menu);
   MENU_FROM_BASE(audio_channels_menu);
   MENU_FROM_BASE(audio_volume_menu);
   MENU_FROM_BASE(audio_record_menu);
   MENU_FROM_BASE(audio_menu);
   MENU_FROM_BASE(video_driver_dos_menu);
   MENU_FROM_BASE(video_driver_windows_menu);
   MENU_FROM_BASE(video_driver_linux_menu);
   MENU_FROM_BASE(video_driver_unix_menu);
   MENU_FROM_BASE(video_driver_menu);
   MENU_FROM_BASE(video_resolution_proportionate_menu);
   MENU_FROM_BASE(video_resolution_extended_menu);
   MENU_FROM_BASE(video_resolution_menu);
   MENU_FROM_BASE(video_colors_menu);
   MENU_FROM_BASE(video_blitter_menu);
   MENU_FROM_BASE(video_filters_menu);
   MENU_FROM_BASE(video_layers_menu);
   MENU_FROM_BASE(video_palette_menu);
   MENU_FROM_BASE(video_advanced_menu);
   MENU_FROM_BASE(video_menu);
   MENU_FROM_BASE(options_input_menu);
   MENU_FROM_BASE(options_cpu_usage_menu);
   MENU_FROM_BASE(options_gui_theme_menu);
   MENU_FROM_BASE(options_menu);
   MENU_FROM_BASE(netplay_menu);
   MENU_FROM_BASE(help_menu);
   MENU_FROM_BASE(top_menu);
}

static INLINE void load_dialogs (void)
{
   DIALOG_FROM_BASE(main_dialog);
   DIALOG_FROM_BASE(main_replay_record_start_dialog);
   DIALOG_FROM_BASE(machine_save_state_save_dialog);
   DIALOG_FROM_BASE(machine_cheat_manager_add_dialog);
   DIALOG_FROM_BASE(machine_cheat_manager_dialog);
   DIALOG_FROM_BASE(options_input_configure_dialog);
   DIALOG_FROM_BASE(netplay_dialog);
   DIALOG_FROM_BASE(lobby_dialog);
   DIALOG_FROM_BASE(help_shortcuts_dialog);
   DIALOG_FROM_BASE(help_about_dialog);
}

/* Undefine helper macros. */
#undef MENU_FROM_BASE
#undef DIALOG_FROM_BASE

static INLINE void unload_menus (void)
{
   unload_menu (main_open_recent_menu);
   unload_menu (main_replay_select_menu);
   unload_menu (main_replay_record_menu);
   unload_menu (main_replay_play_menu);
   unload_menu (main_replay_menu);
   unload_menu (main_menu);
   unload_menu (machine_save_state_select_menu);
   unload_menu (machine_save_state_autosave_menu);
   unload_menu (machine_save_state_menu);
   unload_menu (machine_region_menu);
   unload_menu (machine_menu);
   unload_menu (audio_subsystem_menu);
   unload_menu (audio_mixing_channels_menu);
   unload_menu (audio_mixing_frequency_menu);
   unload_menu (audio_mixing_quality_menu);
   unload_menu (audio_mixing_menu);
   unload_menu (audio_effects_menu);
   unload_menu (audio_filters_menu);
   unload_menu (audio_channels_menu);
   unload_menu (audio_volume_menu);
   unload_menu (audio_record_menu);
   unload_menu (audio_menu);
   unload_menu (video_driver_dos_menu);
   unload_menu (video_driver_windows_menu);
   unload_menu (video_driver_linux_menu);
   unload_menu (video_driver_unix_menu);
   unload_menu (video_driver_menu);
   unload_menu (video_resolution_proportionate_menu);
   unload_menu (video_resolution_extended_menu);
   unload_menu (video_resolution_menu);
   unload_menu (video_colors_menu);
   unload_menu (video_blitter_menu);
   unload_menu (video_filters_menu);
   unload_menu (video_layers_menu);
   unload_menu (video_palette_menu);
   unload_menu (video_advanced_menu);
   unload_menu (video_menu);
   unload_menu (options_input_menu);
   unload_menu (options_cpu_usage_menu);
   unload_menu (options_gui_theme_menu);
   unload_menu (options_menu);
   unload_menu (netplay_menu);
   unload_menu (help_menu);
   unload_menu (top_menu);
}

static INLINE void unload_dialogs (void)
{
   unload_dialog (main_dialog);
   unload_dialog (main_replay_record_start_dialog);
   unload_dialog (machine_save_state_save_dialog);
   unload_dialog (machine_cheat_manager_add_dialog);
   unload_dialog (machine_cheat_manager_dialog);
   unload_dialog (options_input_configure_dialog);
   unload_dialog (netplay_dialog);
   unload_dialog (lobby_dialog);
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
   static USTRING audio_volume_text;

#ifndef USE_OPENAL
   DISABLE_MENU_ITEM(audio_subsystem_menu_openal);
#endif

   SET_MENU_ITEM_ENABLED(audio_mixing_quality_menu_interpolation,
      (audio_subsystem != AUDIO_SUBSYSTEM_OPENAL));

   if (!rom_is_loaded)
   {
      DISABLE_MENU_ITEM(main_menu_resume);
      DISABLE_MENU_ITEM(main_menu_close);
      DISABLE_SUBMENU(main_replay_menu);
      DISABLE_MENU_ITEM(main_menu_save_snapshot);
      DISABLE_MENU_ITEM(machine_menu_soft_reset);
      DISABLE_MENU_ITEM(machine_menu_hard_reset);
      DISABLE_SUBMENU(machine_save_state_menu);
      DISABLE_MENU_ITEM(machine_menu_cheat_manager);
      DISABLE_MENU_ITEM(video_layers_menu_flip_mirroring);
   }

   if (!apu_stereo_mode)
   {
      DSP_DISABLE_EFFECTOR(DSP_EFFECTOR_SWAP_CHANNELS);
      DSP_DISABLE_EFFECTOR(DSP_EFFECTOR_WIDE_STEREO_TYPE_1);
      DSP_DISABLE_EFFECTOR(DSP_EFFECTOR_WIDE_STEREO_TYPE_2);
      DSP_DISABLE_EFFECTOR(DSP_EFFECTOR_WIDE_STEREO_TYPE_3);

      DISABLE_MENU_ITEM(audio_mixing_channels_menu_swap_channels);
      DISABLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_1);
      DISABLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_2);
      DISABLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_3);
   }
   else
   {
      ENABLE_MENU_ITEM(audio_mixing_channels_menu_swap_channels);
      ENABLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_1);
      ENABLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_2);
      ENABLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_3);
   }

   TOGGLE_MENU_ITEM(main_open_recent_menu_lock, lock_recent);

   TOGGLE_MENU_ITEM(main_replay_select_menu_0, (replay_index == 0));
   TOGGLE_MENU_ITEM(main_replay_select_menu_1, (replay_index == 1));
   TOGGLE_MENU_ITEM(main_replay_select_menu_2, (replay_index == 2));
   TOGGLE_MENU_ITEM(main_replay_select_menu_3, (replay_index == 3));
   TOGGLE_MENU_ITEM(main_replay_select_menu_4, (replay_index == 4));

   TOGGLE_MENU_ITEM(machine_save_state_select_menu_0, (save_state_index == 0));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_1, (save_state_index == 1));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_2, (save_state_index == 2));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_3, (save_state_index == 3));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_4, (save_state_index == 4));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_5, (save_state_index == 5));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_6, (save_state_index == 6));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_7, (save_state_index == 7));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_8, (save_state_index == 8));
   TOGGLE_MENU_ITEM(machine_save_state_select_menu_9, (save_state_index == 9));

   TOGGLE_MENU_ITEM(machine_save_state_autosave_menu_disabled,   (input_autosave_interval == 0));
   TOGGLE_MENU_ITEM(machine_save_state_autosave_menu_10_seconds, (input_autosave_interval == 10));
   TOGGLE_MENU_ITEM(machine_save_state_autosave_menu_30_seconds, (input_autosave_interval == 30));
   TOGGLE_MENU_ITEM(machine_save_state_autosave_menu_60_seconds, (input_autosave_interval == 60));

   TOGGLE_MENU_ITEM(machine_region_menu_automatic, (machine_region == MACHINE_REGION_AUTOMATIC));
   TOGGLE_MENU_ITEM(machine_region_menu_ntsc,      (machine_region == MACHINE_REGION_NTSC));
   TOGGLE_MENU_ITEM(machine_region_menu_pal,       (machine_region == MACHINE_REGION_PAL));

   TOGGLE_MENU_ITEM(audio_menu_enabled, audio_enable_output);

   TOGGLE_MENU_ITEM(audio_subsystem_menu_none,   (audio_subsystem == AUDIO_SUBSYSTEM_NONE));
   TOGGLE_MENU_ITEM(audio_subsystem_menu_allegro,(audio_subsystem == AUDIO_SUBSYSTEM_ALLEGRO));
   TOGGLE_MENU_ITEM(audio_subsystem_menu_openal, (audio_subsystem == AUDIO_SUBSYSTEM_OPENAL));

   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_8000_hz,  (audio_sample_rate == 8000));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_11025_hz, (audio_sample_rate == 11025));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_16000_hz, (audio_sample_rate == 16000));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_22050_hz, (audio_sample_rate == 22050));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_32000_hz, (audio_sample_rate == 32000));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_44100_hz, (audio_sample_rate == 44100));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_48000_hz, (audio_sample_rate == 48000));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_80200_hz, (audio_sample_rate == 80200));
   TOGGLE_MENU_ITEM(audio_mixing_frequency_menu_96000_hz, (audio_sample_rate == 96000));

   TOGGLE_MENU_ITEM(audio_mixing_channels_menu_mono,                  !apu_stereo_mode);
   TOGGLE_MENU_ITEM(audio_mixing_channels_menu_stereo_mix,            (apu_stereo_mode == APU_STEREO_MODE_4));
   TOGGLE_MENU_ITEM(audio_mixing_channels_menu_virtual_stereo_mode_1, (apu_stereo_mode == APU_STEREO_MODE_1));
   TOGGLE_MENU_ITEM(audio_mixing_channels_menu_virtual_stereo_mode_2, (apu_stereo_mode == APU_STEREO_MODE_2));
   TOGGLE_MENU_ITEM(audio_mixing_channels_menu_stereo,                (apu_stereo_mode == APU_STEREO_MODE_3));
   TOGGLE_MENU_ITEM(audio_mixing_channels_menu_swap_channels,         dsp_get_effector_enabled (DSP_EFFECTOR_SWAP_CHANNELS));

   TOGGLE_MENU_ITEM(audio_mixing_quality_menu_low_8_bit,     (audio_sample_size == 8));
   TOGGLE_MENU_ITEM(audio_mixing_quality_menu_high_16_bit,   (audio_sample_size == 16));
   TOGGLE_MENU_ITEM(audio_mixing_quality_menu_interpolation, audio_interpolation);
   TOGGLE_MENU_ITEM(audio_mixing_quality_menu_dithering,     dsp_get_effector_enabled (DSP_EFFECTOR_DITHER));

   TOGGLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_1, dsp_get_effector_enabled (DSP_EFFECTOR_WIDE_STEREO_TYPE_1));
   TOGGLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_2, dsp_get_effector_enabled (DSP_EFFECTOR_WIDE_STEREO_TYPE_2));
   TOGGLE_MENU_ITEM(audio_effects_menu_wide_stereo_type_3, dsp_get_effector_enabled (DSP_EFFECTOR_WIDE_STEREO_TYPE_3));

   TOGGLE_MENU_ITEM(audio_filters_menu_low_pass_type_1,    dsp_get_effector_enabled (DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_1));
   TOGGLE_MENU_ITEM(audio_filters_menu_low_pass_type_2,    dsp_get_effector_enabled (DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_2));
   TOGGLE_MENU_ITEM(audio_filters_menu_low_pass_type_3,    dsp_get_effector_enabled (DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_3));
   TOGGLE_MENU_ITEM(audio_filters_menu_high_pass,          dsp_get_effector_enabled (DSP_EFFECTOR_HIGH_PASS_FILTER));
   TOGGLE_MENU_ITEM(audio_filters_menu_delta_sigma_filter, dsp_get_effector_enabled (DSP_EFFECTOR_DELTA_SIGMA_FILTER));

   TOGGLE_MENU_ITEM(audio_channels_menu_square_wave_a, dsp_get_channel_enabled (APU_CHANNEL_SQUARE_1));
   TOGGLE_MENU_ITEM(audio_channels_menu_square_wave_b, dsp_get_channel_enabled (APU_CHANNEL_SQUARE_2));
   TOGGLE_MENU_ITEM(audio_channels_menu_triangle_wave, dsp_get_channel_enabled (APU_CHANNEL_TRIANGLE));
   TOGGLE_MENU_ITEM(audio_channels_menu_white_noise,   dsp_get_channel_enabled (APU_CHANNEL_NOISE));
   TOGGLE_MENU_ITEM(audio_channels_menu_digital,       dsp_get_channel_enabled (APU_CHANNEL_DMC));
   TOGGLE_MENU_ITEM(audio_channels_menu_extended,      dsp_get_channel_enabled (APU_CHANNEL_EXTRA));

#ifdef ALLEGRO_DOS

   TOGGLE_MENU_ITEM(video_driver_dos_menu_vga,           (gfx_driver->id == GFX_VGA));
   TOGGLE_MENU_ITEM(video_driver_dos_menu_vga_mode_x,    (gfx_driver->id == GFX_MODEX));
   TOGGLE_MENU_ITEM(video_driver_dos_menu_vesa,          (gfx_driver->id == GFX_VESA1));
   TOGGLE_MENU_ITEM(video_driver_dos_menu_vesa_2_banked, (gfx_driver->id == GFX_VESA2B));
   TOGGLE_MENU_ITEM(video_driver_dos_menu_vesa_2_linear, (gfx_driver->id == GFX_VESA2L));
   TOGGLE_MENU_ITEM(video_driver_dos_menu_vesa_3,        (gfx_driver->id == GFX_VESA3));
   TOGGLE_MENU_ITEM(video_driver_dos_menu_vesa_vbe_af,   (gfx_driver->id == GFX_VBEAF));

#endif   /* ALLEGRO_DOS */

#ifdef ALLEGRO_WINDOWS

   TOGGLE_MENU_ITEM(video_driver_windows_menu_directx,         (gfx_driver->id == GFX_DIRECTX));
   TOGGLE_MENU_ITEM(video_driver_windows_menu_directx_window,  (gfx_driver->id == GFX_DIRECTX_WIN));
   TOGGLE_MENU_ITEM(video_driver_windows_menu_directx_overlay, (gfx_driver->id == GFX_DIRECTX_OVL));
   TOGGLE_MENU_ITEM(video_driver_windows_menu_gdi,             (gfx_driver->id == GFX_GDI));

#endif   /* ALLEGRO_WINDOWS */

#ifdef ALLEGRO_LINUX

   TOGGLE_MENU_ITEM(video_driver_linux_menu_vga,         (gfx_driver->id == GFX_VGA));
   TOGGLE_MENU_ITEM(video_driver_linux_menu_vga_mode_x,  (gfx_driver->id == GFX_MODEX));
   TOGGLE_MENU_ITEM(video_driver_linux_menu_vesa_vbe_af, (gfx_driver->id == GFX_VBEAF));
#ifdef GFX_FBCON
   TOGGLE_MENU_ITEM(video_driver_linux_menu_framebuffer, (gfx_driver->id == GFX_FBCON));
#endif
#ifdef GFX_SVGALIB
   TOGGLE_MENU_ITEM(video_driver_linux_menu_svgalib,     (gfx_driver->id == GFX_SVGALIB));
#endif

#endif   /* ALLEGRO_LINUX */

#ifdef ALLEGRO_UNIX

   TOGGLE_MENU_ITEM(video_driver_unix_menu_x_windows,      (gfx_driver->id == GFX_XWINDOWS));
   TOGGLE_MENU_ITEM(video_driver_unix_menu_x_windows_full, (gfx_driver->id == GFX_XWINDOWS_FULLSCREEN));
   TOGGLE_MENU_ITEM(video_driver_unix_menu_x_dga,          (gfx_driver->id == GFX_XDGA));
   TOGGLE_MENU_ITEM(video_driver_unix_menu_x_dga_full,     (gfx_driver->id == GFX_XDGA_FULLSCREEN));
   TOGGLE_MENU_ITEM(video_driver_unix_menu_x_dga_2,        (gfx_driver->id == GFX_XDGA2));

#endif   /* ALLEGRO_UNIX */

   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_256_224,   ((SCREEN_W == 256)  && (SCREEN_H == 224)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_256_240,   ((SCREEN_W == 256)  && (SCREEN_H == 240)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_512_448,   ((SCREEN_W == 512)  && (SCREEN_H == 448)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_512_480,   ((SCREEN_W == 512)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_768_672,   ((SCREEN_W == 768)  && (SCREEN_H == 672)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_768_720,   ((SCREEN_W == 768)  && (SCREEN_H == 720)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_1024_896,  ((SCREEN_W == 1024) && (SCREEN_H == 896)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_1024_960,  ((SCREEN_W == 1024) && (SCREEN_H == 960)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_1280_1120, ((SCREEN_W == 1280) && (SCREEN_H == 1120)));
   TOGGLE_MENU_ITEM(video_resolution_proportionate_menu_1280_1200, ((SCREEN_W == 1280) && (SCREEN_H == 1200)));

   TOGGLE_MENU_ITEM(video_resolution_menu_320_240,   ((SCREEN_W == 320)  && (SCREEN_H == 240)));
   TOGGLE_MENU_ITEM(video_resolution_menu_640_480,   ((SCREEN_W == 640)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(video_resolution_menu_800_600,   ((SCREEN_W == 800)  && (SCREEN_H == 600)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1024_768,  ((SCREEN_W == 1024) && (SCREEN_H == 768)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1152_864,  ((SCREEN_W == 1152) && (SCREEN_H == 864)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1280_1024, ((SCREEN_W == 1280) && (SCREEN_H == 1024)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1600_1200, ((SCREEN_W == 1600) && (SCREEN_H == 1200)));

   TOGGLE_MENU_ITEM(video_resolution_extended_menu_400_300,  ((SCREEN_W == 400)  && (SCREEN_H == 300)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_480_360,  ((SCREEN_W == 480)  && (SCREEN_H == 360)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_512_384,  ((SCREEN_W == 512)  && (SCREEN_H == 384)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_640_400,  ((SCREEN_W == 640)  && (SCREEN_H == 400)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_720_480,  ((SCREEN_W == 720)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_720_576,  ((SCREEN_W == 720)  && (SCREEN_H == 576)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_848_480,  ((SCREEN_W == 848)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_1280_720, ((SCREEN_W == 1280) && (SCREEN_H == 720)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_1280_960, ((SCREEN_W == 1280) && (SCREEN_H == 960)));
   TOGGLE_MENU_ITEM(video_resolution_extended_menu_1360_768, ((SCREEN_W == 1360) && (SCREEN_H == 768)));

   TOGGLE_MENU_ITEM(video_colors_menu_paletted_8_bit,    (video_get_color_depth () == 8));
   TOGGLE_MENU_ITEM(video_colors_menu_true_color_15_bit, (video_get_color_depth () == 15));
   TOGGLE_MENU_ITEM(video_colors_menu_true_color_16_bit, (video_get_color_depth () == 16));
   TOGGLE_MENU_ITEM(video_colors_menu_true_color_24_bit, (video_get_color_depth () == 24));
   TOGGLE_MENU_ITEM(video_colors_menu_true_color_32_bit, (video_get_color_depth () == 32));

   TOGGLE_MENU_ITEM(video_blitter_menu_automatic,       (video_get_blitter () == VIDEO_BLITTER_AUTOMATIC));
   TOGGLE_MENU_ITEM(video_blitter_menu_normal,          (video_get_blitter () == VIDEO_BLITTER_NORMAL));
   TOGGLE_MENU_ITEM(video_blitter_menu_des,             (video_get_blitter () == VIDEO_BLITTER_DES));
   TOGGLE_MENU_ITEM(video_blitter_menu_interpolated_2x, (video_get_blitter () == VIDEO_BLITTER_INTERPOLATED_2X));
   TOGGLE_MENU_ITEM(video_blitter_menu_2xscl,           (video_get_blitter () == VIDEO_BLITTER_2XSCL));
   TOGGLE_MENU_ITEM(video_blitter_menu_desii,           (video_get_blitter () == VIDEO_BLITTER_DESII));
   TOGGLE_MENU_ITEM(video_blitter_menu_super_2xscl,     (video_get_blitter () == VIDEO_BLITTER_SUPER_2XSCL));
   TOGGLE_MENU_ITEM(video_blitter_menu_ultra_2xscl,     (video_get_blitter () == VIDEO_BLITTER_ULTRA_2XSCL));
   TOGGLE_MENU_ITEM(video_blitter_menu_hq2x,            (video_get_blitter () == VIDEO_BLITTER_HQ2X));
   TOGGLE_MENU_ITEM(video_blitter_menu_nes_ntsc,        (video_get_blitter () == VIDEO_BLITTER_NES_NTSC));
   TOGGLE_MENU_ITEM(video_blitter_menu_interpolated_3x, (video_get_blitter () == VIDEO_BLITTER_INTERPOLATED_3X));
   TOGGLE_MENU_ITEM(video_blitter_menu_hq3x,            (video_get_blitter () == VIDEO_BLITTER_HQ3X));
   TOGGLE_MENU_ITEM(video_blitter_menu_hq4x,            (video_get_blitter () == VIDEO_BLITTER_HQ4X));
   TOGGLE_MENU_ITEM(video_blitter_menu_stretched,       (video_get_blitter () == VIDEO_BLITTER_STRETCHED));

   TOGGLE_MENU_ITEM(video_filters_menu_scanlines_25_percent,  (video_get_filter_list () & VIDEO_FILTER_SCANLINES_LOW));
   TOGGLE_MENU_ITEM(video_filters_menu_scanlines_50_percent,  (video_get_filter_list () & VIDEO_FILTER_SCANLINES_MEDIUM));
   TOGGLE_MENU_ITEM(video_filters_menu_scanlines_100_percent, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_HIGH));

   TOGGLE_MENU_ITEM(video_menu_page_buffer, video_enable_page_buffer);
   TOGGLE_MENU_ITEM(video_menu_vsync,       video_enable_vsync);

   TOGGLE_MENU_ITEM(video_palette_menu_ntsc_color,     (video_get_palette_id () == DATA_INDEX(DEFAULT_PALETTE)));
   TOGGLE_MENU_ITEM(video_palette_menu_ntsc_grayscale, (video_get_palette_id () == DATA_INDEX(GRAYSCALE_PALETTE)));
   TOGGLE_MENU_ITEM(video_palette_menu_gnuboy,         (video_get_palette_id () == DATA_INDEX(GNUBOY_PALETTE)));
   TOGGLE_MENU_ITEM(video_palette_menu_nester,         (video_get_palette_id () == DATA_INDEX(NESTER_PALETTE)));
   TOGGLE_MENU_ITEM(video_palette_menu_nesticle,       (video_get_palette_id () == DATA_INDEX(NESTICLE_PALETTE)));
   TOGGLE_MENU_ITEM(video_palette_menu_modern_ntsc,    (video_get_palette_id () == DATA_INDEX(MODERN_NTSC_PALETTE)));
   TOGGLE_MENU_ITEM(video_palette_menu_modern_pal,     (video_get_palette_id () == DATA_INDEX(MODERN_PAL_PALETTE)));
   TOGGLE_MENU_ITEM(video_palette_menu_ega_mode_1,     (video_get_palette_id () == DATA_INDEX(EGA_PALETTE_1)));
   TOGGLE_MENU_ITEM(video_palette_menu_ega_mode_2,     (video_get_palette_id () == DATA_INDEX(EGA_PALETTE_2)));
   TOGGLE_MENU_ITEM(video_palette_menu_custom,         (video_get_palette_id () == -1));

   TOGGLE_MENU_ITEM(video_advanced_menu_force_fullscreen, video_force_fullscreen);

   TOGGLE_MENU_ITEM(video_layers_menu_sprites_a,                 ppu_enable_sprite_layer_a);
   TOGGLE_MENU_ITEM(video_layers_menu_sprites_b,                 ppu_enable_sprite_layer_b);
   TOGGLE_MENU_ITEM(video_layers_menu_background,                ppu_enable_background_layer);
   TOGGLE_MENU_ITEM(video_layers_menu_hide_horizontal_scrolling, (video_edge_clipping & VIDEO_EDGE_CLIPPING_HORIZONTAL));
   TOGGLE_MENU_ITEM(video_layers_menu_hide_vertical_scrolling,   (video_edge_clipping & VIDEO_EDGE_CLIPPING_VERTICAL));

   TOGGLE_MENU_ITEM(options_input_menu_enable_zapper, input_enable_zapper);

   TOGGLE_MENU_ITEM(options_menu_show_status, video_display_status);

   TOGGLE_MENU_ITEM(options_cpu_usage_menu_passive,    (cpu_usage == CPU_USAGE_PASSIVE));
   TOGGLE_MENU_ITEM(options_cpu_usage_menu_normal,     (cpu_usage == CPU_USAGE_NORMAL));
   TOGGLE_MENU_ITEM(options_cpu_usage_menu_aggressive, (cpu_usage == CPU_USAGE_AGGRESSIVE));

   TOGGLE_MENU_ITEM(options_gui_theme_menu_classic,         (last_theme == &classic_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_stainless_steel, (last_theme == &stainless_steel_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_zero_4,          (last_theme == &zero_4_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_panta,           (last_theme == &panta_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_xodiac,          (last_theme == &xodiac_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_monochrome,      (last_theme == &monochrome_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_essence,         (last_theme == &essence_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_voodoo,          (last_theme == &voodoo_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_hugs_and_kisses, (last_theme == &hugs_and_kisses_theme));

   /* TODO: Find a better way to do this. */
   uszprintf (audio_volume_text, sizeof (audio_volume_text), "Current "
      "level: %d%%", (int)ROUND((dsp_master_volume * 100.0f)));
   audio_volume_menu[0].text = audio_volume_text;
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
      if (background_image->h < 200)
      {
         blit (background_image, bmp, 0, 0, ((bmp->w / 2) -
            (background_image->w / 2)), ((bmp->h / 2) - (background_image->h
               / 2)), background_image->w, background_image->h);
      }
      else
      {
         BITMAP *buffer;
   
         /* Hack to handle color conversion. */
   
         buffer = create_bitmap (background_image->w, background_image->h);
         if (!buffer)
         {
            WARN("Failed to create background buffer");
            return;
         }
   
         blit (background_image, buffer, 0, 0, 0, 0, background_image->w,
            background_image->h);
         stretch_blit (buffer, bmp, 0, 0, buffer->w, buffer->h, 0, 0,
            bmp->w, bmp->h);
   
         destroy_bitmap (buffer);
      }
   }
}

int gui_init (void)
{
   int index;

   /* Set up replacement objects. */
   gui_menu_draw_menu = sl_draw_menu;
   gui_menu_draw_menu_item = sl_draw_menu_item;

   /* Set up menus & dialogs. */
   load_menus ();
   load_dialogs ();

#ifdef ALLEGRO_DOS

   DISABLE_SUBMENU(video_driver_windows_menu);
   DISABLE_SUBMENU(video_driver_linux_menu);
   DISABLE_SUBMENU(video_driver_unix_menu);
   DISABLE_MENU_ITEM(video_advanced_menu_force_fullscreen);
   DISABLE_SUBMENU(options_cpu_usage_menu);
   DISABLE_SUBMENU(netplay_menu);

#endif   /* ALLEGRO_DOS */

#ifdef ALLEGRO_WINDOWS

   DISABLE_SUBMENU(video_driver_dos_menu);
   DISABLE_SUBMENU(video_driver_linux_menu);
   DISABLE_SUBMENU(video_driver_unix_menu);

#endif   /* ALLEGRO_WINDOWS */

#ifdef ALLEGRO_UNIX

   DISABLE_SUBMENU(video_driver_dos_menu);
   DISABLE_SUBMENU(video_driver_windows_menu);
 
#ifdef ALLEGRO_LINUX

#ifndef GFX_FBCON
   DISABLE_MENU_ITEM(video_driver_linux_menu_framebuffer);
#endif
#ifndef GFX_SVGALIB
   DISABLE_MENU_ITEM(video_driver_linux_menu_svgalib);
#endif

#else /* ALLEGRO_LINUX */

   DISABLE_SUBMENU(video_driver_linux_menu);

#endif   /* !ALLEGRO_LINUX */

#endif   /* ALLEGRO_UNIX */

#ifndef USE_HAWKNL
   DISABLE_SUBMENU(netplay_menu);
#endif

   /* Select default palette. */
   CHECK_MENU_ITEM(video_palette_menu_modern_ntsc);

   /* Load configuration */
   gui_theme_id = get_config_int ("gui", "theme",       GUI_THEME_PANTA);
   lock_recent  = get_config_int ("gui", "lock_recent", FALSE);

   /* Load up recent items. */

   main_open_recent_menu_clear ();

   for (index = 0; index < OPEN_RECENT_SLOTS; index++)
   {
      USTRING key;
      const char *path;
      UCHAR *filename = open_recent_filenames[index];
      UCHAR *text     = open_recent_menu_texts[index];
      MENU  *menu     = &main_open_recent_menu[index];

      USTRING_CLEAR(key);
      uszprintf (key, sizeof (key), "recent%d", index);

      path = get_config_string ("gui", key, NULL);
      if (!path)
         continue;

      uszprintf (filename, USTRING_SIZE, "%s", path);
      uszprintf (text,     USTRING_SIZE, "&%d: %s", index, get_filename
         (path));

      /* Update menu. */
      menu->text = text;

      /* Enable menu. */
      menu->flags &= ~D_DISABLED;
   }

   /* Cheap hack to fix palette. */
   gui_is_active = TRUE;
   set_theme ();
   gui_is_active = FALSE;

   return (0);
}

void gui_exit (void)
{
   int index;
   STRING save_path;
   STRING host;

   /* Save configuration. */
   STRING_CLEAR(save_path);
   strncpy (save_path, get_config_string ("gui", "save_path", "./"),
      sizeof (save_path) - 1);
   set_config_string ("gui", "save_path", save_path);

   STRING_CLEAR(host);
   strncpy (host, get_config_string ("netplay", "host", ""), (sizeof (host)
      - 1));
   set_config_string ("netplay", "host", host);

   set_config_int ("gui", "theme",       gui_theme_id);
   set_config_int ("gui", "lock_recent", lock_recent);

   /* Save recent items. */

   for (index = 0; index < OPEN_RECENT_SLOTS; index++)
   {
      USTRING key;
      const UCHAR *filename = open_recent_filenames[index];

      if (!filename)
         continue;

      USTRING_CLEAR(key);
      uszprintf (key, sizeof (key), "recent%d", index);

      set_config_string ("gui", key, filename);
   }

   unload_menus ();
   unload_dialogs ();
}

static INLINE void cycle_video (void);

int show_gui (BOOL first_run)
{
   gui_needs_restart = FALSE;
   gui_is_active = TRUE;

   audio_suspend ();

   want_exit = FALSE;

   /* Set up menus. */
   update_menus ();

   cycle_video ();

   if (first_run)
   {
      /* Show welcome message. */
      help_menu_version ();
   }

   run_dialog (main_dialog);

   /* Shut down GUI. */
   gui_is_active = FALSE;

   cycle_video ();

   audio_resume ();

   return (want_exit);
}

static INLINE void cycle_audio (void);

void gui_handle_keypress (int c)
{
   switch ((c >> 8))
   {
      case KEY_F1:
      {
         /* Save snapshot. */
         main_menu_save_snapshot ();

         break;
      }

      case KEY_F2:
      {
         /* Toggle status display. */
         options_menu_show_status ();

         break;
      }

      case KEY_F3:
      {
         /* Quick save state. */
         machine_save_state_menu_quick_save ();

         break;
      }

      case KEY_F4:
      {
         /* Quick load state. */

         if (!(input_mode & INPUT_MODE_REPLAY))
            machine_save_state_menu_quick_load ();

         break;
      }

      case KEY_F5:
      {
         /* Save state. */
         machine_save_state_menu_save ();

         break;
      }

      case KEY_F6:
      {
         /* Load state. */

         if (!(input_mode & INPUT_MODE_REPLAY))
            machine_save_state_menu_restore ();

         break;
      }

      case KEY_F7:
      {
         /* Toggle sprites. */

         video_layers_menu_sprites_a ();
         video_layers_menu_sprites_b ();

         break;
      }

      case KEY_F8:
      {
         /* Toggle background. */
         video_layers_menu_background ();

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

         cycle_audio ();

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
            save_state_index = ((c >> 8) - KEY_0);
    
            message_local ("Machine state slot set to %d.",
               save_state_index);
         }

         break;
      }

      case KEY_MINUS:
      case KEY_MINUS_PAD:
      {
         audio_volume_menu_decrease ();

         break;
      }

      case KEY_EQUALS:
      case KEY_PLUS_PAD:
      {
         audio_volume_menu_increase ();

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

/* --- Utility functions. --- */

static INLINE void set_autosave (int interval)
{
   /* This function simply sets the save state autosave interval to
      'interval' seconds (in game speed, not real world speed :b). */

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
   apu_update ();
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

static void update_machine_type (void)
{
   /* This function resyncs machine_type to the value of machine_region. */

   switch (machine_region)
   {
      case MACHINE_REGION_AUTOMATIC:
      {
         if (rom_is_loaded)
         {
            /* Try to determine a suitable machine type by searching for
               country codes in the ROM's filename. */

            if (ustrstr (global_rom.filename, "(E)"))
            {
               /* Europe. */
               machine_type = MACHINE_TYPE_PAL;
            }
            else
            {
               /* Default to NTSC. */
               machine_type = MACHINE_TYPE_NTSC;
            }
         }
         else  
         {
            /* Default to NTSC. */
            machine_type = MACHINE_TYPE_NTSC;
         }

         break;
      }

      case MACHINE_REGION_NTSC:
      {
         /* NTSC (60 Hz). */
         machine_type = MACHINE_TYPE_NTSC;

         break;
      }

      case MACHINE_REGION_PAL:
      {
         /* PAL (50 Hz). */
         machine_type = MACHINE_TYPE_PAL;

         break;
      }
   }

   /* Cycle audio to match new emulation speeds. */
   cycle_audio ();
}

static int main_replay_menu_select (void);

static INLINE int load_file (const UCHAR *filename)
{
   /* This function loads the ROM specified by filename.  The file is NOT
      automatically added to the recent items list.  That must be done
      manually (currently by main_menu_open()).

      The return value of this function should be passed back to the calling
      dialog (e.g, D_CLOSE to close it and start the emulation, etc.). */

   ROM rom;

   if (load_rom (filename, &rom) != 0)
   {
      gui_message (GUI_ERROR_COLOR, "Failed to load ROM!");

      return (D_O_K);
   }
   else
   {
      USTRING scratch;

      if (rom_is_loaded)
      {
         /* Close currently open ROM and save data. */
         main_menu_close ();
      }

      memcpy (&global_rom, &rom, sizeof (ROM));

      /* Update save state titles. */
      machine_save_state_menu_select ();
      /* Update replay titles. */
      main_replay_menu_select ();

      rom_is_loaded = TRUE;

      /* Fixup machine type from region. */
      update_machine_type ();

      /* Initialize machine. */
      machine_init ();

      ENABLE_MENU_ITEM(main_menu_resume);
      ENABLE_MENU_ITEM(main_menu_close);
      ENABLE_SUBMENU(main_replay_menu);
      ENABLE_MENU_ITEM(main_menu_save_snapshot);
      ENABLE_MENU_ITEM(machine_menu_soft_reset);
      ENABLE_MENU_ITEM(machine_menu_hard_reset);
      ENABLE_SUBMENU(machine_save_state_menu);
      ENABLE_MENU_ITEM(machine_menu_cheat_manager);
      ENABLE_MENU_ITEM(video_layers_menu_flip_mirroring);

      /* Update window title. */
      uszprintf (scratch, sizeof (scratch), "FakeNES - %s", get_filename
         (global_rom.filename));
      set_window_title (scratch);

      return (D_CLOSE);
   }
}

static int open_lobby (void)
{
   /* This function handles the entire GUI end of the NetPlay lobby.  It
      does not return until the NetPlay session has been terminated.

      Returns one of the following:
         D_O_K   - The NetPlay session has been closed, by pressing either
                   the [ x] close button or the Cancel button.
         D_CLOSE - The Netplay session is still open, all neccessary data
                   has been distributed and subsequently laoded, and control
                   should be transfered to the main loop. */

   BITMAP *bmp;                       
   DIALOG *dialog;
   int index = 0;
   DIALOG *obj_frame;
   DIALOG *obj_chat;
   DIALOG *obj_list;
   DIALOG *obj_message;
   DIALOG *obj_load;
   DIALOG *obj_ok;
   USTRING chat;
   USTRING list;
   USTRING message;
   DIALOG_PLAYER *player;
   int object_id;

   bmp = gui_get_screen ();

   /* Clear screen. */
   clear_bitmap (bmp);

   /* Get dialog. */
   dialog = lobby_dialog;

   /* Center dialog. */
   centre_dialog (dialog);

   while (dialog[index].d1 != SL_FRAME_END)
   {
      /* Update colors. */

      DIALOG *object = &dialog[index];

      object->fg = GUI_TEXT_COLOR;
      object->bg = gui_bg_color;

      index++;
   }

   /* Get dialog objects. */
   obj_frame   = &dialog[LOBBY_DIALOG_FRAME];
   obj_chat    = &dialog[LOBBY_DIALOG_CHAT];
   obj_list    = &dialog[LOBBY_DIALOG_LIST];
   obj_message = &dialog[LOBBY_DIALOG_MESSAGE];
   obj_load    = &dialog[LOBBY_DIALOG_LOAD_BUTTON];
   obj_ok      = &dialog[LOBBY_DIALOG_OK_BUTTON];

   /* Set up dialog objects. */

   obj_frame->dp3 = DATA_TO_FONT(LARGE_FONT);

   obj_chat->bg = makecol (0, 0, 0);
   obj_chat->fg = makecol (240, 240, 240);
   obj_chat->d1 = (sizeof (chat) - 1);
   obj_chat->dp = chat;

   obj_list->bg = makecol (0, 0, 0);
   obj_list->fg = makecol (240, 240, 240);
   obj_list->d1 = (sizeof (list) - 1);
   obj_list->dp = list;

   obj_message->d1 = (sizeof (message) - 1);
   obj_message->dp = message;

   if (netplay_mode != NETPLAY_MODE_SERVER_OPEN)
      obj_load->flags |= D_DISABLED;

   obj_ok->flags |= D_DISABLED;

   /* Clear text buffers. */
   USTRING_CLEAR(chat);
   USTRING_CLEAR(list);
   USTRING_CLEAR(message);

   /* Run dialog. */

   player = init_dialog (dialog, -1);
   if (!player)
   {
      gui_message (GUI_ERROR_COLOR, "Failed to create dialog player!");
      return (D_O_K);
   }

   while (update_dialog (player))
   {
      netplay_process ();

      gui_heartbeat ();
   }

   object_id = shutdown_dialog (player);

   switch (object_id)
   {
      case LOBBY_DIALOG_OK_BUTTON:
         return (D_CLOSE);

      default:
      {
         /* End NetPlay session. */
         netplay_close ();
   
         /* Clear screen. */
         clear_bitmap (bmp);
      
         /* Draw background. */
         draw_background ();
      
         message_local ("NetPlay session closed.");

         return (D_O_K);
      }
   }
}

/* --- Menu handlers. --- */

static int main_menu_resume (void)
{
    return (D_CLOSE);
}

static int main_menu_open (void)
{
   USTRING path;
   BITMAP *bmp;
   int w, h;
   int result;
   USTRING scratch;

   /* Retrive path from configuration file. */
   USTRING_CLEAR(path);
   ustrncat (path, get_config_string ("gui", "open_path", "/"), (sizeof
      (path) - 1));

   /* Get drawing surface. */
   bmp = gui_get_screen ();

   /* Calculate file selector dimensions. */
   w = ROUND((bmp->w * 0.80f));
   h = ROUND((bmp->h * 0.67f));

#ifdef USE_ZLIB
   result = file_select_ex ("Supported formats (*.NES, *.GZ, *.ZIP)", path,
      "NES;nes;GZ;gz;ZIP;zip", sizeof (path), w, h);
#else
   result = file_select_ex ("Supported formats (*.NES)", path, "NES;nes",
      sizeof (path), w, h);
#endif

   /* Update path. */
   set_config_string ("gui", "open_path", replace_filename (scratch, path,
      "", sizeof (scratch)));

   if (result != 0)
   {
      /* Dialog was OK'ed. */

      int result;

      result = load_file (path);

      if ((result == D_CLOSE) && !lock_recent)
      {
         /* Load succeeded; add file to recent items list. */

         int index;

         /* Move all existing entries down by 1 slot. */
         for (index = (OPEN_RECENT_SLOTS - 2); index >= 0; index--)
         {
            ustrncpy (open_recent_filenames[(index + 1)],
               open_recent_filenames[index], USTRING_SIZE);
         }

         /* Add new entry to the beginning of the list. */
         uszprintf (open_recent_filenames[0], USTRING_SIZE, "%s", path);

         /* Update menus. */

         for (index = 0; index < OPEN_RECENT_SLOTS; index++)
         {
            const UCHAR *filename = open_recent_filenames[index];
            UCHAR       *text     = open_recent_menu_texts[index];
            MENU        *menu     = &main_open_recent_menu[index];

            if (filename[0])
            {
               /* Build menu text. */
               uszprintf (text, USTRING_SIZE, "&%d: %s", index,
                  get_filename (filename));

               /* Enable menu. */
               menu->flags &= ~D_DISABLED;
            }
            else
            {
               /* Build menu text. */
               uszprintf (text, USTRING_SIZE, "&%d: %s", index,
                  UNUSED_SLOT_TEXT);

               /* Disable menu. */
               menu->flags |= D_DISABLED;
            }

            /* Set menu text. */
            menu->text = text;
         }
      }

      return (result);
   }

   /* Dialog was cancelled. */
   return (D_O_K);
}

#define OPEN_RECENT_MENU_HANDLER(index) \
   static int main_open_recent_menu_##index (void)  \
   {  \
      return (load_file (open_recent_filenames[index])); \
   }

OPEN_RECENT_MENU_HANDLER(0)
OPEN_RECENT_MENU_HANDLER(1)
OPEN_RECENT_MENU_HANDLER(2)
OPEN_RECENT_MENU_HANDLER(3)
OPEN_RECENT_MENU_HANDLER(4)
OPEN_RECENT_MENU_HANDLER(5)
OPEN_RECENT_MENU_HANDLER(6)
OPEN_RECENT_MENU_HANDLER(7)
OPEN_RECENT_MENU_HANDLER(8)
OPEN_RECENT_MENU_HANDLER(9)

#undef OPEN_RECENT_MENU_HANDLER

static int main_open_recent_menu_lock (void)
{
   lock_recent = !lock_recent;
   update_menus ();

   return (D_O_K);
}

static int main_open_recent_menu_clear (void)
{
   int index;

   for (index = 0; index < OPEN_RECENT_SLOTS; index++)
   {
      UCHAR *filename = open_recent_filenames[index];
      UCHAR *text     = open_recent_menu_texts[index];
      MENU  *menu     = &main_open_recent_menu[index];

      USTRING_CLEAR(filename);

      /* Build menu text. */
      uszprintf (text, USTRING_SIZE, "&%d: %s", index, UNUSED_SLOT_TEXT);

      /* Update menu. */
      menu->text = text;

      /* Disable menu. */
      menu->flags |= D_DISABLED;
   }

   return (D_O_K);
}

static int main_menu_close (void)
{
   /* Save SRAM. */
   save_sram ();      

   /* Save patches. */
   save_patches ();

   /* Unload ROM. */
   free_rom (&global_rom);
   rom_is_loaded = FALSE;

   update_menus ();

   cycle_video ();

   return (D_REDRAW);
}

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
REPLAY_SELECT_MENU_HANDLER(5)
REPLAY_SELECT_MENU_HANDLER(6)
REPLAY_SELECT_MENU_HANDLER(7)
REPLAY_SELECT_MENU_HANDLER(8)
REPLAY_SELECT_MENU_HANDLER(9)

#undef REPLAY_SELECT_MENU_HANDLER

static int main_replay_menu_select (void)
{
   int index;

   for (index = 0; index < REPLAY_SLOTS; index++)
   {
      UCHAR *title;
      UCHAR *text;

      title = replay_titles[index];
      text = replay_menu_texts[index];

      /* Get title. */
      get_replay_title (index, title, USTRING_SIZE);

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

   /* Duplicate title. */
   ustrncpy (title, replay_titles[replay_index], USTRING_SIZE);

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

   /* Open replay file. */
   if (!open_replay (replay_index, "w", title))
   {
      gui_message (GUI_ERROR_COLOR, "Failed to open new machine state "
         "file.");

      return (D_O_K);
   }

   DISABLE_MENU_ITEM(main_menu_open);
   DISABLE_SUBMENU(main_open_recent_menu);
   DISABLE_MENU_ITEM(main_replay_record_menu_start);
   ENABLE_MENU_ITEM(main_replay_record_menu_stop);
   DISABLE_SUBMENU(main_replay_select_menu);
   DISABLE_SUBMENU(main_replay_play_menu);
   DISABLE_SUBMENU(machine_save_state_autosave_menu);
   DISABLE_SUBMENU(netplay_menu);

   /* Enter replay recording mode. */
   input_mode |= INPUT_MODE_REPLAY;
   input_mode |= INPUT_MODE_REPLAY_RECORD;
 
   message_local ("Replay recording session started.");
 
   /* Update replay titles. */
   main_replay_menu_select ();

   return (D_CLOSE);
}

static int main_replay_record_menu_stop (void)
{
   /* Close replay. */
   close_replay ();

   /* Exit replay recording mode. */
   input_mode &= ~INPUT_MODE_REPLAY;
   input_mode &= ~INPUT_MODE_REPLAY_RECORD;

   ENABLE_MENU_ITEM(main_menu_open);
   ENABLE_SUBMENU(main_open_recent_menu);
   ENABLE_MENU_ITEM(main_replay_record_menu_start);
   DISABLE_MENU_ITEM(main_replay_record_menu_stop);
   ENABLE_SUBMENU(main_replay_select_menu);
   ENABLE_SUBMENU(main_replay_play_menu);
   ENABLE_SUBMENU(machine_save_state_autosave_menu);
   ENABLE_SUBMENU(netplay_menu);

   message_local ("Replay recording session stopped.");

   return (D_O_K);
}

static int main_replay_play_menu_start (void)
{
   if (!open_replay (replay_index, "r", NULL))
   {                       
      gui_message (GUI_ERROR_COLOR, "Failed to open machine state file.");

      return (D_O_K);
   }

   DISABLE_MENU_ITEM(main_menu_open);
   DISABLE_SUBMENU(main_open_recent_menu);
   DISABLE_MENU_ITEM(main_replay_play_menu_start);
   ENABLE_MENU_ITEM(main_replay_play_menu_stop);
   DISABLE_SUBMENU(main_replay_select_menu);
   DISABLE_SUBMENU(main_replay_record_menu);
   DISABLE_MENU_ITEM(machine_menu_soft_reset);
   DISABLE_MENU_ITEM(machine_menu_hard_reset);
   DISABLE_MENU_ITEM(machine_save_state_menu_quick_load);
   DISABLE_MENU_ITEM(machine_save_state_menu_restore);
   DISABLE_MENU_ITEM(machine_menu_cheat_manager);
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
   /* Close replay. */
   close_replay ();

   /* Exit replay playback mode. */
   if (!(input_mode & INPUT_MODE_CHAT))
      input_mode |= INPUT_MODE_PLAY;
   input_mode &= ~INPUT_MODE_REPLAY;
   input_mode &= ~INPUT_MODE_REPLAY_PLAY;

   ENABLE_MENU_ITEM(main_menu_open);
   ENABLE_SUBMENU(main_open_recent_menu);
   ENABLE_MENU_ITEM(main_replay_play_menu_start);
   DISABLE_MENU_ITEM(main_replay_play_menu_stop);
   ENABLE_SUBMENU(main_replay_select_menu);
   ENABLE_SUBMENU(main_replay_record_menu);
   ENABLE_MENU_ITEM(machine_menu_soft_reset);
   ENABLE_MENU_ITEM(machine_menu_hard_reset);
   ENABLE_MENU_ITEM(machine_save_state_menu_quick_load);
   ENABLE_MENU_ITEM(machine_save_state_menu_restore);
   ENABLE_MENU_ITEM(machine_menu_cheat_manager);
   ENABLE_SUBMENU(netplay_menu);

   if (gui_is_active)
      message_local ("Replay playback stopped.");
   else
      message_local ("Replay playback finished.");

   return (D_O_K);
}

static int main_menu_save_snapshot (void)
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

static int main_menu_exit (void)
{
   want_exit = TRUE;

   return (D_CLOSE);
}

static int machine_menu_soft_reset (void)
{
    cpu_reset ();

    return (D_CLOSE);
}

static int machine_menu_hard_reset (void)
{
    machine_reset ();

    return (D_CLOSE);
}

#define SAVE_STATE_SELECT_MENU_HANDLER(index)   \
   static int machine_save_state_select_menu_##index (void)   \
   {  \
      save_state_index = index;  \
      update_menus ();  \
      message_local ("Machine state slot set to %d.", index);  \
      return (D_O_K);   \
   }

SAVE_STATE_SELECT_MENU_HANDLER(0);
SAVE_STATE_SELECT_MENU_HANDLER(1);
SAVE_STATE_SELECT_MENU_HANDLER(2);
SAVE_STATE_SELECT_MENU_HANDLER(3);
SAVE_STATE_SELECT_MENU_HANDLER(4);
SAVE_STATE_SELECT_MENU_HANDLER(5);
SAVE_STATE_SELECT_MENU_HANDLER(6);
SAVE_STATE_SELECT_MENU_HANDLER(7);
SAVE_STATE_SELECT_MENU_HANDLER(8);
SAVE_STATE_SELECT_MENU_HANDLER(9);

#undef SAVE_STATE_MENU_HANDLER

static int machine_save_state_menu_quick_save (void)
{
   if (!save_state (-1, "QUICKSAVE"))
   {
      gui_message (GUI_ERROR_COLOR, "Quick Save failed.");

      return (D_O_K);
   }

   return (D_CLOSE);
}

static int machine_save_state_menu_quick_load (void)
{
   if (!load_state (-1))
   {
      gui_message (GUI_ERROR_COLOR, "Quick Load failed.");

      return (D_O_K);
   }

   return (D_CLOSE);
}

static int machine_save_state_menu_save (void)
{
   USTRING title;
   USTRING filename;

   /* Duplicate title. */
   ustrncpy (title, save_state_titles[save_state_index], sizeof (title));

   /* Patch up duplicate. */
   fix_save_title (title, sizeof (title));

   if (gui_is_active)
   {
      /* Allow user to customize title before save. */

      machine_save_state_save_dialog[4].d1 = (SAVE_TITLE_SIZE - 1);
      machine_save_state_save_dialog[4].dp = title;

      if (show_dialog (machine_save_state_save_dialog) != 5)
         return (D_O_K);
   }

   if (!save_state (save_state_index, title))
   {
      gui_message (GUI_ERROR_COLOR, "Failed to open new machine state "
         "file.");

      return (D_O_K);
   }

   /* Update save state titles. */
   machine_save_state_menu_select ();

   if (!input_autosave_triggered)
      message_local ("Machine state saved in slot %d.", save_state_index);

   return (D_CLOSE);
}

static int machine_save_state_menu_restore (void)
{
   if (!load_state (save_state_index))
   {
      gui_message (GUI_ERROR_COLOR, "Failed to open machine state file.");

      return (D_O_K);
   }

   message_local ("Machine state loaded from slot %d.", save_state_index);

   return (D_CLOSE);
}

static int machine_save_state_menu_select (void)
{
   int index;

   for (index = 0; index < SAVE_STATE_SLOTS; index++)
   {
      UCHAR *title;
      UCHAR *text;

      title = save_state_titles[index];
      text = save_state_menu_texts[index];

      /* Get title. */
      get_state_title (index, title, USTRING_SIZE);

      /* Build menu text. */
      uszprintf (text, USTRING_SIZE, "&%d: %s", index, title);

      /* Update menu. */
      machine_save_state_select_menu[index].text = text;
   }

   return (D_O_K);
}

static int machine_save_state_autosave_menu_disabled (void)
{
   set_autosave (0);

   return (D_O_K);
}

static int machine_save_state_autosave_menu_10_seconds (void)
{
   set_autosave (10);

   return (D_O_K);
}

static int machine_save_state_autosave_menu_30_seconds (void)
{
   set_autosave (30);

   return (D_O_K);
}

static int machine_save_state_autosave_menu_60_seconds (void)
{
   set_autosave (60);

   return (D_O_K);
}

static int machine_region_menu_automatic (void)
{
   machine_region = MACHINE_REGION_AUTOMATIC;
   update_machine_type ();
   update_menus ();

   message_local ("System region set to automatic.");

   return (D_O_K);
}

static int machine_region_menu_ntsc (void)
{
   machine_region = MACHINE_REGION_NTSC;
   update_machine_type ();
   update_menus ();

   message_local ("System region set to NTSC.");

   return (D_O_K);
}

static int machine_region_menu_pal (void)
{
   machine_region = MACHINE_REGION_PAL;
   update_machine_type ();
   update_menus ();

   message_local ("System region set to PAL.");

   return (D_O_K);
}

static int machine_menu_cheat_manager (void)
{
   if (show_dialog (machine_cheat_manager_dialog) ==
      MACHINE_CHEAT_MANAGER_DIALOG_SAVE_BUTTON)
   {
      save_patches ();
   }

   return (D_O_K);
}

static int audio_menu_enabled (void)
{
   audio_enable_output = !audio_enable_output;
   update_menus ();

   cycle_audio ();

   message_local ("Audio rendering and output %s.", get_enabled_text
      (audio_enable_output));

   return (D_O_K);
}

static int audio_subsystem_menu_none (void)
{
   audio_subsystem = AUDIO_SUBSYSTEM_NONE;
   update_menus ();

   cycle_audio ();

   message_local ("Audio subsystem set to NONE.");

   return (D_O_K);
}

static int audio_subsystem_menu_allegro (void)
{
   audio_subsystem = AUDIO_SUBSYSTEM_ALLEGRO;
   update_menus ();

   cycle_audio ();

   message_local ("Audio subsystem set to Allegro.");

   return (D_O_K);
}

static int audio_subsystem_menu_openal (void)
{
   audio_subsystem = AUDIO_SUBSYSTEM_OPENAL;
   update_menus ();

   cycle_audio ();

   message_local ("Audio subsystem set to OpenAL.");

   return (D_O_K);
}

#define MIXING_FREQUENCY_MENU_HANDLER(freq)  \
   static int audio_mixing_frequency_menu_##freq##_hz (void) \
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

static int audio_mixing_channels_menu_mono (void)
{
   apu_stereo_mode = FALSE;
   update_menus ();

   cycle_audio ();

   gui_message (GUI_TEXT_COLOR, "Audio channels set to mono.");

   return (D_O_K);
}

static int audio_mixing_channels_menu_stereo_mix (void)
{
   apu_stereo_mode = APU_STEREO_MODE_4;
   update_menus ();
    
   cycle_audio ();
   
   message_local ("Audio channels set to mono with stereo mixing.");

   return (D_O_K);
}

static int audio_mixing_channels_menu_virtual_stereo_mode_1 (void)
{
   apu_stereo_mode = APU_STEREO_MODE_1;
   update_menus ();
    
   cycle_audio ();

   message_local ("Audio channels set to virtual stereo (mode 1).");

   return (D_O_K);
}

static int audio_mixing_channels_menu_virtual_stereo_mode_2 (void)
{
   apu_stereo_mode = APU_STEREO_MODE_2;
   update_menus ();

   cycle_audio ();

   message_local ("Audio channels set to virtual stereo (mode 2).");

   return (D_O_K);
}

static int audio_mixing_channels_menu_stereo (void)
{
   apu_stereo_mode = APU_STEREO_MODE_3;
   update_menus ();

   cycle_audio ();

   message_local ("Audio channels set to stereo.");

   return (D_O_K);
}

static int audio_mixing_channels_menu_swap_channels (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_SWAP_CHANNELS);
   update_menus ();

   message_local ("Audio stereo channel swapping %s.", get_enabled_text
      (dsp_get_effector_enabled (DSP_EFFECTOR_SWAP_CHANNELS)));

   return (D_O_K);
}

static int audio_mixing_quality_menu_low_8_bit (void)
{
   audio_sample_size = 8;
   update_menus ();

   cycle_audio ();

   message_local ("Audio mixing quality set to low (8-bit).");

   return (D_O_K);
}

static int audio_mixing_quality_menu_high_16_bit (void)
{
   audio_sample_size = 16;
   update_menus ();

   cycle_audio ();

   message_local ("Audio mixing quality set to high (16-bit).");

   return (D_O_K);
}

static int audio_mixing_quality_menu_interpolation (void)
{
   audio_interpolation = !audio_interpolation;
   update_menus ();

   cycle_audio ();

   message_local ("Audio interpolation %s.", get_enabled_text
      (audio_interpolation));

   return (D_O_K);
}

static int audio_mixing_quality_menu_dithering (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_DITHER);
   update_menus ();

   message_local ("Audio dithering %s.", get_enabled_text
      (dsp_get_effector_enabled (DSP_EFFECTOR_DITHER)));

   return (D_O_K);
}

static int audio_effects_menu_wide_stereo_type_1 (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_WIDE_STEREO_TYPE_1);
   update_menus ();

   message_local ("Audio wide stereo effect %s.",
      (dsp_get_effector_enabled (DSP_EFFECTOR_WIDE_STEREO_TYPE_1) ?
         "enabled (type 1)" : "disabled"));

   return (D_O_K);
}

static int audio_effects_menu_wide_stereo_type_2 (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_WIDE_STEREO_TYPE_2);
   update_menus ();

   message_local ("Audio wide stereo effect %s.",
      (dsp_get_effector_enabled (DSP_EFFECTOR_WIDE_STEREO_TYPE_2) ?
         "enabled (type 2)" : "disabled"));

   return (D_O_K);
}

static int audio_effects_menu_wide_stereo_type_3 (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_WIDE_STEREO_TYPE_3);
   update_menus ();

   message_local ("Audio wide stereo effect %s.",
      (dsp_get_effector_enabled (DSP_EFFECTOR_WIDE_STEREO_TYPE_3) ?
         "enabled (type 3)" : "disabled"));

   return (D_O_K);
}

static int audio_filters_menu_low_pass_type_1 (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_1);
   update_menus ();

   message_local ("Low pass audio filter %s.", get_enabled_text_ex
      (dsp_get_effector_enabled (DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_1),
         "enabled (type 1)"));

   return (D_O_K);
}

static int audio_filters_menu_low_pass_type_2 (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_2);
   update_menus ();

   message_local ("Low pass audio filter %s.", get_enabled_text_ex
      (dsp_get_effector_enabled (DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_2),
         "enabled (type 2)"));

   return (D_O_K);
}

static int audio_filters_menu_low_pass_type_3 (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_3);
   update_menus ();

   message_local ("Low pass audio filter %s.", get_enabled_text_ex
      (dsp_get_effector_enabled (DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_3),
         "enabled (type 3)"));

   return (D_O_K);
}

static int audio_filters_menu_high_pass (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_HIGH_PASS_FILTER);
   update_menus ();

   message_local ("High pass audio filter %s.", get_enabled_text
      (dsp_get_effector_enabled (DSP_EFFECTOR_HIGH_PASS_FILTER)));

   return (D_O_K);
}

static int audio_filters_menu_delta_sigma_filter (void)
{
   DSP_TOGGLE_EFFECTOR(DSP_EFFECTOR_DELTA_SIGMA_FILTER);
   update_menus ();

   message_local ("Delta-Sigma audio filter %s.", get_enabled_text
      (dsp_get_effector_enabled (DSP_EFFECTOR_DELTA_SIGMA_FILTER)));

   return (D_O_K);
}

static int audio_channels_menu_square_wave_a (void)
{
   dsp_set_channel_enabled (APU_CHANNEL_SQUARE_1,
      DSP_SET_ENABLED_MODE_INVERT, 0);
   update_menus ();

   message_local ("Audio square wave channel A %s.", get_enabled_text
      (dsp_get_channel_enabled (APU_CHANNEL_SQUARE_1)));

   return (D_O_K);
}

static int audio_channels_menu_square_wave_b (void)
{
   dsp_set_channel_enabled (APU_CHANNEL_SQUARE_2,
      DSP_SET_ENABLED_MODE_INVERT, 0);
   update_menus ();

   message_local ("Audio square wave channel B %s.", get_enabled_text
      (dsp_get_channel_enabled (APU_CHANNEL_SQUARE_2)));

   return (D_O_K);
}

static int audio_channels_menu_triangle_wave (void)
{
   dsp_set_channel_enabled (APU_CHANNEL_TRIANGLE,
      DSP_SET_ENABLED_MODE_INVERT, 0);
   update_menus ();

   message_local ("Audio triangle wave channel %s.", get_enabled_text
      (dsp_get_channel_enabled (APU_CHANNEL_TRIANGLE)));

   return (D_O_K);
}

static int audio_channels_menu_white_noise (void)
{
   dsp_set_channel_enabled (APU_CHANNEL_NOISE, DSP_SET_ENABLED_MODE_INVERT,
      0);
   update_menus ();

   message_local ("Audio white noise channel %s.", get_enabled_text
      (dsp_get_channel_enabled (APU_CHANNEL_NOISE)));

   return (D_O_K);
}

static int audio_channels_menu_digital (void)
{
   dsp_set_channel_enabled (APU_CHANNEL_DMC, DSP_SET_ENABLED_MODE_INVERT,
      0);
   update_menus ();

   message_local ("Audio digital channel %s.", get_enabled_text
      (dsp_get_channel_enabled (APU_CHANNEL_DMC)));

   return (D_O_K);
}

static int audio_channels_menu_extended (void)
{
   dsp_set_channel_enabled (APU_CHANNEL_EXTRA, DSP_SET_ENABLED_MODE_INVERT,
      0);
   update_menus ();

   message_local ("Audio extended channels %s.", get_enabled_text
      (dsp_get_channel_enabled (APU_CHANNEL_EXTRA)));

   return (D_O_K);
}

static int audio_volume_menu_increase (void)
{
   dsp_master_volume += 0.25f;
   if (dsp_master_volume > 1.5f)
      dsp_master_volume = 1.5f;

   update_menus ();

   message_local ("Audio master volume level increased.");

   return (D_O_K);
}

static int audio_volume_menu_decrease (void)
{
   dsp_master_volume -= 0.25f;
   if (dsp_master_volume < 0)
      dsp_master_volume = 0;

   update_menus ();

   message_local ("Audio master volume level decreased.");

   return (D_O_K);
}

static int audio_volume_menu_reset (void)
{
   dsp_master_volume = 1.0f;
   update_menus ();

   message_local ("Audio master volume level reset.");

   return (D_O_K);
}

static int audio_record_menu_start (void)
{
   int index;

   for (index = 0; index < 9999; index++)
   {
      USTRING filename;

      uszprintf (filename, sizeof (filename), "wave%04d.wav", index);

      if (exists (filename))
         continue;

      if (dsp_open_wav (filename, audio_sample_rate, (apu_stereo_mode ? 2 :
         1), audio_sample_size) == 0)
      {
         DISABLE_MENU_ITEM(audio_record_menu_start);
         ENABLE_MENU_ITEM(audio_record_menu_stop);
      }
   
      message_local ("Audio recording session started to %s.", filename);

      return (D_O_K);
   }
   
   gui_message (GUI_ERROR_COLOR, "Couldn't find a suitable image "
      "filename.");

   return (D_O_K);
}

static int audio_record_menu_stop (void)
{
   dsp_close_wav ();

   ENABLE_MENU_ITEM(audio_record_menu_start);
   DISABLE_MENU_ITEM(audio_record_menu_stop);

   message_local ("Audio recording session stopped.");

   return (D_O_K);
}

#define DRIVER_MENU_HANDLER(system, driver, id) \
   static int video_driver_##system##_menu_##driver (void)   \
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

static int video_driver_menu_automatic (void)
{
   video_set_driver (GFX_AUTODETECT);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

#define RESOLUTION_MENU_HANDLER(width, height)  \
   static int video_resolution_menu_##width##_##height (void)   \
   {  \
      video_set_resolution (width, height);  \
      gui_needs_restart = TRUE;  \
      return (D_CLOSE); \
   }

#define RESOLUTION_MENU_HANDLER_EX(type, width, height)  \
   static int video_resolution_##type##_menu_##width##_##height (void)   \
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

static int video_colors_menu_paletted_8_bit (void)
{
   video_set_color_depth (8);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_colors_menu_true_color_15_bit (void)
{
   video_set_color_depth (15);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_colors_menu_true_color_16_bit (void)
{
   video_set_color_depth (16);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_colors_menu_true_color_24_bit (void)
{
   video_set_color_depth (24);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_colors_menu_true_color_32_bit (void)
{
   video_set_color_depth (32);

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

#define BLITTER_MENU_HANDLER(name, caption, id) \
   static int video_blitter_menu_##name (void)   \
   {  \
      video_set_blitter (id); \
      update_menus ();  \
      cycle_video ();   \
      message_local ("Video blitter set to %s.", caption);  \
      return (D_O_K);   \
   }

BLITTER_MENU_HANDLER(automatic,       "automatic",           VIDEO_BLITTER_AUTOMATIC)
BLITTER_MENU_HANDLER(normal,          "normal",              VIDEO_BLITTER_NORMAL)
BLITTER_MENU_HANDLER(des,             "des engine",          VIDEO_BLITTER_DES)
BLITTER_MENU_HANDLER(interpolated_2x, "interpolated (2x)",   VIDEO_BLITTER_INTERPOLATED_2X)
BLITTER_MENU_HANDLER(2xscl,           "2xSCL engine",        VIDEO_BLITTER_2XSCL)
BLITTER_MENU_HANDLER(desii,           "des 2 engine",        VIDEO_BLITTER_DESII)
BLITTER_MENU_HANDLER(super_2xscl,     "super 2xSCL engine",  VIDEO_BLITTER_SUPER_2XSCL)
BLITTER_MENU_HANDLER(ultra_2xscl,     "ultra 2xSCL engine",  VIDEO_BLITTER_ULTRA_2XSCL)
BLITTER_MENU_HANDLER(hq2x,            "hq2x filter",         VIDEO_BLITTER_HQ2X)
BLITTER_MENU_HANDLER(nes_ntsc,        "nes_ntsc engine",     VIDEO_BLITTER_NES_NTSC)
BLITTER_MENU_HANDLER(interpolated_3x, "interpolated (3x)",   VIDEO_BLITTER_INTERPOLATED_3X)
BLITTER_MENU_HANDLER(hq3x,            "hq3x filter",         VIDEO_BLITTER_HQ3X)
BLITTER_MENU_HANDLER(hq4x,            "hq4x filter",         VIDEO_BLITTER_HQ4X)
BLITTER_MENU_HANDLER(stretched,       "stretched",           VIDEO_BLITTER_STRETCHED)

#undef BLITTER_MENU_HANDLER

static int video_filters_menu_scanlines_25_percent (void)
{
   LIST filters;

   filters = video_get_filter_list ();
   LIST_TOGGLE(filters, VIDEO_FILTER_SCANLINES_LOW);
   video_set_filter_list (filters);

   update_menus ();

   cycle_video ();

   message_local ("Scanlines video filter %s.", get_enabled_text_ex
      ((filters & VIDEO_FILTER_SCANLINES_LOW), "enabled (25%)"));

   return (D_O_K);
}

static int video_filters_menu_scanlines_50_percent (void)
{
   LIST filters;

   filters = video_get_filter_list ();
   LIST_TOGGLE(filters, VIDEO_FILTER_SCANLINES_MEDIUM);
   video_set_filter_list (filters);

   update_menus ();

   cycle_video ();

   message_local ("Scanlines video filter %s.", get_enabled_text_ex
      ((filters & VIDEO_FILTER_SCANLINES_MEDIUM), "enabled (50%)"));

   return (D_O_K);
}

static int video_filters_menu_scanlines_100_percent (void)
{
   LIST filters;

   filters = video_get_filter_list ();
   LIST_TOGGLE(filters, VIDEO_FILTER_SCANLINES_HIGH);
   video_set_filter_list (filters);

   update_menus ();

   cycle_video ();

   message_local ("Scanlines video filter %s.", get_enabled_text_ex
      ((filters & VIDEO_FILTER_SCANLINES_HIGH), "enabled (100%)"));

   return (D_O_K);
}

static int video_menu_page_buffer (void)
{
   video_enable_page_buffer = !video_enable_page_buffer;
   update_menus ();

   video_reinit ();

   cycle_video ();

   message_local ("Page buffering %s.", get_enabled_text
      (video_enable_page_buffer));

   return (D_O_K);
}

static int video_menu_vsync (void)
{
   video_enable_vsync = !video_enable_vsync;
   update_menus ();

   message_local ("VSync %s.", get_enabled_text (video_enable_vsync));

   return (D_O_K);
}

static int video_layers_menu_sprites_a (void)
{
   ppu_enable_sprite_layer_a = !ppu_enable_sprite_layer_a;
   update_menus ();

   message_local ("Video sprites layer A %s.", get_enabled_text
      (ppu_enable_sprite_layer_a));

   return (D_O_K);
}

static int video_layers_menu_sprites_b (void)
{
   ppu_enable_sprite_layer_b = !ppu_enable_sprite_layer_b;
   update_menus ();

   message_local ("Video sprites layer B %s.", get_enabled_text
      (ppu_enable_sprite_layer_b));

   return (D_O_K);
}


static int video_layers_menu_background (void)
{
   ppu_enable_background_layer = !ppu_enable_background_layer;
   update_menus ();

   message_local ("Video background layer %s.", get_enabled_text
      (ppu_enable_background_layer));

   return (D_O_K);
}

static int video_layers_menu_hide_horizontal_scrolling (void)
{
   LIST clipping = video_edge_clipping;

   if (clipping & VIDEO_EDGE_CLIPPING_HORIZONTAL)
      clipping &= ~VIDEO_EDGE_CLIPPING_HORIZONTAL;
   else
      clipping |= VIDEO_EDGE_CLIPPING_HORIZONTAL;

   video_edge_clipping = clipping;

   update_menus ();

   message_local ("Video horizontal edge clipping %s.", get_enabled_text
      (video_edge_clipping & VIDEO_EDGE_CLIPPING_HORIZONTAL));

   return (D_O_K);
}

static int video_layers_menu_hide_vertical_scrolling (void)
{
   LIST clipping = video_edge_clipping;

   if (clipping & VIDEO_EDGE_CLIPPING_VERTICAL)
      clipping &= ~VIDEO_EDGE_CLIPPING_VERTICAL;
   else
      clipping |= VIDEO_EDGE_CLIPPING_VERTICAL;

   video_edge_clipping = clipping;

   update_menus ();

   message_local ("Video vertical edge clipping %s.", get_enabled_text
      (video_edge_clipping & VIDEO_EDGE_CLIPPING_VERTICAL));

   return (D_O_K);
}

static int video_layers_menu_flip_mirroring (void)
{
   ppu_invert_mirroring ();

   return (D_CLOSE);
}

#define PALETTE_MENU_HANDLER(name, caption, id) \
   static int video_palette_menu_##name (void)   \
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

static int video_palette_menu_custom (void)
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

static int video_advanced_menu_force_fullscreen (void)
{
   video_force_fullscreen = !video_force_fullscreen;
   video_reinit ();

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int options_menu_show_status (void)
{
   video_display_status = (! video_display_status);
   update_menus ();

   return (D_O_K);
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
   show_dialog (options_input_configure_dialog);

   return (D_O_K);
}

static int options_cpu_usage_menu_passive (void)
{
    cpu_usage = CPU_USAGE_PASSIVE;
    update_menus ();

    message_local ("System CPU usage set to passive.");

    return (D_O_K);
}

static int options_cpu_usage_menu_normal (void)
{
    cpu_usage = CPU_USAGE_NORMAL;
    update_menus ();

    message_local ("System CPU usage set to normal.");

    return (D_O_K);
}

static int options_cpu_usage_menu_aggressive (void)
{
    cpu_usage = CPU_USAGE_AGGRESSIVE;
    update_menus ();

    message_local ("System CPU usage set to aggressive.");

    return (D_O_K);
}

#define OPTIONS_GUI_THEME_MENU_HANDLER(name)   \
   static int options_gui_theme_menu_##name (void) \
   {  \
      set_##name##_theme ();  \
      gui_needs_restart = TRUE;  \
      return (D_CLOSE); \
   }

OPTIONS_GUI_THEME_MENU_HANDLER(classic)
OPTIONS_GUI_THEME_MENU_HANDLER(stainless_steel)
OPTIONS_GUI_THEME_MENU_HANDLER(zero_4)
OPTIONS_GUI_THEME_MENU_HANDLER(panta)
OPTIONS_GUI_THEME_MENU_HANDLER(xodiac)
OPTIONS_GUI_THEME_MENU_HANDLER(monochrome)
OPTIONS_GUI_THEME_MENU_HANDLER(essence)
OPTIONS_GUI_THEME_MENU_HANDLER(voodoo)
OPTIONS_GUI_THEME_MENU_HANDLER(hugs_and_kisses)

#undef OPTIONS_GUI_THEME_MENU_HANDLER

static int netplay_menu_start_as_server (void)
{
   STRING host;
   int port;
   STRING port_str;
   USTRING nick;
   DIALOG *dialog;
   DIALOG *obj_host_label;
   DIALOG *obj_host;
   DIALOG *obj_port;
   DIALOG *obj_nick;

   /* Load configuration. */

   STRING_CLEAR(host);

   port = get_config_int ("netplay", "port", NETPLAY_DEFAULT_PORT);

   STRING_CLEAR(port_str);
   sprintf (port_str, "%d", port);

   USTRING_CLEAR(nick);
   ustrncat (nick, get_config_string ("netplay", "nick", ""), (sizeof (nick)
      - 1));

   /* Get dialog. */
   dialog = netplay_dialog;

   /* Get dialog objects. */
   obj_host_label = &dialog[NETPLAY_DIALOG_HOST_LABEL];
   obj_host       = &dialog[NETPLAY_DIALOG_HOST];
   obj_port       = &dialog[NETPLAY_DIALOG_PORT];
   obj_nick       = &dialog[NETPLAY_DIALOG_NICK];

   /* Set up dialog objects. */

   obj_host_label->flags |= D_DISABLED;
  
   obj_host->d1 = 0;
   obj_host->dp = host;
   obj_host->flags |= D_DISABLED;

   obj_port->d1 = (sizeof (port_str) - 1);
   obj_port->dp = port_str;

   obj_nick->d1 = (sizeof (nick) - 1);
   obj_nick->dp = nick;

   /* Display dialog. */
   if (show_dialog (dialog) != NETPLAY_DIALOG_OK_BUTTON)
      return (D_O_K);

   /* Integerize port. */
   port = atoi (port_str);

   /* Save configuration. */
   set_config_int    ("netplay", "port", port);
   set_config_string ("netplay", "nick", nick);

   /* Start NetPlay session. */

   if (!netplay_open_server (port))
   {
      gui_message (GUI_ERROR_COLOR, "Failed to open server!");
      return (D_O_K);
   }

   message_local ("NetPlay session opened.");

   /* Set nickname. */
   netplay_set_nickname (nick);

   /* Open lobby. */
   return (open_lobby ());
}

static int netplay_menu_start_as_client (void)
{
   STRING host;
   int port;
   STRING port_str;
   USTRING nick;
   DIALOG *dialog;
   DIALOG *obj_host_label;
   DIALOG *obj_host;
   DIALOG *obj_port;
   DIALOG *obj_nick;
          
   /* Load configuration. */

   STRING_CLEAR(host);
   strncat (host, get_config_string ("netplay", "host", ""), (sizeof (host)
      - 1));

   port = get_config_int ("netplay", "port", NETPLAY_DEFAULT_PORT);

   STRING_CLEAR(port_str);
   sprintf (port_str, "%d", port);

   USTRING_CLEAR(nick);
   ustrncat (nick, get_config_string ("netplay", "nick", ""), (sizeof (nick)
      - 1));

   /* Get dialog. */
   dialog = netplay_dialog;

   /* Get dialog objects. */
   obj_host_label = &dialog[NETPLAY_DIALOG_HOST_LABEL];
   obj_host       = &dialog[NETPLAY_DIALOG_HOST];
   obj_port       = &dialog[NETPLAY_DIALOG_PORT];
   obj_nick       = &dialog[NETPLAY_DIALOG_NICK];

   /* Set up dialog objects. */

   obj_host_label->flags &= ~D_DISABLED;

   obj_host->d1 = (sizeof (host) - 1);
   obj_host->dp = host;
   obj_host->flags &= ~D_DISABLED;

   obj_port->d1 = (sizeof (port_str) - 1);
   obj_port->dp = port_str;

   obj_nick->d1 = (sizeof (nick) - 1);
   obj_nick->dp = nick;

   /* Display dialog. */
   if (show_dialog (dialog) != NETPLAY_DIALOG_OK_BUTTON)
      return (D_O_K);

   /* Integerize port. */
   port = atoi (port_str);

   /* Save configuration. */
   set_config_string ("netplay", "host", host);
   set_config_int    ("netplay", "port", port);
   set_config_string ("netplay", "nick", nick);

   /* Start NetPlay session. */

   if (!netplay_open_client (host, port))
   {
      gui_message (GUI_ERROR_COLOR, "Failed to connect to remote host!");
      return (D_O_K);
   }

   message_local ("NetPlay session opened.");

   /* Set nickname. */
   netplay_set_nickname (nick);

   /* Open lobby. */
   return (open_lobby ());
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

static int help_menu_version (void)
{
   alert ("FakeNES version " VERSION_STRING " " ALLEGRO_PLATFORM_STR, "",
      "Get the latest from http://fakenes.sourceforge.net/.", "&OK", NULL,
         'o', 0);

   return (D_O_K);
}


/* ---- Dialog handlers. ---- */


static int machine_cheat_manager_dialog_list (DIALOG *dialog)
{
   CPU_PATCH *patch;
   DIALOG *obj_enabled;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
      return (D_O_K);

   patch = &cpu_patch_info[dialog->d1];

   obj_enabled = &dialog[MACHINE_CHEAT_MANAGER_DIALOG_ENABLED_CHECKBOX];

   if (patch->enabled)
      obj_enabled->flags |= D_SELECTED;
   else
      obj_enabled->flags &= ~D_SELECTED;

   scare_mouse ();
   object_message (obj_enabled, MSG_DRAW, 0);
   unscare_mouse ();

   return (D_O_K);
}

static int machine_cheat_manager_dialog_add (DIALOG *dialog)
{
   DIALOG *main_dialog;
   DIALOG *obj_title;
   DIALOG *obj_code;
   USTRING title;
   USTRING code;
   CPU_PATCH *patch;
   UINT8 value;

   if (cpu_patch_count >= CPU_MAX_PATCHES)
   {
      alert ("- Error -", NULL, "The patch list is already full.", "&OK",
         NULL, 'o', 0);

      return (D_O_K);
   }

   /* Get dialog. */
   main_dialog = machine_cheat_manager_add_dialog;

   /* Get dialog objects. */
   obj_title = &main_dialog[MACHINE_CHEAT_MANAGER_ADD_DIALOG_TITLE];
   obj_code  = &main_dialog[MACHINE_CHEAT_MANAGER_ADD_DIALOG_CODE];

   /* Set up dialog objects. */

   USTRING_CLEAR(title);
   obj_title->d1 = (SAVE_TITLE_SIZE - 1);
   obj_title->dp = title;

   USTRING_CLEAR(code);
   obj_code->d1 = (11 - 1);
   obj_code->dp = code;

   /* Show dialog. */
   if (show_dialog (main_dialog) != MACHINE_CHEAT_MANAGER_ADD_DIALOG_OK_BUTTON)
      return (D_O_K);

   patch = &cpu_patch_info[cpu_patch_count];

   if (cheats_decode (code, &patch->address, &patch->value,
      &patch->match_value) != 0)
   {
      alert ("- Error -", NULL, "You must enter a valid Game Genie (or "
         "NESticle raw) code.", "&OK", NULL, 'o', 0);

      return (D_O_K);
   }

   /* Copy title. */
   ustrncat (patch->title, title, sizeof (title));

   /* Enable patch. */
   patch->enabled = TRUE;

   cpu_patch_count++;

   if ((value = cpu_read (patch->address)) == patch->match_value)
   {
      /* Activate patch. */
      patch->active = TRUE;

      cpu_patch_table[patch->address] = (patch->value - value);
   }

   return (D_REDRAW);
}

static int machine_cheat_manager_dialog_remove (DIALOG *dialog)
{
   DIALOG *main_dialog;
   int start;
   CPU_PATCH *src;
   int index;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
      return (D_O_K);

   main_dialog = machine_cheat_manager_dialog;

   start = main_dialog[MACHINE_CHEAT_MANAGER_DIALOG_LIST].d1;
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
   {
      main_dialog[MACHINE_CHEAT_MANAGER_DIALOG_ENABLED_CHECKBOX].flags &=
         ~D_SELECTED;
   }

   return (D_REDRAW);
}

static int machine_cheat_manager_dialog_enabled (DIALOG *dialog)
{
   DIALOG *obj_list;
   CPU_PATCH *patch;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
   {
      dialog->flags &= ~D_SELECTED;

      return (D_O_K);
   }

   obj_list = &machine_cheat_manager_dialog[MACHINE_CHEAT_MANAGER_DIALOG_LIST];

   patch = &cpu_patch_info[obj_list->d1];

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

      if ((value = cpu_read (patch->address)) == patch->match_value)
      {
         /* Enable patch. */
         patch->active = TRUE;
    
         cpu_patch_table[patch->address] = (patch->value - value);
      }
   }

   scare_mouse ();
   object_message (obj_list, MSG_DRAW, 0);
   unscare_mouse ();

   return (D_O_K);
}

static USTRING machine_cheat_manager_dialog_list_texts[CPU_MAX_PATCHES];

static char *machine_cheat_manager_dialog_list_filler (int index, int *list_size)
{
   if (index >= 0)
   {
      CPU_PATCH *patch = &cpu_patch_info[index];
      UCHAR *text = machine_cheat_manager_dialog_list_texts[index];

      USTRING_CLEAR(text);
      uszprintf (text, USTRING_SIZE, "$%04x -$%02x +$%02x %s ",
         patch->address, patch->match_value, patch->value, (patch->active ?
            "Active" : " Idle "));

      /* Copy title. */
      ustrncat (text, patch->title, USTRING_SIZE);
      
      return (text);
   }
   else
   {
      RT_ASSERT(list_size);

      *list_size = cpu_patch_count;

      return (NULL);
   }
}

static int selected_player = -1;
static int selected_player_device = 0;

static int options_input_configure_dialog_player_select (DIALOG *dialog)
{
   DIALOG *main_dialog;
   int first, last;
   int index;

   RT_ASSERT(dialog);

   selected_player = (dialog->d2 - 1);
   selected_player_device = input_get_player_device (selected_player);

   main_dialog = options_input_configure_dialog;

   first = OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_1_SELECT;
   last  = OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_5_SELECT;

   for (index = first; index <= last; index++)
      main_dialog[index].flags  &= ~D_SELECTED;

   main_dialog[(first + selected_player_device)].flags |= D_SELECTED;

   scare_mouse ();

   for (index = first; index <= last; index++)
      object_message (&main_dialog[index],  MSG_DRAW, 0);

   unscare_mouse ();

   return (D_O_K);
}

static int options_input_configure_dialog_device_select (DIALOG *dialog)
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

static int options_input_configure_dialog_set_buttons (DIALOG *dialog)
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
   else if (selected_player_device == INPUT_DEVICE_MOUSE)
   {
      alert ("- Error -", "", "Unable to set buttons for mouse at this"
         " time.", "&OK", NULL, 'o', 0);
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

      case INPUT_DEVICE_MOUSE:
         break;

      default:
         WARN_GENERIC();
   }

   return (D_O_K);
}

static int options_input_configure_dialog_calibrate (DIALOG *dialog)
{
   RT_ASSERT(dialog);

   if (selected_player < 0)
   {
      alert ("- Error -", "", "Please select a player to modify first.",
         "&OK", NULL, 'o', 0);

      return (D_O_K);
   }

   switch (selected_player_device)
   {
      case INPUT_DEVICE_JOYSTICK_1:
      case INPUT_DEVICE_JOYSTICK_2:
      {
         int index;

         index = (selected_player_device - INPUT_DEVICE_JOYSTICK_1);

         while (joy[index].flags & JOYFLAG_CALIBRATE)
         {
            message_local ("%s, and press any key.\n",
               calibrate_joystick_name (index));

            while (!keypressed ())
               gui_heartbeat ();

            if ((readkey () >> 8) == KEY_ESC)
            {
               gui_message (GUI_ERROR_COLOR, "Joystick calibration "
                  "cancelled.");
    
               return (D_O_K);
            }

            if (calibrate_joystick (index) != 0)
            {
               alert ("- Error -", "", "An unknown error occured while "
                  "attempting to calibrate the device.", "&OK", NULL, 'o',
                     0);
   
               return (D_O_K);
            }
         }

         alert ("- Calibration Complete - ", "", "The selected device has "
            "been calibrated.", "&Save", NULL, 's', 0);

         save_joystick_data (NULL);

         break;
      }

      default:
      {
         alert ("- Error -", "", "The selected device does not require "
            "calibration.", "&OK", NULL, 'o', 0);

         break;
      }
   }

   return (D_O_K);
}
