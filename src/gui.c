

#include <allegro.h>


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


int show_gui (void)
{
    gui_bg_color = 3;

    gui_fg_color = 33;


    gui_mg_color = 17;


    gui_is_active = TRUE;


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

        video_status_display = FALSE;
    }
    else
    {
        machine_menu [2].flags |= D_SELECTED;

        video_status_display = TRUE;
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
