

/*

FakeNES - A portable, Open Source NES emulator.

gui.c: Implementation of the object-based GUI.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "apu.h"

#include "audio.h"

#include "cpu.h"

#include "gui.h"

#include "papu.h"

#include "ppu.h"

#include "rom.h"

#include "video.h"


#include "data.h"

#include "misc.h"


#include "netplay.h"


#include "timing.h"


static int dialog_x = 0;

static int dialog_y = 0;


static int restart_dialog = FALSE;


/* Keep these in order! */

#include "gui/objects.h"

#include "gui/menus.h"

#include "gui/dialogs.h"


int gui_needs_restart = FALSE;

int gui_is_active = FALSE;


static int want_exit = FALSE;


static UINT8 message_buffer [256];


static RGB * current_palette = NULL;

static PALETTE custom_palette;


static FILE * log_file = NULL;


static int shadow_color = 0;

static int error_color = 0;


static void update_colors (void)
{
    gui_bg_color = makecol (127, 127, 127);

    gui_fg_color = makecol (255, 255, 255);


    gui_mg_color = makecol (191, 191, 191);


    shadow_color = makecol (0, 0, 0);


    error_color  = makecol (255, 63, 0);
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


    vline (screen, (x2 + 1), (y + 1), (y2 + 1), shadow_color);

    hline (screen, (x + 1), (y2 + 1), (x2 + 1), shadow_color);


    rectfill (screen, x, y, x2, y2, gui_bg_color);


    rect (screen, x, y, x2, y2, gui_fg_color);
}


