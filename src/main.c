 

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

int frame_skip_max = 8;


volatile int timing_fps = 0;

volatile int timing_hertz = 0;


volatile int timing_audio_fps = 0;

volatile int timing_audio_hertz = 0;


#ifdef POSIX

char * homedir = NULL;

char * datfile = NULL;

char * sramdir = NULL;

char * confdir = NULL;

static DIR *tmpdir = NULL;

#endif


static int redraw_flag = TRUE;


static int frame_count = 1;


static int emulated_frames = 0;

static int displayed_frames = 0;


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


    printf ("\nFakeNES version 0.1.1 (CVS), by stainless and TRAC.\n"
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

    set_window_title ("FakeNES");

#ifdef POSIX

    /* by amit */

    /* Configuration directory checking */
    homedir = getenv("HOME");
    if(homedir != NULL)
    {
        const char confdir_base[] = "/.fakenes";
        const char sramdir_base[] = "/sram";

        confdir = (char *) malloc(strlen(homedir) +
            sizeof(confdir_base));
        if (confdir != NULL)
        {
            strcpy(confdir, homedir);
            strcat(confdir, confdir_base);

            sramdir = (char *) malloc(strlen(confdir) + sizeof(sramdir_base));
            if (sramdir != NULL)
            {
                strcpy(sramdir, confdir);
                strcat(sramdir, sramdir_base);
            }
        }
        else
        {
            sramdir=NULL;
        }

        if (confdir == NULL)
        {
            fprintf(stderr, "Allocation error when generating configuration path. Configuration will not be saved.\n");
        }
        else
        {
            /* Just see if we can open the dir */
            if((tmpdir = opendir(confdir)) == NULL)
            {
                /* Directory doesn't exist, create it */
                if(errno == ENOENT)
                {
                    if(mkdir(confdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
                    {
                        fprintf(stderr, "Error creating \"%s\". Configuration will not be saved.\n", confdir);
                        free(confdir);
                        confdir=NULL;
                    } else /* mkdir was successful, go on and make the sram dir */
                    {
                        /* Error checking for sramdir happens later */
                        mkdir(sramdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
                    }
                } else if(errno == ENOTDIR) /* Not a directory */
                {
                    fprintf(stderr, "Warning: \"%s\" is not a directory, configuration will not be saved.\n", confdir);
                    free(confdir);
                    confdir=NULL;
                } else if(errno == EACCES) /* Permission denied */
                {
                    fprintf(stderr, "Warning: Permission denied when opening \"%s\". Configuration will not be saved.\n", confdir);
                    free(confdir);
                    confdir=NULL;
                } else /* Something else */
                {
                    fprintf(stderr, "Warning: an error occured when opening \"%s\". Configuration will not be saved.\n", confdir);
                    free(confdir);
                    confdir=NULL;
                }
            }
            else
            {
                /* Close the dir we just opened */
                closedir(tmpdir);
            }
        }

    }
    else
    {
	fprintf(stderr, "$HOME is not set, configuration will not be saved.\n");
    }

    /* Load up the config file */
    if(confdir != NULL)
    {
        const char conffile_base[] = "/config";

        char *conffile = malloc(strlen(confdir) + sizeof(conffile_base));
	strcpy(conffile, confdir);
        strcat(conffile, conffile_base);
	set_config_file (conffile);
    } else
    {
	set_config_file ("/dev/null");
    }

    /* Check the sram dir */
    if(sramdir == NULL)
    {
        /* if we have a valid home dir, there was an allocation error */
        if (homedir != NULL)
        {
            fprintf(stderr, "Allocation error when generating save path. SRAM files will not be saved.\n");
        }
    }
    else
    {
        if((tmpdir = opendir(sramdir)) == NULL)
        {
            if(errno == ENOENT)
            {
                if(mkdir(sramdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
                {
                    fprintf(stderr, "Error creating sram directory \"%s\"", sramdir);
                    free(sramdir);
                    sramdir=NULL;
                }
            } else if(errno == ENOTDIR) /* Not a directory */
            {
                fprintf(stderr, "Warning: \"%s\" is not a directory, configuration will not be saved.\n", confdir);
                free(confdir);
                sramdir=NULL;
            } else if(errno == EACCES) /* Permission denied */
            {
                fprintf(stderr, "Warning: Permission denied when opening \"%s\". SRAM files will not be saved.\n", sramdir);
                free(sramdir);
                sramdir=NULL;
            } else /* Something else */
            {
                fprintf(stderr, "Warning: an error occured when opening \"%s\". SRAM files will not be saved.\n", sramdir);
                free(sramdir);
                sramdir=NULL;
            }
        }
        else
        {
            /* Close the dir we just opened */
            closedir(tmpdir);
        }
    }
#else

    set_config_file ("fakenes.cfg");

#endif


    frame_skip_min = get_config_int ("timing", "frame_skip_min", 0);

    frame_skip_max = get_config_int ("timing", "frame_skip_max", 8);


    machine_type = get_config_int ("timing", "machine_type", MACHINE_TYPE_NTSC);


    install_timer ();


#ifdef POSIX

    if(confdir != NULL)
    {
        const char datfile_base[] = "/fakenes.dat";

        datfile = malloc(strlen(confdir) + sizeof(datfile_base));
        if (datfile == NULL)
        {
            fprintf(stderr, "Allocation error when generating datafile path, trying cwd.\n");
            datfile = "fakenes.dat";
            data = load_datafile ("fakenes.dat");
        }
        else
        {
            strcpy(datfile, confdir);
            strcat(datfile, datfile_base);
            data = load_datafile (datfile);
            if (data == NULL)
            {
                fprintf(stderr, "Datafile not found in configuration path, trying cwd.\n");
                datfile = "fakenes.dat";
                data = load_datafile (datfile);
            }
        }
    } else /* confdir unset, try cwd */
    {
        datfile = "fakenes.dat";
        data = load_datafile (datfile);
    }

#else

    data = load_datafile ("fakenes.dat");

#endif

    if (! data)
    {
        fprintf (stderr,
#ifdef POSIX
            "PANIC: Failed to load datafile: \"%s\".\n", datfile);
#else
            "PANIC: Failed to load datafile: fakenes.dat.\n");
#endif

        return (1);
    }


    if (input_init () != 0)
    {
        fprintf (stderr,
            "PANIC: Failed to initialize input interface!\n");

        return (1);
    }


    input_reset ();


    if (argc >= 2)
    {
        if (load_rom (argv [1], &global_rom) != 0)
        {
            fprintf (stderr,
                "PANIC: Failed to load ROM file (bad format?).\n");
    
            return (1);
        }


        rom_is_loaded = TRUE;
    }
    else
    {
        rom_is_loaded = FALSE;
    }


    result = machine_init ();

    if (result != 0)
    {
        return (result);
    }


    if (audio_init () != 0)
    {
        fprintf (stderr,
            "PANIC: Failed to initialize audio interface!\n");


        free_rom (&global_rom);

        return (1);
    }


    papu_init ();

    papu_reset ();


    if (video_init () != 0)
    {
        fprintf (stderr,
            "PANIC: Failed to initialize video interface!\n");


        free_rom (&global_rom);

        return (1);
    }


    LOCK_VARIABLE (timing_fps);

    LOCK_VARIABLE (timing_hertz);


    LOCK_VARIABLE (audio_fps);

    LOCK_VARIABLE (timing_audio_fps);


    LOCK_VARIABLE (actual_fps_count);

    LOCK_VARIABLE (virtual_fps_count);
  

    LOCK_VARIABLE (throttle_counter);


    LOCK_FUNCTION (fps_interrupt);

    LOCK_FUNCTION (throttle_interrupt);


    if (! rom_is_loaded)
    {
        show_gui ();
    }


    if (rom_is_loaded)
    {
        resume_timing ();


        while (! input_process ())
        {
            if (-- frame_count > 0)
            {
                redraw_flag = FALSE;
            }
            else
            {
                redraw_flag = TRUE;
    
    
                actual_fps_count ++;
    
                displayed_frames ++;
    
    
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
    
            emulated_frames ++;
    
    
            switch (machine_type)
            {
                case MACHINE_TYPE_PAL:
                    ppu_frame_last_line = TOTAL_LINES_PAL - 1;
                    break;

                case MACHINE_TYPE_NTSC:
                default:
                    ppu_frame_last_line = TOTAL_LINES_NTSC - 1;
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

                        cpu_execute (HBLANK_CLOCKS - HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP);
                    }
                    else
                    {
                        cpu_execute (HBLANK_CLOCKS);
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

                        cpu_execute (HBLANK_CLOCKS - HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP);
                    }
                    else
                    {
                        cpu_execute (HBLANK_CLOCKS);
                    }


                }


                papu_process ();
            }
        }
    }


    set_config_int ("timing", "frame_skip_min", frame_skip_min);

    set_config_int ("timing", "frame_skip_max", frame_skip_max);


    set_config_int ("timing", "machine_type", machine_type);


    papu_exit ();

    audio_exit ();


    video_exit ();

    ppu_exit ();


    if (rom_is_loaded)
    {
        cpu_exit ();
    }


    input_exit ();


    if (rom_is_loaded)
    {
        printf ("Emulated frames: %d (%d "
            "displayed).\n", emulated_frames, displayed_frames);

        free_rom (&global_rom);
    }


    unload_datafile (data);


    return (0);
}

END_OF_MAIN ();


int machine_init (void)
{
    if (rom_is_loaded)
    {
        cpu_memmap_init ();

        if (mmc_init () != 0)
        {
            fprintf (stderr,
                "PANIC: mmc_init() failed (unsupported mapper?).\n");
    
    
            free_rom (&global_rom);
    
            return (1);
        }


        if (cpu_init () != 0)
        {
            fprintf (stderr,
                "PANIC: Failed to initialize the CPU core!\n");
    
    
            free_rom (&global_rom);
    
            return (1);
        }
    
        
        if (ppu_init () != 0)
        {
            fprintf (stderr,
                "PANIC: Failed to initialize the PPU core!\n");
    
    
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
