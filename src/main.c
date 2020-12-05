/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   main.c: Implementation of the main emulation.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */
 
#define ALLEGRO_USE_CONSOLE
#include <allegro.h>
#include "build.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef POSIX
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
extern int errno;
#endif
#include "audio.h"
#include "common.h"
#include "cpu.h"
#include "data.h"
#include "gui.h"
#include "input.h"
#include "log.h"
#include "mmc.h"
#include "netplay.h"
#include "papu.h"
#include "ppu.h"
#include "rom.h"
#include "timing.h"
#include "version.h"
#include "video.h"


int disable_gui = FALSE;


int machine_type = MACHINE_TYPE_NTSC;


int frame_skip_min = 0;

int frame_skip_max = 0;


static int enable_rsync = FALSE;

static int rsync_grace_frames = 0;


static int fast_forward = FALSE;


static int first_run = FALSE;


int timing_fps = 0;

int timing_hertz = 0;


int timing_audio_fps = 0;

int timing_audio_hertz = 0;


int timing_half_speed = FALSE;


#ifdef POSIX

UINT8 * homedir = NIL;

UINT8 * logdir = NIL;

UINT8 * confdir = NIL;


static DIR * tmpdir = NIL;

#endif


UINT8 logfile [256];


static int redraw_flag = TRUE;


static int frame_count = 1;


static int executed_frames = 0;

static int rendered_frames = 0;


static int actual_fps_count = 0;

static int virtual_fps_count = 0;


static int average_fps = 0;


static volatile int throttle_counter = 0;


static volatile int frame_interrupt = FALSE;


static void fps_interrupt (void)
{
    frame_interrupt = TRUE;
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


    redraw_flag = TRUE;


    actual_fps_count = 0;

    virtual_fps_count = 0;


    frame_count = 1;


    throttle_counter = 0;


    frame_interrupt = FALSE;
}


void resume_timing (void)
{
    int speed;


    redraw_flag = TRUE;


    actual_fps_count = 0;

    virtual_fps_count = 0;


    frame_count = 1;


    throttle_counter = 0;


    frame_interrupt = FALSE;


    speed = ((machine_type == MACHINE_TYPE_NTSC) ? 60 : 50);


    if (timing_half_speed)
    {
        speed /= 2;
    }


    install_int_ex (fps_interrupt, BPS_TO_TIMER (1));

    install_int_ex (throttle_interrupt, BPS_TO_TIMER (speed));
}


void suspend_throttling (void)
{
    remove_int (throttle_interrupt);


    redraw_flag = TRUE;


    frame_count = 1;


    throttle_counter = 0;
}


void resume_throttling (void)
{
    int speed;


    redraw_flag = TRUE;


    frame_count = 1;


    throttle_counter = 0;


    speed = ((machine_type == MACHINE_TYPE_NTSC) ? 60 : 50);


    if (timing_half_speed)
    {
        speed /= 2;
    }


    install_int_ex (throttle_interrupt, BPS_TO_TIMER (speed));


    if (enable_rsync)
    {
        while (throttle_counter == 0);

        throttle_counter = 0;
    }
}


