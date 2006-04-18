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
#include "debug.h"
#include "gui.h"
#include "input.h"
#include "log.h"
#include "mmc.h"
#include "net.h"
#include "netplay.h"
#include "platform.h"
#include "ppu.h"
#include "rewind.h"
#include "rom.h"
#include "timing.h"
#include "version.h"
#include "video.h"

ENUM machine_type = MACHINE_TYPE_NTSC;

int frame_skip_min = 0;
int frame_skip_max = 0;

ENUM cpu_usage = CPU_USAGE_NORMAL;

static BOOL fast_forward = FALSE;
static BOOL first_run = FALSE;

int timing_fps = 0;
int timing_hertz = 0;
int timing_audio_fps = 0;
int timing_audio_hertz = 0;
BOOL timing_half_speed = FALSE;
static BOOL redraw_flag = TRUE;
static int frame_count = 1;
static int executed_frames = 0;
static int rendered_frames = 0;
static int actual_fps_count = 0;
static int virtual_fps_count = 0;
static int average_fps = 0;
static volatile int throttle_counter = 0;
static volatile BOOL frame_interrupt = FALSE;

static void fps_timer (void)
{
   frame_interrupt = TRUE;
}
END_OF_STATIC_FUNCTION(fps_timer);

static void throttle_timer (void)
{
   throttle_counter++;
}
END_OF_STATIC_FUNCTION(throttle_timer);

void suspend_timing (void)
{
   remove_int (fps_timer);
   remove_int (throttle_timer);

   redraw_flag = TRUE;

   actual_fps_count = 0;
   virtual_fps_count = 0;

   frame_count = 1;

   throttle_counter = 0;

   frame_interrupt = FALSE;
}

void resume_timing (void)
{
   redraw_flag = TRUE;

   actual_fps_count = 0;
   virtual_fps_count = 0;

   frame_count = 1;

   throttle_counter = 0;

   frame_interrupt = FALSE;

   install_int_ex (fps_timer, BPS_TO_TIMER(1));
   install_int_ex (throttle_timer, BPS_TO_TIMER(timing_get_speed ()));
}

void suspend_throttling (void)
{
   remove_int (throttle_timer);

   redraw_flag = TRUE;

   frame_count = 1;

   throttle_counter = 0;
}

void resume_throttling (void)
{
   redraw_flag = TRUE;

   frame_count = 1;

   throttle_counter = 0;

   install_int_ex (throttle_timer, BPS_TO_TIMER(timing_get_speed ()));
}

