

#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "gui.h"

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


    gui_message (33, "Emulation suspended.");


    unscare_mouse ();

    do_dialog (main_dialog, -1);

    scare_mouse ();


    gui_is_active = FALSE;


    clear (screen);


    return (want_exit);
}


static int file_menu_load_rom (void)
{
    UINT8 buffer [256];


    memset (buffer, NULL, sizeof (buffer));


#ifdef USE_ZLIB

    if (file_select_ex ("iNES ROMs (*.NES)",
        buffer, "NES;nes;GZ;gz", sizeof (buffer), 0, 0) != 0)
    {

#else

    if (file_select_ex ("iNES ROMs (*.NES)",
        buffer, "NES;nes", sizeof (buffer), 0, 0) != 0)
    {

#endif

        if (rom_is_loaded)
        {
            free_rom (&global_rom);
        }


        load_rom (buffer, &global_rom);

        rom_is_loaded = TRUE;


        machine_init ();


        return (D_CLOSE);
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
                    video_buffer, DATA_NES_PALETTE);


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
        machine_menu [2].flags = 0;

        video_display_status = FALSE;
    }
    else
    {
        machine_menu [2].flags |= D_SELECTED;

        video_display_status = TRUE;
    }


    return (D_O_K);
}


static int options_video_menu_vsync (void)
{
    if (options_video_menu [0].flags & D_SELECTED)
    {
        options_video_menu [0].flags = 0;

        video_enable_vsync = FALSE;
    }
    else
    {
        options_video_menu [0].flags |= D_SELECTED;

        video_enable_vsync = TRUE;
    }


    return (D_O_K);
}


static int options_menu_gb_mode (void)
{
    if (options_menu [2].flags & D_SELECTED)
    {
        options_menu [2].flags = 0;

        set_palette (DATA_NES_PALETTE);
    }
    else
    {
        options_menu [2].flags |= D_SELECTED;

        set_palette (DATA_GB_PALETTE);
    }


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
