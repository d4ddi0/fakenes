

#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "apu.h"

#include "audio.h"

#include "cpu.h"

#include "gui.h"

#include "papu.h"

#include "rom.h"

#include "video.h"


#include "data.h"

#include "misc.h"


#include "timing.h"


#include "gui/objects.h"

#include "gui/menus.h"

#include "gui/dialogs.h"


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


void gui_message (int color, AL_CONST UINT8 * message, ...)
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


#define GUI_COLOR_GRAY      palette_color [17]

#define GUI_COLOR_WHITE     palette_color [33]


#define GUI_COLOR_BLUE      palette_color [3]

#define GUI_COLOR_RED       palette_color [6]


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
    file_menu_snapshot ();
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


void gui_show_dialog (DIALOG * dialog, int items)
{
    int index;


    centre_dialog (dialog);


    dialog [0].dp3 = DATA_LARGE_FONT;


    for (index = 0; index < (items - 1); index ++)
    {
        dialog [index].dp = screen;
    }


    popup_dialog (dialog, -1);
}


static INLINE void update_menus (void)
{
    TOGGLE_MENU (machine_type_menu,
        0, (machine_type == MACHINE_TYPE_NTSC));

    TOGGLE_MENU (machine_type_menu,
        2, (machine_type == MACHINE_TYPE_PAL));


    TOGGLE_MENU (machine_menu, 2, video_display_status);


    TOGGLE_MENU (options_audio_filter_menu,
        0, (papu_filter_type == APU_FILTER_NONE));


    TOGGLE_MENU (options_audio_filter_low_pass_menu,
        0, (papu_filter_type == APU_FILTER_LOWPASS));

    TOGGLE_MENU (options_audio_filter_low_pass_menu,
        2, (papu_filter_type == APU_FILTER_WEIGHTED));

    TOGGLE_MENU (options_audio_filter_low_pass_menu,
        4, (papu_filter_type == APU_FILTER_DYNAMIC));


    TOGGLE_MENU (options_video_menu, 0, video_enable_vsync);
}


int show_gui (void)
{
    LOCK_VARIABLE (message_buffer);

    LOCK_VARIABLE (redraw_flag);


    LOCK_FUNCTION (reset_timer);


    main_dialog [1].dp = screen;

    main_dialog [1].dp2 = message_buffer;


    gui_bg_color = GUI_COLOR_BLUE;

    gui_fg_color = GUI_COLOR_WHITE;


    gui_mg_color = GUI_COLOR_GRAY;


    gui_is_active = TRUE;


    update_menus ();


    if (! rom_is_loaded)
    {
        DISABLE_MENU (file_menu, 2);


        DISABLE_MENU (machine_menu, 0);


        DISABLE_MENU (machine_state_menu, 2);

        DISABLE_MENU (machine_state_menu, 4);
    }


    audio_suspend ();


    gui_message (GUI_COLOR_WHITE, "Emulation suspended.");


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
            gui_message (GUI_COLOR_RED, "Failed to load ROM!");

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


            ENABLE_MENU (file_menu, 2);


            ENABLE_MENU (machine_menu, 0);


            ENABLE_MENU (machine_state_menu, 2);

            ENABLE_MENU (machine_state_menu, 4);


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


    for (count = 0; count < 999; count ++)
    {
        sprintf (filename, "snap%03d.pcx", count);

        filename [11] = NULL;


        if (! exists (filename))
        {
            count = 1000;

            save_bitmap (filename,
                video_buffer, DATA_DEFAULT_PALETTE);


            gui_message (GUI_COLOR_WHITE,
                "Snapshot saved to %s.", filename);
        }
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
    video_display_status = (! video_display_status);

    update_menus ();


    return (D_O_K);
}


static int machine_type_menu_ntsc (void)
{
    machine_type = MACHINE_TYPE_NTSC;

    update_menus ();


    audio_exit ();

    audio_init ();


    papu_reinit ();


    return (D_O_K);
}


static int machine_type_menu_pal (void)
{
    machine_type = MACHINE_TYPE_PAL;

    update_menus ();


    audio_exit ();

    papu_exit ();


    audio_init ();

    papu_init ();


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


static int options_audio_filter_menu_none (void)
{
    papu_filter_type = APU_FILTER_NONE;

    update_menus ();


    apu_setfilter (APU_FILTER_NONE);


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_simple (void)
{
    papu_filter_type = APU_FILTER_LOWPASS;

    update_menus ();


    apu_setfilter (APU_FILTER_LOWPASS);


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_weighted (void)
{
    papu_filter_type = APU_FILTER_WEIGHTED;

    update_menus ();


    apu_setfilter (APU_FILTER_WEIGHTED);


    return (D_O_K);
}


static int options_audio_filter_low_pass_menu_dynamic (void)
{
    papu_filter_type = APU_FILTER_DYNAMIC;

    update_menus ();


    apu_setfilter (APU_FILTER_DYNAMIC);


    return (D_O_K);
}


static int options_video_menu_vsync (void)
{
    video_enable_vsync = (! video_enable_vsync);

    update_menus ();


    return (D_O_K);
}


static int options_video_palette_menu_default (void)
{
    UNCHECK_MENU (options_video_palette_menu, 2);

    UNCHECK_MENU (options_video_palette_menu, 4);


    CHECK_MENU (options_video_palette_menu, 0);


    set_palette (DATA_DEFAULT_PALETTE);


    return (D_O_K);
}


static int options_video_palette_menu_gnuboy (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 4);


    CHECK_MENU (options_video_palette_menu, 2);


    set_palette (DATA_GNUBOY_PALETTE);


    return (D_O_K);
}


static int options_video_palette_menu_nester (void)
{
    UNCHECK_MENU (options_video_palette_menu, 0);

    UNCHECK_MENU (options_video_palette_menu, 2);


    CHECK_MENU (options_video_palette_menu, 4);


    set_palette (DATA_NESTER_PALETTE);


    return (D_O_K);
}


static int help_menu_shortcuts (void)
{
    gui_show_dialog (help_shortcuts_dialog, 10);


    return (D_O_K);
}


static int help_menu_about (void)
{
    gui_show_dialog (help_about_dialog, 10);


    return (D_O_K);
}
