/* FakeNES - A free, portable, Open Source NES emulator.

   gui.c: Implementation of the object-based GUI.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#ifdef USE_ALLEGROGL
#include <alleggl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apu.h"
#include "audio.h"
#include "cheats.h"
#include "common.h"
#include "config.h"
#include "cpu.h"
#include "data.h"
#include "debug.h"
#include "gui.h"
#include "input.h"
#include "load.h"
#include "log.h"
#include "machine.h"
#include "main.h"
#include "mmc.h"
#include "netplay.h"
#include "nsf.h"
#include "ppu.h"
#include "rom.h"
#include "save.h"
#include "timing.h"
#include "types.h"
#include "version.h"
#include "video.h"

// TODO: Add configuration for the real-time game rewinder.

static BITMAP *gui_buffer = NULL;

static int dialog_x = 0;
static int dialog_y = 0;
static BOOL restart_dialog = FALSE;
 
GUI_THEME gui_theme;
ENUM gui_theme_id = -1;
const GUI_THEME *last_theme = NULL;
void (*gui_theme_callback) (void);

static BITMAP *gui_mouse_sprite = NULL;
static BITMAP *background_image = NULL;

BOOL gui_is_active = FALSE;
static BOOL gui_needs_restart = FALSE;

/* This is the location of the mouse pointer when the GUI is running. It differs
   from mouse_x/y because it takes into account the letterbox system. */
int gui_mouse_x_position = 0, gui_mouse_y_position = 0;

/* This is the location and size of the game screen when the GUI is running. */
int gui_game_x = 0, gui_game_y = 0, gui_game_width = 0, gui_game_height = 0;

#define GUI_MESSAGE_HISTORY_SIZE	20
static USTRING gui_message_history[GUI_MESSAGE_HISTORY_SIZE];
static int gui_last_message = 0;

/* This is the text of the status bar. */
static USTRING gui_status_text;
static int gui_status_color = 0;

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

/* Keep these in order! */
#include "gui/themes.h"
#include "gui/objects.h"
#include "gui/menus.h"
#include "gui/dialogs.h"
#include "gui/util.h"
#include "gui/file.h"

/* Big bag of hacks. */
#ifdef ALLEGRO_LINUX
#  ifndef GFX_SVGALIB
#    define GFX_SVGALIB 0xDEADBEEF
#    define MISSING_SVGALIB
#  endif
#  ifndef GFX_FBCON
#    define GFX_FBCON 0xDEADBEEF
#    define MISSING_FBCON
#  endif
#endif

static INLINE void update_menus (void)
{
   static USTRING audio_menu_volume_text;

#ifndef USE_OPENAL
   DISABLE_MENU_ITEM(audio_output_menu_subsystem_openal);
#endif

   DISABLE_MENU_ITEM(main_menu_hide_gui);
   DISABLE_MENU_ITEM(main_menu_close);
   DISABLE_SUBMENU(main_replay_menu);
   DISABLE_MENU_ITEM(main_menu_cheat_manager);
   DISABLE_MENU_ITEM(main_menu_save_snapshot);
   DISABLE_MENU_ITEM(main_menu_advance_frame);
   DISABLE_SUBMENU(main_record_audio_menu);
   DISABLE_MENU_ITEM(system_menu_reset);
   DISABLE_MENU_ITEM(system_menu_power_cycle);
   DISABLE_SUBMENU(system_save_state_menu);
   DISABLE_MENU_ITEM(options_menu_reset_clock);

   if(rom_is_loaded)
   {
      ENABLE_MENU_ITEM(main_menu_hide_gui);
      ENABLE_MENU_ITEM(main_menu_close);
      ENABLE_SUBMENU(main_replay_menu);
      ENABLE_MENU_ITEM(main_menu_cheat_manager);
      ENABLE_MENU_ITEM(main_menu_save_snapshot);
      ENABLE_MENU_ITEM(main_menu_advance_frame);
      ENABLE_SUBMENU(main_record_audio_menu);
      ENABLE_MENU_ITEM(system_menu_reset);
      ENABLE_MENU_ITEM(system_menu_power_cycle);
      ENABLE_SUBMENU(system_save_state_menu);
      ENABLE_MENU_ITEM(options_menu_reset_clock);
   }
   else if(nsf_is_loaded)
   {
      /* NSF only has a limited subset of functionality. */
      ENABLE_MENU_ITEM(main_menu_hide_gui);
      ENABLE_MENU_ITEM(main_menu_close);
      ENABLE_MENU_ITEM(main_menu_save_snapshot);
      ENABLE_SUBMENU(main_record_audio_menu);
      ENABLE_MENU_ITEM(options_menu_reset_clock);
   }
 
   TOGGLE_MENU_ITEM(main_open_recent_menu_lock, lock_recent);

   TOGGLE_MENU_ITEM(main_replay_select_menu_0, (replay_index == 0));
   TOGGLE_MENU_ITEM(main_replay_select_menu_1, (replay_index == 1));
   TOGGLE_MENU_ITEM(main_replay_select_menu_2, (replay_index == 2));
   TOGGLE_MENU_ITEM(main_replay_select_menu_3, (replay_index == 3));
   TOGGLE_MENU_ITEM(main_replay_select_menu_4, (replay_index == 4));

   TOGGLE_MENU_ITEM(system_menu_show_status, video_get_profile_boolean(VIDEO_PROFILE_OPTION_HUD));

   TOGGLE_MENU_ITEM(system_menu_timing_smoothest,     (machine_timing == MACHINE_TIMING_SMOOTH));
   TOGGLE_MENU_ITEM(system_menu_timing_most_accurate, (machine_timing == MACHINE_TIMING_ACCURATE));

   TOGGLE_MENU_ITEM(system_save_state_select_menu_0, (save_state_index == 0));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_1, (save_state_index == 1));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_2, (save_state_index == 2));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_3, (save_state_index == 3));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_4, (save_state_index == 4));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_5, (save_state_index == 5));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_6, (save_state_index == 6));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_7, (save_state_index == 7));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_8, (save_state_index == 8));
   TOGGLE_MENU_ITEM(system_save_state_select_menu_9, (save_state_index == 9));

   TOGGLE_MENU_ITEM(system_save_state_autosave_menu_disabled,   (input_autosave_interval == 0));
   TOGGLE_MENU_ITEM(system_save_state_autosave_menu_10_seconds, (input_autosave_interval == 10));
   TOGGLE_MENU_ITEM(system_save_state_autosave_menu_30_seconds, (input_autosave_interval == 30));
   TOGGLE_MENU_ITEM(system_save_state_autosave_menu_60_seconds, (input_autosave_interval == 60));

   TOGGLE_MENU_ITEM(system_region_menu_automatic, (machine_region == MACHINE_REGION_AUTOMATIC));
   TOGGLE_MENU_ITEM(system_region_menu_ntsc,      (machine_region == MACHINE_REGION_NTSC));
   TOGGLE_MENU_ITEM(system_region_menu_pal,       (machine_region == MACHINE_REGION_PAL));

   TOGGLE_MENU_ITEM(system_speed_up_down_menu_50_percent,  COMPARE_TWO_REALS(timing_speed_multiplier, 0.5));
   TOGGLE_MENU_ITEM(system_speed_up_down_menu_100_percent, COMPARE_TWO_REALS(timing_speed_multiplier, 1.0));
   TOGGLE_MENU_ITEM(system_speed_up_down_menu_200_percent, COMPARE_TWO_REALS(timing_speed_multiplier, 2.0));

   TOGGLE_MENU_ITEM(system_menu_speed_cap, speed_cap);

   TOGGLE_MENU_ITEM(system_frame_skip_menu_automatic,     (frame_skip == -1));
   TOGGLE_MENU_ITEM(system_frame_skip_menu_disabled,      (frame_skip == 0));
   TOGGLE_MENU_ITEM(system_frame_skip_menu_skip_1_frames, (frame_skip == 1));
   TOGGLE_MENU_ITEM(system_frame_skip_menu_skip_2_frames, (frame_skip == 2));
   TOGGLE_MENU_ITEM(system_frame_skip_menu_skip_3_frames, (frame_skip == 3));

   TOGGLE_MENU_ITEM(audio_menu_enable_apu,    apu_options.enabled);
   TOGGLE_MENU_ITEM(audio_menu_enable_output, audio_options.enable_output);

   TOGGLE_MENU_ITEM(audio_menu_emulation_fast,         (apu_options.emulation == APU_EMULATION_FAST));
   TOGGLE_MENU_ITEM(audio_menu_emulation_accurate,     (apu_options.emulation == APU_EMULATION_ACCURATE));
   TOGGLE_MENU_ITEM(audio_menu_emulation_high_quality, (apu_options.emulation == APU_EMULATION_HIGH_QUALITY));

   TOGGLE_MENU_ITEM(audio_menu_volume_logarithmic,    apu_options.logarithmic);
   TOGGLE_MENU_ITEM(audio_menu_volume_auto_gain,      apu_options.agc);
   TOGGLE_MENU_ITEM(audio_menu_volume_auto_normalize, apu_options.normalize);

   TOGGLE_MENU_ITEM(audio_channels_menu_square_1, apu_options.enable_square_1);
   TOGGLE_MENU_ITEM(audio_channels_menu_square_2, apu_options.enable_square_2);
   TOGGLE_MENU_ITEM(audio_channels_menu_triangle, apu_options.enable_triangle);
   TOGGLE_MENU_ITEM(audio_channels_menu_noise,    apu_options.enable_noise);
   TOGGLE_MENU_ITEM(audio_channels_menu_dmc,      apu_options.enable_dmc);
   TOGGLE_MENU_ITEM(audio_channels_menu_extra_1,  apu_options.enable_extra_1);
   TOGGLE_MENU_ITEM(audio_channels_menu_extra_2,  apu_options.enable_extra_2);
   TOGGLE_MENU_ITEM(audio_channels_menu_extra_3,  apu_options.enable_extra_3);

   TOGGLE_MENU_ITEM(audio_output_menu_subsystem_automatic, (audio_options.subsystem == AUDIO_SUBSYSTEM_AUTOMATIC));
   TOGGLE_MENU_ITEM(audio_output_menu_subsystem_safe,      (audio_options.subsystem == AUDIO_SUBSYSTEM_SAFE));
   TOGGLE_MENU_ITEM(audio_output_menu_subsystem_allegro,   (audio_options.subsystem == AUDIO_SUBSYSTEM_ALLEGRO));
   TOGGLE_MENU_ITEM(audio_output_menu_subsystem_openal,    (audio_options.subsystem == AUDIO_SUBSYSTEM_OPENAL));

   TOGGLE_MENU_ITEM(audio_output_menu_sampling_rate_automatic, (audio_options.sample_rate_hint == -1));
   TOGGLE_MENU_ITEM(audio_output_menu_sampling_rate_22050_hz,  (audio_options.sample_rate_hint == 22050));
   TOGGLE_MENU_ITEM(audio_output_menu_sampling_rate_44100_hz,  (audio_options.sample_rate_hint == 44100));
   TOGGLE_MENU_ITEM(audio_output_menu_sampling_rate_48000_hz,  (audio_options.sample_rate_hint == 48000));

   TOGGLE_MENU_ITEM(audio_output_menu_mixing_mono,           !apu_options.stereo);
   TOGGLE_MENU_ITEM(audio_output_menu_mixing_stereo,         (apu_options.stereo && !apu_options.swap_channels));
   TOGGLE_MENU_ITEM(audio_output_menu_mixing_reverse_stereo, (apu_options.stereo && apu_options.swap_channels)); 

   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_automatic, (audio_options.buffer_length_ms_hint == -1));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_30ms,      (audio_options.buffer_length_ms_hint == 30));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_50ms,      (audio_options.buffer_length_ms_hint == 50));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_75ms,      (audio_options.buffer_length_ms_hint == 75));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_100ms,     (audio_options.buffer_length_ms_hint == 100));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_125ms,     (audio_options.buffer_length_ms_hint == 125));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_150ms,     (audio_options.buffer_length_ms_hint == 150));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_175ms,     (audio_options.buffer_length_ms_hint == 175));
   TOGGLE_MENU_ITEM(audio_output_buffer_size_menu_200ms,     (audio_options.buffer_length_ms_hint == 200));

#ifdef USE_ALLEGROGL
   TOGGLE_MENU_ITEM(video_driver_menu_opengl_fullscreen, (gfx_driver->id == GFX_OPENGL_FULLSCREEN));
   TOGGLE_MENU_ITEM(video_driver_menu_opengl_windowed,   (gfx_driver->id == GFX_OPENGL_WINDOWED));
#endif

#ifdef ALLEGRO_DOS
   TOGGLE_MENU_ITEM(video_driver_menu_vga,           (gfx_driver->id == GFX_VGA));
   TOGGLE_MENU_ITEM(video_driver_menu_vga_mode_x,    (gfx_driver->id == GFX_MODEX));
   TOGGLE_MENU_ITEM(video_driver_menu_vesa,          (gfx_driver->id == GFX_VESA1));
   TOGGLE_MENU_ITEM(video_driver_menu_vesa_2_banked, (gfx_driver->id == GFX_VESA2B));
   TOGGLE_MENU_ITEM(video_driver_menu_vesa_2_linear, (gfx_driver->id == GFX_VESA2L));
   TOGGLE_MENU_ITEM(video_driver_menu_vesa_3,        (gfx_driver->id == GFX_VESA3));
   TOGGLE_MENU_ITEM(video_driver_menu_vesa_vbe_af,   (gfx_driver->id == GFX_VBEAF));