int main (int argc, char * argv [])
{
    int result;


    UINT8 buffer [256];

    UINT8 buffer2 [256];


    if (VERSION == 0x030)
    {
        printf ("\nThis release is dedicated to those who fell in the 9/11 attacks.\n");
    }


    printf ("\n");
    printf ("FakeNES version " VERSION_STRING ", by Siloh and TRAC.\n");
    printf ("Using Allegro version " ALLEGRO_VERSION_STR " (" ALLEGRO_PLATFORM_STR ").\n");
    printf ("\n");
    printf ("Assistance provided by amit, Astxist, ipher, KCat,\n");
    printf ("Lord_Nightmare, Mexandrew, and others.  See the\n");
    printf ("About box for a complete listing.\n");
    printf ("\n");
    printf ("Uses the Nofrendo NES APU core by Matthew Conte.\n");
    printf ("\n");
    printf ("Be sure to visit http://fakenes.sourceforge.net/.\n");
    printf ("Report bugs to fakenes-bugs@lists.sourceforge.net.\n");
    printf ("\n");
    printf ("Copyright (c) 2001-2006, FakeNES Team.\n");
#ifdef POSIX
    printf ("This is free software.  See 'LICENSE' for details.\n");
    printf ("You must read and accept the license prior to use.\n");
#else
    printf ("This is free software.  See 'LICENSE.TXT' for details.\n");
    printf ("You must read and accept the license prior to use.\n");
#endif
    printf ("\n");


    allegro_init ();


    set_window_title ("FakeNES");


#ifdef POSIX

    /* by amit */


    /* Configuration directory checking */

    homedir = getenv ("HOME");


    if (homedir)
    {
        const UINT8 confdir_base [] = "/.fakenes";


        const UINT8 logdir_base [] = "/logs";


        confdir = ((UINT8 *) malloc (strlen (homedir) + sizeof (confdir_base)));


        if (confdir)
        {
            strcpy (confdir, homedir);

            strcat (confdir, confdir_base);


            logdir = ((UINT8 *) malloc (strlen (confdir) + sizeof (logdir_base)));


            if (logdir)
            {
                strcpy (logdir, confdir);

                strcat (logdir, logdir_base);


                memset (logfile, NIL, sizeof (logfile));


                strcat (logfile, logdir);

                strcat (logfile, "/messages");
            }
        }
        else
        {
            logdir = NIL;
        }


        if (! confdir)
        {
            fprintf (stderr, "Error when generating configuration path.\nConfiguration will not be saved.\n\n");
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
                        fprintf (stderr, "Error creating \"%s\".\nConfiguration will not be saved.\n\n", confdir);


                        free (confdir);

                        confdir = NIL;
                    }
                    else    /* mkdir was successful, make the log dir. */
                    {
                        /* Error checking for logdir happens later. */

                        mkdir (logdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
                    }
                }
                else
                {
                    UINT8 errorbuf [300] = { NIL };


                    strcat (errorbuf, confdir);

                    perror (errorbuf);


                    fprintf (stderr, "%s.\nConfiguration will not be saved.\n\n", errorbuf);


                    free (confdir);

                    confdir = NIL;
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
        fprintf (stderr, "$HOME appears to not be set.\nConfiguration will not be saved.\n\n");
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


    /* Check the logs directory. */

    if (! logdir)
    {
        /* If we have a valid home directory, there was an error. */

        if (homedir)
        {
            fprintf (stderr, "Error when generating log path.\nLogs will not be saved.\n\n");
        }
    }
    else
    {
        if (! (tmpdir = opendir (logdir)))
        {
            if (errno == ENOENT)
            {
                if (mkdir (logdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
                {
                    fprintf (stderr, "Error creating \"%s\"\n\n", logdir);


                    free (logdir);

                    logdir = NIL;
                }
            }
            else
            {
                UINT8 errorbuf [300] = { NIL };


                strcat (errorbuf, confdir);

                perror (errorbuf);


                fprintf (stderr, "%s.\nConfiguration will not be saved.\n\n", errorbuf);


                free (confdir);

                logdir = NIL;
            }
        }
        else
        {
            /* Close the directory we just opened. */

            closedir (tmpdir);
        }
    }


    log_open (logfile);

#else


    memset (buffer, NIL, sizeof (buffer));

    memset (buffer2, NIL, sizeof (buffer2));


    get_executable_name (buffer, sizeof (buffer));


#ifdef ALLEGRO_WINDOWS

   replace_filename (buffer2, buffer, "fakenesw.cfg", sizeof (buffer2));

#else

   replace_filename (buffer2, buffer, "fakenes.cfg", sizeof (buffer2));

#endif


    set_config_file (buffer2);


    replace_filename (buffer2, buffer, "messages.log", sizeof (buffer2));


    log_open (buffer2);


    memset (logfile, NIL, sizeof (logfile));


    strcat (logfile, buffer2);

#endif


    frame_skip_min = get_config_int ("timing", "frame_skip_min", 0);

    frame_skip_max = get_config_int ("timing", "frame_skip_max", 8);


    machine_type = get_config_int ("timing", "machine_type", MACHINE_TYPE_NTSC);


    enable_rsync = get_config_int ("timing", "enable_rsync", TRUE);

    rsync_grace_frames = get_config_int ("timing", "rsync_grace_frames", 10);


    disable_gui = get_config_int ("gui", "disable_gui", FALSE);


    first_run = get_config_int ("gui", "first_run", TRUE);


    install_timer ();


    /*
#ifdef POSIX

    data = load_datafile ("fakenes.dat");


    if (! data)
    {
        printf ("Loading datafile from '/usr/share/fakenes.dat'.\n\n");


        data = load_datafile ("/usr/share/fakenes.dat");
    }

#else

    data = load_datafile ("fakenes.dat");

#endif


    if (! data)
    {
        fprintf (stderr, "PANIC: Failed to load datafile: fakenes.dat.\n");


        return (1);
    }
    */


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
        if (disable_gui)
        {
            fprintf (stderr, "The GUI is currently disabled in your configuration.\n");
    
            fprintf (stderr, "You must specify a path to a ROM file to load.\n");
    
    
            return (1);
        }
        else
        {
            rom_is_loaded = FALSE;
        }
    }


    if (audio_init () != 0)
    {
        fprintf (stderr, "PANIC: Failed to initialize audio interface!\n");


        free_rom (&global_rom);

        return (1);
    }


    papu_init ();

    papu_reset ();


    netplay_init ();


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


    if (! disable_gui)
    {
        gui_init ();
    }


    LOCK_VARIABLE (frame_interrupt);


    LOCK_VARIABLE (throttle_counter);


    LOCK_FUNCTION (fps_interrupt);

    LOCK_FUNCTION (throttle_interrupt);


    if (! disable_gui)
    {
        if (! rom_is_loaded)
        {
          show:
    
            show_gui (first_run);
    
    
            if (first_run)
            {
                first_run = FALSE;
            }
    
    
            if (gui_needs_restart)
            {
                goto show;
            }
        }
    }


    if (rom_is_loaded)
    {
        resume_timing ();


        while (! input_process ())
        {
            int speed;


            speed = ((machine_type == MACHINE_TYPE_NTSC) ? 60 : 50);


            if (frame_interrupt)
            {
                if (average_fps == 0)
                {
                    average_fps = speed;
                }


                average_fps += actual_fps_count;

                average_fps /= 2;

                
                if ((frame_skip_min == 0) && (! fast_forward))
                {
                    timing_fps = fix (actual_fps_count, 1, speed);
                
                    timing_hertz = fix (virtual_fps_count, 1, speed);
                
    
                    timing_audio_fps = fix (audio_fps, 1, speed);
                }
                else
                {
                    timing_fps = actual_fps_count;
                
                    timing_hertz = virtual_fps_count;
                
    
                    timing_audio_fps = audio_fps;
                }


                if ((enable_rsync) && (! fast_forward))
                {
                    if ((actual_fps_count < (average_fps - rsync_grace_frames)) && (average_fps > 0))
                    {
                        suspend_timing ();


                        resume_timing ();


                        while (throttle_counter == 0);

                        throttle_counter = 0;
                    }
                }
                else
                {
                    actual_fps_count = 0;
    
                    virtual_fps_count = 0;
    
    
                    audio_fps = 0;
                }


                frame_interrupt = FALSE;
            }


            executed_frames ++;


            /* decrement frame skip counter */
            /* when # of frame periods between start of first skipped frame  */
            /*  before last drawn frame and end of last drawn frame have     */
            /*  passed, draw another */
            if (-- frame_count > 0)
            {
                redraw_flag = FALSE;
            }
            else
            {
                redraw_flag = TRUE;
    
                rendered_frames ++;


                actual_fps_count ++;
    
    
                if ((key [KEY_TILDE]) && (! (input_mode & INPUT_MODE_CHAT)))
                {
                    if (! fast_forward)
                    {
                        /* Fast forward. */
    
                        fast_forward = TRUE;
    
    
                        suspend_throttling ();
                    }


                    frame_count = frame_skip_max;

                    /* zero speed-throttle counter as we're bypassing it */
                    /* throttle_counter = 0; */
                }
                else
                {
                    if (fast_forward)
                    {
                        fast_forward = FALSE;


                        resume_throttling ();
                    }


                    if (frame_skip_min == 0)
                    {
                        while (throttle_counter == 0)
                        {
                            rest (1);
                        }
                    }
    

                    frame_count = throttle_counter;

                    /* update speed-throttle counter */
                    /* using subtract so as to not lose ticks */
                    throttle_counter -= frame_count;


                    /* enforce limits. */
                    /* if we hit one, don't check the other... */
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
            

                    if (mmc_hblank_start)
                    {
                        int type = mmc_hblank_start (ppu_scanline);
                        if (type != CPU_INTERRUPT_NONE)
                        {
                           cpu_interrupt (type);
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
                        int type = mmc_scanline_end (ppu_scanline);
                        if (type != CPU_INTERRUPT_NONE)
                        {
                           cpu_interrupt (type);
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


                    if (mmc_hblank_start)
                    {
                        int type = mmc_hblank_start (ppu_scanline);
                        if (type != CPU_INTERRUPT_NONE)
                        {
                           cpu_interrupt (type);
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
                        int type = mmc_scanline_end (ppu_scanline);
                        if (type != CPU_INTERRUPT_NONE)
                        {
                           cpu_interrupt (type);
                        }
                    }
                }


                papu_process ();
            }


            rest (0);
        }
    }


    set_config_int ("timing", "frame_skip_min", frame_skip_min);

    set_config_int ("timing", "frame_skip_max", frame_skip_max);


    set_config_int ("timing", "machine_type", machine_type);


    set_config_int ("timing", "enable_rsync", enable_rsync);

    set_config_int ("timing", "rsync_grace_frames", rsync_grace_frames);


    set_config_int ("gui", "disable_gui", disable_gui);


    set_config_int ("gui", "first_run", first_run);


    papu_exit ();

    audio_exit ();


    fade_out (4);


    gui_exit ();


    video_exit ();

    ppu_exit ();


    netplay_exit ();


    if (rom_is_loaded)
    {
        cpu_exit ();
    }


    input_exit ();


    log_close ();


    if (rom_is_loaded)
    {
        printf ("Executed frames: %d (%d rendered).\n", executed_frames, rendered_frames);


        free_rom (&global_rom);
    }


    /* unload_datafile (data); */


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
