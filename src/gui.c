

#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "apu.h"

#include "audio.h"

#include "gui.h"

#include "papu.h"

#include "rom.h"

#include "video.h"


#include "data.h"

#include "gui2.h"

#include "misc.h"


#include "timing.h"


int gui_is_active = FALSE;


static int want_exit = FALSE;


static UINT8 message_buffer [256];

static int redraw_flag = FALSE;


static void reset_timer (void)
{
    remove_int (reset_timer);


    message_buffer [0] = NULL;

    redraw_flag = TRUE;
};

END_OF_STATIC_FUNCTION (reset_timer);


void gui_message (int color, CONST UINT8 * message, ...)
{
    va_list format;


    reset_timer ();


    main_dialog [1].fg = color;


    main_dialog [1].x = 16;

    main_dialog [1].y = ((SCREEN_H - 16) - text_height (font));


    va_start (format, message);

    vsprintf (message_buffer, message, format);

    va_end (format);


    redraw_flag = TRUE;


    install_int (reset_timer, 2000);
}


static int gui_redraw_callback (int msg, DIALOG * d, int c)
{
    /* HACK HACK HACK HACK */

    if (redraw_flag)
    {
        redraw_flag = FALSE;


        clear (screen);

        video_blit ();


        return (D_REDRAW);
    }
    else
    {
        return (D_O_K);
    }
}


int show_gui (void)
{
    LOCK_VARIABLE (message_buffer);

    LOCK_VARIABLE (redraw_flag);


    LOCK_FUNCTION (reset_timer);


    main_dialog [1].dp = screen;

    main_dialog [1].dp2 = message_buffer;


    gui_bg_color = 3;

    gui_fg_color = 33;


    gui_mg_color = 17;


    gui_is_active = TRUE;


    if (! rom_is_loaded)
    {
        machine_menu [0].flags |= D_DISABLED;
    }


    audio_suspend ();


    gui_message (33, "Emulation suspended.");


    unscare_mouse ();

    do_dialog (main_dialog, -1);

    scare_mouse ();


    gui_is_active = FALSE;


    clear (screen);


    audio_resume ();


    return (want_exit);
}


static int file_menu_load_rom (void)
{
    ROM test_rom;

    UINT8 buffer [256];


    memset (buffer, NULL, sizeof (buffer));


#ifdef USE_ZLIB

    if (file_select_ex ("iNES ROMs (*.NES, *.GZ)",
        buffer, "NES;nes;GZ;gz", sizeof (buffer), 0, 0) != 0)
    {

#else

    if (file_select_ex ("iNES ROMs (*.NES)",
        buffer, "NES;nes", sizeof (buffer), 0, 0) != 0)
    {

#endif

        if (load_rom (buffer, &test_rom) != 0)
        {
            gui_message (6, "Failed to load ROM!");

            return (D_O_K);
        }
        else
        {
            free_rom (&test_rom);


            if (rom_is_loaded)
            {
                free_rom (&global_rom);
            }

            load_rom (buffer, &global_rom);


            rom_is_loaded = TRUE;
    
            machine_init ();


            machine_menu [0].flags &= ~D_DISABLED;


            return (D_CLOSE);
        }
    }
    else
    {
        return (D_O_K);
    }
}


static int file_menu_snapshot (void)
{
    int count;

    UINT8 filename [12];


    if (rom_is_loaded)
    {
        for (count = 0; count < 999; count ++)
        {
            sprintf (filename, "snap%03d.pcx", count);
    
            filename [11] = NULL;


            if (! exists (filename))
            {
                count = 1000;
    
                save_bitmap (filename,
                    video_buffer, DATA_DEFAULT_PALETTE);


                gui_message (33,
                    "Snapshot saved to %s.", filename);
            }
        }
    }
    else
    {
        gui_message (6, "No ROM loaded!");
    }


    return (D_O_K);
}


static int file_menu_exit (void)
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
    if (machine_menu [2].flags & D_SELECTED)
    {
        machine_menu [2].flags &= ~D_SELECTED;

        video_display_status = FALSE;
    }
    else
    {
        machine_menu [2].flags |= D_SELECTED;

        video_display_status = TRUE;
    }


    return (D_O_K);
}


static int options_audio_filter_menu_none (void)
{
    options_audio_filter_low_pass_menu [0].flags &= ~D_SELECTED;

    options_audio_filter_low_pass_menu [2].flags &= ~D_SELECTED;


    options_audio_filter_menu [0].flags |= D_SELECTED;


    papu_filter_type = APU_FILTER_NONE;

    apu_setfilter (APU_FILTER_NONE);


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_simple (void)
{
    options_audio_filter_menu [0].flags &= ~D_SELECTED;


    options_audio_filter_low_pass_menu [2].flags &= ~D_SELECTED;

    options_audio_filter_low_pass_menu [0].flags |= D_SELECTED;


    papu_filter_type = APU_FILTER_LOWPASS;

    apu_setfilter (APU_FILTER_LOWPASS);


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_weighted (void)
{
    options_audio_filter_menu [0].flags &= ~D_SELECTED;


    options_audio_filter_low_pass_menu [0].flags &= ~D_SELECTED;

    options_audio_filter_low_pass_menu [2].flags |= D_SELECTED;


    papu_filter_type = APU_FILTER_WEIGHTED;

    apu_setfilter (APU_FILTER_WEIGHTED);


    return (D_O_K);
}


static int options_video_menu_vsync (void)
{
    if (options_video_menu [0].flags & D_SELECTED)
    {
        options_video_menu [0].flags &= ~D_SELECTED;

        video_enable_vsync = FALSE;


        gui_message (33, "VSync disabled.");
    }
    else
    {
        options_video_menu [0].flags |= D_SELECTED;

        video_enable_vsync = TRUE;


        gui_message (33, "VSync enabled.");
    }


    return (D_O_K);
}


static int options_video_palette_menu_default (void)
{
    options_video_palette_menu [2].flags &= ~D_SELECTED;

    options_video_palette_menu [4].flags &= ~D_SELECTED;


    options_video_palette_menu [0].flags |= D_SELECTED;


    set_palette (DATA_DEFAULT_PALETTE);


    return (D_O_K);
}


static int options_video_palette_menu_gnuboy (void)
{
    options_video_palette_menu [0].flags &= ~D_SELECTED;

    options_video_palette_menu [4].flags &= ~D_SELECTED;


    options_video_palette_menu [2].flags |= D_SELECTED;


    set_palette (DATA_GNUBOY_PALETTE);


    return (D_O_K);
}


static int options_video_palette_menu_nester (void)
{
    options_video_palette_menu [0].flags &= ~D_SELECTED;

    options_video_palette_menu [2].flags &= ~D_SELECTED;


    options_video_palette_menu [4].flags |= D_SELECTED;


    set_palette (DATA_NESTER_PALETTE);


    return (D_O_K);
}


static int help_menu_about (void)
{
    int index;


    centre_dialog (help_about_dialog);


    help_about_dialog [0].dp3 = DATA_LARGE_FONT;


    for (index = 0; index < 9; index ++)
    {
        help_about_dialog [index].dp = screen;
    }


    popup_dialog (help_about_dialog, -1);


    return (D_O_K);
}
