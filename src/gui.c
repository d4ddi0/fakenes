

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

gui.c: Implementation of the object-based GUI.

Copyright (c) 2005, Randy McDowell.
Copyright (c) 2005, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <stdio.h>

#include <stdlib.h>

#include <string.h>


#include "apu.h"

#include "audio.h"

#include "cpu.h"

#include "gui.h"

#include "input.h"

#include "mmc.h"

#include "papu.h"

#include "ppu.h"

#include "rom.h"

#include "video.h"


#include "data.h"

#include "misc.h"

#include "version.h"


#include "genie.h"

#include "netplay.h"


#include "timing.h"


static int dialog_x = 0;

static int dialog_y = 0;


static int restart_dialog = FALSE;


GUI_THEME gui_theme;

int gui_theme_id = 0;


GUI_THEME * last_theme = NIL;


RGB * gui_image_palette = NIL;


static BITMAP * gui_mouse_sprite = NIL;


static BITMAP * background_image = NIL;


/* Keep these in order! */

#include "gui/themes.h"

#include "gui/objects.h"

#include "gui/menus.h"

#include "gui/dialogs.h"


int gui_needs_restart = FALSE;

int gui_is_active = FALSE;


static int want_exit = FALSE;


static UINT8 message_buffer [256];


static RGB * current_palette = NIL;

static PALETTE custom_palette;


static MENU * load_menu (const MENU * menu)
{
    MENU * new_menu;


    int size = 0;


    int index = 0;


    while (menu [index].text || menu [index].proc)
    {
        size += sizeof (MENU);


        index ++;
    }


    /* Once more for the end marker. */

    size += sizeof (MENU);


    new_menu = malloc (size);

    ASSERT (new_menu);

    if (! new_menu)
    {
        return (NIL);
    }


    memcpy (new_menu, menu, size);


    index = 0;


    while (new_menu [index].text || menu [index].proc)
    {
        /* R3m3mb3r k1ds: Ug1y h4cks 4r3 n0t t3h l33t! */

        if (new_menu [index].child)
        {
            new_menu [index].child = * (MENU * *) new_menu [index].child;
        }


        index ++;
    }


    return (new_menu);
}


#define ROUND(value)    (value + 0.5)


static DIALOG * load_dialog (const DIALOG * dialog)
{
    DIALOG * new_dialog;


    int size = 0;


    int index = 0;


    int width;

    int height;


    while (dialog [index].proc)
    {
        size += sizeof (DIALOG);


        index ++;
    }


    /* Once more for the end marker. */

    size += sizeof (DIALOG);


    new_dialog = malloc (size);

    ASSERT (new_dialog);

    if (! new_dialog)
    {
        return (NIL);
    }


    memcpy (new_dialog, dialog, size);


    /* Font scaling. */

    width = text_length (font, "x");

    height = text_height (font);


    index = 0;


    while (new_dialog [index].proc)
    {
        if (new_dialog [index].proc == d_menu_proc)
        {
            new_dialog [index].dp = * (MENU * *) new_dialog [index].dp;
        }


        if (font != small_font)
        {
            switch (index)
            {
                case 0: /* sl_frame. */
    
                    new_dialog [index].w = ROUND (((new_dialog [index].w / 5.0) * width));
    
                    new_dialog [index].h = (ROUND (((new_dialog [index].h / 6.0) * height)) - height);
    
    
                    break;
    
    
                case 1: /* sl_x_button. */
    
                    new_dialog [index].x = ROUND (((new_dialog [index].x / 5.0) * width));
    
    
                    break;
    
    
                default:
    
                    new_dialog [index].x = ROUND (((new_dialog [index].x / 5.0) * width));
            
                    new_dialog [index].y = (ROUND (((new_dialog [index].y / 6.0) * height)) - height);
            
                    new_dialog [index].w = ROUND (((new_dialog [index].w / 5.0) * width));
            
                    new_dialog [index].h = ROUND (((new_dialog [index].h / 6.0) * height));
    
    
                    break;
            }
        }


        index ++;
    }


    return (new_dialog);
}


static void unload_menu (MENU * menu)
{
    if (menu)
    {
        free (menu);
    }
}


static void unload_dialog (DIALOG * dialog)
{
    if (dialog)
    {
        free (dialog);
    }
}


#define MENU_FROM_BASE(name)    (name = load_menu (name ##_base))

#define DIALOG_FROM_BASE(name)  (name = load_dialog (name ##_base))


static void load_menus (void)
{
    MENU_FROM_BASE (main_state_select_menu);

    MENU_FROM_BASE (main_state_autosave_menu);

    MENU_FROM_BASE (main_state_menu);

    MENU_FROM_BASE (main_replay_select_menu);

    MENU_FROM_BASE (main_replay_record_menu);

    MENU_FROM_BASE (main_replay_play_menu);

    MENU_FROM_BASE (main_replay_menu);

    MENU_FROM_BASE (main_menu);


    MENU_FROM_BASE (netplay_protocol_menu);

    MENU_FROM_BASE (netplay_server_menu);

    MENU_FROM_BASE (netplay_client_menu);

    MENU_FROM_BASE (netplay_menu);


    MENU_FROM_BASE (options_gui_theme_menu);

    MENU_FROM_BASE (options_gui_menu);

    MENU_FROM_BASE (options_system_menu);

    MENU_FROM_BASE (options_audio_mixing_channels_menu);

    MENU_FROM_BASE (options_audio_mixing_frequency_menu);

    MENU_FROM_BASE (options_audio_mixing_quality_menu);

    MENU_FROM_BASE (options_audio_mixing_anti_aliasing_menu);

    MENU_FROM_BASE (options_audio_mixing_menu);

    MENU_FROM_BASE (options_audio_effects_menu);

    MENU_FROM_BASE (options_audio_filters_menu);

    MENU_FROM_BASE (options_audio_channels_menu);

    MENU_FROM_BASE (options_audio_advanced_menu);

    MENU_FROM_BASE (options_audio_record_menu);

    MENU_FROM_BASE (options_audio_menu);

    MENU_FROM_BASE (options_video_driver_dos_menu);

    MENU_FROM_BASE (options_video_driver_windows_menu);

    MENU_FROM_BASE (options_video_driver_linux_menu);

    MENU_FROM_BASE (options_video_driver_unix_menu);

    MENU_FROM_BASE (options_video_driver_menu);

    MENU_FROM_BASE (options_video_resolution_proportionate_menu);

    MENU_FROM_BASE (options_video_resolution_extended_menu);

    MENU_FROM_BASE (options_video_resolution_menu);

    MENU_FROM_BASE (options_video_colors_menu);

    MENU_FROM_BASE (options_video_blitter_menu);

    MENU_FROM_BASE (options_video_filters_menu);

    MENU_FROM_BASE (options_video_layers_menu);

    MENU_FROM_BASE (options_video_palette_menu);

    MENU_FROM_BASE (options_video_advanced_menu);

    MENU_FROM_BASE (options_video_menu);

    MENU_FROM_BASE (options_menu);


    MENU_FROM_BASE (help_menu);


    MENU_FROM_BASE (top_menu);
}


static void load_dialogs (void)
{
    DIALOG_FROM_BASE (main_dialog);


    DIALOG_FROM_BASE (main_state_save_dialog);

    DIALOG_FROM_BASE (main_replay_record_start_dialog);

    DIALOG_FROM_BASE (main_messages_dialog);


    DIALOG_FROM_BASE (options_input_dialog);

    DIALOG_FROM_BASE (options_patches_add_dialog);

    DIALOG_FROM_BASE (options_patches_dialog);


    DIALOG_FROM_BASE (netplay_client_connect_dialog);


    DIALOG_FROM_BASE (help_shortcuts_dialog);

    DIALOG_FROM_BASE (help_about_dialog);
}


static void unload_menus (void)
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

    unload_menu (options_menu);


    unload_menu (help_menu);


    unload_menu (top_menu);
}


static void unload_dialogs (void)
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


static INLINE void pack_color (GUI_COLOR * color)
{
    int red;

    int green;

    int blue;


    red = (color -> red * 255);

    green = (color -> green * 255);

    blue = (color -> blue * 255);


    color -> packed = video_create_color (red, green, blue);
}


void gui_set_theme (GUI_THEME * theme)
{
    int index;


    last_theme = theme;


    memcpy (&gui_theme, theme, sizeof (GUI_THEME));


    video_set_palette (NIL);


    for (index = 0; index < GUI_TOTAL_COLORS; index ++)
    {
        pack_color (&gui_theme [index]);
    }


    gui_bg_color = GUI_FILL_COLOR;

    gui_fg_color = GUI_TEXT_COLOR;


    gui_mg_color = GUI_DISABLED_COLOR;
}


static void update_colors (void)
{
    if (last_theme)
    {
        gui_set_theme (last_theme);
    }


    broadcast_dialog_message (MSG_DRAW, NIL);
}


static void gui_message_border (void)
{
    /* No longer a bevel, but... */

    int x;

    int y;


    int x2;

    int y2;


    x = 16;

    y = (((SCREEN_H - 16) - text_height (font)) - 8);


    x2 = (SCREEN_W - 16);

    y2 = (SCREEN_H - 16);


    vline (screen, (x2 + 1), (y + 1), (y2 + 1), GUI_SHADOW_COLOR);

    hline (screen, (x + 1), (y2 + 1), (x2 + 1), GUI_SHADOW_COLOR);


    rectfill (screen, x, y, x2, y2, GUI_FILL_COLOR);


    rect (screen, x, y, x2, y2, GUI_BORDER_COLOR);
}


void gui_message (int color, AL_CONST UINT8 * message, ...)
{
    va_list format;


    va_start (format, message);

    vsprintf (message_buffer, message, format);

    va_end (format);


    if (gui_is_active)
    {
        gui_message_border ();
    
    
        textout_centre_ex (screen, font, message_buffer, (SCREEN_W / 2), ((SCREEN_H - 19) - text_height (font)), 0, -1);
    
        textout_centre_ex (screen, font, message_buffer, ((SCREEN_W / 2) - 1), (((SCREEN_H - 19) - text_height (font)) - 1), color, -1);


        if (log_file)
        {
            fprintf (log_file, "GUI: %s\n", message_buffer);
        }
    }
    else
    {
        video_message (message_buffer);


        video_message_duration = 3000;
    }
}


#define CHECK_MENU(menu, item)   \
    (menu [item].flags |= D_SELECTED)

#define UNCHECK_MENU(menu, item) \
    (menu [item].flags &= ~D_SELECTED)


#define ENABLE_MENU(menu, item)  \
    (menu [item].flags &= ~D_DISABLED)

#define DISABLE_MENU(menu, item) \
    (menu [item].flags |= D_DISABLED)


#define TOGGLE_MENU(menu, item, condition) \
    {                                      \
        if (condition)                     \
            CHECK_MENU (menu, item);       \
        else                               \
            UNCHECK_MENU (menu, item);     \
    }



/* For save states... */

static int machine_state_index = 0;


/* For replays... */

static int replay_index = 0;


void gui_stop_replay (void)
{
    main_replay_play_menu_stop ();
}


void gui_handle_keypress (int index)
{
    switch ((index >> 8))
    {
        case KEY_F1:

            main_menu_snapshot ();


            break;


        case KEY_F2:

            options_menu_status ();


            break;


        case KEY_F3:

            main_state_menu_save ();


            break;


        case KEY_F4:

            if (! (input_mode & INPUT_MODE_REPLAY))
            {
                main_state_menu_restore ();
            }

    
            break;


        case KEY_F7:

            options_video_layers_menu_sprites_a ();

            options_video_layers_menu_sprites_b ();


            break;


        case KEY_F8:

            options_video_layers_menu_background ();


            break;


        case KEY_F9:

            timing_half_speed = (! timing_half_speed);


            if (! gui_is_active)
            {
                suspend_timing ();


                resume_timing ();
            }


            audio_exit ();

            audio_init ();


            papu_reinit ();


            break;


        case KEY_F12:

            if (! (input_mode & INPUT_MODE_REPLAY_PLAY))
            {
                if (input_mode & INPUT_MODE_REPLAY_RECORD)
                {
                    main_replay_record_menu_stop ();
                }
                else
                {
                    main_replay_record_menu_start ();
                }
            }


            break;


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

            if (! (input_mode & INPUT_MODE_CHAT))
            {
                machine_state_index = ((index >> 8) - KEY_0);
    
    
                gui_message (GUI_TEXT_COLOR, "Machine state slot set to %d.", machine_state_index);
            }


            break;


        default:

            break;
    }
}


