

/*

FakeNES - A portable, open-source NES emulator.

main.c: Implementation of the main emulation.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#define USE_CONSOLE


#include <allegro.h>

#include "build.h"


#include <stdio.h>

#include <stdlib.h>

#include <string.h>


#ifdef UNIX

#include <sys/stat.h>

#include <sys/types.h>


#include <dirent.h>

#include <errno.h>


extern int errno;

#endif


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

int frame_skip_max = 6;


volatile int timing_fps = 0;

volatile int timing_hertz = 0;


#ifdef UNIX

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
    install_int_ex (fps_interrupt, BPS_TO_TIMER(1));

    install_int_ex (throttle_interrupt, BPS_TO_TIMER(60));
}


int main (int argc, char * argv [])
{
    int line;

    int result;


    printf ("\nFakeNES 0.0.1 (pre0-5), by stainless and RobotBebop.\n");

    printf ("\nBe sure to visit http://fakenes.sourceforge.net/.\n"
            "Report bugs to fakenes-bugs@lists.sourceforge.net.\n");


#ifdef UNIX

    printf ("\nCopyright (c) 2001, Randy McDowell and Ian Smith.\n"
            "All rights reserved.  See 'LICENSE' for details.\n\n");

#else

    printf ("\nCopyright (c) 2001, Randy McDowell and Ian Smith.\n"
            "All rights reserved.  See 'LICENSE.TXT' for details.\n\n");

#endif


    allegro_init ();


#ifdef UNIX

    // by amit
    set_window_title ("FakeNES");

    // Configuration directory checking
    if(getenv("HOME")!=NULL)
    {
        // the 11 comes from strlen("/.fakenes") and the \0 at the end
        confdir = (char *) malloc(strlen(getenv("HOME")) + 11);
        strcpy(confdir, getenv("HOME"));
        strcat(confdir, "/.fakenes");

	sramdir = (char *) malloc(strlen(confdir) + 6);
	strcpy(sramdir, confdir);
	strcat(sramdir, "/sram");

	// Just see if we can open the dir
	if((tmpdir = opendir(confdir)) == NULL)
	{
	    // Directory doesn't exist, create it
	    if(errno == ENOENT)
	    {
		if(mkdir(confdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
		{
		    fprintf(stderr, "Error creating \"%s\". Configuration will not be saved.\n", confdir);
		    free(confdir);
		    confdir=NULL;
		} else // mkdir was successful, go on and make the sram dir
		{
		    // Error checking for sramdir happens later
		    mkdir(sramdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
		}
	    } else if(errno == ENOTDIR) // Not a directory
	    {
		fprintf(stderr, "Warning: \"%s\" is not a directory, configuration will not be saved.\n", confdir);
		free(confdir);
		confdir=NULL;
	    } else if(errno == EACCES) // Permission denied
	    {
		fprintf(stderr, "Warning: Permission denied when opening \"%s\". Configuration will not be saved.\n", confdir);
		free(confdir);
		confdir=NULL;
	    } else // Something else
	    {
		fprintf(stderr, "Warning: an error occured when opening \"%s\". Configuration will not be saved.\n", confdir);
		free(confdir);
		confdir=NULL;
	    }
	}
	closedir(tmpdir);

    }
    else
    {
	fprintf(stderr, "$HOME is not set, configuration will not be saved.\n");
    }

    // Load up the config file
    if(confdir != NULL)
    {
	char *conffile = malloc(strlen(confdir) + 7);
	strcpy(conffile, confdir);
	strcat(conffile, "/config");
	set_config_file (conffile);
    } else
    {
	set_config_file ("/dev/null");
    }

    // Check the sram dir
    if(sramdir != NULL && (tmpdir = opendir(sramdir)) == NULL)
    {
	if(errno == ENOENT)
	{
	    if(mkdir(sramdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1)
	    {
		fprintf(stderr, "Error creating sram directory \"%s\"", sramdir);
		free(sramdir);
		sramdir=NULL;
	    }
	} else if(errno == ENOTDIR) // Not a directory
	{
	    fprintf(stderr, "Warning: \"%s\" is not a directory, configuration will not be saved.\n", confdir);
	    free(confdir);
	    confdir=NULL;
	} else if(errno == EACCES) // Permission denied
	{
	    fprintf(stderr, "Warning: Permission denied when opening \"%s\". SRAM files will not be saved.\n", sramdir);
	    free(sramdir);
	    confdir=NULL;
	} else // Something else
	{
	    fprintf(stderr, "Warning: an error occured when opening \"%s\". SRAM files will not be saved.\n", sramdir);
	    free(sramdir);
	    confdir=NULL;
	}
    }

#else

    set_window_title ("FakeNESw");

    set_config_file ("fakenes.cfg");

#endif


    frame_skip_min = get_config_int ("timing", "frame_skip_min", 0);

    frame_skip_max = get_config_int ("timing", "frame_skip_max", 6);


    install_timer ();


#ifdef UNIX

    if(confdir != NULL)
    {
	datfile = malloc(strlen(confdir) + 13);
	strcpy(datfile, confdir);
	strcat(datfile, "/fakenes.dat");
	data = load_datafile (datfile);
    } else // confdir unset, try cwd
	data = load_datafile ("fakenes.dat");
    
#else

    data = load_datafile ("fakenes.dat");

#endif

    if (! data)
    {
        fprintf (stderr,
#ifdef UNIX
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


    if (video_init () != 0)
    {
        fprintf (stderr,
            "PANIC: Failed to initialize video interface!\n");


        free_rom (&global_rom);

        return (1);
    }


    LOCK_VARIABLE (timing_fps);

    LOCK_VARIABLE (timing_hertz);


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
        while (! input_process ())
        {
            papu_update_length_counter ();


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
    
    
                emulated_frames ++;
            }
    
    
            virtual_fps_count ++;
    
            emulated_frames ++;
    
    
            if (redraw_flag)
            {
                /* Perform a full render. */
    
                video_clear ();
    
        
                if (machine_type == MACHINE_TYPE_NTSC)
                {
                    /* Use NTSC timing. */
    
                    for (line = 0; line < TOTAL_LINES_NTSC; line ++)
                    {
                        if (mmc_scanline_start)
                        {
                            if (! mmc_disable_irqs)
                            {
                                if (mmc_scanline_start (line))
                                {
                                    cpu_interrupt (CPU_INTERRUPT_IRQ);
                                }
                            }
                        }


                        if ((line >= FIRST_DISPLAYED_LINE) &&
                            (line <= LAST_DISPLAYED_LINE))
                        {
                            ppu_start_line ();

                            ppu_render_line (line);

                            ppu_end_line ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else if (line == FIRST_VBLANK_LINE)
                        {
                            ppu_render ();

                            cpu_execute (SCANLINE_CLOCKS - 1);
                        }
                        else if (line == (TOTAL_LINES_NTSC - 1))
                        {
                            ppu_clear ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else
                        {
                            cpu_execute (SCANLINE_CLOCKS);
                        }
            
                    }
                }
                else
                {
                    /* Use PAL timing. */
    
                    for (line = 0; line < TOTAL_LINES_PAL; line ++)
                    {
                        if (mmc_scanline_start)
                        {
                            if (! mmc_disable_irqs)
                            {
                                if (mmc_scanline_start (line))
                                {
                                    cpu_interrupt (CPU_INTERRUPT_IRQ);
                                }
                            }
                        }


                        if ((line >= FIRST_DISPLAYED_LINE) &&
                            (line <= LAST_DISPLAYED_LINE))
                        {
                            ppu_start_line ();

                            ppu_render_line (line);

                            ppu_end_line ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else if (line == FIRST_VBLANK_LINE)
                        {
                            ppu_render ();

                            cpu_execute (SCANLINE_CLOCKS - 1);
                        }
                        else if (line == (TOTAL_LINES_PAL - 1))
                        {
                            ppu_clear ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else
                        {
                            cpu_execute (SCANLINE_CLOCKS);
                        }
    
                    }
                }
            }
            else
            {
                /* Perform a partial render. */
    
                if (machine_type == MACHINE_TYPE_NTSC)
                {
                    /* Use NTSC timing. */
    
                    for (line = 0; line < TOTAL_LINES_NTSC; line ++)
                    {
                        if (mmc_scanline_start)
                        {
                            if (! mmc_disable_irqs)
                            {
                                if (mmc_scanline_start (line))
                                {
                                    cpu_interrupt (CPU_INTERRUPT_IRQ);
                                }
                            }
                        }
    

                        if ((line >= FIRST_DISPLAYED_LINE) &&
                            (line <= LAST_DISPLAYED_LINE))
                        {
                            ppu_start_line ();

                            ppu_end_line ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else if (line == FIRST_VBLANK_LINE)
                        {
                            ppu_vblank ();

                            cpu_execute (SCANLINE_CLOCKS - 1);
                        }
                        else if (line == (TOTAL_LINES_NTSC - 1))
                        {
                            ppu_clear ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else
                        {
                            cpu_execute (SCANLINE_CLOCKS);
                        }
            
                    }
                }
                else
                {
                    /* Use PAL timing. */
    
                    for (line = 0; line < TOTAL_LINES_PAL; line ++)
                    {
                        if (mmc_scanline_start)
                        {
                            if (! mmc_disable_irqs)
                            {
                                if (mmc_scanline_start (line))
                                {
                                    cpu_interrupt (CPU_INTERRUPT_IRQ);
                                }
                            }
                        }
    

                        if ((line >= FIRST_DISPLAYED_LINE) &&
                            (line <= LAST_DISPLAYED_LINE))
                        {
                            ppu_start_line ();

                            ppu_end_line ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else if (line == FIRST_VBLANK_LINE)
                        {
                            ppu_vblank ();

                            cpu_execute (SCANLINE_CLOCKS - 1);
                        }
                        else if (line == (TOTAL_LINES_PAL - 1))
                        {
                            ppu_clear ();

                            cpu_execute (SCANLINE_CLOCKS);
                        }
                        else
                        {
                            cpu_execute (SCANLINE_CLOCKS);
                        }
            
                    }
                }
            }
        }
    }


    set_config_int ("timing", "frame_skip_min", frame_skip_min);

    set_config_int ("timing", "frame_skip_max", frame_skip_max);


    video_exit ();

    ppu_exit ();


    cpu_exit ();

    mmc_exit ();


    input_exit ();


    if (rom_is_loaded)
    {
        printf ("Emulated frames: %d (%d "
            "displayed).\n", emulated_frames, displayed_frames);
    }


    free_rom (&global_rom);

    unload_datafile (data);


    return (0);
}

END_OF_MAIN ();


int machine_init (void)
{
    if (rom_is_loaded)
    {
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
    

        resume_timing ();
    }


    return (0);
}


void machine_reset (void)
{
    input_reset ();


    mmc_reset ();


    ppu_reset ();

    cpu_reset ();
}
