/* FakeNES - A free, portable, Open Source NES emulator.

   main.c: Implementation of the main emulation.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */
 
#define ALLEGRO_USE_CONSOLE
#include <allegro.h>
#include <stdio.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "cpu.h"
#include "data.h"
#include "gui.h"
#include "input.h"
#include "mmc.h"
#include "net.h"
#include "netplay.h"
#include "platform.h"
#include "ppu.h"
#include "rom.h"
#include "timing.h"
#include "version.h"
#include "video.h"


int disable_gui = FALSE;


ENUM machine_type = MACHINE_TYPE_NTSC;


int frame_skip_min = 0;

int frame_skip_max = 0;


ENUM cpu_usage = CPU_USAGE_NORMAL;


static int fast_forward = FALSE;


static int first_run = FALSE;


int timing_fps = 0;

int timing_hertz = 0;


int timing_audio_fps = 0;

int timing_audio_hertz = 0;


BOOL timing_half_speed = FALSE;


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
}


int main (int argc, char * argv [])
{
    int result;


    /* Save argc and argv. */
    saved_argc = argc;
    saved_argv = argv;


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


    if ((result = platform_init ()) != 0)
        return ((8 + result));


    frame_skip_min = get_config_int ("timing", "frame_skip_min", 0);

    frame_skip_max = get_config_int ("timing", "frame_skip_max", 4);


    machine_region = get_config_int ("timing", "machine_region", MACHINE_REGION_AUTOMATIC);

    /* Note: machine_type is set later by the ROM loading code. */


    cpu_usage = get_config_int ("timing", "cpu_usage", CPU_USAGE_NORMAL);


    disable_gui = get_config_int ("gui", "disable_gui", FALSE);


    first_run = get_config_int ("gui", "first_run", TRUE);


    install_timer ();


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


    apu_init ();


    net_init ();

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

                
                timing_fps = actual_fps_count;
            
                timing_hertz = virtual_fps_count;
            

                timing_audio_fps = audio_fps;


                actual_fps_count = 0;

                virtual_fps_count = 0;
 
 
                audio_fps = 0;


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
                            if (cpu_usage == CPU_USAGE_NORMAL)
                            {
                                rest (0);
                            }
                            else if (cpu_usage == CPU_USAGE_PASSIVE)
                            {
                                 rest (1);
                            }
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

                ppu_start_frame ();

                if (input_enable_zapper)
                {
                    input_update_zapper_offsets ();
                }

                for (ppu_scanline = 0; ppu_scanline <= ppu_frame_last_line; ppu_scanline ++)
                {
                    cpu_start_new_scanline ();


                    if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                        (ppu_scanline <= LAST_DISPLAYED_LINE))
                    {
                        ppu_start_line ();


                        ppu_render_line (ppu_scanline);

                         /* handle zapper emulation */
                         if (input_enable_zapper && (input_zapper_y_offset == ppu_scanline) &&
                             input_zapper_on_screen)
                         {
                             input_update_zapper ();
                         }

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
                        cpu_interrupt (mmc_hblank_start (ppu_scanline));
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
                        cpu_interrupt (mmc_scanline_end (ppu_scanline));
                    }
                }


                video_blit (screen);

                apu_process ();

                audio_update ();
            }
            else
            {
                /* Perform a partial render. */

                ppu_start_frame ();

                if (input_enable_zapper)
                {
                    input_update_zapper_offsets ();
                }

                for (ppu_scanline = 0; ppu_scanline <= ppu_frame_last_line; ppu_scanline ++)
                {
                    cpu_start_new_scanline ();


                    if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                        (ppu_scanline <= LAST_DISPLAYED_LINE))
                    {
                        ppu_start_line ();

                      /* draw lines for zapper emulation */
                  
                      if (input_enable_zapper && (input_zapper_y_offset == ppu_scanline) &&
                          input_zapper_on_screen)
                      {
                          ppu_render_line (ppu_scanline);

                          input_update_zapper ();
                      }
                      else
                      {
                          ppu_stub_render_line (ppu_scanline);
                      }

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
                        cpu_interrupt (mmc_hblank_start (ppu_scanline));
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
                        cpu_interrupt (mmc_scanline_end (ppu_scanline));
                    }
                }


                apu_process ();

                audio_update ();
            }


            if ((cpu_usage == CPU_USAGE_PASSIVE) ||
                (cpu_usage == CPU_USAGE_NORMAL))
            {
                rest (0);
            }
        }
    }


    set_config_int ("timing", "frame_skip_min", frame_skip_min);

    set_config_int ("timing", "frame_skip_max", frame_skip_max);


    set_config_int ("timing", "machine_region", machine_region);


    set_config_int ("timing", "cpu_usage", cpu_usage);


    set_config_int ("gui", "disable_gui", disable_gui);


    set_config_int ("gui", "first_run", first_run);


    apu_exit ();

    audio_exit ();


    fade_out (4);


    gui_exit ();


    video_exit ();

    ppu_exit ();


    netplay_exit ();

    net_exit ();


    if (rom_is_loaded)
    {
        cpu_exit ();
    }


    input_exit ();


    platform_exit ();


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
    mmc_reset ();

    apu_reset ();

    ppu_reset ();

    input_reset ();

    cpu_reset ();
}