int gui_show_dialog (DIALOG * dialog)
{
    BITMAP * saved;


    int position;


    UINT16 x = 0;

    UINT16 y = 0;


    int moved = FALSE;


    int index = 0;


    saved = create_bitmap (SCREEN_W, SCREEN_H);


    scare_mouse ();

    blit (screen, saved, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    unscare_mouse ();


    position = get_config_hex ("dialogs", dialog [0].dp2, -1);


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


    dialog [0].dp3 = DATA_TO_FONT (LARGE_FONT);


    while (dialog [index].d1 != SL_FRAME_END)
    {
        if ((dialog [index].proc == sl_text) || (dialog [index].proc == sl_frame))
        {
            dialog [index].dp = screen;
        }


        dialog [index].fg = GUI_TEXT_COLOR;

        dialog [index].bg = gui_bg_color;


        index ++;
    }


  again:

    index = do_dialog (dialog, -1);


    scare_mouse ();

    blit (saved, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    unscare_mouse ();


    if (restart_dialog)
    {
        restart_dialog = FALSE;


        x = dialog_x;

        y = dialog_y;


        position_dialog (dialog, x, y);


        moved = TRUE;


        goto again;
    }


    if (moved)
    {
        set_config_hex ("dialogs", dialog [0].dp2, ((x << 16) | y));
    }


    destroy_bitmap (saved);


    return (index);
}


static INLINE void update_menus (void)
{
    if (! audio_pseudo_stereo)
    {
        papu_swap_channels = FALSE;


        papu_spatial_stereo = FALSE;
        

        DISABLE_MENU (options_audio_mixing_channels_menu, 10);


        DISABLE_MENU (options_audio_effects_menu, 2);

        DISABLE_MENU (options_audio_effects_menu, 4);

        DISABLE_MENU (options_audio_effects_menu, 6);
    }
    else
    {
        ENABLE_MENU (options_audio_mixing_channels_menu, 10);


        ENABLE_MENU (options_audio_effects_menu, 2);

        ENABLE_MENU (options_audio_effects_menu, 4);

        ENABLE_MENU (options_audio_effects_menu, 6);
    }


    if (audio_sample_size != 8)
    {
        papu_dithering = FALSE;


        DISABLE_MENU (options_audio_mixing_quality_menu, 4);
    }
    else
    {
        ENABLE_MENU (options_audio_mixing_quality_menu, 4);
    }


    TOGGLE_MENU (main_state_select_menu, 0, (machine_state_index == 0));

    TOGGLE_MENU (main_state_select_menu, 2, (machine_state_index == 1));

    TOGGLE_MENU (main_state_select_menu, 4, (machine_state_index == 2));

    TOGGLE_MENU (main_state_select_menu, 6, (machine_state_index == 3));

    TOGGLE_MENU (main_state_select_menu, 8, (machine_state_index == 4));

    TOGGLE_MENU (main_state_select_menu, 10, (machine_state_index == 5));

    TOGGLE_MENU (main_state_select_menu, 12, (machine_state_index == 6));

    TOGGLE_MENU (main_state_select_menu, 14, (machine_state_index == 7));

    TOGGLE_MENU (main_state_select_menu, 16, (machine_state_index == 8));

    TOGGLE_MENU (main_state_select_menu, 18, (machine_state_index == 9));


    TOGGLE_MENU (main_state_autosave_menu, 0, (input_autosave_interval == 0));

    TOGGLE_MENU (main_state_autosave_menu, 2, (input_autosave_interval == 10));

    TOGGLE_MENU (main_state_autosave_menu, 4, (input_autosave_interval == 30));

    TOGGLE_MENU (main_state_autosave_menu, 6, (input_autosave_interval == 60));


    TOGGLE_MENU (main_replay_select_menu, 0, (replay_index == 0));

    TOGGLE_MENU (main_replay_select_menu, 2, (replay_index == 1));

    TOGGLE_MENU (main_replay_select_menu, 4, (replay_index == 2));

    TOGGLE_MENU (main_replay_select_menu, 6, (replay_index == 3));

    TOGGLE_MENU (main_replay_select_menu, 8, (replay_index == 4));


    TOGGLE_MENU (options_menu, 0, video_display_status);


    TOGGLE_MENU (options_gui_theme_menu, 0, (last_theme == &classic_theme));

    TOGGLE_MENU (options_gui_theme_menu, 2, (last_theme == &stainless_steel_theme));

    TOGGLE_MENU (options_gui_theme_menu, 4, (last_theme == &zero_4_theme));

    TOGGLE_MENU (options_gui_theme_menu, 6, (last_theme == &panta_theme));


    TOGGLE_MENU (options_system_menu, 0, (machine_type == MACHINE_TYPE_NTSC));

    TOGGLE_MENU (options_system_menu, 2, (machine_type == MACHINE_TYPE_PAL));


    TOGGLE_MENU (options_audio_menu, 0, audio_enable_output);


    TOGGLE_MENU (options_audio_mixing_frequency_menu, 0, (audio_sample_rate == 8000));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 2, (audio_sample_rate == 11025));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 4, (audio_sample_rate == 16000));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 6, (audio_sample_rate == 22050));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 8, (audio_sample_rate == 32000));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 10, (audio_sample_rate == 44100));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 12, (audio_sample_rate == 48000));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 14, (audio_sample_rate == 80200));

    TOGGLE_MENU (options_audio_mixing_frequency_menu, 16, (audio_sample_rate == 96000));


    TOGGLE_MENU (options_audio_mixing_channels_menu, 0, (! audio_pseudo_stereo));

    TOGGLE_MENU (options_audio_mixing_channels_menu, 2, (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_1));

    TOGGLE_MENU (options_audio_mixing_channels_menu, 4, (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_2));

    TOGGLE_MENU (options_audio_mixing_channels_menu, 6, (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_3));

    TOGGLE_MENU (options_audio_mixing_channels_menu, 8, (audio_pseudo_stereo == AUDIO_PSEUDO_STEREO_MODE_4));

    TOGGLE_MENU (options_audio_mixing_channels_menu, 10, papu_swap_channels);


    TOGGLE_MENU (options_audio_mixing_quality_menu, 0, (audio_sample_size == 8));

    TOGGLE_MENU (options_audio_mixing_quality_menu, 2, (audio_sample_size == 16));

    TOGGLE_MENU (options_audio_mixing_quality_menu, 4, papu_dithering);


    TOGGLE_MENU (options_audio_mixing_anti_aliasing_menu, 0, (papu_interpolate == 0));

    TOGGLE_MENU (options_audio_mixing_anti_aliasing_menu, 2, (papu_interpolate == 1));

    TOGGLE_MENU (options_audio_mixing_anti_aliasing_menu, 4, (papu_interpolate == 2));

    TOGGLE_MENU (options_audio_mixing_anti_aliasing_menu, 6, (papu_interpolate == 3));

    TOGGLE_MENU (options_audio_mixing_anti_aliasing_menu, 8, (papu_interpolate == 4));


    TOGGLE_MENU (options_audio_effects_menu, 0, papu_linear_echo);

    TOGGLE_MENU (options_audio_effects_menu, 2, (papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_1));

    TOGGLE_MENU (options_audio_effects_menu, 4, (papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_2));

    TOGGLE_MENU (options_audio_effects_menu, 6, (papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_3));


    TOGGLE_MENU (options_audio_filters_menu, 0, (papu_get_filter_list () & PAPU_FILTER_LOW_PASS_MODE_1));

    TOGGLE_MENU (options_audio_filters_menu, 2, (papu_get_filter_list () & PAPU_FILTER_LOW_PASS_MODE_2));

    TOGGLE_MENU (options_audio_filters_menu, 4, (papu_get_filter_list () & PAPU_FILTER_LOW_PASS_MODE_3));

    TOGGLE_MENU (options_audio_filters_menu, 6, (papu_get_filter_list () & PAPU_FILTER_HIGH_PASS));


    TOGGLE_MENU (options_audio_channels_menu, 0, papu_enable_square_1);

    TOGGLE_MENU (options_audio_channels_menu, 2, papu_enable_square_2);

    TOGGLE_MENU (options_audio_channels_menu, 4, papu_enable_triangle);

    TOGGLE_MENU (options_audio_channels_menu, 6, papu_enable_noise);

    TOGGLE_MENU (options_audio_channels_menu, 8, papu_enable_dmc);

    TOGGLE_MENU (options_audio_channels_menu, 10, papu_enable_exsound);


    TOGGLE_MENU (options_audio_advanced_menu, 0, papu_ideal_triangle);

    TOGGLE_MENU (options_audio_advanced_menu, 2, audio_hard_sync);


#ifdef ALLEGRO_DOS

    TOGGLE_MENU (options_video_driver_dos_menu, 0, (gfx_driver -> id == GFX_VGA));

    TOGGLE_MENU (options_video_driver_dos_menu, 2, (gfx_driver -> id == GFX_MODEX));

    TOGGLE_MENU (options_video_driver_dos_menu, 4, (gfx_driver -> id == GFX_VESA1));

    TOGGLE_MENU (options_video_driver_dos_menu, 6, (gfx_driver -> id == GFX_VESA2B));

    TOGGLE_MENU (options_video_driver_dos_menu, 8, (gfx_driver -> id == GFX_VESA2L));

    TOGGLE_MENU (options_video_driver_dos_menu, 10, (gfx_driver -> id == GFX_VESA3));

    TOGGLE_MENU (options_video_driver_dos_menu, 12, (gfx_driver -> id == GFX_VBEAF));

#endif


#ifdef ALLEGRO_WINDOWS

    TOGGLE_MENU (options_video_driver_windows_menu, 0, (gfx_driver -> id == GFX_DIRECTX));

    TOGGLE_MENU (options_video_driver_windows_menu, 2, (gfx_driver -> id == GFX_DIRECTX_WIN));

    TOGGLE_MENU (options_video_driver_windows_menu, 4, (gfx_driver -> id == GFX_DIRECTX_OVL));

    TOGGLE_MENU (options_video_driver_windows_menu, 6, (gfx_driver -> id == GFX_GDI));

#endif


#ifdef ALLEGRO_LINUX

    TOGGLE_MENU (options_video_driver_linux_menu, 0, (gfx_driver -> id == GFX_VGA));

    TOGGLE_MENU (options_video_driver_linux_menu, 2, (gfx_driver -> id == GFX_MODEX));

    TOGGLE_MENU (options_video_driver_linux_menu, 4, (gfx_driver -> id == GFX_VBEAF));

#ifdef GFX_FBCON

    TOGGLE_MENU (options_video_driver_linux_menu, 6, (gfx_driver -> id == GFX_FBCON));

#endif

#ifdef GFX_SVGALIB

    TOGGLE_MENU (options_video_driver_linux_menu, 8, (gfx_driver -> id == GFX_SVGALIB));

#endif

#endif


#ifdef ALLEGRO_UNIX

    TOGGLE_MENU (options_video_driver_unix_menu, 0, (gfx_driver -> id == GFX_XWINDOWS));

    TOGGLE_MENU (options_video_driver_unix_menu, 2, (gfx_driver -> id == GFX_XWINDOWS_FULLSCREEN));

    TOGGLE_MENU (options_video_driver_unix_menu, 4, (gfx_driver -> id == GFX_XDGA));

    TOGGLE_MENU (options_video_driver_unix_menu, 6, (gfx_driver -> id == GFX_XDGA_FULLSCREEN));

    TOGGLE_MENU (options_video_driver_unix_menu, 8, (gfx_driver -> id == GFX_XDGA2));

