/* FakeNES - A free, portable, Open Source NES emulator.

   main.c: Implementation of the main emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */
 
#include <allegro.h>
#include <stdio.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "config.h"
#include "cpu.h"
#include "data.h"
#include "debug.h"
#include "gui.h"
#include "input.h"
#include "log.h"
#include "main.h"
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

/* Whether or not this is the first run of the emulator. */
static BOOL first_run = TRUE;

/* Machine region (auto/NTSC/PAL). */
ENUM machine_region = MACHINE_REGION_AUTOMATIC;

/* Machine type (NTSC/PAL). */
ENUM machine_type = MACHINE_TYPE_NTSC;

/* Timing mode (smooth/accurate).

   Smooth uses approximated timings that are much less likely to desync with
   the host machine while under heavy CPU load. */
ENUM machine_timing = MACHINE_TIMING_ACCURATE;

/* CPU usage (passive/normal/aggressive). */
ENUM cpu_usage = CPU_USAGE_PASSIVE;

/* Whether or not speeed will be capped at timing_get_speed(). */
BOOL speed_cap = TRUE;

/* Amount of frames to skip when we fall behind.  -1 = auto. */
int frame_skip = -1;

/* Timing mode.  This should generally always be set to INDIRECT.  See 'timing.h' for comments. */
ENUM timing_mode = TIMING_MODE_INDIRECT;

/* Speed modifiers (all apply in the order listed). */
REAL timing_speed_multiplier = 1.0f;
BOOL timing_half_speed = FALSE;
BOOL timing_fast_forward = FALSE;

/* Public counters (updated once per frame). */
int timing_fps = 0;
int timing_hertz = 0;
int timing_audio_fps = 0;

/* Game clock (in seconds). */
unsigned timing_clock = 0;

/* Number of frames to execute before re-entering the GUI automatically.
   -1 = disabled */
int frames_to_execute = -1;

/* Counters. */
static int executed_frames = 0;
static int rendered_frames = 0;

/* Internal stuff. */
static int actual_fps_count = 0;
static int virtual_fps_count = 0;
static int frame_count = 1;
static volatile BOOL frame_interrupt = FALSE;
static volatile int throttle_counter = 0;

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
   /* Remove timers. */
   remove_int (fps_timer);
   remove_int (throttle_timer);

   /* Reset variables. */
   actual_fps_count = 0;
   virtual_fps_count = 0;
   frame_count = 1;
   frame_interrupt = FALSE;
   throttle_counter = 0;
}

void resume_timing (void)
{
   int timer_ticks_1_hz;
   int timer_ticks;

   /* Reset variables. */
   actual_fps_count = 0;
   virtual_fps_count = 0;
   frame_count = 1;
   frame_interrupt = FALSE;
   throttle_counter = 0;

   /* Determine how many timer ticks to a second. */
   timer_ticks_1_hz = SECS_TO_TIMER(1);

   /* Determine how often our throttle timer will execute, in timer ticks. */
   timer_ticks = ROUND((REAL)timer_ticks_1_hz / timing_get_frame_rate ());

   /* Install timers. */
   install_int_ex (fps_timer,      timer_ticks_1_hz);
   install_int_ex (throttle_timer, timer_ticks);
}