void gui_message (int color, AL_CONST UINT8 * message, ...)
{
    va_list format;


    va_start (format, message);

    vsprintf (message_buffer, message, format);

    va_end (format);


    gui_message_border ();


    textout_centre (screen, font, message_buffer, (SCREEN_W / 2), ((SCREEN_H - 19) - text_height (font)), 0);

    textout_centre (screen, font, message_buffer, ((SCREEN_W / 2) - 1), (((SCREEN_H - 19) - text_height (font)) - 1), color);


    if (log_file)
    {
        fprintf (log_file, "GUI: %s\n", message_buffer);
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


void gui_spawn_file_menu_snapshot (void)
{
    main_menu_snapshot ();
}


void gui_spawn_machine_menu_status (void)
{
    machine_menu_status ();
}


void gui_spawn_machine_state_menu_save (void)
{
    machine_state_menu_save ();
}


void gui_spawn_machine_state_menu_restore (void)
{
    machine_state_menu_restore ();
}


void gui_spawn_options_video_layers_menu_sprites_a (void)
{
    options_video_layers_menu_sprites_a ();
}


void gui_spawn_options_video_layers_menu_sprites_b (void)
{
    options_video_layers_menu_sprites_b ();
}


void gui_spawn_options_video_layers_menu_background (void)
{
    options_video_layers_menu_background ();
}


int gui_show_dialog (DIALOG * dialog)
{
    BITMAP * saved;


    int index = 0;


    saved = create_bitmap (SCREEN_W, SCREEN_H);


    scare_mouse ();

    blit (screen, saved, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    unscare_mouse ();


    centre_dialog (dialog);


    dialog [0].dp3 = DATA_LARGE_FONT;


    while (dialog [index].d1 != SL_FRAME_END)
    {
        if ((dialog [index].proc == sl_text) || (dialog [index].proc == sl_frame))
        {
            dialog [index].dp = screen;
        }


        dialog [index].fg = gui_fg_color;

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


        position_dialog (dialog, dialog_x, dialog_y);


        goto again;
    }


    destroy_bitmap (saved);


    return (index);
}


static INLINE void update_menus (void)
{
    if (! audio_pseudo_stereo)
    {
        papu_surround_sound = FALSE;


        DISABLE_MENU (options_audio_effects_menu, 2);
    }
    else
    {
        ENABLE_MENU (options_audio_effects_menu, 2);
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


    /* Yuck. */

    if (video_get_color_depth () == 8)
    {
        int filters;


        filters = video_get_filter_list ();


        filters &= ~VIDEO_FILTER_SCANLINES_LOW;

        filters &= ~VIDEO_FILTER_SCANLINES_MEDIUM;


        video_set_filter_list (filters);


        DISABLE_MENU (options_video_filters_scanlines_menu, 2);

        DISABLE_MENU (options_video_filters_scanlines_menu, 4);


        if (video_get_blitter () == VIDEO_BLITTER_SUPER_2XSCL)
        {
            video_set_blitter (VIDEO_BLITTER_NORMAL);
        }


        ENABLE_MENU (options_video_blitter_menu, 4);

        ENABLE_MENU (options_video_blitter_menu, 6);


        DISABLE_MENU (options_video_blitter_menu, 8);
    }
    else
    {
        if ((video_get_blitter () == VIDEO_BLITTER_2XSOE) || (video_get_blitter () == VIDEO_BLITTER_2XSCL))
        {
            video_set_blitter (VIDEO_BLITTER_NORMAL);
        }


        DISABLE_MENU (options_video_blitter_menu, 4);

        DISABLE_MENU (options_video_blitter_menu, 6);


        ENABLE_MENU (options_video_blitter_menu, 8);


        ENABLE_MENU (options_video_filters_scanlines_menu, 2);

        ENABLE_MENU (options_video_filters_scanlines_menu, 4);
    }


    TOGGLE_MENU (machine_speed_menu, 0, (machine_type == MACHINE_TYPE_NTSC));

    TOGGLE_MENU (machine_speed_menu, 2, (machine_type == MACHINE_TYPE_PAL));


    TOGGLE_MENU (machine_menu, 2, video_display_status);


    TOGGLE_MENU (netplay_protocol_menu, 0, (netplay_protocol == NETPLAY_PROTOCOL_TCPIP));

    TOGGLE_MENU (netplay_protocol_menu, 2, (netplay_protocol == NETPLAY_PROTOCOL_SPX));


    TOGGLE_MENU (options_audio_menu, 0, audio_enable_output);


    TOGGLE_MENU (options_audio_mixing_menu, 0, (! audio_pseudo_stereo));


    TOGGLE_MENU (options_audio_mixing_stereo_menu, 0, (audio_pseudo_stereo == 1));

    TOGGLE_MENU (options_audio_mixing_stereo_menu, 2, (audio_pseudo_stereo == 2));

    TOGGLE_MENU (options_audio_mixing_stereo_menu, 4, (audio_pseudo_stereo == 3));


    TOGGLE_MENU (options_audio_mixing_quality_menu, 0, (audio_sample_size == 8));

    TOGGLE_MENU (options_audio_mixing_quality_menu, 2, (audio_sample_size == 16));

    TOGGLE_MENU (options_audio_mixing_quality_menu, 4, papu_dithering);


    TOGGLE_MENU (options_audio_effects_menu, 0, papu_linear_echo);

    TOGGLE_MENU (options_audio_effects_menu, 2, papu_surround_sound);


    TOGGLE_MENU (options_audio_filter_menu, 0, (papu_filter_type == APU_FILTER_NONE));

    TOGGLE_MENU (options_audio_filter_menu, 4, (papu_filter_type == APU_FILTER_HIGHPASS));


    TOGGLE_MENU (options_audio_filter_low_pass_menu, 0, (papu_filter_type == APU_FILTER_LOWPASS));

    TOGGLE_MENU (options_audio_filter_low_pass_menu, 2, (papu_filter_type == APU_FILTER_WEIGHTED));

    TOGGLE_MENU (options_audio_filter_low_pass_menu, 4, (papu_filter_type == APU_FILTER_DYNAMIC));


    TOGGLE_MENU (options_audio_channels_menu, 0, papu_enable_square_1);

    TOGGLE_MENU (options_audio_channels_menu, 2, papu_enable_square_2);

    TOGGLE_MENU (options_audio_channels_menu, 4, papu_enable_triangle);

    TOGGLE_MENU (options_audio_channels_menu, 6, papu_enable_noise);

    TOGGLE_MENU (options_audio_channels_menu, 8, papu_enable_dmc);

    TOGGLE_MENU (options_audio_channels_menu, 10, papu_enable_exsound);


    TOGGLE_MENU (options_audio_advanced_menu, 0, papu_ideal_triangle);


    TOGGLE_MENU (options_video_resolution_menu, 0, ((SCREEN_W == 256) && (SCREEN_H == 224)));

    TOGGLE_MENU (options_video_resolution_menu, 2, ((SCREEN_W == 256) && (SCREEN_H == 240)));

    TOGGLE_MENU (options_video_resolution_menu, 4, ((SCREEN_W == 320) && (SCREEN_H == 240)));

    TOGGLE_MENU (options_video_resolution_menu, 6, ((SCREEN_W == 400) && (SCREEN_H == 300)));

    TOGGLE_MENU (options_video_resolution_menu, 8, ((SCREEN_W == 512) && (SCREEN_H == 384)));

    TOGGLE_MENU (options_video_resolution_menu, 10, ((SCREEN_W == 640) && (SCREEN_H == 480)));


    TOGGLE_MENU (options_video_colors_menu, 0, (video_get_color_depth () == 8));

    TOGGLE_MENU (options_video_colors_menu, 2, (video_get_color_depth () == 15));

    TOGGLE_MENU (options_video_colors_menu, 4, (video_get_color_depth () == 16));


    TOGGLE_MENU (options_video_blitter_menu, 0, (video_get_blitter () == VIDEO_BLITTER_NORMAL));

    TOGGLE_MENU (options_video_blitter_menu, 2, (video_get_blitter () == VIDEO_BLITTER_STRETCHED));

    TOGGLE_MENU (options_video_blitter_menu, 4, (video_get_blitter () == VIDEO_BLITTER_2XSOE));

    TOGGLE_MENU (options_video_blitter_menu, 6, (video_get_blitter () == VIDEO_BLITTER_2XSCL));

    TOGGLE_MENU (options_video_blitter_menu, 8, (video_get_blitter () == VIDEO_BLITTER_SUPER_2XSCL));


    TOGGLE_MENU (options_video_filters_scanlines_menu, 0, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_HIGH));

    TOGGLE_MENU (options_video_filters_scanlines_menu, 2, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_MEDIUM));

    TOGGLE_MENU (options_video_filters_scanlines_menu, 4, (video_get_filter_list () & VIDEO_FILTER_SCANLINES_LOW));


    TOGGLE_MENU (options_video_menu, 8, video_enable_vsync);


    TOGGLE_MENU (options_video_advanced_menu, 0, video_force_window);


    TOGGLE_MENU (options_video_layers_menu, 0, ppu_enable_sprite_layer_a);

    TOGGLE_MENU (options_video_layers_menu, 2, ppu_enable_sprite_layer_b);

    TOGGLE_MENU (options_video_layers_menu, 4, ppu_enable_background_layer);
}


extern UINT8 logfile [256];


int show_gui (int first_run)
{
    time_t start;


    update_colors ();


    gui_needs_restart = FALSE;

    gui_is_active = TRUE;


    // gui_menu_opening_delay = 500;


    gui_menu_draw_menu = sl_draw_menu;

    gui_menu_draw_menu_item = sl_draw_menu_item;


#ifdef POSIX

    log_file = fopen (logfile, "w");
#else

    log_file = fopen ("messages.log", "w");
#endif


    if (log_file)
    {
        time (&start);


        fprintf (log_file, "\n--- %s", asctime (localtime (&start)));
    }


    update_menus ();


    if (! rom_is_loaded)
    {
        DISABLE_MENU (main_menu, 2);

        DISABLE_MENU (main_menu, 4);


        DISABLE_MENU (machine_menu, 0);
    }


    DISABLE_MENU (machine_state_menu, 2);

    DISABLE_MENU (machine_state_menu, 4);


#ifdef ALLEGRO_DOS

    DISABLE_MENU (options_video_advanced_menu, 0);


    DISABLE_MENU (top_menu, 3);
#endif


    audio_suspend ();


    video_blit (screen);


    gui_message (gui_fg_color, "GUI initialized (%dx%dx%s, %s).", SCREEN_W, SCREEN_H,
        ((video_get_color_depth () == 8) ? "256" : ((video_get_color_depth () == 15) ? "32K" : "64K")), gfx_driver -> name);


    set_mouse_sprite_focus (8, 8);

    unscare_mouse ();


    if (first_run)
    {
        alert ("FakeNES version 0.2.0 (final) " ALLEGRO_PLATFORM_STR, "", "Get "
            "the latest from http://fakenes.sourceforge.net/.", "&OK", NULL, 0, 0);
    }


    do_dialog (main_dialog, -1);


    scare_mouse ();


    gui_is_active = FALSE;


    audio_resume ();


    if (log_file)
    {
        fclose (log_file);


        log_file = NULL;
    }


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


            gui_message (gui_fg_color, "Accepted connection from client.");


            result = main_menu_load_rom ();

            if (result == D_CLOSE)
            {
                return (D_CLOSE);
            }
            else
            {
                gui_message (gui_fg_color, "NetPlay session canceled.");
            }
        }
    }


    return (D_O_K);
}