#endif

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 0, ((SCREEN_W == 256) && (SCREEN_H == 224)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 2, ((SCREEN_W == 256) && (SCREEN_H == 240)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 4, ((SCREEN_W == 512) && (SCREEN_H == 448)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 6, ((SCREEN_W == 512) && (SCREEN_H == 480)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 8, ((SCREEN_W == 768) && (SCREEN_H == 672)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 10, ((SCREEN_W == 768) && (SCREEN_H == 720)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 12, ((SCREEN_W == 1024) && (SCREEN_H == 896)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 14, ((SCREEN_W == 1024) && (SCREEN_H == 960)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 16, ((SCREEN_W == 1280) && (SCREEN_H == 1120)));

    TOGGLE_MENU (options_video_resolution_proportionate_menu, 18, ((SCREEN_W == 1280) && (SCREEN_H == 1200)));


    TOGGLE_MENU (options_video_resolution_menu, 2, ((SCREEN_W == 320) && (SCREEN_H == 240)));

    TOGGLE_MENU (options_video_resolution_menu, 4, ((SCREEN_W == 640) && (SCREEN_H == 480)));

    TOGGLE_MENU (options_video_resolution_menu, 6, ((SCREEN_W == 800) && (SCREEN_H == 600)));

    TOGGLE_MENU (options_video_resolution_menu, 8, ((SCREEN_W == 1024) && (SCREEN_H == 768)));

    TOGGLE_MENU (options_video_resolution_menu, 10, ((SCREEN_W == 1152) && (SCREEN_H == 864)));

    TOGGLE_MENU (options_video_resolution_menu, 12, ((SCREEN_W == 1280) && (SCREEN_H == 1024)));

    TOGGLE_MENU (options_video_resolution_menu, 14, ((SCREEN_W == 1600) && (SCREEN_H == 1200)));


    TOGGLE_MENU (options_video_resolution_extended_menu, 0, ((SCREEN_W == 400) && (SCREEN_H == 300)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 2, ((SCREEN_W == 480) && (SCREEN_H == 360)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 4, ((SCREEN_W == 512) && (SCREEN_H == 384)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 6, ((SCREEN_W == 640) && (SCREEN_H == 400)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 8, ((SCREEN_W == 720) && (SCREEN_H == 480)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 10, ((SCREEN_W == 720) && (SCREEN_H == 576)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 12, ((SCREEN_W == 848) && (SCREEN_H == 480)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 14, ((SCREEN_W == 1280) && (SCREEN_H == 720)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 16, ((SCREEN_W == 1280) && (SCREEN_H == 960)));

    TOGGLE_MENU (options_video_resolution_extended_menu, 18, ((SCREEN_W == 1360) && (SCREEN_H == 768)));


    TOGGLE_MENU (options_video_colors_menu, 0, (video_get_color_depth () == 8));

    TOGGLE_MENU (options_video_colors_menu, 2, (video_get_color_depth () == 15));

    TOGGLE_MENU (options_video_colors_menu, 4, (video_get_color_depth () == 16));

    TOGGLE_MENU (options_video_colors_menu, 6, (video_get_color_depth () == 32));


    TOGGLE_MENU (options_video_blitter_menu, 0, (video_get_blitter () == VIDEO_BLITTER_AUTOMATIC));

    TOGGLE_MENU (options_video_blitter_menu, 2, (video_get_blitter () == VIDEO_BLITTER_NORMAL));

    TOGGLE_MENU (options_video_blitter_menu, 4, (video_get_blitter () == VIDEO_BLITTER_STRETCHED));

    TOGGLE_MENU (options_video_blitter_menu, 6, (video_get_blitter () == VIDEO_BLITTER_INTERPOLATED_2X));

    TOGGLE_MENU (options_video_blitter_menu, 8, (video_get_blitter () == VIDEO_BLITTER_INTERPOLATED_3X));

    TOGGLE_MENU (options_video_blitter_menu, 10, (video_get_blitter () == VIDEO_BLITTER_2XSOE));

    TOGGLE_MENU (options_video_blitter_menu, 12, (video_get_blitter () == VIDEO_BLITTER_2XSCL));

    TOGGLE_MENU (options_video_blitter_menu, 14, (video_get_blitter () == VIDEO_BLITTER_SUPER_2XSOE));

    TOGGLE_MENU (options_video_blitter_menu, 16, (video_get_blitter () == VIDEO_BLITTER_SUPER_2XSCL));

    TOGGLE_MENU (options_video_blitter_menu, 18, (video_get_blitter () == VIDEO_BLITTER_ULTRA_2XSCL));


    TOGGLE_MENU (options_video_filters_menu, 0, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_LOW));

    TOGGLE_MENU (options_video_filters_menu, 2, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_MEDIUM));

    TOGGLE_MENU (options_video_filters_menu, 4, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_HIGH));


    TOGGLE_MENU (options_video_menu, 10, video_enable_vsync);


    TOGGLE_MENU (options_video_advanced_menu, 0, video_force_window);


    TOGGLE_MENU (options_video_layers_menu, 0, ppu_enable_sprite_layer_a);

    TOGGLE_MENU (options_video_layers_menu, 2, ppu_enable_sprite_layer_b);

    TOGGLE_MENU (options_video_layers_menu, 4, ppu_enable_background_layer);


    TOGGLE_MENU (netplay_protocol_menu, 0, (netplay_protocol == NETPLAY_PROTOCOL_TCPIP));

    TOGGLE_MENU (netplay_protocol_menu, 2, (netplay_protocol == NETPLAY_PROTOCOL_SPX));

}


extern UINT8 logfile [256];


static INLINE void draw_background (void)
{
    if (! rom_is_loaded)
    {
        rectfill (screen, 0, 0, SCREEN_W, SCREEN_H, GUI_BACKGROUND_COLOR);


        if (background_image)
        {
            blit (background_image, screen, 0, 0, ((SCREEN_W / 2) - (background_image -> w / 2)), ((SCREEN_H / 2) - (background_image -> h / 2)), background_image -> w, background_image -> h);
        }
    }
}


int gui_init (void)
{
    gui_menu_draw_menu = sl_draw_menu;

    gui_menu_draw_menu_item = sl_draw_menu_item;


    load_menus ();

    load_dialogs ();


#ifdef ALLEGRO_DOS

    DISABLE_MENU (options_video_driver_menu, 4);

    DISABLE_MENU (options_video_driver_menu, 6);

    DISABLE_MENU (options_video_driver_menu, 8);


    DISABLE_MENU (options_video_advanced_menu, 0);


    DISABLE_MENU (top_menu, 3);

#endif


#ifdef ALLEGRO_WINDOWS

    DISABLE_MENU (options_video_driver_menu, 2);

    DISABLE_MENU (options_video_driver_menu, 6);

    DISABLE_MENU (options_video_driver_menu, 8);

#endif


#ifdef ALLEGRO_UNIX

    DISABLE_MENU (options_video_driver_menu, 2);

    DISABLE_MENU (options_video_driver_menu, 4);


#ifdef ALLEGRO_LINUX

#ifndef GFX_FBCON

    DISABLE_MENU (options_video_driver_linux_menu, 6);

#endif

#ifndef GFX_SVGALIB

    DISABLE_MENU (options_video_driver_linux_menu, 8);

#endif

#else

    DISABLE_MENU (options_video_driver_menu, 6);

#endif

#endif


    CHECK_MENU (options_video_palette_menu, 10);


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


int show_gui (int first_run)
{
    gui_needs_restart = FALSE;

    gui_is_active = TRUE;


    want_exit = FALSE;


    if (gui_mouse_sprite)
    {
        set_mouse_sprite (gui_mouse_sprite);
    }


    update_menus ();


    if (! rom_is_loaded)
    {
        DISABLE_MENU (main_menu, 2);

        DISABLE_MENU (main_menu, 4);

        DISABLE_MENU (main_menu, 6);

        DISABLE_MENU (main_menu, 8);

        DISABLE_MENU (main_menu, 10);


        DISABLE_MENU (options_menu, 12);
    }


    audio_suspend ();


    video_blit (screen);


    draw_background ();


    switch (video_get_color_depth ())
    {
        case 8:

            gui_message (GUI_TEXT_COLOR, "%dx%d 8-bit, %s.", SCREEN_W, SCREEN_H, gfx_driver -> name);


            break;


        case 15:

            gui_message (GUI_TEXT_COLOR, "%dx%d 15-bit, %s.", SCREEN_W, SCREEN_H, gfx_driver -> name);


            break;


        case 16:

            gui_message (GUI_TEXT_COLOR, "%dx%d 16-bit, %s.", SCREEN_W, SCREEN_H, gfx_driver -> name);


            break;


        case 32:

            gui_message (GUI_TEXT_COLOR, "%dx%d 32-bit, %s.", SCREEN_W, SCREEN_H, gfx_driver -> name);


            break;


        default:

            break;
    }


    set_mouse_sprite_focus (8, 8);

    show_mouse (screen);


    if (first_run)
    {
        alert ("FakeNES version " VERSION_STRING " " ALLEGRO_PLATFORM_STR, "", "Get "
            "the latest from http://fakenes.sourceforge.net/.", "&OK", NIL, 'o', 0);
    }


    do_dialog (main_dialog, -1);


    show_mouse (NULL);


    /* Cheap hack to make it appear in the file... */

    {
        char save_path[256];
        char ip_address[256];


        memset (save_path, NIL, sizeof (save_path));

        strncpy(save_path, get_config_string ("gui", "save_path", "./"), sizeof (save_path) - 1);

        set_config_string ("gui", "save_path", save_path);


        memset (ip_address, NIL, sizeof (ip_address));

        strncpy(ip_address, get_config_string ("netplay", "ip_address", "0.0.0.0"), sizeof (ip_address) - 1);

        set_config_string ("netplay", "ip_address", ip_address);
    }


    set_config_int ("gui", "theme", gui_theme_id);


    /* Is this needed for Zapper? */

    set_mouse_sprite_focus (0, 0);


    gui_is_active = FALSE;


    clear (screen);

    video_blit (screen);


    draw_background ();


    audio_resume ();


    return (want_exit);
}


static int client_connected = FALSE;


static int netplay_handler (int message, DIALOG * dialog, int key)
{
    if (netplay_server_active && (! client_connected))
    {
        client_connected = netplay_poll_server ();


        if (client_connected)
        {
            int result;


            gui_message (GUI_TEXT_COLOR, "Accepted connection from client.");


            result = main_menu_load_rom ();

            if (result == D_CLOSE)
            {
                return (D_CLOSE);
            }
            else
            {
                gui_message (GUI_TEXT_COLOR, "NetPlay session canceled.");
            }
        }
    }


    return (D_O_K);
}


/* ---- Menu handlers. ---- */


#define REPLAY_SELECT_MENU_HANDLER(slot)        \
    static int main_replay_select_menu_##slot (void)    \
    {                                           \
        replay_index = slot;                    \
                                                \
        update_menus ();                        \
                                                \
                                                \
        gui_message (GUI_TEXT_COLOR, "Replay"     \
            " slot set to %d.", replay_index);          \
                                                \
                                                \
        return (D_O_K);                         \
    }


REPLAY_SELECT_MENU_HANDLER (0)

REPLAY_SELECT_MENU_HANDLER (1)

REPLAY_SELECT_MENU_HANDLER (2)

REPLAY_SELECT_MENU_HANDLER (3)

REPLAY_SELECT_MENU_HANDLER (4)


static UINT8 replay_menu_texts [5] [20];

static UINT8 replay_titles [5] [16];


static int main_replay_menu_select (void)
{
    UINT8 buffer [256];

    UINT8 buffer2 [16];

    UINT8 buffer3 [4];


    int index;


    PACKFILE * file;


    for (index = 0; index < 5; index ++)
    {
        memset (buffer, NIL, sizeof (buffer));
    
        memset (buffer2, NIL, sizeof (buffer2));
    
        memset (buffer3, NIL, sizeof (buffer3));


        sprintf (buffer3, "fr%d", index);
    

        strcat (buffer, get_config_string ("gui", "save_path", "./"));
    
        put_backslash (buffer);
    
    
        strcat (buffer, get_filename (global_rom.filename));
    
    
        replace_extension (buffer, buffer, buffer3, sizeof (buffer));
        
    
        file = pack_fopen (buffer, "r");
    
        if (file)
        {
            UINT8 signature [4];
    
    
            int version;
    

            /* Probably don't need to verify these... */
    
            pack_fread (signature, 4, file);
        
        
            version = pack_igetw (file);
        

            pack_fread (buffer2, sizeof (buffer2), file);


            memset (replay_titles [index], NIL, 16);

            strcat (replay_titles [index], buffer2);


            pack_fclose (file);
        }
        else
        {
            memset (replay_titles [index], NIL, 16);

            strcat (replay_titles [index], "Empty");
        }


        memset (replay_menu_texts [index], NIL, 20);

        sprintf (replay_menu_texts [index], "&%d: %s", index, replay_titles [index]);


        main_replay_select_menu [index * 2].text = replay_menu_texts [index];
    }


    return (D_O_K);
}


static int main_replay_record_menu_start (void)
{
    UINT8 buffer [256];

    UINT8 buffer2 [16];

    UINT8 buffer3 [4];


    PACKFILE * file;


    memset (buffer, NIL, sizeof (buffer));

    memset (buffer2, NIL, sizeof (buffer2));

    memset (buffer3, NIL, sizeof (buffer3));


    if (gui_is_active)
    {
        main_replay_record_start_dialog [4].d1 = (sizeof (buffer2) - 1);
        
        main_replay_record_start_dialog [4].dp = buffer2;
        

        if (strcmp (replay_titles [replay_index], "Empty") == 0)
        {
            strcat (buffer2, "Untitled");
        }
        else
        {
            strcat (buffer2, replay_titles [replay_index]);
        }
        
        
        if (gui_show_dialog (main_replay_record_start_dialog) != 5)
        {
            return (D_O_K);
        }
    }
    else
    {
        /* Save using last title. */

        if (strcmp (replay_titles [replay_index], "Empty") == 0)
        {
            strcat (buffer2, "Untitled");
        }
        else
        {
            strcat (buffer2, replay_titles [replay_index]);
        }
    }


    sprintf (buffer3, "fr%d", replay_index);


    strcat (buffer, get_config_string ("gui", "save_path", "./"));

    put_backslash (buffer);


    strcat (buffer, get_filename (global_rom.filename));


    replace_extension (buffer, buffer, buffer3, sizeof (buffer));


    replay_file = pack_fopen (buffer, "w");

    if (replay_file)
    {
        PACKFILE * file;


        int version;


        file = replay_file;


        pack_fwrite ("FNSS", 4, file);
    
    
        version = 0x0100;
    

        pack_iputw (version, file);
    

        pack_fwrite (buffer2, sizeof (buffer2), file);


        pack_iputl (global_rom.trainer_crc32, file);


        pack_iputl (global_rom.prg_rom_crc32, file);

        pack_iputl (global_rom.chr_rom_crc32, file);


        pack_fwrite ("CPU\0", 4, file);

        cpu_save_state (file, version);


        pack_fwrite ("PPU\0", 4, file);

        ppu_save_state (file, version);


        pack_fwrite ("PAPU", 4, file);

        papu_save_state (file, version);


        pack_fwrite ("MMC\0", 4, file);

        mmc_save_state (file, version);


        pack_fwrite ("CTRL", 4, file);

        input_save_state (file, version);
    

        pack_fwrite ("REPL", 4, replay_file);


        replay_file_chunk = pack_fopen_chunk (replay_file, FALSE);


        DISABLE_MENU (main_replay_record_menu, 0);
    
        ENABLE_MENU (main_replay_record_menu, 2);
    
    
        DISABLE_MENU (main_menu, 0);
    

        DISABLE_MENU (main_replay_menu, 0);

        DISABLE_MENU (main_replay_menu, 4);


        DISABLE_MENU (main_state_menu, 4);
    
    
        DISABLE_MENU (top_menu, 2);
    
    
        input_mode |= INPUT_MODE_REPLAY;
    
        input_mode |= INPUT_MODE_REPLAY_RECORD;
    
    
        gui_message (GUI_TEXT_COLOR, "Replay recording session started.");
    

        /* Update save state titles. */

        main_replay_menu_select ();


        return (D_CLOSE);
    }
    else
    {
        gui_message (GUI_ERROR_COLOR, "Failed to open new machine state file.");
    }


    return (D_O_K);
}


static int main_replay_record_menu_stop (void)
{
    pack_fclose_chunk (replay_file_chunk);


    pack_fclose (replay_file);


    input_mode &= ~INPUT_MODE_REPLAY;

    input_mode &= ~INPUT_MODE_REPLAY_RECORD;


    ENABLE_MENU (main_replay_record_menu, 0);

    DISABLE_MENU (main_replay_record_menu, 2);


    ENABLE_MENU (main_menu, 0);


    ENABLE_MENU (main_replay_menu, 0);

    ENABLE_MENU (main_replay_menu, 4);


    ENABLE_MENU (main_state_menu, 4);


    ENABLE_MENU (top_menu, 2);


    gui_message (GUI_TEXT_COLOR, "Replay recording session stopped.");


    return (D_O_K);
}


static int main_replay_play_menu_start (void)
{
    UINT8 buffer [256];

    UINT8 buffer2 [16];

    UINT8 buffer3 [4];


    PACKFILE * file;


    memset (buffer, NIL, sizeof (buffer));

    memset (buffer3, NIL, sizeof (buffer3));


    sprintf (buffer3, "fr%d", replay_index);


    strcat (buffer, get_config_string ("gui", "save_path", "./"));

    put_backslash (buffer);


    strcat (buffer, get_filename (global_rom.filename));


    replace_extension (buffer, buffer, buffer3, sizeof (buffer));


    replay_file = pack_fopen (buffer, "r");

    if (replay_file)
    {
        PACKFILE * file;


        UINT8 signature [4];
    
    
        int version;


        UINT32 trainer_crc;


        UINT32 prg_rom_crc;

        UINT32 chr_rom_crc;


        file = replay_file;


        pack_fread (signature, 4, file);
    
    
        if (strncmp (signature, "FNSS", 4))
        {
            gui_message (GUI_ERROR_COLOR, "Machine state file is invalid.");
    
    
            pack_fclose (file);
    
    
            return (D_O_K);
        }
    
    
        version = pack_igetw (file);
    
    
        if (version > 0x0100)
        {
            gui_message (GUI_ERROR_COLOR, "Machine state file is of a future version.");
    
    
            pack_fclose (file);
    
    
            return (D_O_K);
        }
    

        pack_fread (buffer2, sizeof (buffer2), file);


        trainer_crc = pack_igetl (file);


        prg_rom_crc = pack_igetl (file);

        chr_rom_crc = pack_igetl (file);


        if ((trainer_crc != global_rom.trainer_crc32) ||
           ((prg_rom_crc != global_rom.prg_rom_crc32) || (chr_rom_crc != global_rom.chr_rom_crc32)))
        {
            gui_message (GUI_ERROR_COLOR, "Machine state file is for a different ROM.");
    
    
            pack_fclose (file);
    
    
            return (D_O_K);
        }


        machine_reset ();
    

        /* We ignore signatures for now, this will be used in the
        future to load chunks in any order. */

        pack_fread (signature, 4, file);

        cpu_load_state (file, version);


        pack_fread (signature, 4, file);

        ppu_load_state (file, version);


        pack_fread (signature, 4, file);
    
        papu_load_state (file, version);
    

        pack_fread (signature, 4, file);

        mmc_load_state (file, version);


        pack_fread (signature, 4, file);
    
        input_load_state (file, version);
    

        pack_fread (signature, 4, file);


        if (strncmp (signature, "REPL", 4))
        {
            gui_message (GUI_ERROR_COLOR, "Machine state file is missing replay chunk.");
    
    
            pack_fclose (file);
    
    
            return (D_O_K);
        }
    

        replay_file_chunk = pack_fopen_chunk (file, FALSE);


        DISABLE_MENU (main_replay_play_menu, 0);
    
        ENABLE_MENU (main_replay_play_menu, 2);
    
    
        DISABLE_MENU (main_menu, 0);
    

        DISABLE_MENU (main_replay_menu, 0);

        DISABLE_MENU (main_replay_menu, 2);


        DISABLE_MENU (main_state_menu, 4);
    
    
        DISABLE_MENU (top_menu, 2);
    
    
        input_mode &= ~INPUT_MODE_PLAY;


        input_mode |= INPUT_MODE_REPLAY;

        input_mode |= INPUT_MODE_REPLAY_PLAY;


        gui_message (GUI_TEXT_COLOR, "Replay playback started.");


        return (D_CLOSE);
    }
    else
    {
        gui_message (GUI_ERROR_COLOR, "Machine state file does not exist.");


        return (D_O_K);
    }
}


static int main_replay_play_menu_stop (void)
{
    pack_fclose_chunk (replay_file_chunk);


    pack_fclose (replay_file);


    if (! (input_mode & INPUT_MODE_CHAT))
    {
        input_mode |= INPUT_MODE_PLAY;
    }


    input_mode &= ~INPUT_MODE_REPLAY;

    input_mode &= ~INPUT_MODE_REPLAY_PLAY;


    ENABLE_MENU (main_replay_play_menu, 0);

    DISABLE_MENU (main_replay_play_menu, 2);


    ENABLE_MENU (main_menu, 0);


    ENABLE_MENU (main_replay_menu, 0);

    ENABLE_MENU (main_replay_menu, 2);


    ENABLE_MENU (main_state_menu, 4);


    ENABLE_MENU (top_menu, 2);


    if (gui_is_active)
    {
        gui_message (GUI_TEXT_COLOR, "Replay playback stopped.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Replay playback finished.");
    }


    return (D_O_K);
}


static int main_menu_load_rom (void)
{
    ROM test_rom;


    UINT8 buffer [256];

    UINT8 buffer2 [256];

    UINT8 buffer3 [256];


    memset (buffer, NIL, sizeof (buffer));

    memset (buffer2, NIL, sizeof (buffer2));

    memset (buffer3, NIL, sizeof (buffer3));


    strcat (buffer, get_config_string ("gui", "load_rom_path", "/"));


#ifdef USE_ZLIB

    if (file_select_ex ("iNES ROMs (*.NES, *.GZ, *.ZIP)", buffer, "NES;nes;GZ;gz;ZIP;zip",
        sizeof (buffer), ((SCREEN_W / 5) * 4), ((SCREEN_H / 6) * 4)) != 0)
    {
#else
    if (file_select_ex ("iNES ROMs (*.NES)", buffer, "NES;nes",
        sizeof (buffer), ((SCREEN_W / 5) * 4), ((SCREEN_H / 6) * 4)) != 0)
    {

#endif

        set_config_string ("gui", "load_rom_path", replace_filename (buffer3, buffer, "", sizeof (buffer3)));


        if (load_rom (buffer, &test_rom) != 0)
        {
            gui_message (GUI_ERROR_COLOR, "Failed to load ROM!");


            if (netplay_server_active)
            {
                netplay_close_server ();
            }


            if (netplay_client_active)
            {
                netplay_close_client ();
            }


            return (D_O_K);
        }
        else
        {
            if (rom_is_loaded)
            {
                if (global_rom.sram_flag)
                {
                    sram_save (global_rom.filename);
                }


                patches_save (global_rom.filename);


                free_rom (&global_rom);
            }


            memcpy (&global_rom, &test_rom, sizeof (ROM));


            /* Update save state titles. */

            main_state_menu_select ();


            /* Update replay titles. */

            main_replay_menu_select ();


            rom_is_loaded = TRUE;
    
            machine_init ();


            if ((! netplay_server_active) && (! netplay_client_active))
            {
                ENABLE_MENU (main_menu, 2);
    
                ENABLE_MENU (main_menu, 4);

                ENABLE_MENU (main_menu, 6);

                ENABLE_MENU (main_menu, 8);

                ENABLE_MENU (main_menu, 10);
    

                ENABLE_MENU (options_menu, 12);
            }


            sprintf (buffer2, "FakeNES - %s", get_filename (global_rom.filename));

            set_window_title (buffer2);


            return (D_CLOSE);
        }
    }
    else
    {
        set_config_string ("gui", "load_rom_path", replace_filename (buffer3, buffer, "", sizeof (buffer3)));


        if (netplay_server_active)
        {
            netplay_close_server ();
        }


        if (netplay_client_active)
        {
            netplay_close_client ();
        }


        return (D_O_K);
    }
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


#define STATE_SELECT_MENU_HANDLER(slot)         \
    static int main_state_select_menu_##slot (void)          \
    {                                           \
        machine_state_index = slot;             \
                                                \
        update_menus ();                        \
                                                \
                                                \
        gui_message (GUI_TEXT_COLOR, "Machine"            \
            " state slot set to %d.", machine_state_index);     \
                                                \
                                                \
        return (D_O_K);                         \
    }


STATE_SELECT_MENU_HANDLER (0);

STATE_SELECT_MENU_HANDLER (1);

STATE_SELECT_MENU_HANDLER (2);

STATE_SELECT_MENU_HANDLER (3);

STATE_SELECT_MENU_HANDLER (4);

STATE_SELECT_MENU_HANDLER (5);

STATE_SELECT_MENU_HANDLER (6);

STATE_SELECT_MENU_HANDLER (7);

STATE_SELECT_MENU_HANDLER (8);

STATE_SELECT_MENU_HANDLER (9);


static UINT8 main_state_menu_texts [10] [20];

static UINT8 main_state_titles [10] [16];


static int main_state_menu_select (void)
{
    UINT8 buffer [256];

    UINT8 buffer2 [16];

    UINT8 buffer3 [4];


    int index;


    PACKFILE * file;


    for (index = 0; index < 10; index ++)
    {
        memset (buffer, NIL, sizeof (buffer));
    
        memset (buffer2, NIL, sizeof (buffer2));
    
        memset (buffer3, NIL, sizeof (buffer3));


        sprintf (buffer3, "fn%d", index);
    

        strcat (buffer, get_config_string ("gui", "save_path", "./"));
    
        put_backslash (buffer);
    
    
        strcat (buffer, get_filename (global_rom.filename));
    
    
        replace_extension (buffer, buffer, buffer3, sizeof (buffer));
        
    
        file = pack_fopen (buffer, "r");
    
        if (file)
        {
            UINT8 signature [4];
    
    
            int version;
    

            /* Probably don't need to verify these... */
    
            pack_fread (signature, 4, file);
        
        
            version = pack_igetw (file);
        

            pack_fread (buffer2, sizeof (buffer2), file);


            memset (main_state_titles [index], NIL, 16);

            strcat (main_state_titles [index], buffer2);


            pack_fclose (file);
        }
        else
        {
            memset (main_state_titles [index], NIL, 16);

            strcat (main_state_titles [index], "Empty");
        }


        memset (main_state_menu_texts [index], NIL, 20);

        sprintf (main_state_menu_texts [index], "&%d: %s", index, main_state_titles [index]);


        main_state_select_menu [index * 2].text = main_state_menu_texts [index];
    }


    return (D_O_K);
}


static int main_state_menu_save (void)
{
    UINT8 buffer [256];

    UINT8 buffer2 [16];

    UINT8 buffer3 [4];


    PACKFILE * file;


    memset (buffer, NIL, sizeof (buffer));

    memset (buffer2, NIL, sizeof (buffer2));

    memset (buffer3, NIL, sizeof (buffer3));


    if (gui_is_active)
    {
        main_state_save_dialog [4].d1 = (sizeof (buffer2) - 1);
        
        main_state_save_dialog [4].dp = buffer2;
        

        if (strcmp (main_state_titles [machine_state_index], "Empty") == 0)
        {
            strcat (buffer2, "Untitled");
        }
        else
        {
            strcat (buffer2, main_state_titles [machine_state_index]);
        }
        
        
        if (gui_show_dialog (main_state_save_dialog) != 5)
        {
            return (D_O_K);
        }
    }
    else if (input_autosave_triggered)
    {
        strcat (buffer2, "Autosave");
    }
    else
    {
        /* Save using last title. */

        if (strcmp (main_state_titles [machine_state_index], "Empty") == 0)
        {
            strcat (buffer2, "Untitled");
        }
        else
        {
            strcat (buffer2, main_state_titles [machine_state_index]);
        }
    }


    sprintf (buffer3, "fn%d", machine_state_index);


    strcat (buffer, get_config_string ("gui", "save_path", "./"));

    put_backslash (buffer);


    strcat (buffer, get_filename (global_rom.filename));


    replace_extension (buffer, buffer, buffer3, sizeof (buffer));


    file = pack_fopen (buffer, "w");

    if (file)
    {
        int version;


        pack_fwrite ("FNSS", 4, file);
    
    
        version = 0x0100;
    

        pack_iputw (version, file);
    

        pack_fwrite (buffer2, sizeof (buffer2), file);


        pack_iputl (global_rom.trainer_crc32, file);


        pack_iputl (global_rom.prg_rom_crc32, file);

        pack_iputl (global_rom.chr_rom_crc32, file);


        pack_fwrite ("CPU\0", 4, file);

        cpu_save_state (file, version);


        pack_fwrite ("PPU\0", 4, file);

        ppu_save_state (file, version);


        pack_fwrite ("PAPU", 4, file);

        papu_save_state (file, version);


        pack_fwrite ("MMC\0", 4, file);

        mmc_save_state (file, version);


        pack_fwrite ("CTRL", 4, file);

        input_save_state (file, version);
    
    
        pack_fclose (file);
    

        if (! input_autosave_triggered)
        {
            gui_message (GUI_TEXT_COLOR, "Machine state saved in slot %d.", machine_state_index);
        }


        /* Update save state titles. */

        main_state_menu_select ();
    }
    else
    {
        gui_message (GUI_ERROR_COLOR, "Failed to open new machine state file.");
    }


    return (D_O_K);
}


static int main_state_menu_restore (void)
{
    UINT8 buffer [256];

    UINT8 buffer2 [16];

    UINT8 buffer3 [4];


    PACKFILE * file;


    memset (buffer, NIL, sizeof (buffer));

    memset (buffer3, NIL, sizeof (buffer3));


    sprintf (buffer3, "fn%d", machine_state_index);


    strcat (buffer, get_config_string ("gui", "save_path", "./"));

    put_backslash (buffer);


    strcat (buffer, get_filename (global_rom.filename));


    replace_extension (buffer, buffer, buffer3, sizeof (buffer));


    file = pack_fopen (buffer, "r");

    if (file)
    {
        UINT8 signature [4];
    
    
        int version;


        UINT32 trainer_crc;


        UINT32 prg_rom_crc;

        UINT32 chr_rom_crc;


        pack_fread (signature, 4, file);
    
    
        if (strncmp (signature, "FNSS", 4))
        {
            gui_message (GUI_ERROR_COLOR, "Machine state file is invalid.");
    
    
            pack_fclose (file);
    
    
            return (D_O_K);
        }
    
    
        version = pack_igetw (file);
    
    
        if (version > 0x0100)
        {
            gui_message (GUI_ERROR_COLOR, "Machine state file is of a future version.");
    
    
            pack_fclose (file);
    
    
            return (D_O_K);
        }
    

        pack_fread (buffer2, sizeof (buffer2), file);


        trainer_crc = pack_igetl (file);


        prg_rom_crc = pack_igetl (file);

        chr_rom_crc = pack_igetl (file);


        if ((trainer_crc != global_rom.trainer_crc32) ||
           ((prg_rom_crc != global_rom.prg_rom_crc32) || (chr_rom_crc != global_rom.chr_rom_crc32)))
        {
            gui_message (GUI_ERROR_COLOR, "Machine state file is for a different ROM.");
    
    
            pack_fclose (file);
    
    
            return (D_O_K);
        }


        machine_reset ();
    

        /* We ignore signatures for now, this will be used in the
        future to load chunks in any order. */

        pack_fread (signature, 4, file);

        cpu_load_state (file, version);


        pack_fread (signature, 4, file);

        ppu_load_state (file, version);


        pack_fread (signature, 4, file);
    
        papu_load_state (file, version);
    

        pack_fread (signature, 4, file);

        mmc_load_state (file, version);


        pack_fread (signature, 4, file);
    
        input_load_state (file, version);
    
    
        pack_fclose (file);
    
    
        gui_message (GUI_TEXT_COLOR, "Machine state loaded from slot %d.", machine_state_index);


        return (D_CLOSE);
    }
    else
    {
        gui_message (GUI_ERROR_COLOR, "Machine state file does not exist.");


        return (D_O_K);
    }
}


static int main_state_autosave_menu_disabled (void)
{
    input_autosave_interval = 0;

    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Autosave disabled.");


    return (D_O_K);
}


static int main_state_autosave_menu_10_seconds (void)
{
    input_autosave_interval = 10;

    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Autosave interval set to 10 seconds.");


    return (D_O_K);
}


static int main_state_autosave_menu_30_seconds (void)
{
    input_autosave_interval = 30;

    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Autosave interval set to 30 seconds.");


    return (D_O_K);
}


static int main_state_autosave_menu_60_seconds (void)
{
    input_autosave_interval = 60;

    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Autosave interval set to 60 seconds.");


    return (D_O_K);
}


static int main_menu_snapshot (void)
{
    int count;

    UINT8 filename [12];


    for (count = 0; count < 999; count ++)
    {
        sprintf (filename, "snap%03d.pcx", count);

        filename [11] = NIL;


        if (! exists (filename))
        {
            count = 1000;


            save_bitmap (filename, video_buffer, current_palette);

            gui_message (GUI_TEXT_COLOR, "Snapshot saved to %s.", filename);
        }
    }


    return (D_O_K);
}


static int main_menu_messages (void)
{
    PACKFILE * file;


    UINT8 * buffer;


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


    gui_show_dialog (main_messages_dialog);


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


static int options_gui_theme_menu_classic (void)
{
    set_classic_theme ();


    gui_needs_restart = TRUE;

    
    return (D_CLOSE);
}


static int options_gui_theme_menu_stainless_steel (void)
{
    set_stainless_steel_theme ();


    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_gui_theme_menu_zero_4 (void)
{
    set_zero_4_theme ();


    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_gui_theme_menu_panta (void)
{
    set_panta_theme ();


    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_system_menu_ntsc_60_hz (void)
{
    machine_type = MACHINE_TYPE_NTSC;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "System type set to NTSC (60 Hz).");


    return (D_O_K);
}


static int options_system_menu_pal_50_hz (void)
{
    machine_type = MACHINE_TYPE_PAL;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "System type set to PAL (50 Hz).");


    return (D_O_K);
}


static int options_audio_menu_enabled (void)
{
    audio_enable_output = (! audio_enable_output);


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    if (! audio_enable_output)
    {
        gui_message (GUI_TEXT_COLOR, "Audio rendering and output disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio rendering and output enabled.");
    }


    return (D_O_K);
}


#define MIXING_SPEED_MENU_HANDLER(speed)        \
    static int options_audio_mixing_frequency_menu_##speed##_hz (void)  \
    {                                           \
        audio_sample_rate = speed;              \
                                                \
                                                \
        audio_exit ();                          \
                                                \
        audio_init ();                          \
                                                \
                                                \
        papu_reinit ();                         \
                                                \
                                                \
        update_menus ();                        \
                                                \
                                                \
        gui_message (GUI_TEXT_COLOR, "Audio mixing speed set to %d Hz.", speed);  \
                                                \
                                                \
        return (D_O_K);                         \
    }


MIXING_SPEED_MENU_HANDLER (8000)

MIXING_SPEED_MENU_HANDLER (11025)

MIXING_SPEED_MENU_HANDLER (16000)

MIXING_SPEED_MENU_HANDLER (22050)

MIXING_SPEED_MENU_HANDLER (32000)

MIXING_SPEED_MENU_HANDLER (44100)

MIXING_SPEED_MENU_HANDLER (48000)

MIXING_SPEED_MENU_HANDLER (80200)

MIXING_SPEED_MENU_HANDLER (96000)


static int options_audio_mixing_channels_menu_mono (void)
{
    audio_pseudo_stereo = FALSE;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio channels set to mono.");


    return (D_O_K);
}


static int options_audio_mixing_channels_menu_stereo_mix (void)
{
    if (! audio_pseudo_stereo)
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_4;


        audio_exit ();
    
        audio_init ();
    
    
        papu_reinit ();
    }
    else
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_4;
    }
    

    update_menus ();
    

    gui_message (GUI_TEXT_COLOR, "Audio channels set to mono with stereo mixing.");


    return (D_O_K);
}


static int options_audio_mixing_channels_menu_pseudo_stereo_mode_1 (void)
{
    if (! audio_pseudo_stereo)
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_1;


        audio_exit ();
    
        audio_init ();
    
    
        papu_reinit ();
    }
    else
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_1;
    }
    

    update_menus ();
    

    gui_message (GUI_TEXT_COLOR, "Audio channels set to pseudo stereo (mode 1).");


    return (D_O_K);
}


static int options_audio_mixing_channels_menu_pseudo_stereo_mode_2 (void)
{
    if (! audio_pseudo_stereo)
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_2;


        audio_exit ();
    
        audio_init ();
    
    
        papu_reinit ();
    }
    else
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_2;
    }
    

    update_menus ();
    

    gui_message (GUI_TEXT_COLOR, "Audio channels set to pseudo stereo (mode 2).");


    return (D_O_K);
}


static int options_audio_mixing_channels_menu_stereo (void)
{
    if (! audio_pseudo_stereo)
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_3;


        audio_exit ();
    
        audio_init ();
    
    
        papu_reinit ();
    }
    else
    {
        audio_pseudo_stereo = AUDIO_PSEUDO_STEREO_MODE_3;
    }
    

    update_menus ();
    

    gui_message (GUI_TEXT_COLOR, "Audio channels set to stereo.");


    return (D_O_K);
}


static int options_audio_mixing_channels_menu_swap_channels (void)
{
    papu_swap_channels = (! papu_swap_channels);


    update_menus ();


    if (! papu_swap_channels)
    {
        gui_message (GUI_TEXT_COLOR, "Audio stereo channel swapping disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio stereo channel swapping enabled.");
    }


    return (D_O_K);
}


static int options_audio_mixing_quality_menu_low_8_bit (void)
{
    audio_sample_size = 8;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio mixing quality set to low (8-bit).");


    return (D_O_K);
}


static int options_audio_mixing_quality_menu_high_16_bit (void)
{
    audio_sample_size = 16;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio mixing quality set to high (16-bit).");


    return (D_O_K);
}


static int options_audio_mixing_quality_menu_dithering (void)
{
    papu_dithering = (! papu_dithering);


    update_menus ();


    if (! papu_dithering)
    {
        gui_message (GUI_TEXT_COLOR, "Audio random noise dithering disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio random noise dithering enabled.");
    }


    return (D_O_K);
}


static int options_audio_mixing_anti_aliasing_menu_disabled (void)
{
    papu_interpolate = 0;


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio anti-aliasing disabled.");


    return (D_O_K);
}


static int options_audio_mixing_anti_aliasing_menu_bilinear_2x (void)
{
    papu_interpolate = 1;


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio anti-aliasing method set to bilinear 2X.");


    return (D_O_K);
}


static int options_audio_mixing_anti_aliasing_menu_bilinear_4x (void)
{
    papu_interpolate = 2;


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio anti-aliasing method set to bilinear 4X.");


    return (D_O_K);
}


static int options_audio_mixing_anti_aliasing_menu_bilinear_8x (void)
{
    papu_interpolate = 3;


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio anti-aliasing method set to bilinear 8X.");


    return (D_O_K);
}


static int options_audio_mixing_anti_aliasing_menu_bilinear_16x (void)
{
    papu_interpolate = 4;


    papu_reinit ();


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Audio anti-aliasing method set to bilinear 16X.");


    return (D_O_K);
}


static int options_audio_effects_menu_linear_echo (void)
{
    papu_linear_echo = (! papu_linear_echo);

    update_menus ();


    papu_reinit ();


    if (! papu_linear_echo)
    {
        gui_message (GUI_TEXT_COLOR, "Audio linear echo effect disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio linear echo effect enabled.");
    }


    return (D_O_K);
}


static int options_audio_effects_menu_spatial_stereo_mode_1 (void)
{
    papu_spatial_stereo = ((papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_1) ? FALSE : PAPU_SPATIAL_STEREO_MODE_1);


    update_menus ();


    papu_reinit ();


    if (! papu_spatial_stereo)
    {
        gui_message (GUI_TEXT_COLOR, "Audio spatial stereo effect disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio spatial stereo effect enabled (mode 1).");
    }


    return (D_O_K);
}


static int options_audio_effects_menu_spatial_stereo_mode_2 (void)
{
    papu_spatial_stereo = ((papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_2) ? FALSE : PAPU_SPATIAL_STEREO_MODE_2);


    update_menus ();


    papu_reinit ();


    if (! papu_spatial_stereo)
    {
        gui_message (GUI_TEXT_COLOR, "Audio spatial stereo effect disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio spatial stereo effect enabled (mode 2).");
    }


    return (D_O_K);
}


static int options_audio_effects_menu_spatial_stereo_mode_3 (void)
{
    papu_spatial_stereo = ((papu_spatial_stereo == PAPU_SPATIAL_STEREO_MODE_3) ? FALSE : PAPU_SPATIAL_STEREO_MODE_3);


    update_menus ();


    papu_reinit ();


    if (! papu_spatial_stereo)
    {
        gui_message (GUI_TEXT_COLOR, "Audio spatial stereo effect disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio spatial stereo effect enabled (mode 3).");
    }


    return (D_O_K);
}


static int options_audio_filters_menu_low_pass_mode_1 (void)
{
    int filters;


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


    if (! (filters & PAPU_FILTER_LOW_PASS_MODE_1))
    {
        gui_message (GUI_TEXT_COLOR, "Low pass audio filter disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Low pass audio filter enabled (mode 1).");
    }


    return (D_O_K);
}


static int options_audio_filters_menu_low_pass_mode_2 (void)
{
    int filters;


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


    if (! (filters & PAPU_FILTER_LOW_PASS_MODE_2))
    {
        gui_message (GUI_TEXT_COLOR, "Low pass audio filter disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Low pass audio filter enabled (mode 2).");
    }


    return (D_O_K);
}


static int options_audio_filters_menu_low_pass_mode_3 (void)
{
    int filters;


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


    if (! (filters & PAPU_FILTER_LOW_PASS_MODE_3))
    {
        gui_message (GUI_TEXT_COLOR, "Low pass audio filter disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Low pass audio filter enabled (mode 3).");
    }


    return (D_O_K);
}


static int options_audio_filters_menu_high_pass (void)
{
    int filters;


    filters = papu_get_filter_list ();


    if (filters & PAPU_FILTER_HIGH_PASS)
    {
        papu_set_filter_list ((filters & ~PAPU_FILTER_HIGH_PASS));
    }
    else
    {
        papu_set_filter_list ((filters | PAPU_FILTER_HIGH_PASS));
    }


    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Toggled high pass audio filter.");


    return (D_O_K);
}


static int options_audio_channels_menu_square_wave_a (void)
{
    papu_enable_square_1 = (! papu_enable_square_1);

    papu_update ();


    update_menus ();


    if (! papu_enable_square_1)
    {
        gui_message (GUI_TEXT_COLOR, "Audio square wave channel A disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio square wave channel A enabled.");
    }


    return (D_O_K);
}


static int options_audio_channels_menu_square_wave_b (void)
{
    papu_enable_square_2 = (! papu_enable_square_2);

    papu_update ();


    update_menus ();


    if (! papu_enable_square_2)
    {
        gui_message (GUI_TEXT_COLOR, "Audio square wave channel B disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio square wave channel B enabled.");
    }


    return (D_O_K);
}


static int options_audio_channels_menu_triangle_wave (void)
{
    papu_enable_triangle = (! papu_enable_triangle);

    papu_update ();


    update_menus ();


    if (! papu_enable_triangle)
    {
        gui_message (GUI_TEXT_COLOR, "Audio triangle wave channel disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio triangle wave channel enabled.");
    }


    return (D_O_K);
}


static int options_audio_channels_menu_white_noise (void)
{
    papu_enable_noise = (! papu_enable_noise);

    papu_update ();


    update_menus ();


    if (! papu_enable_noise)
    {
        gui_message (GUI_TEXT_COLOR, "Audio white noise channel disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio white noise channel enabled.");
    }


    return (D_O_K);
}


static int options_audio_channels_menu_digital (void)
{
    papu_enable_dmc = (! papu_enable_dmc);

    papu_update ();


    update_menus ();


    if (! papu_enable_dmc)
    {
        gui_message (GUI_TEXT_COLOR, "Audio digital channel disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio digital channel enabled.");
    }


    return (D_O_K);
}


static int options_audio_channels_menu_extended (void)
{
    papu_enable_exsound = (! papu_enable_exsound);

    papu_update ();


    update_menus ();


    if (! papu_enable_exsound)
    {
        gui_message (GUI_TEXT_COLOR, "Audio extended channels disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio extended channels enabled.");
    }
                                              

    return (D_O_K);
}


static int options_audio_advanced_menu_ideal_triangle (void)
{
    papu_ideal_triangle = (! papu_ideal_triangle);

    papu_update ();


    update_menus ();


    if (! papu_ideal_triangle)
    {
        gui_message (GUI_TEXT_COLOR, "Audio ideal triangle emulation disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio ideal triangle emulation enabled.");
    }


    return (D_O_K);
}


static int options_audio_advanced_menu_hard_sync (void)
{
    audio_hard_sync = (! audio_hard_sync);


    update_menus ();


    if (! audio_hard_sync)
    {
        gui_message (GUI_TEXT_COLOR, "Audio hard synchronization disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Audio hard synchronization enabled.");
    }


    return (D_O_K);
}


static int options_audio_record_menu_start (void)
{
    if (papu_start_record () == 0)
    {
        DISABLE_MENU (options_audio_record_menu, 0);

        ENABLE_MENU (options_audio_record_menu, 2);
    }


    gui_message (GUI_TEXT_COLOR, "Audio recording session started.");


    return (D_O_K);
}


static int options_audio_record_menu_stop (void)
{
    papu_stop_record ();


    ENABLE_MENU (options_audio_record_menu, 0);

    DISABLE_MENU (options_audio_record_menu, 2);


    gui_message (GUI_TEXT_COLOR, "Audio recording session stopped.");


    return (D_O_K);
}


#define DRIVER_MENU_HANDLER(system, driver, id)   \
    static int options_video_driver_##system##_menu_##driver (void)   \
    {                                           \
        video_set_driver (id);                  \
                                                \
        gui_needs_restart = TRUE;               \
                                                \
                                                \
        return (D_CLOSE);                       \
    }


#ifdef ALLEGRO_DOS

DRIVER_MENU_HANDLER (dos, vga, GFX_VGA)

DRIVER_MENU_HANDLER (dos, vga_mode_x, GFX_MODEX)

DRIVER_MENU_HANDLER (dos, vesa, GFX_VESA1)

DRIVER_MENU_HANDLER (dos, vesa_2_banked, GFX_VESA2B)

DRIVER_MENU_HANDLER (dos, vesa_2_linear, GFX_VESA2L)

DRIVER_MENU_HANDLER (dos, vesa_3, GFX_VESA3)

DRIVER_MENU_HANDLER (dos, vesa_vbe_af, GFX_VBEAF)

#endif


#ifdef ALLEGRO_WINDOWS

DRIVER_MENU_HANDLER (windows, directx, GFX_DIRECTX)

DRIVER_MENU_HANDLER (windows, directx_window, GFX_DIRECTX_WIN)

DRIVER_MENU_HANDLER (windows, directx_overlay, GFX_DIRECTX_OVL)

DRIVER_MENU_HANDLER (windows, gdi, GFX_GDI)

#endif


#ifdef ALLEGRO_LINUX

DRIVER_MENU_HANDLER (linux, vga, GFX_VGA)

DRIVER_MENU_HANDLER (linux, vga_mode_x, GFX_MODEX)

DRIVER_MENU_HANDLER (linux, vesa_vbe_af, GFX_VBEAF)

#ifdef GFX_FBCON

DRIVER_MENU_HANDLER (linux, framebuffer, GFX_FBCON)

#else

DRIVER_MENU_HANDLER (linux, framebuffer, NIL)

#endif

#ifdef GFX_SVGALIB

DRIVER_MENU_HANDLER (linux, svgalib, GFX_SVGALIB)

#else

DRIVER_MENU_HANDLER (linux, svgalib, NIL)

#endif

#endif


#ifdef ALLEGRO_UNIX

DRIVER_MENU_HANDLER (unix, x_windows, GFX_XWINDOWS)

DRIVER_MENU_HANDLER (unix, x_windows_full, GFX_XWINDOWS_FULLSCREEN)

DRIVER_MENU_HANDLER (unix, x_dga, GFX_XDGA)

DRIVER_MENU_HANDLER (unix, x_dga_full, GFX_XDGA_FULLSCREEN)

DRIVER_MENU_HANDLER (unix, x_dga_2, GFX_XDGA2)

#endif


static int options_video_driver_menu_automatic (void)
{
    video_set_driver (GFX_AUTODETECT);

    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


#define RESOLUTION_MENU_HANDLER(width, height)         \
    static int options_video_resolution_menu_##width##_##height (void)    \
    {                                           \
        video_set_resolution (width, height);   \
                                                \
        gui_needs_restart = TRUE;               \
                                                \
                                                \
        return (D_CLOSE);                       \
    }


#define RESOLUTION_MENU_HANDLER_EX(type, width, height)     \
    static int options_video_resolution_##type##_menu_##width##_##height (void)     \
    {                                           \
        video_set_resolution (width, height);   \
                                                \
        gui_needs_restart = TRUE;               \
                                                \
                                                \
        return (D_CLOSE);                       \
    }


RESOLUTION_MENU_HANDLER_EX (proportionate, 256, 224)

RESOLUTION_MENU_HANDLER_EX (proportionate, 256, 240)

RESOLUTION_MENU_HANDLER_EX (proportionate, 512, 448)

RESOLUTION_MENU_HANDLER_EX (proportionate, 512, 480)

RESOLUTION_MENU_HANDLER_EX (proportionate, 768, 672)

RESOLUTION_MENU_HANDLER_EX (proportionate, 768, 720)

RESOLUTION_MENU_HANDLER_EX (proportionate, 1024, 896)

RESOLUTION_MENU_HANDLER_EX (proportionate, 1024, 960)

RESOLUTION_MENU_HANDLER_EX (proportionate, 1280, 1120)

RESOLUTION_MENU_HANDLER_EX (proportionate, 1280, 1200)


RESOLUTION_MENU_HANDLER (320, 240)

RESOLUTION_MENU_HANDLER (640, 480)

RESOLUTION_MENU_HANDLER (800, 600)

RESOLUTION_MENU_HANDLER (1024, 768)

RESOLUTION_MENU_HANDLER (1152, 864)

RESOLUTION_MENU_HANDLER (1280, 1024)

RESOLUTION_MENU_HANDLER (1600, 1200)


RESOLUTION_MENU_HANDLER_EX (extended, 400, 300)

RESOLUTION_MENU_HANDLER_EX (extended, 480, 360)

RESOLUTION_MENU_HANDLER_EX (extended, 512, 384)

RESOLUTION_MENU_HANDLER_EX (extended, 640, 400)

RESOLUTION_MENU_HANDLER_EX (extended, 720, 480)

RESOLUTION_MENU_HANDLER_EX (extended, 720, 576)

RESOLUTION_MENU_HANDLER_EX (extended, 848, 480)

RESOLUTION_MENU_HANDLER_EX (extended, 1280, 720)

RESOLUTION_MENU_HANDLER_EX (extended, 1280, 960)

RESOLUTION_MENU_HANDLER_EX (extended, 1360, 768)


static int options_video_colors_menu_paletted_8_bit (void)
{
    video_set_color_depth (8);


    update_colors ();


    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_video_colors_menu_true_color_15_bit (void)
{
    video_set_color_depth (15);


    update_colors ();


    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_video_colors_menu_true_color_16_bit (void)
{
    video_set_color_depth (16);


    update_colors ();


    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_video_colors_menu_true_color_32_bit (void)
{
    video_set_color_depth (32);


    update_colors ();


    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_video_blitter_menu_automatic (void)
{
    video_set_blitter (VIDEO_BLITTER_AUTOMATIC);
    
    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to automatic.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_normal (void)
{
    video_set_blitter (VIDEO_BLITTER_NORMAL);
    
    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to normal.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_stretched (void)
{
    video_set_blitter (VIDEO_BLITTER_STRETCHED);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to stretched.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_interpolated_2x (void)
{
    video_set_blitter (VIDEO_BLITTER_INTERPOLATED_2X);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to interpolated (2x).");


    return (D_REDRAW);
}


static int options_video_blitter_menu_interpolated_3x (void)
{
    video_set_blitter (VIDEO_BLITTER_INTERPOLATED_3X);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to interpolated (3x).");


    return (D_REDRAW);
}


static int options_video_blitter_menu_2xsoe (void)
{
    video_set_blitter (VIDEO_BLITTER_2XSOE);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to 2xSOE engine.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_2xscl (void)
{
    video_set_blitter (VIDEO_BLITTER_2XSCL);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to 2xSCL engine.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_super_2xsoe (void)
{
    video_set_blitter (VIDEO_BLITTER_SUPER_2XSOE);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to super 2xSOE engine.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_super_2xscl (void)
{
    video_set_blitter (VIDEO_BLITTER_SUPER_2XSCL);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to super 2xSCL engine.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_ultra_2xscl (void)
{
    video_set_blitter (VIDEO_BLITTER_ULTRA_2XSCL);

    update_menus ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video blitter set to ultra 2xSCL engine.");


    return (D_REDRAW);
}


static int options_video_filters_menu_scanlines_25_percent (void)
{
    int filters;


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


    clear (screen);

    video_blit (screen);


    draw_background ();


    if (! (filters & VIDEO_FILTER_SCANLINES_LOW))
    {
        gui_message (GUI_TEXT_COLOR, "Scanlines video filter disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Scanlines video filter enabled (25%%).");
    }


    return (D_REDRAW);
}


static int options_video_filters_menu_scanlines_50_percent (void)
{
    int filters;


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


    clear (screen);

    video_blit (screen);


    draw_background ();


    if (! (filters & VIDEO_FILTER_SCANLINES_MEDIUM))
    {
        gui_message (GUI_TEXT_COLOR, "Scanlines video filter disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Scanlines video filter enabled (50%%).");
    }


    return (D_REDRAW);
}


static int options_video_filters_menu_scanlines_100_percent (void)
{
    int filters;


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


    clear (screen);

    video_blit (screen);


    draw_background ();


    if (! (filters & VIDEO_FILTER_SCANLINES_HIGH))
    {
        gui_message (GUI_TEXT_COLOR, "Scanlines video filter disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Scanlines video filter enabled (100%%%).");
    }


    return (D_REDRAW);
}


static int options_video_menu_vsync (void)
{
    video_enable_vsync = (! video_enable_vsync);

    update_menus ();


    if (! video_enable_vsync)
    {
        gui_message (GUI_TEXT_COLOR, "VSync disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "VSync enabled.");
    }


    return (D_O_K);
}


static int options_video_layers_menu_sprites_a (void)
{
    ppu_enable_sprite_layer_a = (! ppu_enable_sprite_layer_a);

    update_menus ();


    if (! ppu_enable_sprite_layer_a)
    {
        gui_message (GUI_TEXT_COLOR, "Video sprites layer A disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Video sprites layer A enabled.");
    }


    return (D_O_K);
}


static int options_video_layers_menu_sprites_b (void)
{
    ppu_enable_sprite_layer_b = (! ppu_enable_sprite_layer_b);

    update_menus ();


    if (! ppu_enable_sprite_layer_b)
    {
        gui_message (GUI_TEXT_COLOR, "Video sprites layer B disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Video sprites layer B enabled.");
    }


    return (D_O_K);
}


static int options_video_layers_menu_background (void)
{
    ppu_enable_background_layer = (! ppu_enable_background_layer);

    update_menus ();


    if (! ppu_enable_background_layer)
    {
        gui_message (GUI_TEXT_COLOR, "Video background layer disabled.");
    }
    else
    {
        gui_message (GUI_TEXT_COLOR, "Video background layer enabled.");
    }


    return (D_O_K);
}


static int options_video_palette_menu_ntsc_color (void)
{
    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 0);


    video_set_palette (DATA_TO_RGB (DEFAULT_PALETTE));

    current_palette = DATA_TO_RGB (DEFAULT_PALETTE);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to NTSC color.");


    return (D_REDRAW);
}


static int options_video_palette_menu_ntsc_grayscale (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 2);


    video_set_palette (DATA_TO_RGB (GRAYSCALE_PALETTE));

    current_palette = DATA_TO_RGB (GRAYSCALE_PALETTE);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to NTSC grayscale.");


    return (D_REDRAW);
}

static int options_video_palette_menu_gnuboy (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 4);


    video_set_palette (DATA_TO_RGB (GNUBOY_PALETTE));

    current_palette = DATA_TO_RGB (GNUBOY_PALETTE);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to gnuboy.");


    return (D_REDRAW);
}


static int options_video_palette_menu_nester (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 6);


    video_set_palette (DATA_TO_RGB (NESTER_PALETTE));

    current_palette = DATA_TO_RGB (NESTER_PALETTE);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to NESter.");


    return (D_REDRAW);
}


static int options_video_palette_menu_nesticle (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 8);


    video_set_palette (DATA_TO_RGB (NESTICLE_PALETTE));

    current_palette = DATA_TO_RGB (NESTICLE_PALETTE);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to NESticle.");


    return (D_REDRAW);
}


static int options_video_palette_menu_modern_ntsc (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 10);


    video_set_palette (DATA_TO_RGB (MODERN_NTSC_PALETTE));

    current_palette = DATA_TO_RGB (MODERN_NTSC_PALETTE);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to modern NTSC.");


    return (D_REDRAW);
}


static int options_video_palette_menu_modern_pal (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 12);


    video_set_palette (DATA_TO_RGB (MODERN_PAL_PALETTE));

    current_palette = DATA_TO_RGB (MODERN_PAL_PALETTE);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to modern PAL.");


    return (D_REDRAW);
}


static int options_video_palette_menu_ega_mode_1 (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 16);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 14);


    video_set_palette (DATA_TO_RGB (EGA_PALETTE_1));

    current_palette = DATA_TO_RGB (EGA_PALETTE_1);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to EGA (mode 1).");


    return (D_REDRAW);
}


static int options_video_palette_menu_ega_mode_2 (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);

    UNCHECK_MENU (options_video_palette_menu, 12);

    UNCHECK_MENU (options_video_palette_menu, 14);

    UNCHECK_MENU (options_video_palette_menu, 18);


    CHECK_MENU (options_video_palette_menu, 16);


    video_set_palette (DATA_TO_RGB (EGA_PALETTE_2));

    current_palette = DATA_TO_RGB (EGA_PALETTE_2);


    update_colors ();


    clear (screen);

    video_blit (screen);


    draw_background ();


    gui_message (GUI_TEXT_COLOR, "Video palette set to EGA (mode 2).");


    return (D_REDRAW);
}


static int options_video_palette_menu_custom (void)
{
    FILE * palette_file;

    int index;


    palette_file = fopen ("fakenes.pal", "rb");

    if (palette_file)
    {
        custom_palette [0].r = custom_palette [0].g = custom_palette [0].b = 0;


        for (index = 1; index <= 64; index ++)
        {
            custom_palette [index].r = (fgetc (palette_file) / 4);

            custom_palette [index].g = (fgetc (palette_file) / 4);

            custom_palette [index].b = (fgetc (palette_file) / 4);
        }


        fclose (palette_file);


        UNCHECK_MENU (options_video_palette_menu, 0);
    
        UNCHECK_MENU (options_video_palette_menu, 2);
    
        UNCHECK_MENU (options_video_palette_menu, 4);

        UNCHECK_MENU (options_video_palette_menu, 6);

        UNCHECK_MENU (options_video_palette_menu, 8);

        UNCHECK_MENU (options_video_palette_menu, 10);

        UNCHECK_MENU (options_video_palette_menu, 12);
    
        UNCHECK_MENU (options_video_palette_menu, 14);

        UNCHECK_MENU (options_video_palette_menu, 16);
    

        CHECK_MENU (options_video_palette_menu, 18);
    

        video_set_palette (((RGB *) custom_palette));
    
        current_palette = ((RGB *) &custom_palette);


        clear (screen);
    
        video_blit (screen);
    

        draw_background ();


        gui_message (GUI_TEXT_COLOR, "Video palette set to custom.");
    
    
        return (D_REDRAW);
    }
    else
    {
        gui_message (GUI_ERROR_COLOR, "Error opening FAKENES.PAL!");
    }


    return (D_O_K);
}


static int options_video_advanced_menu_force_window (void)
{
    video_force_window = (! video_force_window);


    video_reinit ();

    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_menu_input (void)
{
    gui_show_dialog (options_input_dialog);


    return (D_O_K);
}


static int help_menu_shortcuts (void)
{
    gui_show_dialog (help_shortcuts_dialog);


    return (D_O_K);
}


static int help_menu_about (void)
{
    gui_show_dialog (help_about_dialog);


    return (D_O_K);
}


/* ---- Dialog handlers. ---- */


static int options_patches_dialog_list (DIALOG * dialog)
{
    CPU_PATCH * patch;


    if (cpu_patch_count == 0)
    {
        return (D_O_K);
    }


    patch = &cpu_patch_info [dialog -> d1];


    if (patch -> enabled)
    {
        options_patches_dialog [5].flags |= D_SELECTED;
    }
    else
    {
        options_patches_dialog [5].flags &= ~D_SELECTED;
    }


    scare_mouse ();

    object_message (&options_patches_dialog [5], MSG_DRAW, 0);

    unscare_mouse ();


    return (D_O_K);
}


static int options_patches_dialog_add (DIALOG * dialog)
{
    UINT8 buffer [16];

    UINT8 buffer2 [11];


    CPU_PATCH * patch;


    UINT8 value;


    if (cpu_patch_count >= MAX_PATCHES)
    {
        alert ("- Error -", NIL, "The patch list is already full.", "&OK", NIL, 'o', 0);


        return (D_O_K);
    }


    memset (buffer, NIL, sizeof (buffer));

    memset (buffer2, NIL, sizeof (buffer2));


    options_patches_add_dialog [4].d1 = (sizeof (buffer) - 1);

    options_patches_add_dialog [4].dp = buffer;


    options_patches_add_dialog [7].d1 = (sizeof (buffer2) - 1);

    options_patches_add_dialog [7].dp = buffer2;


    if (gui_show_dialog (options_patches_add_dialog) != 8)
    {
        return (D_O_K);
    }


    patch = &cpu_patch_info [cpu_patch_count];


    if (genie_decode (buffer2, &patch -> address, &patch -> value, &patch -> match_value) != 0)
    {
        alert ("- Error -", NIL, "You must enter a valid Game Genie (or NESticle raw) code.", "&OK", NIL, 'o', 0);


        return (D_O_K);
    }


    if (patch -> title)
    {
        sprintf (patch -> title, "%s", buffer);
    }


    patch -> enabled = TRUE;


    cpu_patch_count ++;


    value = cpu_read (patch -> address);


    if (value == patch -> match_value)
    {
        /* Enable patch. */

        patch -> active = TRUE;


        cpu_patch_table [patch -> address] = (patch -> value - value);
    }


    return (D_REDRAW);
}


static int options_patches_dialog_remove (DIALOG * dialog)
{
    int index;


    CPU_PATCH * source;

    CPU_PATCH * target;


    int start;


    if (cpu_patch_count == 0)
    {
        return (D_O_K);
    }


    start = options_patches_dialog [2].d1;


    source = &cpu_patch_info [start];


    /* Disable patch. */

    if (source -> active)
    {
        if (alert ("Confirmation", NIL, "Really deactivate and remove this patch?", "&OK", "&Cancel", 'o', 'c') == 2)
        {
            return (D_O_K);
        }


        cpu_patch_table [source -> address] = 0;
    }


    for (index = (start + 1); index < cpu_patch_count; index ++)
    {
        source = &cpu_patch_info [index];

        target = &cpu_patch_info [index - 1];


        target -> active = source -> active;


        target -> address = source -> address;


        target -> value = source -> value;

        target -> match_value = source -> value;


        target -> enabled = source -> enabled;


        sprintf (target -> title, "%s", source -> title);
    }


    source = &cpu_patch_info [cpu_patch_count - 1];


    source -> active = FALSE;


    source -> address = 0x0000;


    source -> value = 0x00;

    source -> match_value = 0x00;


    source -> enabled = FALSE;


    sprintf (source -> title, "?");


    cpu_patch_count --;


    if (cpu_patch_count == 0)
    {
        options_patches_dialog [5].flags &= ~D_SELECTED;
    }


    return (D_REDRAW);
}


static int options_patches_dialog_enabled (DIALOG * dialog)
{
    CPU_PATCH * patch;


    if (cpu_patch_count == 0)
    {
        dialog -> flags &= ~D_SELECTED;


        return (D_O_K);
    }


    patch = &cpu_patch_info [options_patches_dialog [2].d1];


    patch -> enabled = (dialog -> flags & D_SELECTED);


    /* Toggle patch. */

    if ((! patch -> enabled) && (patch -> active))
    {
        patch -> active = FALSE;


        cpu_patch_table [patch -> address] = 0;
    }
    else if ((patch -> enabled) && (! patch -> active))
    {
        UINT8 value;


        value = cpu_read (patch -> address);
    
    
        if (value == patch -> match_value)
        {
            /* Enable patch. */
    
            patch -> active = TRUE;
    
    
            cpu_patch_table [patch -> address] = (patch -> value - value);
        }
    }


    scare_mouse ();

    object_message (&options_patches_dialog [2], MSG_DRAW, 0);

    unscare_mouse ();


    return (D_O_K);
}


static UINT8 options_patches_dialog_list_texts [MAX_PATCHES] [38];


static char * options_patches_dialog_list_filler (int index, int * list_size)
{
    if (index >= 0)
    {
        CPU_PATCH * patch;


        memset (options_patches_dialog_list_texts [index], NIL, 38);


        patch = &cpu_patch_info [index];


        sprintf (options_patches_dialog_list_texts [index], "$%04x -$%02x +$%02x %s ", patch -> address,
            patch -> match_value, patch -> value, (patch -> active ? "Active" : " Idle "));


        if (patch -> title)
        {
            strncat (options_patches_dialog_list_texts [index], patch -> title, 37);
        }
        else
        {
            strcat (options_patches_dialog_list_texts [index], "No title");
        }


        return (options_patches_dialog_list_texts [index]);
    }
    else
    {
        *list_size = cpu_patch_count;


        return (NIL);
    }
}


static int selected_player = -1;


static int selected_player_device = 0;


static int options_input_dialog_player_select (DIALOG * dialog)
{
    selected_player = (dialog -> d2 - 1);


    selected_player_device = input_get_player_device (selected_player);


    options_input_dialog [8].flags &= ~D_SELECTED;

    options_input_dialog [9].flags &= ~D_SELECTED;

    options_input_dialog [10].flags &= ~D_SELECTED;

    options_input_dialog [11].flags &= ~D_SELECTED;


    options_input_dialog [(7 + selected_player_device)].flags |= D_SELECTED;


    scare_mouse ();

    object_message (&options_input_dialog [8], MSG_DRAW, 0);

    object_message (&options_input_dialog [9], MSG_DRAW, 0);

    object_message (&options_input_dialog [10], MSG_DRAW, 0);

    object_message (&options_input_dialog [11], MSG_DRAW, 0);

    unscare_mouse ();


    return (D_O_K);
}


static int options_input_dialog_device_select (DIALOG * dialog)
{
    if (selected_player < 0)
    {
        alert ("- Error -", "", "Please select a player to modify first.", "&OK", NIL, 'o', 0);


        return (D_O_K);
    }


    selected_player_device = dialog -> d2;


    input_set_player_device (selected_player, selected_player_device);


    return (D_O_K);
}


static int options_input_dialog_set_buttons (DIALOG * dialog)
{
    int button;


    int index;


    if (selected_player < 0)
    {
        alert ("- Error -", "", "Please select a player to modify first.", "&OK", NIL, 'o', 0);


        return (D_O_K);
    }


    button = dialog -> d2;


    if ((selected_player_device == INPUT_DEVICE_JOYSTICK_1) || (selected_player_device == INPUT_DEVICE_JOYSTICK_2))
    {
        if ((button == INPUT_DEVICE_BUTTON_UP) || (button == INPUT_DEVICE_BUTTON_DOWN) ||
            (button == INPUT_DEVICE_BUTTON_LEFT) || (button == INPUT_DEVICE_BUTTON_RIGHT))
        {
            alert ("- Error -", "", "Unable to set direction buttons for joystick devices.", "&OK", NIL, 'o', 0);
    
    
            return (D_O_K);
        }
    }
        

    switch (selected_player_device)
    {
        case INPUT_DEVICE_KEYBOARD_1:

        case INPUT_DEVICE_KEYBOARD_2:

            gui_message (GUI_TEXT_COLOR, "Press any key.");
    
    
            clear_keybuf ();
    
            while (! keypressed ());
    
    
            index = (readkey () >> 8);
    
    
            input_map_device_button (selected_player_device, button, index);
    
    
            gui_message (GUI_TEXT_COLOR, "Button mapped to scancode %d.", index);


            break;


        case INPUT_DEVICE_JOYSTICK_1:

            gui_message (GUI_TEXT_COLOR, "Press any button on joystick #1.");
    
    
            clear_keybuf ();
    
    
            for (;;)
            {
                poll_joystick ();
    
    
                for (index = 0; index < joy [0].num_buttons; index ++)
                {
                    if (joy [0].button [index].b)
                    {
                        input_map_device_button (selected_player_device, button, index);
                
                
                        gui_message (GUI_TEXT_COLOR, "Button mapped to joystick #1 button %d.", index);
    
    
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
            }


            break;


        case INPUT_DEVICE_JOYSTICK_2:

            gui_message (GUI_TEXT_COLOR, "Press any button on joystick #2.");
    
    
            clear_keybuf ();
    
    
            for (;;)
            {
                poll_joystick ();
    
    
                for (index = 0; index < joy [1].num_buttons; index ++)
                {
                    if (joy [1].button [index].b)
                    {
                        input_map_device_button (selected_player_device, button, index);
                
                
                        gui_message (GUI_TEXT_COLOR, "Button mapped to joystick #1 button %d.", index);
    
    
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
            }


            break;


        default:

            break;
    }


    return (D_O_K);
}


static int options_menu_patches (void)
{
    if (gui_show_dialog (options_patches_dialog) == 6)
    {
        patches_save (global_rom.filename);
    }


    return (D_O_K);
}


static int netplay_protocol_menu_tcpip (void)
{
    netplay_protocol = NETPLAY_PROTOCOL_TCPIP;

    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Netplay protocol set to TCP/IP.");


    return (D_O_K);
}


static int netplay_protocol_menu_spx (void)
{
    netplay_protocol = NETPLAY_PROTOCOL_SPX;

    update_menus ();


    gui_message (GUI_TEXT_COLOR, "Netplay protocol set to SPX.");


    return (D_O_K);
}


static int netplay_server_menu_start (void)
{
    if (netplay_open_server () != 0)
    {
        gui_message (GUI_ERROR_COLOR, "Failed to start the netplay server!");
    }


    DISABLE_MENU (top_menu, 0);
    
    DISABLE_MENU (top_menu, 1);

    DISABLE_MENU (top_menu, 3);


    DISABLE_MENU (netplay_menu, 0);

    DISABLE_MENU (netplay_menu, 4);


    DISABLE_MENU (netplay_server_menu, 0);


    ENABLE_MENU (netplay_server_menu, 2);


    gui_message (GUI_TEXT_COLOR, "Started NetPlay server, awaiting client.");


    return (D_REDRAW);
}


static int netplay_server_menu_stop (void)
{
    netplay_close_server ();


    DISABLE_MENU (netplay_server_menu, 2);


    ENABLE_MENU (top_menu, 0);
    
    ENABLE_MENU (top_menu, 1);

    ENABLE_MENU (top_menu, 3);


    ENABLE_MENU (netplay_menu, 0);

    ENABLE_MENU (netplay_menu, 4);


    ENABLE_MENU (netplay_server_menu, 0);


    gui_message (GUI_TEXT_COLOR, "Stopped NetPlay server.");


    return (D_REDRAW);
}


static int netplay_client_menu_connect (void)
{
    if (netplay_protocol == NETPLAY_PROTOCOL_TCPIP)
    {
        UINT8 buffer [16];
    
    
        memset (buffer, NIL, sizeof (buffer));


        netplay_client_connect_dialog [4].d1 = (sizeof (buffer) - 1);
    
        netplay_client_connect_dialog [4].dp = buffer;
    
    
        strcat (buffer, get_config_string ("netplay", "ip_address", "0.0.0.0"));
    
    
        if (gui_show_dialog (netplay_client_connect_dialog) != 5)
        {
            return (D_O_K);
        }


        if (netplay_open_client (buffer) != 0)
        {
            gui_message (GUI_ERROR_COLOR, "Failed to connect to the server!");


            return (D_O_K);
        }


        set_config_string ("netplay", "ip_address", buffer);
    }
    else
    {
        if (netplay_open_client (NIL) != 0)
        {
            gui_message (GUI_ERROR_COLOR, "Failed to connect to the server!");


            return (D_O_K);
        }
    }


    DISABLE_MENU (top_menu, 0);
    
    DISABLE_MENU (top_menu, 1);

    DISABLE_MENU (top_menu, 3);


    DISABLE_MENU (netplay_menu, 0);

    DISABLE_MENU (netplay_menu, 2);


    DISABLE_MENU (netplay_client_menu, 0);


    ENABLE_MENU (netplay_client_menu, 2);


    gui_message (GUI_TEXT_COLOR, "NetPlay client connected to the server.");


    return (D_O_K);
}


static int netplay_client_menu_disconnect (void)
{
    netplay_close_client ();


    DISABLE_MENU (netplay_client_menu, 2);


    ENABLE_MENU (top_menu, 0);
    
    ENABLE_MENU (top_menu, 1);

    ENABLE_MENU (top_menu, 3);


    ENABLE_MENU (netplay_menu, 0);

    ENABLE_MENU (netplay_menu, 2);


    ENABLE_MENU (netplay_client_menu, 0);


    gui_message (GUI_TEXT_COLOR, "NetPlay client disconnected from the server.");


    return (D_O_K);
}