int main (int argc, char *argv[])
{
   int result;
   BOOL want_exit = FALSE;
   BOOL enter_gui = TRUE;

   /* Clear the console. */
   console_clear ();

   if (VERSION == 0x030)
   {
      console_printf ("This release is dedicated to those who fell in the "
         "9/11 attacks.\n\n");
   }

   console_printf ("FakeNES - A free, portable, Open Source NES emulator.\n");
   console_printf ("\n");
   console_printf ("Copyright (c) 2001-2007, FakeNES Team.\n");
   console_printf ("\n");
   console_printf( "This software is provided 'as-is', without any express or implied\n");
   console_printf( "warranty.  In no event will the authors be held liable for any damages\n");
   console_printf( "arising from the use of this software.\n");
   console_printf ("\n");
   console_printf( "Permission is granted to anyone to use this software for any purpose,\n");
   console_printf( "including commercial applications, and to alter it and redistribute it\n");
   console_printf( "freely, subject to the following restrictions:\n");
   console_printf ("\n");
   console_printf( "1. The origin of this software must not be misrepresented; you must not\n");
   console_printf( "   claim that you wrote the original software. If you use this software\n");
   console_printf( "   in a product, an acknowledgment in the product documentation would be\n");
   console_printf( "   appreciated but is not required.\n");
   console_printf( "2. Altered source versions must be plainly marked as such, and must not be\n");
   console_printf( "   misrepresented as being the original software.\n");
   console_printf( "3. This notice may not be removed or altered from any source distribution.\n");

   allegro_init ();

   set_window_title ("FakeNES");

   if ((result = platform_init ()) != 0)
      return ((8 + result));


   /* Load configuration. */
   load_config ();

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
      /* WARN("Failed to initialize audio interface"); */


      /* free_rom (&global_rom);
         return (1); */
      
      WARN("Oops!  It looks like audio failed to initialize.\n"
           "Make sure another application isn't using it (a common problem).\n"
           "\n"
           "I'm disabling it for now.\n"
           "You can try to re-enable it from the Audio menu once inside the program.");
      audio_options.enable_output = FALSE;
      audio_init ();
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
   LOCK_VARIABLE(throttle_Counter);
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
         BOOL redraw_flag;

         if (frame_interrupt)
         {
            /* The FPS timer was triggered; sync counters. */

            timing_fps = actual_fps_count;
            timing_hertz = virtual_fps_count;
            timing_audio_fps = audio_fps;

            actual_fps_count = 0;
            virtual_fps_count = 0;
            audio_fps = 0;

            /* Increment our clock by one second. */
            timing_clock++;

            /* Clear interrupt flag so it doesn't fire again. */
            frame_interrupt = FALSE;
         }


         /* NetPlay. */
         if (netplay_mode)
            netplay_process ();


         /* Fast forward. */

         if ((key [KEY_TILDE]) && (!(input_mode & INPUT_MODE_CHAT)))
         {
            if (!timing_fast_forward)
            {
               /* Enter fast forward mode. */
               timing_fast_forward = TRUE;
               timing_update_timing ();
            }
         }
         else
         {
            if (timing_fast_forward)
            {
               /* Exit fast forward mode. */
               timing_fast_forward = FALSE;
               timing_update_timing ();
            }
         }


         if (--frame_count > 0)
         {
            /* This frame will be executed, but not drawn. */
            redraw_flag = FALSE;
         }
         else
         {
            /* This frame will be executed, and drawn. */
            redraw_flag = TRUE;

            if (speed_cap)
            {
               /* Speed throttling. */
      
               while (throttle_counter == 0)
               {
                  if (cpu_usage == CPU_USAGE_NORMAL)
                     rest (0);
                  else if (cpu_usage == CPU_USAGE_PASSIVE)
                     rest (1);
               }
   
               /* Get all currently pending frames into the frame
                  counter. */
               frame_count = throttle_counter;
   
               /* We use subtract here to avoid losing ticks if the timer
                  fires between this and the last statement. */
               throttle_counter -= frame_count;
   
               /* Enforce frame skip setting if it is not auto. */
               if ((frame_skip != -1) &&
                   (frame_count > frame_skip))
               {
                  frame_count = frame_skip;
               }
            }
         }


         if (rewind_is_enabled ())
         {
            /* Game rewinding. */

            if (input_mode & INPUT_MODE_PLAY)
            {
               if (key[KEY_BACKSLASH])
               {
                  if (!rewind_load_snapshot ())
                  {
                     /* Skip remainder of this frame. */
                     /* TODO: Do user interface input processing before this
                        by splitting it away from the emulation input
                        processing, somehow. */
   
                     /* TODO: Is one update per frame enough for the new audio system?  Not that it's going to perform
                        very wellduring rewinding anyway. :b */
                     apu_sync_update ();
                     audio_update ();
   
                     continue;
                  }
               }
               else
               {
                  rewind_save_snapshot ();
               }
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

               default:
                  break;
            }
       

            input_handle_keypress (c, scancode);
            video_handle_keypress (c, scancode);
            gui_handle_keypress (c, scancode);
         }
         

         /* --- Emulation follows --- */

         executed_frames++;
         virtual_fps_count++;

         input_process ();

         if(redraw_flag) {
            /* Perform a full render. */

            rendered_frames++;
            actual_fps_count++;

            ppu_enable_rendering();
         }
         else {
            ppu_disable_rendering();
         }

         int line, total_lines;
         if(machine_type == MACHINE_TYPE_NTSC)
            total_lines = PPU_TOTAL_LINES_NTSC;
         else
            total_lines = PPU_TOTAL_LINES_PAL;

         for(line = 0; line < total_lines; line++) {
            apu_predict_irqs(SCANLINE_CLOCKS);

            if(mmc_predict_irqs)
               mmc_predict_irqs(SCANLINE_CLOCKS);

            ppu_predict_nmi(SCANLINE_CLOCKS);

            cpu_execute(SCANLINE_CLOCKS);

            apu_sync_update();
            ppu_sync_update();
         }

         if ((frames_to_execute != -1) &&
             (frames_to_execute > 0))
         {
            frames_to_execute--;
            if (frames_to_execute == 0)
            {
               frames_to_execute = -1; /* Disable. */
               enter_gui = TRUE;       /* Schedule GUI reentry. */
            }
         }

         if ((cpu_usage == CPU_USAGE_PASSIVE) ||
             (cpu_usage == CPU_USAGE_NORMAL))
         {
            rest (0);
         }
      }
   }


   /* Save configuration. */
   save_config ();


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

void main_load_config (void)
{
   first_run               = get_config_int   ("gui",    "first_run",    first_run);
   machine_region          = get_config_int   ("timing", "region",       machine_region);
   machine_timing          = get_config_int   ("timing", "mode",         machine_timing);
   cpu_usage               = get_config_int   ("timing", "cpu_usage",    cpu_usage);
   speed_cap               = get_config_int   ("timing", "speed_cap",    speed_cap);
   frame_skip              = get_config_int   ("timing", "frame_skip",   frame_skip);
   timing_speed_multiplier = get_config_float ("timing", "speed_factor", timing_speed_multiplier);

    /* Note: machine_type is set later by the ROM loading code, or more
       specifically, machine_init(). */
}

void main_save_config (void)
{
   set_config_int   ("gui",    "first_run",    first_run);
   set_config_int   ("timing", "region",       machine_region);
   set_config_int   ("timing", "mode",         machine_timing);
   set_config_int   ("timing", "frame_skip",   frame_skip);
   set_config_int   ("timing", "speed_cap",    speed_cap);
   set_config_int   ("timing", "cpu_usage",    cpu_usage);
   set_config_float ("timing", "speed_factor", timing_speed_multiplier);
}

/* TODO: Move all of this into a machine.c or something - try to keep it separate from main. */
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