static int main_menu_load_rom (void)
{
    ROM test_rom;


    UINT8 buffer [256];

    UINT8 buffer2 [256];

    UINT8 buffer3 [256];


    memset (buffer, NULL, sizeof (buffer));

    memset (buffer2, NULL, sizeof (buffer2));

    memset (buffer3, NULL, sizeof (buffer3));


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
            gui_message (error_color, "Failed to load ROM!");


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


                free_rom (&global_rom);
            }


            memcpy (&global_rom, &test_rom, sizeof (ROM));


            rom_is_loaded = TRUE;
    
            machine_init ();


            if ((! netplay_server_active) && (! netplay_client_active))
            {
                ENABLE_MENU (main_menu, 2);
    
                ENABLE_MENU (main_menu, 4);
    
    
                ENABLE_MENU (machine_menu, 0);
    
    
                // ENABLE_MENU (machine_state_menu, 2);
    
                // ENABLE_MENU (machine_state_menu, 4);
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


static int main_menu_snapshot (void)
{
    int count;

    UINT8 filename [12];


    for (count = 0; count < 999; count ++)
    {
        sprintf (filename, "snap%03d.pcx", count);

        filename [11] = NULL;


        if (! exists (filename))
        {
            count = 1000;


            save_bitmap (filename, video_buffer, current_palette);

            gui_message (gui_fg_color, "Snapshot saved to %s.", filename);
        }
    }


    return (D_O_K);
}


static int main_menu_exit (void)
{
    want_exit = TRUE;


    return (D_CLOSE);
}


static int machine_menu_reset (void)
{
    machine_reset ();


    return (D_CLOSE);
}


static int machine_menu_status (void)
{
    video_display_status = (! video_display_status);

    update_menus ();


    return (D_O_K);
}


static int machine_speed_menu_ntsc_60_hz (void)
{
    machine_type = MACHINE_TYPE_NTSC;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (gui_fg_color, "Emulation speed set to NTSC (60 Hz).");


    return (D_O_K);
}


static int machine_speed_menu_pal_50_hz (void)
{
    machine_type = MACHINE_TYPE_PAL;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (gui_fg_color, "Emulation speed set to PAL (50 Hz).");


    return (D_O_K);
}


static int machine_state_menu_select (void)
{
    return (D_O_K);
}


static int machine_state_menu_save (void)
{
    return (D_O_K);
}


static int machine_state_menu_restore (void)
{
    return (D_O_K);
}


static int netplay_protocol_menu_tcpip (void)
{
    netplay_protocol = NETPLAY_PROTOCOL_TCPIP;

    update_menus ();


    gui_message (gui_fg_color, "Netplay protocol set to TCP/IP.");


    return (D_O_K);
}


static int netplay_protocol_menu_spx (void)
{
    netplay_protocol = NETPLAY_PROTOCOL_SPX;

    update_menus ();


    gui_message (gui_fg_color, "Netplay protocol set to SPX.");


    return (D_O_K);
}


static int netplay_server_menu_start (void)
{
    if (netplay_open_server () != 0)
    {
        gui_message (error_color, "Failed to start the netplay server!");
    }


    DISABLE_MENU (top_menu, 0);
    
    DISABLE_MENU (top_menu, 1);

    DISABLE_MENU (top_menu, 2);

    DISABLE_MENU (top_menu, 4);


    DISABLE_MENU (netplay_menu, 0);

    DISABLE_MENU (netplay_menu, 4);


    DISABLE_MENU (netplay_server_menu, 0);


    ENABLE_MENU (netplay_server_menu, 2);


    gui_message (gui_fg_color, "Started NetPlay server, awaiting client.");


    return (D_REDRAW);
}


static int netplay_server_menu_stop (void)
{
    netplay_close_server ();


    DISABLE_MENU (netplay_server_menu, 2);


    ENABLE_MENU (top_menu, 0);
    
    ENABLE_MENU (top_menu, 1);

    ENABLE_MENU (top_menu, 2);

    ENABLE_MENU (top_menu, 4);


    ENABLE_MENU (netplay_menu, 0);

    ENABLE_MENU (netplay_menu, 4);


    ENABLE_MENU (netplay_server_menu, 0);


    gui_message (gui_fg_color, "Stopped NetPlay server.");


    return (D_REDRAW);
}


static int netplay_client_menu_connect (void)
{
    if (netplay_protocol == NETPLAY_PROTOCOL_TCPIP)
    {
        UINT8 buffer [16];
    
    
        memset (buffer, NULL, sizeof (buffer));


        netplay_client_connect_dialog [4].d1 = (sizeof (buffer) - 1);
    
        netplay_client_connect_dialog [4].dp = buffer;
    
    
        strcat (buffer, get_config_string ("netplay", "ip_address", "0.0.0.0"));
    
    
        if (gui_show_dialog (netplay_client_connect_dialog) != 5)
        {
            return (D_O_K);
        }


        if (netplay_open_client (buffer) != 0)
        {
            gui_message (error_color, "Failed to connect to the server!");


            return (D_O_K);
        }


        set_config_string ("netplay", "ip_address", buffer);
    }
    else
    {
        if (netplay_open_client (NULL) != 0)
        {
            gui_message (error_color, "Failed to connect to the server!");


            return (D_O_K);
        }
    }


    DISABLE_MENU (top_menu, 0);
    
    DISABLE_MENU (top_menu, 1);

    DISABLE_MENU (top_menu, 2);

    DISABLE_MENU (top_menu, 4);


    DISABLE_MENU (netplay_menu, 0);

    DISABLE_MENU (netplay_menu, 2);


    DISABLE_MENU (netplay_client_menu, 0);


    ENABLE_MENU (netplay_client_menu, 2);


    gui_message (gui_fg_color, "NetPlay client connected to the server.");


    return (D_O_K);
}


static int netplay_client_menu_disconnect (void)
{
    netplay_close_client ();


    DISABLE_MENU (netplay_client_menu, 2);


    ENABLE_MENU (top_menu, 0);
    
    ENABLE_MENU (top_menu, 1);

    ENABLE_MENU (top_menu, 2);

    ENABLE_MENU (top_menu, 4);


    ENABLE_MENU (netplay_menu, 0);

    ENABLE_MENU (netplay_menu, 2);


    ENABLE_MENU (netplay_client_menu, 0);


    gui_message (gui_fg_color, "NetPlay client disconnected from the server.");


    return (D_O_K);
}


static int options_audio_menu_enabled (void)
{
    audio_enable_output = (! audio_enable_output);


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled audio rendering and output.");


    return (D_O_K);
}


static int options_audio_mixing_menu_normal (void)
{
    audio_pseudo_stereo = FALSE;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (gui_fg_color, "Audio mixing set to normal (mono).");


    return (D_O_K);
}


static int options_audio_mixing_stereo_menu_classic (void)
{
    if (! audio_pseudo_stereo)
    {
        audio_pseudo_stereo = AUDIO_CLASSIC_STEREO_MIXING;


        audio_exit ();
    
        audio_init ();
    
    
        papu_reinit ();
    }
    else
    {
        audio_pseudo_stereo = AUDIO_CLASSIC_STEREO_MIXING;
    }
    

    update_menus ();
    

    gui_message (gui_fg_color, "Audio mixing set to classic stereo.");


    return (D_O_K);
}


static int options_audio_mixing_stereo_menu_enhanced (void)
{
    if (! audio_pseudo_stereo)
    {
        audio_pseudo_stereo = AUDIO_ENHANCED_STEREO_MIXING;


        audio_exit ();
    
        audio_init ();
    
    
        papu_reinit ();
    }
    else
    {
        audio_pseudo_stereo = AUDIO_ENHANCED_STEREO_MIXING;
    }
    

    update_menus ();
    

    gui_message (gui_fg_color, "Audio mixing set to enhanced stereo.");


    return (D_O_K);
}


static int options_audio_mixing_stereo_menu_accurate (void)
{
    if (! audio_pseudo_stereo)
    {
        audio_pseudo_stereo = AUDIO_ACCURATE_STEREO_MIXING;


        audio_exit ();
    
        audio_init ();
    
    
        papu_reinit ();
    }
    else
    {
        audio_pseudo_stereo = AUDIO_ACCURATE_STEREO_MIXING;
    }
    

    update_menus ();
    

    gui_message (gui_fg_color, "Audio mixing set to accurate stereo.");


    return (D_O_K);
}


static int options_audio_mixing_quality_menu_low_8_bit (void)
{
    audio_sample_size = 8;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (gui_fg_color, "Audio mixing quality set to low.");


    return (D_O_K);
}


static int options_audio_mixing_quality_menu_high_16_bit (void)
{
    audio_sample_size = 16;


    audio_exit ();

    audio_init ();


    papu_reinit ();


    update_menus ();


    gui_message (gui_fg_color, "Audio mixing quality set to high.");


    return (D_O_K);
}


static int options_audio_mixing_quality_menu_dithering (void)
{
    papu_dithering = (! papu_dithering);


    update_menus ();


    gui_message (gui_fg_color, "Toggled audio random noise dithering.");


    return (D_O_K);
}


static int options_audio_effects_menu_linear_echo (void)
{
    papu_linear_echo = (! papu_linear_echo);

    update_menus ();


    papu_reinit ();


    gui_message (gui_fg_color, "Toggled linear echo audio effect.");


    return (D_O_K);
}


static int options_audio_effects_menu_surround_sound (void)
{
    papu_surround_sound = (! papu_surround_sound);

    update_menus ();


    if (papu_surround_sound)
    {
        alert ("- Warning -", "", "Surround sound may have odd side effects.", "&OK", NULL, 0, 0);
    }


    papu_reinit ();


    gui_message (gui_fg_color, "Toggled surround sound audio effect.");


    return (D_O_K);
}


static int options_audio_filter_menu_none (void)
{
    papu_filter_type = APU_FILTER_NONE;

    update_menus ();


    apu_setfilter (APU_FILTER_NONE);


    gui_message (gui_fg_color, "Audio filtering disabled.");


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_simple (void)
{
    papu_filter_type = APU_FILTER_LOWPASS;

    update_menus ();


    apu_setfilter (APU_FILTER_LOWPASS);


    gui_message (gui_fg_color, "Audio filter set to low pass.");


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_weighted (void)
{
    papu_filter_type = APU_FILTER_WEIGHTED;

    update_menus ();


    apu_setfilter (APU_FILTER_WEIGHTED);


    gui_message (gui_fg_color, "Audio filter set to weighted low pass.");


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_dynamic (void)
{
    papu_filter_type = APU_FILTER_DYNAMIC;

    update_menus ();


    apu_setfilter (APU_FILTER_DYNAMIC);


    gui_message (gui_fg_color, "Audio filter set to dynamic low pass.");


    return (D_O_K);
}


static int options_audio_filter_menu_high_pass (void)
{
    papu_filter_type = APU_FILTER_HIGHPASS;

    update_menus ();


    apu_setfilter (APU_FILTER_HIGHPASS);


    gui_message (gui_fg_color, "Audio filter set to high pass.");


    return (D_O_K);
}


static int options_audio_channels_menu_square_1 (void)
{
    papu_enable_square_1 = (! papu_enable_square_1);

    papu_update ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled mixing of square wave channel A.");


    return (D_O_K);
}


static int options_audio_channels_menu_square_2 (void)
{
    papu_enable_square_2 = (! papu_enable_square_2);

    papu_update ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled mixing of square wave channel B.");


    return (D_O_K);
}


static int options_audio_channels_menu_triangle (void)
{
    papu_enable_triangle = (! papu_enable_triangle);

    papu_update ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled mixing of triangle channel.");


    return (D_O_K);
}


static int options_audio_channels_menu_noise (void)
{
    papu_enable_noise = (! papu_enable_noise);

    papu_update ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled mixing of noise channel.");


    return (D_O_K);
}


static int options_audio_channels_menu_dmc (void)
{
    papu_enable_dmc = (! papu_enable_dmc);

    papu_update ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled mixing of delta modulation channel.");


    return (D_O_K);
}


static int options_audio_channels_menu_exsound (void)
{
    papu_enable_exsound = (! papu_enable_exsound);

    papu_update ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled mixing of external channels.");
                                              

    return (D_O_K);
}


static int options_audio_advanced_menu_ideal_triangle (void)
{
    papu_ideal_triangle = (! papu_ideal_triangle);

    papu_update ();


    update_menus ();


    gui_message (gui_fg_color, "Toggled emulation of ideal triangle.");


    return (D_O_K);
}


static int options_audio_record_menu_start (void)
{
    if (papu_start_record () == 0)
    {
        DISABLE_MENU (options_audio_record_menu, 0);

        ENABLE_MENU (options_audio_record_menu, 2);
    }


    gui_message (gui_fg_color, "Audio recording session started.");


    return (D_O_K);
}


static int options_audio_record_menu_stop (void)
{
    papu_stop_record ();


    ENABLE_MENU (options_audio_record_menu, 0);

    DISABLE_MENU (options_audio_record_menu, 2);


    gui_message (gui_fg_color, "Audio recording session halted.");


    return (D_O_K);
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


RESOLUTION_MENU_HANDLER (256, 224)

RESOLUTION_MENU_HANDLER (256, 240)

RESOLUTION_MENU_HANDLER (320, 240)

RESOLUTION_MENU_HANDLER (400, 300)

RESOLUTION_MENU_HANDLER (512, 384)

RESOLUTION_MENU_HANDLER (640, 480)


static int options_video_colors_menu_few_8_bit (void)
{
    video_set_color_depth (8);

    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_video_colors_menu_many_15_bit (void)
{
    video_set_color_depth (15);

    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_video_colors_menu_lots_16_bit (void)
{
    video_set_color_depth (16);

    gui_needs_restart = TRUE;


    return (D_CLOSE);
}


static int options_video_blitter_menu_normal (void)
{
    video_set_blitter (VIDEO_BLITTER_NORMAL);
    
    update_menus ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video blitter set to normal.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_stretched (void)
{
    video_set_blitter (VIDEO_BLITTER_STRETCHED);

    update_menus ();


    clear (screen);

    video_blit (screen);


    if (video_get_color_depth () != 8)
    {
        gui_message (gui_fg_color, "Video blitter set to stretched (buffered!).");
    }
    else
    {
        gui_message (gui_fg_color, "Video blitter set to stretched.");
    }


    return (D_REDRAW);
}


static int options_video_blitter_menu_2xsoe (void)
{
    video_set_blitter (VIDEO_BLITTER_2XSOE);

    update_menus ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video blitter set to 2xSOE engine.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_2xscl (void)
{
    video_set_blitter (VIDEO_BLITTER_2XSCL);

    update_menus ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video blitter set to 2xSCL engine.");


    return (D_REDRAW);
}


static int options_video_blitter_menu_super_2xscl (void)
{
    video_set_blitter (VIDEO_BLITTER_SUPER_2XSCL);

    update_menus ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video blitter set to super 2xSCL engine.");


    return (D_REDRAW);
}


static int options_video_filters_scanlines_menu_high (void)
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


    gui_message (gui_fg_color, "Toggled scanlines video filter to high.");


    return (D_REDRAW);
}


static int options_video_filters_scanlines_menu_medium (void)
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


    gui_message (gui_fg_color, "Toggled scanlines video filter to medium.");


    return (D_REDRAW);
}


static int options_video_filters_scanlines_menu_low (void)
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


    gui_message (gui_fg_color, "Toggled scanlines video filter to low.");


    return (D_REDRAW);
}


static int options_video_menu_vsync (void)
{
    video_enable_vsync = (! video_enable_vsync);

    update_menus ();


    gui_message (gui_fg_color, "Toggled vsync synchronization.");


    return (D_O_K);
}


static int options_video_layers_menu_sprites_a (void)
{
    ppu_enable_sprite_layer_a = (! ppu_enable_sprite_layer_a);

    update_menus ();


    gui_message (gui_fg_color, "Toggled PPU sprites layer A (priority 1).");


    return (D_O_K);
}


static int options_video_layers_menu_sprites_b (void)
{
    ppu_enable_sprite_layer_b = (! ppu_enable_sprite_layer_b);

    update_menus ();


    gui_message (gui_fg_color, "Toggled PPU sprites layer B (priority 0).");


    return (D_O_K);
}


static int options_video_layers_menu_background (void)
{
    ppu_enable_background_layer = (! ppu_enable_background_layer);

    update_menus ();


    gui_message (gui_fg_color, "Toggled PPU background layer.");


    return (D_O_K);
}


static int options_video_palette_menu_default (void)
{
    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);


    CHECK_MENU (options_video_palette_menu, 0);


    video_set_palette (DATA_DEFAULT_PALETTE);

    current_palette = DATA_DEFAULT_PALETTE;


    update_colors ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video palette set to default.");


    return (D_REDRAW);
}


static int options_video_palette_menu_grayscale (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);


    CHECK_MENU (options_video_palette_menu, 2);


    video_set_palette (DATA_GRAYSCALE_PALETTE);

    current_palette = DATA_GRAYSCALE_PALETTE;


    update_colors ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video palette set to grayscale.");


    return (D_REDRAW);
}

static int options_video_palette_menu_gnuboy (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);


    CHECK_MENU (options_video_palette_menu, 4);


    video_set_palette (DATA_GNUBOY_PALETTE);

    current_palette = DATA_GNUBOY_PALETTE;


    update_colors ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video palette set to gnuboy.");


    return (D_REDRAW);
}


static int options_video_palette_menu_nester (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 8);

    UNCHECK_MENU (options_video_palette_menu, 10);


    CHECK_MENU (options_video_palette_menu, 6);


    video_set_palette (DATA_NESTER_PALETTE);

    current_palette = DATA_NESTER_PALETTE;


    update_colors ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video palette set to NESter.");


    return (D_REDRAW);
}


static int options_video_palette_menu_nesticle (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);

    UNCHECK_MENU (options_video_palette_menu, 6);

    UNCHECK_MENU (options_video_palette_menu, 10);


    CHECK_MENU (options_video_palette_menu, 8);


    video_set_palette (DATA_NESTICLE_PALETTE);

    current_palette = DATA_NESTICLE_PALETTE;


    update_colors ();


    clear (screen);

    video_blit (screen);


    gui_message (gui_fg_color, "Video palette set to NESticle.");


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
    
    
        CHECK_MENU (options_video_palette_menu, 10);
    

        video_set_palette (((RGB *) custom_palette));
    
        current_palette = ((RGB *) &custom_palette);


        clear (screen);
    
        video_blit (screen);
    
    
        gui_message (gui_fg_color, "Video palette set to custom.");
    
    
        return (D_REDRAW);
    }
    else
    {
        gui_message (error_color, "Error opening FAKENES.PAL!");
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