int main (int argc, char *argv[])
{
   int result;
   BOOL want_exit = FALSE;
   BOOL enter_gui = TRUE;

   /* Save argc and argv. */
   saved_argc = argc;
   saved_argv = argv;

   /* Clear the console. */
   console_clear ();

   if (VERSION == 0x030)
   {
      console_printf ("This release is dedicated to those who fell in the "
         "9/11 attacks.\n\n");
   }

   console_printf ("FakeNES version " VERSION_STRING ", by Siloh and TRAC.\n");
   console_printf ("Using Allegro version " ALLEGRO_VERSION_STR " (" ALLEGRO_PLATFORM_STR ").\n");
   console_printf ("\n");
   console_printf ("Assistance provided by amit, Astxist, ipher, KCat,\n");
   console_printf ("Lord_Nightmare, Mexandrew, and others.  See the\n");
   console_printf ("About box for a complete listing.\n");
   console_printf ("\n");
   console_printf ("Uses the Nofrendo NES APU core by Matthew Conte.\n");
   console_printf ("\n");
   console_printf ("Be sure to visit http://fakenes.sourceforge.net/.\n");
   console_printf ("Report bugs to fakenes-bugs@lists.sourceforge.net.\n");
   console_printf ("\n");
   console_printf ("Copyright (c) 2001-2006, FakeNES Team.\n");
#ifdef POSIX
   console_printf ("This is free software.  See 'LICENSE' for details.\n");
   console_printf ("You must read and accept the license prior to use.\n");
#else
   console_printf ("This is free software.  See 'LICENSE.TXT' for details.\n");
   console_printf ("You must read and accept the license prior to use.\n");
#endif
   console_printf ("\n");

   allegro_init ();

   set_window_title ("FakeNES");

   if ((result = platform_init ()) != 0)
      return ((8 + result));

   frame_skip_min = get_config_int ("timing", "frame_skip_min", 0);
   frame_skip_max = get_config_int ("timing", "frame_skip_max", 4);

   machine_region = get_config_int ("timing", "machine_region", MACHINE_REGION_AUTOMATIC);

    /* Note: machine_type is set later by the ROM loading code. */

   cpu_usage = get_config_int ("timing", "cpu_usage", CPU_USAGE_NORMAL);

   first_run = get_config_int ("gui", "first_run", TRUE);


   install_timer ();


   if (argc >= 2)
   {
      if (load_rom (argv[1], &global_rom) != 0)
      {
         WARN("Failed to load ROM (bad format?)");

         platform_exit ();
         return (1);
      }

      rom_is_loaded = TRUE;

      /* Initialize machine. */
      machine_init ();

      /* Head straight into emulation mode. */
      enter_gui = FALSE;
   }


   net_init ();

   netplay_init ();


   if (input_init () != 0)
   {          
      WARN("PANIC: Failed to initialize input interface");

      return (1);
   }


   if (audio_init () != 0)
   {
      WARN("Failed to initialize audio interface");

      free_rom (&global_rom);
      return (1);
   }


   fade_out (4);


   if (video_init () != 0)
   {
      set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);

      WARN("Failed to initialize video interface");

      free_rom (&global_rom);
      return (1);
   }


   gui_init ();


   LOCK_VARIABLE(frame_interrupt);
   LOCK_VARIABLE(throttle_counter);
   LOCK_FUNCTION(fps_timer);
   LOCK_FUNCTION(throttle_timer);

   /* Start timers. */
   resume_timing ();


   while (!want_exit)
   {
      if (enter_gui)
      {
         want_exit = show_gui (first_run);

         if (first_run)
            first_run = FALSE;

         enter_gui = FALSE;

         /* Skip everything else. */
         continue;
      }

      if (rom_is_loaded)
      {
         if (frame_interrupt)
         {
            timing_fps = actual_fps_count;
            timing_hertz = virtual_fps_count;
            timing_audio_fps = audio_fps;

            actual_fps_count = 0;
            virtual_fps_count = 0;
            audio_fps = 0;

            frame_interrupt = FALSE;
         }

         executed_frames++;
         virtual_fps_count++;

         /* decrement frame skip counter */
         /* when # of frame periods between start of first skipped frame  */
         /*  before last drawn frame and end of last drawn frame have     */
         /*  passed, draw another */
         if (--frame_count > 0)
         {
            redraw_flag = FALSE;
         }
         else
         {
            redraw_flag = TRUE;
 
            rendered_frames++;
            actual_fps_count++;
    
            if ((key [KEY_TILDE]) && (!(input_mode & INPUT_MODE_CHAT)))
            {
               if (!fast_forward)
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
                        rest (0);
                     else if (cpu_usage == CPU_USAGE_PASSIVE)
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
                  frame_count = frame_skip_max;
                else if (frame_count <= frame_skip_min)
                  frame_count = frame_skip_min;
             }
          }

          if (input_mode & INPUT_MODE_PLAY)
          {
            /* Game rewinding. */

            if (key[KEY_BACKSLASH])
            {
               if (!rewind_load_snapshot ())
               {
                  /* Skip remainder of this frame. */
                  /* TODO: Do user interface input processing before this
                     by splitting it away from the emulation input
                     processing, somehow. */

                  audio_update ();

                  continue;
               }
            }
            else
            {
               rewind_save_snapshot ();
            }
         }        

         /* Process input. */
         while (keypressed ())
         {
            int c, scancode;

            c = ureadkey (&scancode);

            switch (scancode)
            {
               case KEY_ESC:
               {
                  /* ESC - Enter GUI. */

                  enter_gui = TRUE;
   
                  break;
               }

               case KEY_BACKSPACE:
               {
                  if (!(input_mode & INPUT_MODE_CHAT))
                  {
                     input_mode &= ~INPUT_MODE_PLAY;
                     input_mode |= INPUT_MODE_CHAT;
                  }
       
                  break;
               }
       
               default:
                  break;
            }
       

            input_handle_keypress (c, scancode);
            video_handle_keypress (c, scancode);
            gui_handle_keypress (c, scancode);
         }
         
         input_process ();


         /* --- Emulation follows --- */

         switch (machine_type)
         {
            case MACHINE_TYPE_PAL:
            {
               ppu_frame_last_line = (TOTAL_LINES_PAL - 1);

               break;
            }

            case MACHINE_TYPE_NTSC:
            {
               ppu_frame_last_line = (TOTAL_LINES_NTSC - 1);

               break;
            }

            default:
               WARN_GENERIC ();
         }

         if (redraw_flag)
         {
            /* Perform a full render. */

            ppu_start_frame ();

            if (input_enable_zapper)
               input_update_zapper_offsets ();

            for (ppu_scanline = 0; ppu_scanline <= ppu_frame_last_line;
               ppu_scanline++)
            {
               cpu_start_new_scanline ();

               if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                   (ppu_scanline <= LAST_DISPLAYED_LINE))
               {
                  ppu_start_line ();

                  ppu_render_line (ppu_scanline);

                  /* handle zapper emulation */
                  if (input_enable_zapper &&
                      (input_zapper_y_offset == ppu_scanline) &&
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
               else if (ppu_scanline == (FIRST_VBLANK_LINE + 1))
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
                  cpu_interrupt (mmc_hblank_start (ppu_scanline));

               if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                   (ppu_scanline <= LAST_DISPLAYED_LINE))
               {
                  cpu_execute (HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP);

                  ppu_end_line ();

                  cpu_execute ((HBLANK_CLOCKS -
                     HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP));
               }
               else
               {
                  cpu_execute (HBLANK_CLOCKS);
               }

               if (mmc_scanline_end)
                  cpu_interrupt (mmc_scanline_end (ppu_scanline));
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
               input_update_zapper_offsets ();

            for (ppu_scanline = 0; ppu_scanline <= ppu_frame_last_line;
               ppu_scanline++)
            {
               cpu_start_new_scanline ();

               if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                   (ppu_scanline <= LAST_DISPLAYED_LINE))
               {
                  ppu_start_line ();

                  /* draw lines for zapper emulation */
                  
                  if (input_enable_zapper &&
                      (input_zapper_y_offset == ppu_scanline) &&
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
               else if (ppu_scanline == (FIRST_VBLANK_LINE + 1))
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
                  cpu_interrupt (mmc_hblank_start (ppu_scanline));

               if ((ppu_scanline >= FIRST_DISPLAYED_LINE) &&
                   (ppu_scanline <= LAST_DISPLAYED_LINE))
               {
                  cpu_execute (HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP);

                  ppu_end_line ();

                  cpu_execute ((HBLANK_CLOCKS -
                     HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP));
               }
               else
               {
                  cpu_execute (HBLANK_CLOCKS);
               }

               if (mmc_scanline_end)
                  cpu_interrupt (mmc_scanline_end (ppu_scanline));
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

   set_config_int ("gui", "first_run", first_run);


   if (rom_is_loaded)
   {
      machine_exit ();

      free_rom (&global_rom);
   }


   fade_out (4);

   video_exit ();


   audio_exit ();


   input_exit ();


   netplay_exit ();

   net_exit ();


   gui_exit ();


     log_printf ("Executed frames: %d (%d rendered).", executed_frames, rendered_frames);


    platform_exit ();


    /* unload_datafile (data); */


    return (0);
}

END_OF_MAIN()

int machine_init (void)
{
   if (!rom_is_loaded)
   {
      WARN("machine_init() called with no ROM loaded");
      return (1);
   }

   /* Fixup machine type from region. */
   timing_update_machine_type ();

   if (cpu_init () != 0)
   {
      WARN("Failed to initialize the CPU core");
 
      free_rom (&global_rom);
      return (2);
   }

   if (mmc_init () != 0)
   {
      WARN("mmc_init() failed (unsupported mapper?)");
 
      free_rom (&global_rom);
      return (3);
   }

   if (ppu_init () != 0)
   {
      WARN("Failed to initialize the PPU core");
 
      free_rom (&global_rom);
      return (4);
   }
 
   if (apu_init () != 0)
   {
      WARN("Failed to initialize the APU core");
 
      free_rom (&global_rom);
      return (5);
   }

   input_reset ();

   if (rewind_init () != 0)
   {
      WARN("Failed to initialize the rewinder");
 
      free_rom (&global_rom);
      return (6);
   }

   /* Reset everything.  Although this should be already performed by the
      respective init functions, we do it again here just in case. */
   machine_reset ();

   /* Return success. */
   return (0);
}

void machine_exit (void)
{
   if (!rom_is_loaded)
   {
      WARN("machine_exit() called with no ROM loaded");
      return;
   }

   rewind_exit ();

   apu_exit ();

   ppu_exit ();

   // mmc_exit ();

   cpu_exit ();


   rewind_clear ();
}

void machine_reset (void)
{
   cpu_reset ();

   mmc_reset ();

   ppu_reset ();

   apu_reset ();

   input_reset ();
}