#endif

#ifdef ALLEGRO_LINUX
   TOGGLE_MENU_ITEM(video_driver_menu_framebuffer, (gfx_driver->id == GFX_FBCON));
   TOGGLE_MENU_ITEM(video_driver_menu_svgalib,     (gfx_driver->id == GFX_SVGALIB));
   TOGGLE_MENU_ITEM(video_driver_menu_vesa_vbe_af, (gfx_driver->id == GFX_VBEAF));
   TOGGLE_MENU_ITEM(video_driver_menu_vga,         (gfx_driver->id == GFX_VGA));
   TOGGLE_MENU_ITEM(video_driver_menu_vga_mode_x,  (gfx_driver->id == GFX_MODEX));
#endif

#ifdef ALLEGRO_UNIX
   TOGGLE_MENU_ITEM(video_driver_menu_dga,                  (gfx_driver->id == GFX_XDGA));
   TOGGLE_MENU_ITEM(video_driver_menu_dga_fullscreen,       (gfx_driver->id == GFX_XDGA_FULLSCREEN));
   TOGGLE_MENU_ITEM(video_driver_menu_dga_2,                (gfx_driver->id == GFX_XDGA2));
   TOGGLE_MENU_ITEM(video_driver_menu_x_windows,            (gfx_driver->id == GFX_XWINDOWS));
   TOGGLE_MENU_ITEM(video_driver_menu_x_windows_fullscreen, (gfx_driver->id == GFX_XWINDOWS_FULLSCREEN));
#endif

#ifdef ALLEGRO_WINDOWS
   TOGGLE_MENU_ITEM(video_driver_menu_directx,          (gfx_driver->id == GFX_DIRECTX));
   TOGGLE_MENU_ITEM(video_driver_menu_directx_overlay,  (gfx_driver->id == GFX_DIRECTX_OVL));
   TOGGLE_MENU_ITEM(video_driver_menu_directx_windowed, (gfx_driver->id == GFX_DIRECTX_WIN));
   TOGGLE_MENU_ITEM(video_driver_menu_gdi,              (gfx_driver->id == GFX_GDI));
