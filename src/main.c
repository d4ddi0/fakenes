 

/*

FakeNES - A portable, Open Source NES emulator.

main.c: Implementation of the main emulation.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#define USE_CONSOLE


#include <allegro.h>

#include "build.h"


#include <stdio.h>

#include <stdlib.h>

#include <string.h>


#ifdef POSIX

#include <sys/stat.h>

#include <sys/types.h>


#include <dirent.h>

#include <errno.h>


extern int errno;

#endif


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


#include "timing.h"


int machine_type = MACHINE_TYPE_NTSC;


int frame_skip_min = 0;

int frame_skip_max = 0;


static int first_run = FALSE;


volatile int timing_fps = 0;

volatile int timing_hertz = 0;


volatile int timing_audio_fps = 0;

volatile int timing_audio_hertz = 0;


#ifdef POSIX

UINT8 * homedir = NULL;

UINT8 * sramdir = NULL;

UINT8 * logdir = NULL;

UINT8 * confdir = NULL;


UINT8 * datfile = NULL;


static DIR * tmpdir = NULL;


UINT8 logfile [256];

#endif


static int redraw_flag = TRUE;


static int frame_count = 1;


static int executed_frames = 0;

static int rendered_frames = 0;


static volatile int actual_fps_count = 0;

static volatile int virtual_fps_count = 0;


static volatile int throttle_counter = 0;


static void fps_interrupt (void)
{
    timing_fps = actual_fps_count;

    actual_fps_count = 0;


    timing_hertz = virtual_fps_count;

    virtual_fps_count = 0;


    timing_audio_fps = audio_fps;

    audio_fps = 0;
}

END_OF_STATIC_FUNCTION (fps_interrupt);


static void throttle_interrupt (void)
{
    throttle_counter ++;
}

END_OF_STATIC_FUNCTION (throttle_interrupt);


void suspend_timing (void)
{
    remove_int (fps_interrupt);

    remove_int (throttle_interrupt);
}


void resume_timing (void)
{
    int speed;


    speed = ((machine_type == MACHINE_TYPE_NTSC) ? 60 : 50);


    install_int_ex (fps_interrupt, BPS_TO_TIMER (1));

    install_int_ex (throttle_interrupt, BPS_TO_TIMER (speed));
}


int main (int argc, char * argv [])
{
    int result;


    printf ("\nFakeNES version 0.1.5 (CVS), by stainless and TRAC.\n"
            "Assistance provided by amit and Lord_Nightmare.\n");

    printf ("\nOriginal concept by stainless and RobotBebop.\n"
            "Uses the Nofrendo NES APU core by Matthew Conte.\n");

    printf ("\nBe sure to visit http://fakenes.sourceforge.net/.\n"
            "Report bugs to fakenes-bugs@lists.sourceforge.net.\n");

    printf ("\nCopyright (c) 2002, Randy McDowell and Ian Smith.\n"
            "Portions copyright (c) 2002, Charles Bilyue'.\n");

#ifdef POSIX

    printf ("\nThis is free software.  See 'LICENSE' for details.\n"
            "You must read and accept the license prior to use.\n\n");

#else

    printf ("\nThis is free software.  See 'LICENSE.TXT' for details.\n"
            "You must read and accept the license prior to use.\n\n");

#endif


    allegro_init ();


    install_keyboard ();


    set_window_title ("FakeNES");

    set_window_close_button (FALSE);

#ifdef POSIX

    /* by amit */


    /* Configuration directory checking */

    homedir = getenv ("HOME");


    if (homedir)
    {
        const UINT8 confdir_base [] = "/.fakenes";


        const UINT8 sramdir_base [] = "/sram";

        const UINT8 logdir_base [] = "/logs";


        confdir = ((UINT8 *) malloc (strlen (homedir) + sizeof (confdir_base)));


        if (confdir)
        {
            strcpy (confdir, homedir);

            strcat (confdir, confdir_base);


            sramdir = ((UINT8 *) malloc (strlen (confdir) + sizeof (sramdir_base)));


            if (sramdir)
            {
                strcpy (sramdir, confdir);

                strcat (sramdir, sramdir_base);
            }


            logdir = ((UINT8 *) malloc (strlen (confdir) + sizeof (logdir_base)));


            if (logdir)
            {
                strcpy (logdir, confdir);

                strcat (logdir, logdir_base);


                memset (logfile, NULL, sizeof (logfile));


                strcat (logfile, logdir);

                strcat (logfile, "/messages");
            }
        }
        else
        {
            sramdir = NULL;

            logdir = NULL;
        }


        if (! confdir)
        {
            fprintf (stderr, "Error when generating configuration path.\nConfiguration will not be saved.\n");
        }
        else
        {
            /* Just see if we can open the directory. */
              
            if (! (tmpdir = opendir (confdir)))
            {
                /* Directory doesn't exist, create it. */

                if (errno == ENOENT)
                {
                    if (mkdir (confdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
                    {
                        fprintf (stderr, "Error creating \"%s\".\nConfiguration will not be saved.\n", confdir);


                        free (confdir);

                        confdir = NULL;
                    }
                    else    /* mkdir was successful, make the sram dir. */
                    {
                        /* Error checking for sramdir happens later. */

                        mkdir (sramdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));


                        /* And likesie for logdir. */

                        mkdir (logdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
                    }
                }
                else
                {
                    UINT8 errorbuf [300] = { NULL };


                    strcat (errorbuf, confdir);

                    perror (errorbuf);


                    fprintf (stderr, "%s.\nConfiguration will not be saved.\n", errorbuf);


                    free (confdir);

                    confdir = NULL;
                }
            }
            else
            {
                /* Close the directory we just opened. */

                closedir (tmpdir);
            }
        }

    }
    else
    {
        fprintf (stderr, "$HOME appears to not be set.\nConfiguration will not be saved.\n");
    }


    /* Load up the configuration file. */

    if (confdir)
    {
        const UINT8 conffile_base [] = "/config";


        UINT8 * conffile = malloc (strlen (confdir) + sizeof (conffile_base));


        strcpy (conffile, confdir);

        strcat (conffile, conffile_base);


        set_config_file (conffile);
    }
    else
    {
        set_config_file ("/dev/null");
    }


    /* Check the sram directory. */

    if (! sramdir)
    {
        /* If we have a valid home directory, there was an error. */

        if (homedir)
        {
            fprintf (stderr, "Error when generating save path.\nSRAM files will not be saved.\n");
        }
    }
    else
    {
        if (! (tmpdir = opendir (sramdir)))
        {
            if (errno == ENOENT)
            {
                if (mkdir (sramdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
                {
                    fprintf (stderr, "Error creating \"%s\"", sramdir);


                    free (sramdir);

                    sramdir = NULL;
                }
            }
            else
            {
                UINT8 errorbuf [300] = { NULL };


                strcat (errorbuf, confdir);

                perror (errorbuf);


                fprintf (stderr, "%s.\nConfiguration will not be saved.\n", errorbuf);


                free (confdir);

                sramdir = NULL;
            }
        }
        else
        {
            /* Close the directory we just opened. */

            closedir (tmpdir);
        }
    }


    /* Check the logs directory. */

    if (! logdir)
    {
        /* If we have a valid home directory, there was an error. */

        if (homedir)
        {
            fprintf (stderr, "Error when generating log path.\nLogs will not be saved.\n");
        }
    }
    else
    {
        if (! (tmpdir = opendir (logdir)))
        {
            if (errno == ENOENT)
            {
                if (mkdir (sramdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
                {
                    fprintf (stderr, "Error creating \"%s\"", logdir);


                    free (logdir);

                    logdir = NULL;
                }
            }
            else
            {
                UINT8 errorbuf [300] = { NULL };


                strcat (errorbuf, confdir);

                perror (errorbuf);


                fprintf (stderr, "%s.\nConfiguration will not be saved.\n", errorbuf);


                free (confdir);

                logdir = NULL;
            }
        }
        else
        {
            /* Close the directory we just opened. */

            closedir (tmpdir);
        }
    }
#else

    set_config_file ("fakenes.cfg");

#endif


    frame_skip_min = get_config_int ("timing", "frame_skip_min", 0);

    frame_skip_max = get_config_int ("timing", "frame_skip_max", 9);


    machine_type = get_config_int ("timing", "machine_type", MACHINE_TYPE_NTSC);


    first_run = get_config_int ("gui", "first_run", TRUE);


    if (first_run)
    {
        printf ("Press any key to continue...\n\n");


        while (! keypressed ());

        clear_keybuf ();


        first_run = FALSE;
    }


    install_timer ();


#ifdef POSIX

    if (confdir)
    {
        const UINT8 datfile_base [] = "/fakenes.dat";


        datfile = malloc (strlen (confdir) + sizeof (datfile_base));

        if (! datfile)
        {
            fprintf (stderr, "Error when generating datafile path, trying cwd.\n");


            datfile = "fakenes.dat";

            data = load_datafile ("fakenes.dat");
        }
        else
        {
            strcpy (datfile, confdir);

            strcat (datfile, datfile_base);


            data = load_datafile (datfile);


            if (! data)
            {
                fprintf (stderr, "Datafile not found in configuration path, trying cwd.\n");


                datfile = "fakenes.dat";

                data = load_datafile (datfile);
            }
        }
    }
    else    /* confdir unset, try cwd. */
    {
        datfile = "fakenes.dat";

        data = load_datafile (datfile);
    }

#else

    data = load_datafile ("fakenes.dat");

#endif


    if (! data)
    {
#ifdef POSIX

        fprintf (stderr, "PANIC: Failed to load datafile: \"%s\".\n", datfile);
#else

        fprintf (stderr, "PANIC: Failed to load datafile: fakenes.dat.\n");
#endif


        return (1);
    }


    if (input_init () != 0)
    {
        fprintf (stderr, "PANIC: Failed to initialize input interface!\n");


        return (1);
    }


    input_reset ();


    if (argc >= 2)
    {
        if (load_rom (argv [1], &global_rom) != 0)
        {
            fprintf (stderr, "PANIC: Failed to load ROM file (bad format?).\n");


            return (1);
        }


        rom_is_loaded = TRUE;
    }
    else
    {
        rom_is_loaded = FALSE;
    }


    if (audio_init () != 0)
    {
        fprintf (stderr, "PANIC: Failed to initialize audio interface!\n");


        free_rom (&global_rom);

        return (1);
    }


    papu_init ();

    papu_reset ();


    result = machine_init ();

    if (result != 0)
    {
        return (result);
    }


    fade_out (4);


    if (video_init () != 0)
    {
        set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);


        fprintf (stderr, "PANIC: Failed to initialize video interface!\n");


        free_rom (&global_rom);

        return (1);
    }


    LOCK_FUNCTION (fps_interrupt);

    LOCK_FUNCTION (throttle_interrupt);


    if (! rom_is_loaded)
    {
        show:

          show_gui ();


        if (gui_needs_restart)
        {
            goto show;
        }
    }


    if (rom_is_loaded)
    {
        resume_timing ();


        while (! input_process ())
        {
            executed_frames ++;


            if (-- frame_count > 0)
            {
                redraw_flag = FALSE;
            }
            else
            {
                redraw_flag = TRUE;
    
                rendered_frames ++;


                actual_fps_count ++;
    
    
                if (key [KEY_TILDE])
                {
                    /* Fast forward. */
    
                    frame_count = frame_skip_max;
    
                    throttle_counter = 0;
                }
                else
                {
                    if (frame_skip_min == 0)
                    {
                        while (throttle_counter == 0);
                    }
    
    
                    frame_count = throttle_counter;
    
                    throttle_counter -= frame_count;
    
    
                    if (frame_count >= frame_skip_max)
                    {
                        frame_count = frame_skip_max;
                    }
                    else if (frame_count <= frame_skip_min)
                    {
                        frame_count = frame_skip_min;
                    }
                }
            }
    
    
            virtual_fps_count ++;
    

            switch (machine_type)
            {
                case MACHINE_TYPE_PAL:

                    ppu_frame_last_line = (TOTAL_LINES_PAL - 1);


                    break;


                case MACHINE_TYPE_NTSC:

                default:

                    ppu_frame_last_line = (TOTAL_LINES_NTSC - 1);


                    break;
            }


            if (redraw_flag)
            {
                /* Perform a full render. */

                ppu_start_render ();


                for (ppu_scanline = 0; ppu_scanline <= ppu_frame_last_line; ppu_scanline ++)
                {
                    cpu_start_new_scanline ();


                    if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                        (ppu_scanline <= LAST_DISPLAYED_LINE))
                    {
                        ppu_start_line ();


                        ppu_render_line (ppu_scanline);


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else if (ppu_scanline == FIRST_VBLANK_LINE)
                    {
                        ppu_end_render ();


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else if (ppu_scanline == FIRST_VBLANK_LINE + 1)
                    {
                        ppu_vblank_nmi ();


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else if (ppu_scanline == ppu_frame_last_line)
                    {
                        ppu_clear ();


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else
                    {
                        cpu_execute (RENDER_CLOCKS);
                    }
            

                    if (mmc_scanline_start)
                    {
                        if (mmc_scanline_start (ppu_scanline))
                        {
                            cpu_interrupt (CPU_INTERRUPT_IRQ);
                        }
                    }


                    if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                        (ppu_scanline <= LAST_DISPLAYED_LINE))
                    {
                        cpu_execute (HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP);


                        ppu_end_line ();


                        cpu_execute ((HBLANK_CLOCKS - HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP));
                    }
                    else
                    {
                        cpu_execute (HBLANK_CLOCKS);
                    }


                    if (mmc_scanline_end)
                    {
                        if (mmc_scanline_end (ppu_scanline))
                        {
                            cpu_interrupt (CPU_INTERRUPT_IRQ);
                        }
                    }
                }


                papu_process ();
            }
            else
            {
                /* Perform a partial render. */

                ppu_start_frame ();


                for (ppu_scanline = 0; ppu_scanline <= ppu_frame_last_line; ppu_scanline ++)
                {
                    cpu_start_new_scanline ();


                    if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                        (ppu_scanline <= LAST_DISPLAYED_LINE))
                    {
                        ppu_start_line ();


                        ppu_stub_render_line (ppu_scanline);


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else if (ppu_scanline == FIRST_VBLANK_LINE)
                    {
                        ppu_vblank ();


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else if (ppu_scanline == FIRST_VBLANK_LINE + 1)
                    {
                        ppu_vblank_nmi ();


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else if (ppu_scanline == ppu_frame_last_line)
                    {
                        ppu_clear ();


                        cpu_execute (RENDER_CLOCKS);
                    }
                    else
                    {
                        cpu_execute (RENDER_CLOCKS);
                    }


                    if (mmc_scanline_start)
                    {
                        if (mmc_scanline_start (ppu_scanline))
                        {
                            cpu_interrupt (CPU_INTERRUPT_IRQ);
                        }
                    }


                    if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                        (ppu_scanline <= LAST_DISPLAYED_LINE))
                    {
                        cpu_execute (HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP);


                        ppu_end_line ();


                        cpu_execute ((HBLANK_CLOCKS - HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP));
                    }
                    else
                    {
                        cpu_execute (HBLANK_CLOCKS);
                    }


                    if (mmc_scanline_end)
                    {
                        if (mmc_scanline_end (ppu_scanline))
                        {
                            cpu_interrupt (CPU_INTERRUPT_IRQ);
                        }
                    }
                }


                papu_process ();
            }
        }
    }


    set_config_int ("timing", "frame_skip_min", frame_skip_min);

    set_config_int ("timing", "frame_skip_max", frame_skip_max);


    set_config_int ("timing", "machine_type", machine_type);


    set_config_int ("gui", "first_run", first_run);


    papu_exit ();

    audio_exit ();


    fade_out (4);


    video_exit ();

    ppu_exit ();


    if (rom_is_loaded)
    {
        cpu_exit ();
    }


    input_exit ();


    if (rom_is_loaded)
    {
        printf ("Executed frames: %d (%d rendered).\n", executed_frames, rendered_frames);


        free_rom (&global_rom);
    }


    unload_datafile (data);


    return (0);
}

END_OF_MAIN ()


int machine_init (void)
{
    if (rom_is_loaded)
    {
        cpu_memmap_init ();


        if (mmc_init () != 0)
        {
            fprintf (stderr, "PANIC: mmc_init() failed (unsupported mapper?).\n");
    
    
            free_rom (&global_rom);
    
            return (1);
        }


        if (cpu_init () != 0)
        {
            fprintf (stderr, "PANIC: Failed to initialize the CPU core!\n");
    
    
            free_rom (&global_rom);
    
            return (1);
        }
    
        
        if (ppu_init () != 0)
        {
            fprintf (stderr, "PANIC: Failed to initialize the PPU core!\n");
    
    
            free_rom (&global_rom);
    
            return (1);
        }
    

        input_reset ();

        cpu_reset ();
    }


    return (0);
}


void machine_reset (void)
{
    audio_exit ();

    audio_init ();


    mmc_reset ();

    papu_reset ();

    ppu_reset ();

    input_reset ();

    cpu_reset ();
}