#endif

   /* TODO: Make these call video_get_profile_integer() instead. */
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
   TOGGLE_MENU_ITEM(video_resolution_menu_400_300,   ((SCREEN_W == 400)  && (SCREEN_H == 300)));
   TOGGLE_MENU_ITEM(video_resolution_menu_640_480,   ((SCREEN_W == 640)  && (SCREEN_H == 480)));
   TOGGLE_MENU_ITEM(video_resolution_menu_800_600,   ((SCREEN_W == 800)  && (SCREEN_H == 600)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1024_768,  ((SCREEN_W == 1024) && (SCREEN_H == 768)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1152_864,  ((SCREEN_W == 1152) && (SCREEN_H == 864)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1280_960,  ((SCREEN_W == 1280) && (SCREEN_H == 960)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1280_1024, ((SCREEN_W == 1280) && (SCREEN_H == 1024)));
   TOGGLE_MENU_ITEM(video_resolution_menu_1600_1200, ((SCREEN_W == 1600) && (SCREEN_H == 1200)));

   TOGGLE_MENU_ITEM(video_color_depth_menu_paletted_8_bit,    (video_get_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH) == 8));
   TOGGLE_MENU_ITEM(video_color_depth_menu_true_color_15_bit, (video_get_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH) == 15));
   TOGGLE_MENU_ITEM(video_color_depth_menu_true_color_16_bit, (video_get_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH) == 16));
   TOGGLE_MENU_ITEM(video_color_depth_menu_true_color_24_bit, (video_get_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH) == 24));
   TOGGLE_MENU_ITEM(video_color_depth_menu_true_color_32_bit, (video_get_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH) == 32));

   TOGGLE_MENU_ITEM(video_blitter_menu_disabled,                (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_NONE));
   TOGGLE_MENU_ITEM(video_blitter_menu_interpolation,           (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_INTERPOLATION));
   TOGGLE_MENU_ITEM(video_blitter_menu_interpolation_scanlines, (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_INTERPOLATION_SCANLINES));
   TOGGLE_MENU_ITEM(video_blitter_menu_interpolation_tv_mode,   (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_INTERPOLATION_TV_MODE));
   TOGGLE_MENU_ITEM(video_blitter_menu_hq2x,                    (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_HQ2X));
   TOGGLE_MENU_ITEM(video_blitter_menu_hq3x,                    (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_HQ3X));
   TOGGLE_MENU_ITEM(video_blitter_menu_hq4x,                    (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_HQ4X));
   TOGGLE_MENU_ITEM(video_blitter_menu_ntsc,                    (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER) == VIDEO_BLITTER_NTSC));

   TOGGLE_MENU_ITEM(video_palette_menu_nester,   (video_get_profile_enum(VIDEO_PROFILE_COLOR_PALETTE) == VIDEO_PALETTE_NESTER));
   TOGGLE_MENU_ITEM(video_palette_menu_nesticle, (video_get_profile_enum(VIDEO_PROFILE_COLOR_PALETTE) == VIDEO_PALETTE_NESTICLE));
   TOGGLE_MENU_ITEM(video_palette_menu_ntsc,     (video_get_profile_enum(VIDEO_PROFILE_COLOR_PALETTE) == VIDEO_PALETTE_NTSC));
   TOGGLE_MENU_ITEM(video_palette_menu_pal,      (video_get_profile_enum(VIDEO_PROFILE_COLOR_PALETTE) == VIDEO_PALETTE_PAL));
   TOGGLE_MENU_ITEM(video_palette_menu_rgb,      (video_get_profile_enum(VIDEO_PROFILE_COLOR_PALETTE) == VIDEO_PALETTE_RGB));

   TOGGLE_MENU_ITEM(video_layers_menu_show_back_sprites,  ppu_get_option(PPU_OPTION_ENABLE_SPRITE_BACK_LAYER));
   TOGGLE_MENU_ITEM(video_layers_menu_show_front_sprites, ppu_get_option(PPU_OPTION_ENABLE_SPRITE_FRONT_LAYER));
   TOGGLE_MENU_ITEM(video_layers_menu_show_background,    ppu_get_option(PPU_OPTION_ENABLE_BACKGROUND_LAYER));

   TOGGLE_MENU_ITEM(input_menu_enable_zapper, input_enable_zapper);

   TOGGLE_MENU_ITEM(options_cpu_usage_menu_passive,    (cpu_usage == CPU_USAGE_PASSIVE));
   TOGGLE_MENU_ITEM(options_cpu_usage_menu_normal,     (cpu_usage == CPU_USAGE_NORMAL));
   TOGGLE_MENU_ITEM(options_cpu_usage_menu_aggressive, (cpu_usage == CPU_USAGE_AGGRESSIVE));

   TOGGLE_MENU_ITEM(options_gui_theme_menu_classic,         (last_theme == &classic_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_stainless_steel, (last_theme == &stainless_steel_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_zero_4,          (last_theme == &zero_4_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_panta,           (last_theme == &panta_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_fireflower,      (last_theme == &fireflower_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_xodiac,          (last_theme == &xodiac_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_monochrome,      (last_theme == &monochrome_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_essence,         (last_theme == &essence_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_voodoo,          (last_theme == &voodoo_theme));
   TOGGLE_MENU_ITEM(options_gui_theme_menu_hugs_and_kisses, (last_theme == &hugs_and_kisses_theme));

   /* TODO: Find a better way to do this. */
   uszprintf (audio_menu_volume_text, sizeof (audio_menu_volume_text),
      "Current volume: %d%%", (int)ROUND((apu_options.volume * 100.0)));
   audio_menu[AUDIO_MENU_VOLUME_TEXT].text = audio_menu_volume_text;
}

void gui_load_config (void)
{
   gui_theme_id = get_config_int ("gui", "theme",       GUI_THEME_FIREFLOWER);
   lock_recent  = get_config_int ("gui", "lock_recent", FALSE);
}

void gui_save_config (void)
{
   STRING save_path;
   STRING host;

   STRING_CLEAR(save_path);
   strncpy (save_path, get_config_string ("gui", "save_path", "./"),
      sizeof (save_path) - 1);
   set_config_string ("gui", "save_path", save_path);

   STRING_CLEAR(host);
   strncpy (host, get_config_string ("netplay", "host", ""), (sizeof(host) - 1));
   set_config_string ("netplay", "host", host);

   set_config_int ("gui", "theme",       gui_theme_id);
   set_config_int ("gui", "lock_recent", lock_recent);
}

void gui_preinit(void)
{
   int index;

   /* Clear stuff. */
   for(index = 0; index < GUI_MESSAGE_HISTORY_SIZE; index++)
      USTRING_CLEAR(gui_message_history[index]);

   gui_last_message = 0;

   USTRING_CLEAR(gui_status_text);
   gui_status_color = 0;
}

int gui_init (void)
{
   int index;

   /* Set up default font. */
   font = video_get_font(VIDEO_FONT_MEDIUM);

   /* Set up replacement objects. */
   gui_menu_draw_menu = sl_draw_menu;
   gui_menu_draw_menu_item = sl_draw_menu_item;

   /* Set up menus & dialogs. */
   load_menus ();
   load_dialogs ();

#ifdef ALLEGRO_DOS

   //CHECK_MENU_ITEM(video_menu_fullscreen);
   //DISABLE_MENU_ITEM(video_menu_fullscreen);
   DISABLE_MENU(options_cpu_usage_menu);

#endif   /* ALLEGRO_DOS */

#ifdef ALLEGRO_LINUX
#  ifdef MISSING_SVGALIB
      DISABLE_MENU_ITEM(video_driver_menu_svgalib);
#   endif
#  ifdef MISSING_FBCON
      DISABLE_MENU_ITEM(video_driver_menu_framebuffer);
#  endif
#endif

   /* Load up recent items. */

   main_open_recent_menu_clear ();

   for (index = 0; index < OPEN_RECENT_SLOTS; index++)
   {
      USTRING key;
      const char *path;
      UDATA *filename = open_recent_filenames[index];
      UDATA *text     = open_recent_menu_texts[index];
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

   set_theme ();

   return (0);
}

void gui_exit (void)
{
   int index;

   /* Save recent items. */

   for (index = 0; index < OPEN_RECENT_SLOTS; index++)
   {
      USTRING key;
      const UDATA *filename = open_recent_filenames[index];

      if (!filename)
         continue;

      USTRING_CLEAR(key);
      uszprintf (key, sizeof (key), "recent%d", index);

      set_config_string ("gui", key, filename);
   }

   unload_menus ();
   unload_dialogs ();
}

void show_gui (BOOL first_run)
{
   do
   {
      int result;

      /* Clear restart flag. */
      gui_needs_restart = FALSE;

      /* Open GUI. */
   
      result = gui_open ();
   
      if (result != 0)
      {
         WARN("Failed to open GUI");
         return;
      }

      if (first_run)
      {
         /* Show version. */
         help_menu_about ();

         /* Clear flag. */
         first_run = FALSE;
      }

      /* Update menu states. */
      update_menus ();

      /* Run main dialog. */
      run_dialog (main_dialog, -1);

      /* Close GUI. */
      gui_close ();

      /* Remove any stale GUI keys (e.g ESC) from the VM keyboard buffer */
      machine_clear_key_buffer();

   } while (gui_needs_restart);
}

void gui_update_display(void) 
{
   update_display();
}

int gui_alert (const UDATA *title, const UDATA *s1, const UDATA *s2, const
   UDATA *s3, const UDATA *b1, const UDATA *b2, int c1, int c2)
{
   /* Alert dialog with 1 or 2 buttons.  The title, first string, and first
      button are required.  The rest are optional and may be NULL.

      This function can even be called when the GUI isn't open.  It'll
      automatically open the GUI long enough to display the dialog. */

   int result;
   BOOL gui_opened = FALSE;
   DIALOG *dialog;
   DIALOG *objframe;
   DIALOG *objxbutton;
   DIALOG *objstr1, *objstr2, *objstr3;
   DIALOG *objbtn1, *objbtn2;
   int s1len, s2len, s3len;
   int collapse = 0;

   RT_ASSERT(title);
   RT_ASSERT(s1);
   RT_ASSERT(b1);

   /* Handle any NULL entries. */

   if (!s2) s2 = empty_string;
   if (!s3) s3 = empty_string;

   if (ustrlen (s2) == 0)
      collapse += (text_height (font) + 3);
   if (ustrlen (s3) == 0)
      collapse += (text_height (font) + 3);

   if (!gui_is_active)
   {
      /* Open GUI. */
   
      result = gui_open ();
   
      if (result != 0)
      {
         WARN("Failed to open GUI");
         return ((8 + result));
      }

      gui_opened = TRUE;
   }

   /* Create dialog. */
   dialog = load_dialog (alert_dialog_base);
   if (!dialog)
   {
      WARN("Failed to create dialog structure");
      return (-1);
   }

   /* Get all objects. */

   objframe   = &dialog[ALERT_DIALOG_FRAME];
   objxbutton = &dialog[ALERT_DIALOG_CLOSE_BUTTON];
   objstr1    = &dialog[ALERT_DIALOG_STRING_1];
   objstr2    = &dialog[ALERT_DIALOG_STRING_2];
   objstr3    = &dialog[ALERT_DIALOG_STRING_3];
   objbtn1    = &dialog[ALERT_DIALOG_BUTTON_1];
   objbtn2    = &dialog[ALERT_DIALOG_BUTTON_2];

   /* Calculate string lengths (in pixels). */

   s1len = text_length (font, s1);
   s2len = text_length (font, s2);
   s3len = text_length (font, s3);

   /* Set up frame. */

   objframe->w   = (9 + MAX3(s1len, s2len, s3len) + 9);
   objframe->dp2 = (char *)title;

   objxbutton->x = ((objframe->w - objxbutton->w) - 4);

   /* Set up strings. */

   objstr1->x   = ((objframe->w / 2) - (s1len / 2));
   objstr1->dp2 = (char *)s1;

   objstr2->x   = ((objframe->w / 2) - (s2len / 2));
   objstr2->dp2 = (char *)s2;

   objstr3->x   = ((objframe->w / 2) - (s3len / 2));
   objstr3->dp2 = (char *)s3;

   /* Set up buttons. */

   objbtn1->x   = ((objframe->w / 2) - (objbtn1->w / 2));
   objbtn1->dp  = (char *)b1;
   objbtn1->key = c1;

   objbtn1->y -= collapse;

   if (b2)
   {
      objbtn1->x -= ((objbtn2->w / 2) + 4);

      objbtn2->x   = ((objbtn1->x + objbtn1->w) + 8);
      objbtn2->dp  = (char *)b2;
      objbtn2->key = c2;

      objbtn2->y -= collapse;
   }
   else
   {
      /* Hide the unused button. */

      objbtn2->flags |= D_HIDDEN;
      objbtn2->flags |= D_DISABLED;
   }

   /* Collapse frame. */
   objframe->h -= collapse;

   /* Show dialog. */
   result = show_dialog (dialog, ALERT_DIALOG_BUTTON_1);

   /* Destroy dialog. */
   unload_dialog (dialog);

   if(gui_opened)
      /* Close GUI. */
      gui_close();

   if (result == ALERT_DIALOG_BUTTON_1)
      return (1); /* OK. */
   else
      return (2); /* Cancel. */
}

void gui_message (int color, const UDATA *message, ...)
{
   va_list format;
   USTRING buffer;

   va_start(format, message);
   uvszprintf(buffer, USTRING_SIZE, message, format);
   va_end(format);

   add_message(buffer);

   if (gui_is_active)
   {
      if (color == -1)
         color = GUI_TEXT_COLOR;

      status_text_color (color, buffer);
   }
}

/* This just sends a preformatted message to the message history.
   Newlines are also converted to separate strings.

   This gets called by log_printf() to display the log in the GUI. */
void gui_log_message(const UDATA* message)
{
   USTRING buffer;
   int length, index, subindex;

   RT_ASSERT(message);

   length = ustrlen(message);
   index = 0;

   USTRING_CLEAR(buffer);
   subindex = 0;

   BOOL done = FALSE;
   while(!done) {
      const UCCHAR c = ugetat(message, index);
      index++;
      if(index == length)
         done = TRUE;

      if(c == '\n') {
         if(ustrlen(buffer) > 0)
            add_message(buffer);

         USTRING_CLEAR(buffer);
         subindex = 0;
      }
      else {  
         uinsert(buffer, subindex, c);
         subindex++;
      }
   }

   if(ustrlen(buffer) > 0)
      add_message(buffer);
}

void gui_heartbeat (void)
{
   /* Called in varous places to process NetPlay (if active), refresh the
      screen, and rest() to minimize CPU usage in the GUI. */

   if (netplay_mode)
      netplay_process ();

   refresh ();

  /* rest() has been moved into refresh() */
}

void gui_handle_keypress (int c, int scancode)
{
   switch (scancode)
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
         system_menu_show_status ();

         break;
      }

      case KEY_F3:
      {
         /* Quick save state. */
         system_save_state_menu_quick_save ();

         /* See if the save succeeded. */
         if (check_save_state (-1))
            status_text ("QuickSave: OK");
         else
            status_text ("QuickSave: Failed");
            
         break;
      }

      case KEY_F4:
      {
         /* Quick load state. */

         if (!(input_mode & INPUT_MODE_REPLAY))
            system_save_state_menu_quick_load ();

         break;
      }

      case KEY_F5:
      {
         /* Save state. */
         system_save_state_menu_save ();

         break;
      }

      case KEY_F6:
      {
         /* Load state. */

         if (!(input_mode & INPUT_MODE_REPLAY))
            system_save_state_menu_restore ();

         break;
      }

      case KEY_F7:
      {
         /* Toggle sprites. */

         video_layers_menu_show_back_sprites ();
         video_layers_menu_show_front_sprites ();

         break;
      }

      case KEY_F8:
      {
         /* Toggle background. */
         video_layers_menu_show_background ();

         break;
      }

      case KEY_F9:
      {
         /* Toggle half speed mode. */

         timing_half_speed = !timing_half_speed;
         timing_update_timing ();

         break;
      }

      case KEY_F12:
      {
         /* Start/stop replay recording. */

         if (!(input_mode & INPUT_MODE_REPLAY_PLAY))
         {
            if (input_mode & INPUT_MODE_REPLAY_RECORD)
               main_replay_menu_record_stop ();
            else
               main_replay_menu_record_start ();

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
            save_state_index = (scancode - KEY_0);
    
            status_text ("Machine state slot set to %d.",
               save_state_index);
         }

         break;
      }

      case KEY_MINUS:
      case KEY_MINUS_PAD:
      {
         audio_menu_volume_decrease ();
         break;
      }

      case KEY_EQUALS:
      case KEY_PLUS_PAD:
      {
         audio_menu_volume_increase ();
         break;
      }

      default:
         break;
   }
}

void gui_stop_replay (void)
{
   main_replay_menu_play_stop ();
}

void gui_set_theme (const GUI_THEME *theme)
{
   int index;

   RT_ASSERT(theme);

   last_theme = theme;

   memcpy (&gui_theme, theme, sizeof (GUI_THEME));

   for (index = 0; index < GUI_TOTAL_COLORS; index++)
      pack_color (&gui_theme[index]);

   gui_bg_color = GUI_FILL_COLOR;
   gui_fg_color = GUI_TEXT_COLOR;
   gui_mg_color = GUI_DISABLED_COLOR;
}

/* --- Utility functions. --- */

static INLINE void set_autosave (int interval)
{
   /* This function simply sets the save state autosave interval to
      'interval' seconds (in game speed, not real world speed :b). */

   input_autosave_interval = interval;
   update_menus ();

   if (interval <= 0)
      status_text ("Autosave disabled.");
   else
      status_text ("Autosave interval set to %d seconds.", interval);
}

static int main_replay_menu_select (void);

static int open_lobby (void)
{
   /* This function handles the entire GUI end of the NetPlay lobby.  It
      does not return until the NetPlay session has been terminated.

      Returns one of the following:
         D_O_K   - The NetPlay session has been closed, by pressing either
                   the [ x] close button or the Cancel button.
         D_CLOSE - The Netplay session is still open, all neccessary data
                   has been distributed and subsequently loaded, and control
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

   obj_chat->bg = makecol (0, 0, 0);
   obj_chat->fg = makecol (240, 240, 240);
   obj_chat->d1 = ((sizeof (chat) / MAX_UTF8_SEGMENTS) - 1);
   obj_chat->dp = chat;

   obj_list->bg = makecol (0, 0, 0);
   obj_list->fg = makecol (240, 240, 240);
   obj_list->d1 = ((sizeof (list) / MAX_UTF8_SEGMENTS) - 1);
   obj_list->dp = list;

   obj_message->d1 = ((sizeof (message) / MAX_UTF8_SEGMENTS) - 1);
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
      status_text_color (GUI_ERROR_COLOR, "Failed to create dialog player!");
      return (D_O_K);
   }

   while (update_dialog (player))
   {
      netplay_enumerate_chat (chat, sizeof(chat));
      netplay_enumerate_clients (list, sizeof(list));

      scare_mouse ();
      object_message (obj_chat, MSG_DRAW, 0);
      object_message (obj_list, MSG_DRAW, 0);
      unscare_mouse ();

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
   
         status_text ("NetPlay session closed.");

         return (D_O_K);
      }
   }
}

/* --- Menu handlers. --- */

static int main_menu_hide_gui (void)
{
    return (D_CLOSE);
}

/* This is a helper for main_menu_open() and the recent files menu.
   This could probably be moved somewhere more suitable, but its not a big deal. */
static BOOL load_helper(const UDATA* filename) {
   const UDATA* error = NULL;

   RT_ASSERT(filename);

   status_text ("Loading, please wait...");

   error = load_file(filename);
   if(error) {
      status_text_color(GUI_ERROR_COLOR, error);
      return FALSE;
   }

   /* Sanity check. */
   if(!file_is_loaded) {
      WARN_GENERIC();
      return FALSE;
   }

   /* Update save state titles. */
   system_save_state_menu_select ();
   /* Update replay titles. */
   main_replay_menu_select ();

   /* Update menus. */
   update_menus();

   /* Display a tip about hiding the GUI. */
   status_text ("Tip: Press ESC to hide the GUI and maximise the game.");

   return TRUE;
}

static int main_menu_open (void)
{
   USTRING path;
   BOOL locked;
   int result;
   USTRING scratch;

   if (rom_is_loaded)
   {
      if (gui_alert ("Confirmation", "A game is already loaded.", "If you continue, any unsaved progress will be lost.", "Are you sure?", "&OK", "&Cancel", 0, 0) == 2)
      {
         /* Cancelled. */
         return (D_O_K);
      }
   }

   /* Retrive path from configuration file. */
   USTRING_CLEAR(path);
#ifdef SYSTEM_POSIX
   ustrncat (path, get_config_string ("gui", "open_path", "~/"), (sizeof(path) - 1));
#else
   ustrncat (path, get_config_string ("gui", "open_path", "/"), (sizeof(path) - 1));
#endif

   locked = get_config_int ("gui", "lock_paths", FALSE);

#ifdef USE_ZLIB
   result = gui_file_select ("Open", "Supported file types (*.NES, *.GZ, *.ZIP, *.NSF)", path, sizeof(path), "*.nes;*.gz;*.zip;*.nsf;*.NES;*.GZ;*.ZIP;*.NSF");
#else
   result = gui_file_select ("Open", "Supported file types (*.NES, *.NSF)", path, sizeof(path), "*.nes;*.nsf;*.NES;*.NSF");
#endif

   if (!locked)
   {  
      /* Update path. */
      set_config_string ("gui", "open_path", replace_filename (scratch, path, "", sizeof (scratch)));
   }

   if (result != 0)
   {
      /* Dialog was OK'ed. */

      if(!load_helper(path)) {
         /* The file failed to load, skip everything else. */
         return D_O_K;
      }
      
      if(!lock_recent)
      {
         /* Load succeeded; add file to recent items list. */

         int index;

         /* Move all existing entries down by 1 slot. */
         for (index = (OPEN_RECENT_SLOTS - 2); index >= 0; index--)
            ustrncpy (open_recent_filenames[(index + 1)], open_recent_filenames[index], USTRING_SIZE);

         /* Add new entry to the beginning of the list. */
         uszprintf (open_recent_filenames[0], USTRING_SIZE, "%s", path);

         /* Update menus. */

         for (index = 0; index < OPEN_RECENT_SLOTS; index++)
         {
            const UDATA *filename = open_recent_filenames[index];
            UDATA       *text     = open_recent_menu_texts[index];
            MENU        *menu     = &main_open_recent_menu[index];

            if (filename[0])
            {
               /* Build menu text. */
               uszprintf (text, USTRING_SIZE, "&%d: %s", index, get_filename (filename));

               /* Enable menu. */
               menu->flags &= ~D_DISABLED;
            }
            else
            {
               /* Build menu text. */
               uszprintf (text, USTRING_SIZE, "&%d: %s", index, UNUSED_SLOT_TEXT);

               /* Disable menu. */
               menu->flags |= D_DISABLED;
            }

            /* Set menu text. */
            menu->text = text;
         }
      }
   }

   return (D_O_K);
}

#define OPEN_RECENT_MENU_HANDLER(index) \
   static int main_open_recent_menu_##index (void)  \
   {  \
      if (rom_is_loaded) \
      { \
         if (gui_alert ("Confirmation", "A game is already loaded.", "If you continue, any unsaved progress will be lost.", "Are you sure?", "&OK", "&Cancel", 0, 0) == 2) \
         { \
            /* Cancelled. */ \
            return (D_O_K); \
         } \
      } \
      \
      load_helper(open_recent_filenames[index]); \
      return D_O_K; \
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
      UDATA *filename = open_recent_filenames[index];
      UDATA *text     = open_recent_menu_texts[index];
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
   /* Since games can lose data easily, we present a confirmation prompt for them.
      This doesn't need to be done for NSFs, obviously. */
   if(rom_is_loaded) {
      if (gui_alert ("Confirmation", "If you continue, any unsaved progress will be lost.", "Are you sure you want to close the file?", NULL, "&OK", "&Cancel", 0, 0) == 2)
      {
         /* Cancelled. */
         return (D_O_K);
      }
   }

   /* Close the file. */
   close_file();

   /* Update menus. */
   update_menus();

   return (D_O_K);
}

#define REPLAY_SELECT_MENU_HANDLER(index) \
   static int main_replay_select_menu_##index (void)  \
   {  \
      replay_index = index;   \
      update_menus ();  \
      status_text ("Replay slot set to %d.", index);   \
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
      UDATA *title;
      UDATA *text;

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

static int main_replay_menu_record_start (void)
{
   USTRING title;

   /* Duplicate title. */
   ustrncpy (title, replay_titles[replay_index], USTRING_SIZE);

   /* Patch up duplicate. */
   fix_save_title (title, sizeof (title));
                 
   if (gui_is_active)
   {
      DIALOG *dialog;
      DIALOG *objtitle;

      /* Allow user to customize title before save. */

      dialog = main_replay_record_start_dialog;

      objtitle = &dialog[MAIN_REPLAY_RECORD_START_DIALOG_TITLE];

      objtitle->d1 = NEW_SAVE_TITLE_SIZE;
      objtitle->dp = title;

      if (show_dialog (dialog, -1) !=
         MAIN_REPLAY_RECORD_START_DIALOG_OK_BUTTON)
      {
         /* Dialog was cancelled. */
         return (D_O_K);
      }
   }

   /* Open replay file. */
   if (!open_replay (replay_index, "w", title))
   {
      status_text_color (GUI_ERROR_COLOR, "Failed to open new machine state "
         "file.");

      return (D_O_K);
   }

   DISABLE_MENU_ITEM(main_menu_open);
   DISABLE_SUBMENU(main_open_recent_menu);
   DISABLE_MENU_ITEM(main_menu_close);
   DISABLE_MENU_ITEM(main_replay_menu_record_start);
   ENABLE_MENU_ITEM(main_replay_menu_record_stop);
   DISABLE_MENU_ITEM(main_replay_menu_play_start);
   DISABLE_SUBMENU(main_replay_select_menu);
   DISABLE_SUBMENU(system_save_state_autosave_menu);
   DISABLE_SUBMENU(netplay_menu);

   /* Enter replay recording mode. */
   input_mode |= INPUT_MODE_REPLAY;
   input_mode |= INPUT_MODE_REPLAY_RECORD;
 
   status_text ("Replay recording session started.");
 
   /* Update replay titles. */
   main_replay_menu_select ();

   return (D_CLOSE);
}

static int main_replay_menu_record_stop (void)
{
   /* Close replay. */
   close_replay ();

   /* Exit replay recording mode. */
   input_mode &= ~INPUT_MODE_REPLAY;
   input_mode &= ~INPUT_MODE_REPLAY_RECORD;

   ENABLE_MENU_ITEM(main_menu_open);
   ENABLE_SUBMENU(main_open_recent_menu);
   ENABLE_MENU_ITEM(main_menu_close);
   ENABLE_MENU_ITEM(main_replay_menu_record_start);
   DISABLE_MENU_ITEM(main_replay_menu_record_stop);
   ENABLE_MENU_ITEM(main_replay_menu_play_start);
   ENABLE_SUBMENU(main_replay_select_menu);
   ENABLE_SUBMENU(system_save_state_autosave_menu);
   ENABLE_SUBMENU(netplay_menu);

   status_text ("Replay recording session stopped.");

   return (D_O_K);
}

static int main_replay_menu_play_start (void)
{
   if (!open_replay (replay_index, "r", NULL))
   {                       
      status_text_color (GUI_ERROR_COLOR, "Failed to open machine state file.");

      return (D_O_K);
   }

   DISABLE_MENU_ITEM(main_menu_open);
   DISABLE_SUBMENU(main_open_recent_menu);
   DISABLE_MENU_ITEM(main_menu_close);
   DISABLE_MENU_ITEM(main_replay_menu_play_start);
   ENABLE_MENU_ITEM(main_replay_menu_play_stop);
   DISABLE_MENU_ITEM(main_replay_menu_record_start);
   DISABLE_SUBMENU(main_replay_select_menu);
   DISABLE_MENU_ITEM(main_menu_cheat_manager);
   DISABLE_MENU_ITEM(system_menu_reset);
   DISABLE_MENU_ITEM(system_menu_power_cycle);
   DISABLE_MENU_ITEM(system_save_state_menu_quick_load);
   DISABLE_MENU_ITEM(system_save_state_menu_restore);
   DISABLE_SUBMENU(netplay_menu);

   /* Enter replay playback mode. */
   input_mode &= ~INPUT_MODE_PLAY;
   input_mode |= INPUT_MODE_REPLAY;
   input_mode |= INPUT_MODE_REPLAY_PLAY;

   status_text ("Replay playback started.");

   return (D_CLOSE);
}

static int main_replay_menu_play_stop (void)
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
   ENABLE_MENU_ITEM(main_menu_close);
   ENABLE_MENU_ITEM(main_replay_menu_play_start);
   DISABLE_MENU_ITEM(main_replay_menu_play_stop);
   ENABLE_MENU_ITEM(main_replay_menu_record_start);
   ENABLE_SUBMENU(main_replay_select_menu);
   ENABLE_MENU_ITEM(main_menu_cheat_manager);
   ENABLE_MENU_ITEM(system_menu_reset);
   ENABLE_MENU_ITEM(system_menu_power_cycle);
   ENABLE_MENU_ITEM(system_save_state_menu_quick_load);
   ENABLE_MENU_ITEM(system_save_state_menu_restore);
   ENABLE_SUBMENU(netplay_menu);

   if (gui_is_active)
      status_text ("Replay playback stopped.");
   else
      status_text ("Replay playback finished.");

   return (D_O_K);
}

static int main_menu_cheat_manager (void)
{
   if (show_dialog (main_cheat_manager_dialog, -1) ==
      MAIN_CHEAT_MANAGER_DIALOG_SAVE_BUTTON)
   {
      save_patches ();
   }

   return (D_O_K);
}

static int main_menu_save_snapshot (void)
{
   int index;

   for (index = 0; index < 999; index++)
   {
      USTRING path;
      USTRING filename;

      uszprintf (filename, sizeof (filename), "%s_%03d.pcx", get_filename
         (global_rom.filename), index);

      /* Merge it with our save path. */
      get_save_path (filename, sizeof (filename));

      if (exists (filename))
         continue;

      save_bitmap (filename, video_get_render_buffer(), NULL);

      status_text ("Snapshot saved to %s.", filename);

      return (D_O_K);
   }

   status_text_color (GUI_ERROR_COLOR, "Couldn't find a suitable image "
      "filename.");

   return (D_O_K);
}

static int main_menu_advance_frame (void)
{
   return D_O_K;
}

static int main_record_audio_menu_start (void)
{
   int index;

   for (index = 0; index < 999; index++)
   {
      USTRING filename;

      uszprintf (filename, sizeof (filename), "%s_%03d.wav", get_filename
         (global_rom.filename), index);

      /* Merge it with our save path. */
      get_save_path (filename, sizeof (filename));

      if (exists (filename))
         continue;

      if (audio_open_wav (filename) == 0)
      {
         DISABLE_MENU_ITEM(main_record_audio_menu_start);
         ENABLE_MENU_ITEM(main_record_audio_menu_stop);
      }
   
      status_text ("Audio WAV recording started to %s.", filename);

      return (D_O_K);
   }
   
   status_text_color (GUI_ERROR_COLOR, "Couldn't find a suitable sound "
      "filename.");

   return (D_O_K);
}

static int main_record_audio_menu_stop (void)
{
   audio_close_wav ();

   ENABLE_MENU_ITEM(main_record_audio_menu_start);
   DISABLE_MENU_ITEM(main_record_audio_menu_stop);

   status_text ("Audio WAV recording stopped.");

   return (D_O_K);
}

static int main_menu_save_configuration (void)
{
   save_config ();
   gui_alert ("Confirmation", "Configuration has been saved.", NULL, NULL, "&OK", NULL, 0, 0);

   return (D_O_K);
}

static int main_menu_exit (void)
{
   if (rom_is_loaded)
   {
      /* Confirm exit. */

      if (gui_alert ("Confirmation", "A game is currently loaded.", "If you continue, any unsaved progress will be lost.", "Really exit?", "&OK", "&Cancel", 0, 0) == 2)
      {
         /* Cancelled. */
         return (D_O_K);
      }
      else
      {
         want_exit = TRUE;
      
         return (D_CLOSE);
      }
   }
   else
   {
      /* Just exit. */

      want_exit = TRUE;
   
      return (D_CLOSE);
   }
}

#define SAVE_STATE_SELECT_MENU_HANDLER(index)   \
   static int system_save_state_select_menu_##index (void)   \
   {  \
      save_state_index = index;  \
      update_menus ();  \
      status_text ("Machine state slot set to %d.", index);  \
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

static int system_save_state_menu_quick_save (void)
{
   if (!save_state (-1, "QUICKSAVE"))
   {
      status_text_color (GUI_ERROR_COLOR, "Quick Save failed.");

      return (D_O_K);
   }

   return (D_CLOSE);
}

static int system_save_state_menu_quick_load (void)
{
   if (!load_state (-1))
   {
      status_text_color (GUI_ERROR_COLOR, "Quick Load failed.");

      return (D_O_K);
   }

   return (D_CLOSE);
}

static int system_save_state_menu_save (void)
{
   USTRING title;
   USTRING filename;

   /* Duplicate title. */
   ustrncpy (title, save_state_titles[save_state_index], sizeof (title));

   /* Patch up duplicate. */
   fix_save_title (title, sizeof (title));

   if (gui_is_active)
   {
      DIALOG *dialog;
      DIALOG *objtitle;

      /* Allow user to customize title before save. */

      dialog = system_save_state_save_dialog;

      objtitle = &dialog[SYSTEM_SAVE_STATE_SAVE_DIALOG_TITLE];

      objtitle->d1 = NEW_SAVE_TITLE_SIZE;
      objtitle->dp = title;

      if (show_dialog (dialog, -1) !=
         SYSTEM_SAVE_STATE_SAVE_DIALOG_OK_BUTTON)
      {
         /* Cancelled. */
         return (D_O_K);
      }
   }

   if (!save_state (save_state_index, title))
   {
      status_text_color (GUI_ERROR_COLOR, "Failed to open new machine state "
         "file.");

      return (D_O_K);
   }

   status_text ("Machine state saved to slot %d.", save_state_index);

   /* Update save state titles. */
   system_save_state_menu_select ();

   return (D_CLOSE);
}

static int system_save_state_menu_restore (void)
{
   if (!load_state (save_state_index))
   {
      status_text_color (GUI_ERROR_COLOR, "Failed to open machine state file.");

      return (D_O_K);
   }

   status_text ("Machine state loaded from slot %d.", save_state_index);

   return (D_CLOSE);
}

static int system_save_state_menu_select (void)
{
   int index;

   for (index = 0; index < SAVE_STATE_SLOTS; index++)
   {
      UDATA *title;
      UDATA *text;

      title = save_state_titles[index];
      text = save_state_menu_texts[index];

      /* Get title. */
      get_state_title (index, title, USTRING_SIZE);

      /* Build menu text. */
      uszprintf (text, USTRING_SIZE, "&%d: %s", index, title);

      /* Update menu. */
      system_save_state_select_menu[index].text = text;
   }

   return (D_O_K);
}

static int system_save_state_autosave_menu_disabled (void)
{
   set_autosave (0);

   return (D_O_K);
}

static int system_save_state_autosave_menu_10_seconds (void)
{
   set_autosave (10);

   return (D_O_K);
}

static int system_save_state_autosave_menu_30_seconds (void)
{
   set_autosave (30);

   return (D_O_K);
}

static int system_save_state_autosave_menu_60_seconds (void)
{
   set_autosave (60);

   return (D_O_K);
}

static int system_save_state_autosave_menu_custom (void)
{
   int seconds;

   seconds = input_autosave_interval;

   if (get_integer_input ("Custom", &seconds, "seconds"))
      set_autosave (seconds);

   return (D_O_K);
}

static int system_region_menu_automatic (void)
{
   machine_region = MACHINE_REGION_AUTOMATIC;
   timing_update_machine_type ();
   update_menus ();

   status_text ("System region set to automatic.");

   return (D_O_K);
}

static int system_region_menu_ntsc (void)
{
   machine_region = MACHINE_REGION_NTSC;
   timing_update_machine_type ();
   update_menus ();

   status_text ("System region set to NTSC.");

   return (D_O_K);
}

static int system_region_menu_pal (void)
{
   machine_region = MACHINE_REGION_PAL;
   timing_update_machine_type ();
   update_menus ();

   status_text ("System region set to PAL.");

   return (D_O_K);
}

static int system_speed_up_down_menu_50_percent (void)
{
   timing_speed_multiplier = 0.5;
   timing_update_timing ();

   update_menus ();

   status_text ("Machine speed factor set to 50%%.");

   return (D_O_K);
}

static int system_speed_up_down_menu_100_percent (void)
{
   timing_speed_multiplier = 1.0;
   timing_update_timing ();

   update_menus ();

   status_text ("Machine speed factor set to 100%%.");

   return (D_O_K);
}

static int system_speed_up_down_menu_200_percent (void)
{
   timing_speed_multiplier = 2.0;
   timing_update_timing ();

   update_menus ();

   status_text ("Machine speed factor set to 200%%.");

   return (D_O_K);
}

static int system_speed_up_down_menu_custom (void)
{
   REAL value;

   value = (timing_speed_multiplier * 100.0);

   if (get_float_input ("Custom", &value, "percent"))
   {
      timing_speed_multiplier = (value / 100.0);
      timing_update_timing ();

      update_menus ();

      status_text ("Machine speed factor set to custom.");
   }

   return (D_O_K);
}

static int system_frame_skip_menu_automatic (void)
{
   frame_skip = -1;
   update_menus ();

   status_text ("Frame skip set to automatic.");

   return (D_O_K);
}

static int system_frame_skip_menu_disabled (void)
{
   frame_skip = 0;
   update_menus ();

   status_text ("Frame skip disabled.");

   return (D_O_K);
}

#define FRAME_SKIP_MENU_HANDLER(frames)   \
   static int system_frame_skip_menu_skip_##frames##_frames (void) \
   {  \
      frame_skip = frames; \
      update_menus ();  \
      status_text ("Frame skip set to %d frames.", frames);  \
      return (D_O_K);   \
   }

FRAME_SKIP_MENU_HANDLER(1)
FRAME_SKIP_MENU_HANDLER(2)
FRAME_SKIP_MENU_HANDLER(3)

#undef FRAME_SKIP_MENU_HANDLER

static int system_frame_skip_menu_custom (void)
{
   int frames;

   frames = frame_skip;

   if (get_integer_input ("Custom", &frames, "frames"))
   {
      frame_skip = frames;
      update_menus ();
   
      status_text ("Frame skip set to %d frames.", frames);
   }

   return (D_O_K);
}

static int system_menu_show_status (void)
{
   const ENUM key = VIDEO_PROFILE_OPTION_HUD;
   video_set_profile_boolean(key, !video_get_profile_boolean(key));
   video_update_settings();

   update_menus ();

   return (D_O_K);
}

static int system_menu_reset (void)
{
   /* Confirm reset. */

   if (gui_alert ("Confirmation", "This action will reset the virtual machine.", "If you continue, any unsaved progress will be lost.", "Really reset the virtual machine?", "&OK", "&Cancel", 0, 0) == 2)
   {
      /* Cancelled. */
      return (D_O_K);
   }
   else
   {
      machine_reset ();

      /* Clear the game clock. */
      options_menu_reset_clock ();

      return (D_CLOSE);
   }
}

static int system_menu_power_cycle (void)
{
   /* Confirm power cycle. */

   if (gui_alert ("Confirmation", "This action will power cycle the virtual machine.", "If you continue, any unsaved progress will be lost.", "Really power cycle the virtual machine?", "&OK", "&Cancel", 0, 0) == 2)
   {
      /* Cancelled. */
      return (D_O_K);
   }
   else
   {
      machine_exit ();
      machine_init ();

      /* Clear the game clock. */
      options_menu_reset_clock ();

      return (D_O_K);
   }
}

static int system_menu_timing_smoothest (void)
{
   machine_timing = MACHINE_TIMING_SMOOTH;
   timing_update_timing ();
   update_menus ();

   status_text ("Machine timing mode set to smoothest.");

   return (D_O_K);
}

static int system_menu_timing_most_accurate (void)
{
   machine_timing = MACHINE_TIMING_ACCURATE;
   timing_update_timing ();
   update_menus ();

   status_text ("Machine timing mode set to most accurate.");

   return (D_O_K);
}

static int system_menu_speed_cap (void)
{
   speed_cap = !speed_cap;
   update_menus ();

   status_text ("Speed cap %s.", get_enabled_text (speed_cap));

   return (D_O_K);
}

static int audio_menu_enable_apu (void)
{
   apu_options.enabled = !apu_options.enabled;
   update_menus ();

   apu_update ();

   status_text ("APU emulation %s.",
      get_enabled_text (apu_options.enabled));

   return (D_O_K);
}

static int audio_menu_enable_output (void)
{
   audio_options.enable_output = !audio_options.enable_output;

   cycle_audio ();
   update_menus ();

   status_text ("Audio output %s.", get_enabled_text (audio_options.enable_output));

   return (D_O_K);
}

static int audio_menu_emulation_fast (void)
{
   apu_options.emulation = APU_EMULATION_FAST;
   update_menus ();

   apu_update ();

   gui_alert ("Accuracy Warning", "The fast emulation mode may introduce permanent accuracy glitches into save states, "
      "replays and the game rewinder.",
      "Only use it if you have a very slow computer that cannot handle one of the other emulation modes.",
      NULL, "&OK", NULL, 0, 0);

   status_text ("APU emulation quality set to fast.");

   return (D_O_K);
}

static int audio_menu_emulation_accurate (void)
{
   apu_options.emulation = APU_EMULATION_ACCURATE;
   update_menus ();

   apu_update ();

   status_text ("APU emulation quality set to accurate.");

   return (D_O_K);
}

static int audio_menu_emulation_high_quality (void)
{
   apu_options.emulation = APU_EMULATION_HIGH_QUALITY;
   update_menus ();

   apu_update ();

   status_text ("APU emulation quality set to high quality.");

   return (D_O_K);
}

#define AUDIO_CHANNELS_MENU_HANDLER(id, name)  \
   static int audio_channels_menu_##id (void) \
   { \
      BOOL *enabled = &apu_options.enable_##id ; \
      *enabled = !*enabled; \
      update_menus (); \
      apu_update (); \
      status_text ("APU " name " %s.", get_enabled_text (*enabled)); \
      return (D_O_K); \
   }

AUDIO_CHANNELS_MENU_HANDLER(square_1, "first square wave channel")
AUDIO_CHANNELS_MENU_HANDLER(square_2, "second square wave channel")
AUDIO_CHANNELS_MENU_HANDLER(triangle, "triangle wave channel")
AUDIO_CHANNELS_MENU_HANDLER(noise,    "noise channel")
AUDIO_CHANNELS_MENU_HANDLER(dmc,      "delta modulation channel")
AUDIO_CHANNELS_MENU_HANDLER(extra_1,  "first expansion channel")
AUDIO_CHANNELS_MENU_HANDLER(extra_2,  "second expansion channel")
AUDIO_CHANNELS_MENU_HANDLER(extra_3,  "third expansion channel")

#undef AUDIO_CHANNELS_MENU_HANDLER

static int audio_channels_menu_enable_all (void)
{               
   apu_options.enable_square_1 = TRUE;
   apu_options.enable_square_2 = TRUE;
   apu_options.enable_triangle = TRUE;
   apu_options.enable_noise    = TRUE;
   apu_options.enable_dmc      = TRUE;
   apu_options.enable_extra_1  = TRUE;
   apu_options.enable_extra_2  = TRUE;
   apu_options.enable_extra_3  = TRUE;

   update_menus ();

   apu_update ();

   status_text ("All APU channels enabled.");

   return (D_O_K);
}

static int audio_channels_menu_disable_all (void)
{
   apu_options.enable_square_1 = FALSE;
   apu_options.enable_square_2 = FALSE;
   apu_options.enable_triangle = FALSE;
   apu_options.enable_noise    = FALSE;
   apu_options.enable_dmc      = FALSE;
   apu_options.enable_extra_1  = FALSE;
   apu_options.enable_extra_2  = FALSE;
   apu_options.enable_extra_3  = FALSE;

   update_menus ();

   apu_update ();

   status_text ("All APU channels disabled.");

   return (D_O_K);
}

static int audio_output_menu_subsystem_automatic (void)
{
   audio_options.subsystem = AUDIO_SUBSYSTEM_AUTOMATIC;
   
   cycle_audio ();
   update_menus ();

   status_text ("Audio subsystem set to Automatic.");
   
   return (D_O_K);
}

static int audio_output_menu_subsystem_safe (void)
{
   audio_options.subsystem = AUDIO_SUBSYSTEM_SAFE;
   
   cycle_audio ();
   update_menus ();

   status_text ("Audio subsystem set to Safe.");
   
   return (D_O_K);
}

static int audio_output_menu_subsystem_allegro (void)
{
   audio_options.subsystem = AUDIO_SUBSYSTEM_ALLEGRO;
   
   cycle_audio ();
   update_menus ();

   status_text ("Audio subsystem set to Allegro.");
   
   return (D_O_K);
}

static int audio_output_menu_subsystem_openal (void)
{
   audio_options.subsystem = AUDIO_SUBSYSTEM_OPENAL;

   cycle_audio ();
   update_menus ();

   status_text ("Audio subsystem set to OpenAL.");

   return (D_O_K);
}

#define AUDIO_OUTPUT_MENU_SAMPLING_RATE_HANDLER(rate)  \
   static int audio_output_menu_sampling_rate_##rate##_hz (void) \
   {  \
      audio_options.sample_rate_hint = rate;  \
      cycle_audio ();   \
      update_menus ();  \
      status_text ("Audio sampling rate set to %d Hz.", rate);  \
      return (D_O_K);   \
   }

AUDIO_OUTPUT_MENU_SAMPLING_RATE_HANDLER(22050)
AUDIO_OUTPUT_MENU_SAMPLING_RATE_HANDLER(44100)
AUDIO_OUTPUT_MENU_SAMPLING_RATE_HANDLER(48000)

#undef AUDIO_OUTPUT_MENU_SAMPLING_RATE_HANDLER

static int audio_output_menu_sampling_rate_automatic (void)
{
   audio_options.sample_rate_hint = -1;

   cycle_audio ();
   update_menus ();

   status_text ("Audio sampling rate set to automatic.");

   return (D_O_K);
}

static int audio_output_menu_sampling_rate_custom (void)
{
   int rate;

   rate = audio_options.sample_rate_hint;

   if (get_integer_input ("Custom", &rate, "Hz"))
   {
      audio_options.sample_rate_hint = rate;

      cycle_audio ();
      update_menus ();

      status_text ("Audio sampling rate set to %d Hz.", rate);
   }

   return (D_O_K);
}

static int audio_output_menu_mixing_mono (void)
{
   apu_options.stereo = FALSE;
   
   cycle_audio ();
   update_menus ();

   status_text_color (GUI_TEXT_COLOR, "Audio output set to mono.");

   return (D_O_K);
}

static int audio_output_menu_mixing_stereo (void)
{
   apu_options.stereo = TRUE;
   apu_options.swap_channels = FALSE;

   cycle_audio ();
   update_menus ();

   gui_alert ("Information", "Stereo output may not be supported for all games, such as those with custom sound hardware (e.g MMC5, VRC6, etc.).",
      "In cases where stereo output is not available, FakeNES will temporarily revert to mono output.",
      "This is a harmless, accuracy-related procedure and should not be considered a bug.",
      "&OK", NULL, 0, 0);
      
   status_text ("Audio output set to stereo.");

   return (D_O_K);
}

static int audio_output_menu_mixing_reverse_stereo (void)
{
   apu_options.stereo = TRUE;
   apu_options.swap_channels = TRUE;

   cycle_audio ();
   update_menus ();
   
   status_text ("Audio output set to reverse stereo.");

   return (D_O_K);
}

#define AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(length_ms) \
   static int audio_output_buffer_size_menu_##length_ms##ms (void) \
   { \
      audio_options.buffer_length_ms_hint = length_ms; \
      cycle_audio (); \
      update_menus (); \
      status_text ("Audio buffer size set to %dms.", length_ms); \
      return (D_O_K); \
   }
                        
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(30)
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(50)
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(75)
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(100)
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(125)
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(150)
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(175)
AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER(200)

#undef AUDIO_OUTPUT_BUFFER_SIZE_MENU_HANDLER

static int audio_output_buffer_size_menu_automatic (void)
{
   audio_options.buffer_length_ms_hint = -1;

   cycle_audio ();
   update_menus ();

   status_text ("Audio buffer size set to automatic.");

   return (D_O_K);
}

static int audio_output_buffer_size_menu_custom (void) 
{
   int ms = audio_options.buffer_length_ms_hint;
   if (get_integer_input ("Custom", &ms, "ms"))
   {
      audio_options.buffer_length_ms_hint = ms;

      cycle_audio ();
      update_menus ();

      status_text ("Audio buffer size set to %dms.", ms);
   }

   return (D_O_K);
}

static int audio_menu_volume_increase (void)
{
   apu_options.volume += 0.25;
   if (apu_options.volume > 4.0)
      apu_options.volume = 4.0;

   update_menus ();

   status_text ("Audio master volume level increased to %d%%.",
      (int)ROUND(apu_options.volume * 100.0));

   return (D_O_K);
}

static int audio_menu_volume_decrease (void)
{
   apu_options.volume -= 0.25;
   if (apu_options.volume < 0)
      apu_options.volume = 0;

   update_menus ();

   status_text ("Audio master volume level decreased to %d%%.",
      (int)ROUND(apu_options.volume * 100.0));

   return (D_O_K);
}

static int audio_menu_volume_custom (void)
{
   int percent;

   percent = ROUND(apu_options.volume * 100.0);

   if (get_integer_input ("Custom", &percent, "percent"))
   {
      apu_options.volume = (percent / 100.0);
      update_menus ();
   
      status_text ("Audio master volume level set to %d%%.", percent);
   }

   return (D_O_K);
}

static int audio_menu_volume_reset (void)
{
   apu_options.volume = 1.0;
   update_menus ();

   status_text ("Audio master volume level reset to %d%%.",
      (int)ROUND(apu_options.volume * 100.0));

   return (D_O_K);
}

static int audio_menu_volume_logarithmic (void)
{
   apu_options.logarithmic = !apu_options.logarithmic;
   update_menus ();

   apu_update ();

   status_text ("Audio logarithmic volume mapping %s.",
      get_enabled_text (apu_options.logarithmic)); 

   return (D_O_K);
}

static int audio_menu_volume_auto_gain (void)
{
   apu_options.agc = !apu_options.agc;
   if(apu_options.normalize)
      apu_options.normalize = !apu_options.agc;

   update_menus ();

   apu_update ();

   status_text ("Audio automatic gain control %s.",
      get_enabled_text (apu_options.agc)); 

   return (D_O_K);
}

static int audio_menu_volume_auto_normalize (void)
{
   apu_options.normalize = !apu_options.normalize;
   if(apu_options.agc)
      apu_options.agc = !apu_options.normalize;

   update_menus ();

   apu_update ();

   status_text ("Audio volume level normalization %s.",
      get_enabled_text (apu_options.normalize)); 

   return (D_O_K);
}

static int video_menu_color (void)
{
   DIALOG *dialog;
   DIALOG *objhue, *objsat, *objbright, *objcon, *objgamma;
   int result;

   /* Get dialog. */
   dialog = video_color_dialog;
   
   /* Get slider objects. */
   
   objhue    = &dialog[VIDEO_COLOR_DIALOG_HUE];
   objsat    = &dialog[VIDEO_COLOR_DIALOG_SATURATION];
   objbright = &dialog[VIDEO_COLOR_DIALOG_BRIGHTNESS];
   objcon    = &dialog[VIDEO_COLOR_DIALOG_CONTRAST];
   objgamma  = &dialog[VIDEO_COLOR_DIALOG_GAMMA];
   
   /* Load configuration. */
     
   objhue->d2    = video_get_profile_real(VIDEO_PROFILE_COLOR_HUE)        + 100;
   objsat->d2    = video_get_profile_real(VIDEO_PROFILE_COLOR_SATURATION) + 100;
   objbright->d2 = video_get_profile_real(VIDEO_PROFILE_COLOR_BRIGHTNESS) + 100;
   objcon->d2    = video_get_profile_real(VIDEO_PROFILE_COLOR_CONTRAST)   + 100;
   objgamma->d2  = video_get_profile_real(VIDEO_PROFILE_COLOR_GAMMA)      + 100;
   
   /* Show dialog. */
   result = show_dialog (dialog, -1);

   if (result == VIDEO_COLOR_DIALOG_SAVE_BUTTON)
   {
      /* Save configuration. */
   
      video_set_profile_real(VIDEO_PROFILE_COLOR_HUE,        objhue->d2    - 100);
      video_set_profile_real(VIDEO_PROFILE_COLOR_SATURATION, objsat->d2    - 100);
      video_set_profile_real(VIDEO_PROFILE_COLOR_BRIGHTNESS, objbright->d2 - 100);
      video_set_profile_real(VIDEO_PROFILE_COLOR_CONTRAST,   objcon->d2    - 100);
      video_set_profile_real(VIDEO_PROFILE_COLOR_GAMMA,      objgamma->d2  - 100);
   }
   else if (result == VIDEO_COLOR_DIALOG_RESET_BUTTON)
   {
      /* Save defaults. */
   
      video_set_profile_real(VIDEO_PROFILE_COLOR_HUE,        0);
      video_set_profile_real(VIDEO_PROFILE_COLOR_SATURATION, 0);
      video_set_profile_real(VIDEO_PROFILE_COLOR_BRIGHTNESS, 0);
      video_set_profile_real(VIDEO_PROFILE_COLOR_CONTRAST,   0);
      video_set_profile_real(VIDEO_PROFILE_COLOR_GAMMA,      0);
   }

   video_update_settings();

   return D_O_K;
}

#define DRIVER_MENU_HANDLER(driver, id)   \
   static int video_driver_menu_##driver (void) \
   {  \
      video_set_profile_integer(VIDEO_PROFILE_DISPLAY_DRIVER, id);  \
      video_update_settings(); \
      gui_needs_restart = TRUE; \
      return (D_CLOSE); \
   }

DRIVER_MENU_HANDLER(automatic, GFX_AUTODETECT)
DRIVER_MENU_HANDLER(safe,      GFX_SAFE)

#ifdef USE_ALLEGROGL
DRIVER_MENU_HANDLER(opengl,            GFX_OPENGL)
DRIVER_MENU_HANDLER(opengl_fullscreen, GFX_OPENGL_FULLSCREEN)
DRIVER_MENU_HANDLER(opengl_windowed,   GFX_OPENGL_WINDOWED)
#endif

#ifdef ALLEGRO_DOS
DRIVER_MENU_HANDLER(vesa,          GFX_VESA1)
DRIVER_MENU_HANDLER(vesa_2_banked, GFX_VESA2B)
DRIVER_MENU_HANDLER(vesa_2_linear, GFX_VESA2L)
DRIVER_MENU_HANDLER(vesa_3,        GFX_VESA3)
DRIVER_MENU_HANDLER(vesa_vbe_af,   GFX_VBEAF)
DRIVER_MENU_HANDLER(vga,           GFX_VGA)
DRIVER_MENU_HANDLER(vga_mode_x,    GFX_MODEX)
#endif

#ifdef ALLEGRO_LINUX
DRIVER_MENU_HANDLER(framebuffer, GFX_FBCON)
DRIVER_MENU_HANDLER(svgalib,     GFX_SVGALIB)
DRIVER_MENU_HANDLER(vesa_vbe_af, GFX_VBEAF)
DRIVER_MENU_HANDLER(vga,         GFX_VGA)
DRIVER_MENU_HANDLER(vga_mode_x,  GFX_MODEX)
#endif

#ifdef ALLEGRO_UNIX
DRIVER_MENU_HANDLER(x_windows,            GFX_XWINDOWS)
DRIVER_MENU_HANDLER(x_windows_fullscreen, GFX_XWINDOWS_FULLSCREEN)
DRIVER_MENU_HANDLER(dga,                  GFX_XDGA)
DRIVER_MENU_HANDLER(dga_fullscreen,       GFX_XDGA_FULLSCREEN)
DRIVER_MENU_HANDLER(dga_2,                GFX_XDGA2)
#endif

#ifdef ALLEGRO_WINDOWS
DRIVER_MENU_HANDLER(directx,          GFX_DIRECTX)
DRIVER_MENU_HANDLER(directx_windowed, GFX_DIRECTX_WIN)
DRIVER_MENU_HANDLER(directx_overlay,  GFX_DIRECTX_OVL)
DRIVER_MENU_HANDLER(gdi,              GFX_GDI)
#endif

#undef DRIVER_MENU_HANDLER

#define RESOLUTION_MENU_HANDLER(width, height)  \
   static int video_resolution_menu_##width##_##height (void)   \
   {  \
      video_set_profile_integer(VIDEO_PROFILE_DISPLAY_WIDTH, width);  \
      video_set_profile_integer(VIDEO_PROFILE_DISPLAY_HEIGHT, height);  \
      video_update_settings(); \
      gui_needs_restart = TRUE;  \
      return (D_CLOSE); \
   }

#define RESOLUTION_MENU_HANDLER_EX(type, width, height)  \
   static int video_resolution_##type##_menu_##width##_##height (void)   \
   {  \
      video_set_profile_integer(VIDEO_PROFILE_DISPLAY_WIDTH, width);  \
      video_set_profile_integer(VIDEO_PROFILE_DISPLAY_HEIGHT, height);  \
      video_update_settings(); \
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
RESOLUTION_MENU_HANDLER(400,  300)
RESOLUTION_MENU_HANDLER(640,  480)
RESOLUTION_MENU_HANDLER(800,  600)
RESOLUTION_MENU_HANDLER(1024, 768)
RESOLUTION_MENU_HANDLER(1152, 864)
RESOLUTION_MENU_HANDLER(1280, 960)
RESOLUTION_MENU_HANDLER(1280, 1024)
RESOLUTION_MENU_HANDLER(1600, 1200)

#undef RESOLUTION_MENU_HANDLER
#undef RESOLUTION_MENU_HANDLER_EX

static int video_resolution_menu_custom (void)
{
   int width, height;

   width  = SCREEN_W;
   height = SCREEN_H;

   if (get_resolution_input ("Custom", &width, &height))
   {
      video_set_profile_integer(VIDEO_PROFILE_DISPLAY_WIDTH, width); 
      video_set_profile_integer(VIDEO_PROFILE_DISPLAY_HEIGHT, height); 
      video_update_settings();

      gui_needs_restart = TRUE;
      return (D_CLOSE);
   }
      
   return (D_O_K);
}

static int video_color_depth_menu_paletted_8_bit (void)
{
   video_set_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH, 8);
   video_update_settings();

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_color_depth_menu_true_color_15_bit (void)
{
   video_set_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH, 15);
   video_update_settings();

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_color_depth_menu_true_color_16_bit (void)
{
   video_set_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH, 16);
   video_update_settings();

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_color_depth_menu_true_color_24_bit (void)
{
   video_set_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH, 24);
   video_update_settings();

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

static int video_color_depth_menu_true_color_32_bit (void)
{
   video_set_profile_integer(VIDEO_PROFILE_DISPLAY_COLOR_DEPTH, 32);
   video_update_settings();

   gui_needs_restart = TRUE;
   return (D_CLOSE);
}

#define BLITTER_MENU_HANDLER(name, caption, id) \
   static int video_blitter_menu_##name (void)   \
   {  \
      video_set_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER, id); \
      video_update_settings(); \
      update_menus ();  \
      status_text ("Video blitter set to %s.", caption);  \
      return (D_O_K);   \
   }

BLITTER_MENU_HANDLER(disabled,                "disabled",                   VIDEO_BLITTER_NONE)
BLITTER_MENU_HANDLER(interpolation,           "HQ2X",                       VIDEO_BLITTER_INTERPOLATION)
BLITTER_MENU_HANDLER(interpolation_scanlines, "interpolation w/ scanlines", VIDEO_BLITTER_INTERPOLATION_SCANLINES)
BLITTER_MENU_HANDLER(interpolation_tv_mode,   "interpolated TV mode",       VIDEO_BLITTER_INTERPOLATION_TV_MODE)
BLITTER_MENU_HANDLER(hq2x,                    "HQ2X",                       VIDEO_BLITTER_HQ2X)
BLITTER_MENU_HANDLER(hq3x,                    "HQ3X",                       VIDEO_BLITTER_HQ3X)
BLITTER_MENU_HANDLER(hq4x,                    "HQ4X",                       VIDEO_BLITTER_HQ4X)
BLITTER_MENU_HANDLER(ntsc,                    "NTSC engine",                VIDEO_BLITTER_NTSC)

#undef BLITTER_MENU_HANDLER

static int video_blitter_menu_configure (void)
{
   switch (video_get_profile_enum(VIDEO_PROFILE_OUTPUT_BLITTER))
   {
      case VIDEO_BLITTER_NTSC:
      {
         DIALOG *dialog;
         DIALOG *objhue, *objsat, *objhuew, *objbright, *objcon, *objgamma,
            *objsharp, *objres, *objart, *objbleed, *objfring, *objmerge,
            *objdbl[3], *objinterp;
         int merge_fields, doubling, interpolated;
         int result;
         int index;

         /* Create dialog. */

         dialog = create_dialog (ntsc_config_dialog_base, "ntsc");
         if (!dialog)
            break;

         /* Get slider objects. */

         objhue     = &dialog[NTSC_CONFIG_DIALOG_HUE];
         objhuew    = &dialog[NTSC_CONFIG_DIALOG_HUE_WARPING];
         objsat     = &dialog[NTSC_CONFIG_DIALOG_SATURATION];
         objbright  = &dialog[NTSC_CONFIG_DIALOG_BRIGHTNESS];
         objcon     = &dialog[NTSC_CONFIG_DIALOG_CONTRAST];
         objgamma   = &dialog[NTSC_CONFIG_DIALOG_GAMMA];
         objsharp   = &dialog[NTSC_CONFIG_DIALOG_SHARPNESS];
         objres     = &dialog[NTSC_CONFIG_DIALOG_RESOLUTION];
         objart     = &dialog[NTSC_CONFIG_DIALOG_ARTIFACTS];
         objbleed   = &dialog[NTSC_CONFIG_DIALOG_COLOR_BLEED];
         objfring   = &dialog[NTSC_CONFIG_DIALOG_COLOR_FRINGING];
         objmerge   = &dialog[NTSC_CONFIG_DIALOG_REDUCE_FLICKER];
         objdbl[0]  = &dialog[NTSC_CONFIG_DIALOG_SCANLINE_DOUBLING_NORMAL];
         objdbl[1]  = &dialog[NTSC_CONFIG_DIALOG_SCANLINE_DOUBLING_BRIGHTEN];
         objdbl[2]  = &dialog[NTSC_CONFIG_DIALOG_SCANLINE_DOUBLING_DARKEN];
         objinterp  = &dialog[NTSC_CONFIG_DIALOG_INTERPOLATED];

         /* Load configuration. */

         objhue->d2    = (get_config_int ("video", "ntsc.hue",         0) + 100);
         objhuew->d2   = (get_config_int ("video", "ntsc.hue_warping", 0) + 100);
         objsat->d2    = (get_config_int ("video", "ntsc.saturation",  20) + 100);
         objbright->d2 = (get_config_int ("video", "ntsc.brightness",  10) + 100);
         objcon->d2    = (get_config_int ("video", "ntsc.contrast",    0) + 100);
         objgamma->d2  = (get_config_int ("video", "ntsc.gamma",       0) + 100);
         objsharp->d2  = (get_config_int ("video", "ntsc.sharpness",   40) + 100);
         objres->d2    = (get_config_int ("video", "ntsc.resolution",  30) + 100);
         objart->d2    = (get_config_int ("video", "ntsc.artifacts",   0) + 100);
         objbleed->d2  = (get_config_int ("video", "ntsc.bleed",       0) + 100);
         objfring->d2  = (get_config_int ("video", "ntsc.fringing",    0) + 100);

         merge_fields = get_config_int ("video", "ntsc.merge_fields", 1); /* Default On */
         if (merge_fields)
            objmerge->flags |= D_SELECTED;
         
         doubling = fix (get_config_int ("video", "ntsc.doubling", 2), 0, 2); /* Default Darken */

         objdbl[doubling]->flags |= D_SELECTED;

         interpolated = get_config_int ("video", "ntsc.interpolated", 1);  /* Default On */
         if (interpolated)
            objinterp->flags |= D_SELECTED;

         /* Show dialog. */
         result = show_dialog (dialog, -1);

         /* Destroy dialog. */
         unload_dialog (dialog);

         if (result == NTSC_CONFIG_DIALOG_SAVE_BUTTON)
         {
            /* Save configuration. */

            set_config_int ("video", "ntsc.hue",         (objhue->d2    - 100));
            set_config_int ("video", "ntsc.hue_warping", (objhuew->d2   - 100));
            set_config_int ("video", "ntsc.saturation",  (objsat->d2    - 100));
            set_config_int ("video", "ntsc.brightness",  (objbright->d2 - 100));
            set_config_int ("video", "ntsc.contrast",    (objcon->d2    - 100));
            set_config_int ("video", "ntsc.gamma",       (objgamma->d2  - 100));
            set_config_int ("video", "ntsc.sharpness",   (objsharp->d2  - 100));
            set_config_int ("video", "ntsc.resolution",  (objres->d2    - 100));
            set_config_int ("video", "ntsc.artifacts",   (objart->d2    - 100));
            set_config_int ("video", "ntsc.bleed",       (objbleed->d2  - 100));
            set_config_int ("video", "ntsc.fringing",    (objfring->d2  - 100));

            merge_fields = ((objmerge->flags & D_SELECTED) ? 1 : 0);

            set_config_int ("video", "ntsc.merge_fields", merge_fields);

            for (index = 0; index < 3; index++)
            {
               if (objdbl[index]->flags & D_SELECTED)
                  doubling = index;
            }

            set_config_int ("video", "ntsc.doubling", doubling);

            interpolated = ((objinterp->flags & D_SELECTED) ? 1 : 0);

            set_config_int ("video", "ntsc.interpolated", interpolated);

            /* Reinitialize blitter to the load new configuration. */
            //video_blitter_reinit ();

         }
         else if (result == NTSC_CONFIG_DIALOG_SET_BUTTON)
         {
            DIALOG *objpres[5];
            int preset = -1;

            /* Set a preset. */

            objpres[0] = &dialog[NTSC_CONFIG_DIALOG_PRESETS_DEFAULT];
            objpres[1] = &dialog[NTSC_CONFIG_DIALOG_PRESETS_COMPOSITE];
            objpres[2] = &dialog[NTSC_CONFIG_DIALOG_PRESETS_SVIDEO];
            objpres[3] = &dialog[NTSC_CONFIG_DIALOG_PRESETS_RGB];
            objpres[4] = &dialog[NTSC_CONFIG_DIALOG_PRESETS_MONOCHROME];

            for (index = 0; index < 5; index++)
            {
               if (objpres[index]->flags & D_SELECTED)
                  preset = index;
            }

            set_config_int ("ntsc", "preset", preset);

            /* Reinitialize blitter to the load new configuration. */
            //video_blitter_reinit ();
         }

         break;
      }

      default:
      {
         gui_alert ("Error", "There are no configuration parameters "
            "available for the selected blitter.", NULL, NULL, "&OK", NULL,
               'o', 0);

         break;
      }
   }

   return (D_O_K);
}

static int video_layers_menu_show_back_sprites (void)
{
   const BOOL setting = ppu_get_option(PPU_OPTION_ENABLE_SPRITE_BACK_LAYER);
   ppu_set_option(PPU_OPTION_ENABLE_SPRITE_BACK_LAYER, !setting);
   update_menus ();

   status_text ("Video sprites layer A %s.", get_enabled_text(!setting));

   return (D_O_K);
}

static int video_layers_menu_show_front_sprites (void)
{
   const BOOL setting = ppu_get_option(PPU_OPTION_ENABLE_SPRITE_FRONT_LAYER);
   ppu_set_option(PPU_OPTION_ENABLE_SPRITE_FRONT_LAYER, !setting);
   update_menus ();

   status_text ("Video sprites layer B %s.", get_enabled_text(!setting));

   return (D_O_K);
}

static int video_layers_menu_show_background (void)
{
   const BOOL setting = ppu_get_option(PPU_OPTION_ENABLE_BACKGROUND_LAYER);
   ppu_set_option(PPU_OPTION_ENABLE_BACKGROUND_LAYER, !setting);
   update_menus ();

   status_text ("Video background layer %s.", get_enabled_text(!setting));

   return (D_O_K);
}

#define PALETTE_MENU_HANDLER(name, caption, id) \
   static int video_palette_menu_##name (void)   \
   {  \
      video_set_profile_enum(VIDEO_PROFILE_COLOR_PALETTE, id); \
      video_update_settings(); \
      update_menus(); \
      status_text ("Video palette set to %s.", caption);  \
      return D_O_K; \
   }

PALETTE_MENU_HANDLER(nester,   "NESter palette",   VIDEO_PALETTE_NESTER)
PALETTE_MENU_HANDLER(nesticle, "NESticle palette", VIDEO_PALETTE_NESTICLE)
PALETTE_MENU_HANDLER(ntsc,     "NTSC",             VIDEO_PALETTE_NTSC)
PALETTE_MENU_HANDLER(pal,      "PAL",              VIDEO_PALETTE_PAL)
PALETTE_MENU_HANDLER(rgb,      "RGB",              VIDEO_PALETTE_RGB)

#undef PALETTE_MENU_HANDLER

static int input_menu_configure (void)
{
   BOOL allow_conflicts, toggled_auto, merge_players;
   REAL turbo_rate;
   DIALOG *dialog;
   DIALOG *objconf, *objauto, *objmerge, *objturbo;

   /* Load configuration. */

   allow_conflicts = get_config_int   ("input", "allow_conflicts", FALSE);
   toggled_auto    = get_config_int   ("input", "toggled_auto",    FALSE);
   merge_players   = get_config_int   ("input", "merge_players",   FALSE);
   turbo_rate      = get_config_float ("input", "turbo_rate",      0.5);

   /* Get dialog. */

   dialog = input_configure_dialog;

   /* Get dialog objects. */

   objconf  = &dialog[INPUT_CONFIGURE_DIALOG_ALLOW_CONFLICTS];
   objauto  = &dialog[INPUT_CONFIGURE_DIALOG_TOGGLED_AUTO];
   objmerge = &dialog[INPUT_CONFIGURE_DIALOG_MERGE_PLAYERS];
   objturbo = &dialog[INPUT_CONFIGURE_DIALOG_TURBO];

   /* Set up objects. */

   if (allow_conflicts)
      objconf->flags |= D_SELECTED;

   if (toggled_auto)
      objauto->flags |= D_SELECTED;

   if (merge_players)
      objmerge->flags |= D_SELECTED;

   objturbo->d2 = ROUND((turbo_rate * 100.0));

   /* Show dialog. */   

   if (show_dialog (dialog, -1) ==
      INPUT_CONFIGURE_DIALOG_SAVE_BUTTON)
   {
      /* Save configuration. */

      allow_conflicts = TRUE_OR_FALSE(objconf->flags  & D_SELECTED);
      toggled_auto    = TRUE_OR_FALSE(objauto->flags  & D_SELECTED);
      merge_players   = TRUE_OR_FALSE(objmerge->flags & D_SELECTED);

      turbo_rate = (objturbo->d2 / 100.0);

      /* Save existing configuration so we don't lose it. */
      input_save_config ();

      /* Make any necessary changes. */

      set_config_int   ("input", "allow_conflicts", allow_conflicts);
      set_config_int   ("input", "toggled_auto",    toggled_auto);
      set_config_int   ("input", "merge_players",   merge_players);
      set_config_float ("input", "turbo_rate",      turbo_rate);

      /* Reload configuration with our changes. */
      input_load_config ();
   }

   return (D_O_K);
}

static int input_menu_enable_zapper (void)
{
   input_enable_zapper = !input_enable_zapper;
   update_menus ();

   status_text ("Zapper emulation %s.", get_enabled_text
      (input_enable_zapper));

   return (D_O_K);
}

static int options_cpu_usage_menu_passive (void)
{
    cpu_usage = CPU_USAGE_PASSIVE;
    update_menus ();

    status_text ("System CPU usage set to passive.");

    return (D_O_K);
}

static int options_cpu_usage_menu_normal (void)
{
    cpu_usage = CPU_USAGE_NORMAL;
    update_menus ();

    status_text ("System CPU usage set to normal.");

    return (D_O_K);
}

static int options_cpu_usage_menu_aggressive (void)
{
    cpu_usage = CPU_USAGE_AGGRESSIVE;
    update_menus ();

    status_text ("System CPU usage set to aggressive.");

    return (D_O_K);
}

static int options_menu_paths (void)
{
   USTRING open_path, save_path;
   BOOL locked;
   DIALOG *dialog;
   DIALOG *objopen, *objlock, *objsave;

   /* Load configuration. */

   USTRING_CLEAR(open_path);
   ustrncat (open_path, get_config_string ("gui", "open_path", "/"),
      (sizeof (open_path) - 1));

   USTRING_CLEAR(save_path);
   ustrncat (save_path, get_config_string ("gui", "save_path", "./"),
      (sizeof (save_path) - 1));

   locked = get_config_int ("gui", "lock_paths", FALSE);
   
   /* Get dialog. */
   dialog = options_paths_dialog;

   /* Get dialog objects. */

   objopen = &dialog[OPTIONS_PATHS_DIALOG_OPEN_PATH];
   objlock = &dialog[OPTIONS_PATHS_DIALOG_LOCKED];
   objsave = &dialog[OPTIONS_PATHS_DIALOG_SAVE_PATH];

   /* Set up objects. */

   objopen->dp = open_path;
   objopen->d1 = ((sizeof (open_path) / MAX_UTF8_SEGMENTS) - 1);

   objsave->dp = save_path;
   objsave->d1 = ((sizeof (save_path) / MAX_UTF8_SEGMENTS) - 1);

   if (locked)
      objlock->flags |= D_SELECTED;

   /* Show dialog. */

   if (show_dialog (dialog, -1) == OPTIONS_PATHS_DIALOG_OK_BUTTON)
   {
      /* Save configuration. */

      set_config_string ("gui", "open_path", open_path);
      set_config_string ("gui", "save_path", save_path);

      set_config_int ("gui", "lock_paths",
         TRUE_OR_FALSE(objlock->flags & D_SELECTED));
   }

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
OPTIONS_GUI_THEME_MENU_HANDLER(fireflower)
OPTIONS_GUI_THEME_MENU_HANDLER(xodiac)
OPTIONS_GUI_THEME_MENU_HANDLER(monochrome)
OPTIONS_GUI_THEME_MENU_HANDLER(essence)
OPTIONS_GUI_THEME_MENU_HANDLER(voodoo)
OPTIONS_GUI_THEME_MENU_HANDLER(hugs_and_kisses)

#undef OPTIONS_GUI_THEME_MENU_HANDLER

static int options_menu_reset_clock (void)
{
   machine_reset_game_clock();

   return (D_O_K);
}

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
   ustrncat (nick, get_config_string ("netplay", "nick", "Player"), (sizeof(nick) - 1));

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

   obj_port->d1 = ((sizeof (port_str) / MAX_UTF8_SEGMENTS) - 1);
   obj_port->dp = port_str;

   obj_nick->d1 = ((sizeof (nick) / MAX_UTF8_SEGMENTS) - 1);
   obj_nick->dp = nick;

   /* Display dialog. */
   if (show_dialog (dialog, -1) != NETPLAY_DIALOG_OK_BUTTON)
      return (D_O_K);

   /* Integerize port. */
   port = atoi (port_str);

   /* Save configuration. */
   set_config_int    ("netplay", "port", port);
   set_config_string ("netplay", "nick", nick);

   /* Start NetPlay session. */

   if (!netplay_open_server (port))
   {
      status_text_color (GUI_ERROR_COLOR, "Failed to open server!");
      return (D_O_K);
   }

   status_text ("NetPlay session opened.");

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
   strncat (host, get_config_string ("netplay", "host", ""), (sizeof(host) - 1));

   port = get_config_int ("netplay", "port", NETPLAY_DEFAULT_PORT);

   STRING_CLEAR(port_str);
   sprintf (port_str, "%d", port);

   USTRING_CLEAR(nick);
   ustrncat (nick, get_config_string ("netplay", "nick", "Player"), (sizeof(nick) - 1));

   /* Get dialog. */
   dialog = netplay_dialog;

   /* Get dialog objects. */
   obj_host_label = &dialog[NETPLAY_DIALOG_HOST_LABEL];
   obj_host       = &dialog[NETPLAY_DIALOG_HOST];
   obj_port       = &dialog[NETPLAY_DIALOG_PORT];
   obj_nick       = &dialog[NETPLAY_DIALOG_NICK];

   /* Set up dialog objects. */

   obj_host_label->flags &= ~D_DISABLED;

   obj_host->d1 = ((sizeof (host) / MAX_UTF8_SEGMENTS) - 1);
   obj_host->dp = host;
   obj_host->flags &= ~D_DISABLED;

   obj_port->d1 = ((sizeof (port_str) / MAX_UTF8_SEGMENTS) - 1);
   obj_port->dp = port_str;

   obj_nick->d1 = ((sizeof (nick) / MAX_UTF8_SEGMENTS) - 1);
   obj_nick->dp = nick;

   /* Display dialog. */
   if (show_dialog (dialog, -1) != NETPLAY_DIALOG_OK_BUTTON)
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
      status_text_color (GUI_ERROR_COLOR, "Failed to connect to remote host!");
      return (D_O_K);
   }

   status_text ("NetPlay session opened.");

   /* Set nickname. */
   netplay_set_nickname (nick);

   /* Open lobby. */
   return (open_lobby ());
}

static int help_menu_view_license (void)
{
   DIALOG *dialog;
   DIALOG *objframe;
   DIALOG *objview;

   /* Create dialog. */
   dialog = load_dialog (viewer_dialog_base);
   if (!dialog)
   {
      WARN("Failed to create dialog structure");
      return (-1);
   }

   /* Get objects. */

   objframe = &dialog[VIEWER_DIALOG_FRAME];
   objview  = &dialog[VIEWER_DIALOG_TEXT];

   /* Set up objects. */

   objframe->dp2 = "License";

   objview->dp = get_console_text ();

   /* Show dialog. */
   show_dialog (dialog, -1);

   /* Destroy dialog. */
   unload_dialog (dialog);

   return (D_O_K);
}

static int help_menu_view_log (void)
{
   DIALOG *dialog;
   DIALOG *objframe;
   DIALOG *objview;

   /* Create dialog. */
   dialog = load_dialog (viewer_dialog_base);
   if (!dialog)
   {
      WARN("Failed to create dialog structure");
      return (-1);
   }

   /* Get objects. */

   objframe = &dialog[VIEWER_DIALOG_FRAME];
   objview  = &dialog[VIEWER_DIALOG_TEXT];

   /* Set up objects. */

   objframe->dp2 = "Log";

   objview->dp = get_log_text ();

   /* Show dialog. */
   show_dialog (dialog, -1);

   /* Destroy dialog. */
   unload_dialog (dialog);

   return (D_O_K);
}

static int help_menu_keyboard_shortcuts (void)
{
   show_dialog (help_keyboard_shortcuts_dialog, -1);

   return (D_O_K);
}

static int help_menu_about (void)
{
   gui_alert ("About", "FakeNES version " VERSION_STRING " "
      ALLEGRO_PLATFORM_STR, "Get the latest from "
         "http://fakenes.sourceforge.net/.", NULL, "&OK", NULL, 'o', 0);

   return (D_O_K);
}

static int help_menu_fakenes_team (void)
{
   show_dialog (help_fakenes_team_dialog, -1);

   return (D_O_K);
}


/* ---- Dialog handlers. ---- */


static int main_cheat_manager_dialog_list (DIALOG *dialog)
{
   CPU_PATCH *patch;
   DIALOG *main_dialog;
   DIALOG *obj_enabled;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
      return (D_O_K);

   patch = &cpu_patch_info[dialog->d1];

   /* Get main dialog. */
   main_dialog = main_cheat_manager_dialog;

   /* Get "Enabled" checkbox. */
   obj_enabled = &main_dialog[MAIN_CHEAT_MANAGER_DIALOG_ENABLED_CHECKBOX];

   if (patch->enabled)
      obj_enabled->flags |= D_SELECTED;
   else
      obj_enabled->flags &= ~D_SELECTED;

   scare_mouse ();
   object_message (obj_enabled, MSG_DRAW, 0);
   unscare_mouse ();

   return (D_O_K);
}

static int main_cheat_manager_dialog_add (DIALOG *dialog)
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
      gui_alert ("Error", "The patch list is already full.", NULL, NULL,
         "&OK", NULL, 'o', 0);

      return (D_O_K);
   }

   /* Get dialog. */
   main_dialog = main_cheat_manager_add_dialog;

   /* Get dialog objects. */
   obj_title = &main_dialog[MAIN_CHEAT_MANAGER_ADD_DIALOG_TITLE];
   obj_code  = &main_dialog[MAIN_CHEAT_MANAGER_ADD_DIALOG_CODE];

   /* Set up dialog objects. */

   USTRING_CLEAR(title);
   obj_title->d1 = NEW_SAVE_TITLE_SIZE;
   obj_title->dp = title;

   USTRING_CLEAR(code);
   obj_code->d1 = (11 - 1);
   obj_code->dp = code;

   /* Show dialog. */
   if (show_dialog (main_dialog, -1) !=
      MAIN_CHEAT_MANAGER_ADD_DIALOG_OK_BUTTON)
   {
      return (D_O_K);
   }

   patch = &cpu_patch_info[cpu_patch_count];

   if (cheats_decode (code, &patch->address, &patch->value,
      &patch->match_value) != 0)
   {
      gui_alert ("Error", "You must enter a valid Game Genie (or NESticle "
         "raw) code.", NULL, NULL, "&OK", NULL, 'o', 0);

      return (D_O_K);
   }

   /* Copy title. */
   ustrncat (patch->title, title, (USTRING_SIZE - 1));

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

static int main_cheat_manager_dialog_remove (DIALOG *dialog)
{
   DIALOG *main_dialog;
   int start;
   CPU_PATCH *src;
   int index;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
      return (D_O_K);

   main_dialog = main_cheat_manager_dialog;

   start = main_dialog[MAIN_CHEAT_MANAGER_DIALOG_LIST].d1;
   src = &cpu_patch_info[start];

   /* Disable patch. */
   if (src->active)
   {
      if (gui_alert ("Confirmation", "Really deactivate and remove this "
         "patch?", NULL, NULL, "&OK", "&Cancel", 'o', 'c') == 2)
      {
         return (D_O_K);
      }

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
      main_dialog[MAIN_CHEAT_MANAGER_DIALOG_ENABLED_CHECKBOX].flags &=
         ~D_SELECTED;
   }

   return (D_REDRAW);
}

static int main_cheat_manager_dialog_enabled (DIALOG *dialog)
{
   DIALOG *obj_list;
   CPU_PATCH *patch;

   RT_ASSERT(dialog);

   if (cpu_patch_count == 0)
   {
      dialog->flags &= ~D_SELECTED;

      return (D_O_K);
   }

   obj_list = &main_cheat_manager_dialog[MAIN_CHEAT_MANAGER_DIALOG_LIST];

   patch = &cpu_patch_info[obj_list->d1];

   patch->enabled = TRUE_OR_FALSE(dialog->flags & D_SELECTED);

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

static USTRING main_cheat_manager_dialog_list_texts[CPU_MAX_PATCHES];

static char *main_cheat_manager_dialog_list_filler (int index, int *list_size)
{
   if (index >= 0)
   {
      CPU_PATCH *patch = &cpu_patch_info[index];
      UDATA *text = main_cheat_manager_dialog_list_texts[index];

      USTRING_CLEAR(text);
      uszprintf (text, USTRING_SIZE, "$%04x -$%02x +$%02x %s ",
         patch->address, patch->match_value, patch->value, (patch->active ?
            "Active" : " Idle "));

      /* Copy title. */
      ustrncat (text, patch->title, (USTRING_SIZE - 1));
      
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
static int selected_player_device = INPUT_DEVICE_NONE;

static int input_configure_dialog_player_select (DIALOG *dialog)
{
   DIALOG *main_dialog;
   int first, last;
   int index;

   RT_ASSERT(dialog);

   selected_player = dialog->d2;
   selected_player_device = input_get_player_device (selected_player);

   main_dialog = input_configure_dialog;

   first = INPUT_CONFIGURE_DIALOG_DEVICE_0_SELECT;
   last  = INPUT_CONFIGURE_DIALOG_DEVICE_7_SELECT;

   for (index = first; index <= last; index++)
      main_dialog[index].flags &= ~D_SELECTED;

   main_dialog[(first + selected_player_device)].flags |= D_SELECTED;

   scare_mouse ();

   for (index = first; index <= last; index++)
      object_message (&main_dialog[index],  MSG_DRAW, 0);

   first = INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_1;
   last  = INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_8;

   for (index = first; index <= last; index++)
   {
      DIALOG *dialog = &main_dialog[index];

      /* d2 = button. */

      if (input_get_player_button_param (selected_player, dialog->d2,
         INPUT_PLAYER_BUTTON_PARAM_AUTO))
      {
         dialog->flags |= D_SELECTED;
      }
      else
      {
         dialog->flags &= ~D_SELECTED;
      }

      object_message (dialog, MSG_DRAW, 0);
   }

   first = INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_1;
   last  = INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_8;

   for (index = first; index <= last; index++)
   {
      DIALOG *dialog = &main_dialog[index];

      /* d2 = button. */

      if (input_get_player_button_param (selected_player, dialog->d2,
         INPUT_PLAYER_BUTTON_PARAM_TURBO))
      {
         dialog->flags |= D_SELECTED;
      }
      else
      {
         dialog->flags &= ~D_SELECTED;
      }

      object_message (dialog, MSG_DRAW, 0);
   }

   unscare_mouse ();

   return (D_O_K);
}

static int input_configure_dialog_device_select (DIALOG *dialog)
{
   RT_ASSERT(dialog);

   if (selected_player < 0)
   {
      gui_alert ("Error", "Please select a player to modify first.", NULL,
         NULL, "&OK", NULL, 'o', 0);

      return (D_O_K);
   }

   selected_player_device = dialog->d2;

   input_set_player_device (selected_player, selected_player_device);

   return (D_O_K);
}

static int input_configure_dialog_set_buttons (DIALOG *dialog)
{
   int button;

   RT_ASSERT(dialog);

   if (selected_player < 0)
   {
      gui_alert ("Error", "Please select a player to modify first.", NULL,
         NULL, "&OK", NULL, 'o', 0);

      if (dialog->proc == sl_checkbox)
         dialog->flags ^= D_SELECTED;

      return (D_O_K);
   }

   button = dialog->d2;

   switch (dialog->d1)
   {
      case 0:  /* Map button. */
      {
         if (selected_player_device == INPUT_DEVICE_NONE)
         {
            gui_alert ("Error", "The selected player is currently "
               "disabled.", NULL, NULL, "&OK", NULL, 'o', 0);
      
            return (D_O_K);
         }
      
         if (selected_player_device == INPUT_DEVICE_MOUSE)
         {
            gui_alert ("Error", "Unable to set buttons for mouse at this "
               "time.", NULL, NULL, "&OK", NULL, 'o', 0);
      
            return (D_O_K);
         }
      
         status_text ("Scanning for device changes, press ESC to cancel.");
          
         input_map_player_button (selected_player, button);

         break;
      }

      case 1:  /* Set auto. */
      {
         input_set_player_button_param (selected_player, button,
            INPUT_PLAYER_BUTTON_PARAM_AUTO, (dialog->flags & D_SELECTED));

         break;
      }

      case 2:  /* Set turbo. */
      {
         input_set_player_button_param (selected_player, button,
            INPUT_PLAYER_BUTTON_PARAM_TURBO, (dialog->flags & D_SELECTED));
            
         break;
      }

      default:
         WARN_GENERIC();
   }

   return (D_O_K);
}

static int input_configure_dialog_calibrate (DIALOG *dialog)
{
   RT_ASSERT(dialog);

   if (selected_player < 0)
   {
      gui_alert ("Error", "Please select a player to modify first.", NULL,
         NULL, "&OK", NULL, 'o', 0);

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
            int scancode;

            status_text ("%s, and press any key.",
               calibrate_joystick_name (index));

            while (!keypressed ()) {
               if(keyboard_needs_poll())
                  poll_keyboard();

               gui_heartbeat ();
            }

            ureadkey (&scancode);

            if (scancode == KEY_ESC)
            {
               status_text_color (GUI_ERROR_COLOR, "Joystick calibration "
                  "cancelled.");
    
               return (D_O_K);
            }

            if (calibrate_joystick (index) != 0)
            {
               gui_alert ("Error", "An unknown error occured while "
                  "attempting to calibrate the device.", NULL, NULL, "&OK",
                     NULL, 'o', 0);
   
               return (D_O_K);
            }
         }

         gui_alert ("Calibration Complete", "The selected device has been "
            "calibrated.", NULL, NULL, "&Save", NULL, 's', 0);

         save_joystick_data (NULL);

         break;
      }

      default:
      {
         gui_alert ("Error", "The selected device does not require "
            "calibration.", NULL, NULL, "&OK", NULL, 'o', 0);

         break;
      }
   }

   return (D_O_K);
}

static int lobby_dialog_load (void)
{
   /* Note: D_CLOSE means success, D_O_K means failure. */
   if (main_menu_open () == D_CLOSE)
   {
      DIALOG *obj_ok;

      obj_ok = &lobby_dialog[LOBBY_DIALOG_OK_BUTTON];

      obj_ok->flags &= ~D_DISABLED;

      scare_mouse ();
      object_message (obj_ok, MSG_DRAW, 0);
      unscare_mouse ();
   }

   return (D_O_K);
}
