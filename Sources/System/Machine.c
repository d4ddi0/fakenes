/* FakeNES - A free, portable, Open Source NES emulator.

   machine.c: Implementation of the virtual machine
   emulation.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "cpu.h"
#include "gui.h"
#include "input.h"
#include "load.h"
#include "machine.h"
#include "main.h"
#include "mmc.h"
#include "netplay.h"
#include "nsf.h"
#include "ppu.h"
#include "rewind.h"
#include "timing.h"
#include "types.h"
#include "video.h"

/* Machine region (auto/NTSC/PAL). */
ENUM machine_region = MACHINE_REGION_AUTOMATIC;

/* Machine type (NTSC/PAL). Derived from the machine region. */
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

/* Timing clock (in milliseconds). Actual accuracy varies depending on the machine type
   and speed modifiers (NTSC gives a base accuracy of about 16ms). */
UINT32 timing_clock = 0;
/* Keep this float so it can be saved as a UINT32 using a cast. */
static float timing_clock_delta = 0;

/* Game clock. This is derived from the timing clock, and is included in save states.
   The only way this is ever cleared is via machine_reset_game_clock(). */
UINT16 game_clock_milliseconds = 0;
UINT8 game_clock_seconds = 0;
UINT8 game_clock_minutes = 0;
UINT8 game_clock_hours = 0;
UINT16 game_clock_days = 0;

/* As the PPU can generate a frame at any time, this is used to lock the main loop with PPU
   frame generation for timing purposes. */
BOOL frame_lock = FALSE;

/* Frame counters. These are just logged to a file at the end of emulation, in order to
   give an idea of system performance. More rendered frames is better. */
int executed_frames = 0;
int rendered_frames = 0;

/* Internal stuff. */
static int actual_fps_count = 0;
static int virtual_fps_count = 0;
static int frame_count = 1;
/* Note: These need to be marked volatile so they won't crash. */
static volatile BOOL frame_interrupt = FALSE;
static volatile int throttle_counter = 0;

/* Since Allegro's GUI routines access the keyboard buffer (which removes keys from it),
   we'll get conflicts if both machine_main() and the GUI are running at hte same time,
   which is not uncommon. To get around this, we maintain our own keyboard buffer. Note that
   this may be accessed from an interrupt context, so it must be volatile, and locked. */
#define KEY_BUFFER_MAX	16
static volatile int key_names[KEY_BUFFER_MAX];
static volatile int key_codes[KEY_BUFFER_MAX];
static volatile int key_buffer_size = 0;

/* Function prototypes. */
static void fps_timer(void);
static void throttle_timer(void);
static int keyboard_handler(int key, int* scancode);

static UINT8 read_registers(const UINT16 address);
static void write_registers(const UINT16 address, const UINT8 data);

void machine_load_config(void)
{
   cpu_usage      = get_config_int("timing", "cpu_usage",  cpu_usage);
   frame_skip     = get_config_int("timing", "frame_skip", frame_skip);
   machine_region = get_config_int("timing", "region",     machine_region);
   machine_timing = get_config_int("timing", "mode",       machine_timing);
   speed_cap      = get_config_int("timing", "speed_cap",  speed_cap);
   timing_speed_multiplier = get_config_float("timing", "speed_factor", timing_speed_multiplier);

  /* Note: machine_type is set later by the ROM loading code, or more
     specifically, machine_init(). */
}

void machine_save_config(void)
{
   set_config_int  ("timing", "cpu_usage",    cpu_usage); 
   set_config_int  ("timing", "frame_skip",   frame_skip);
   set_config_int  ("timing", "mode",         machine_timing);
   set_config_int  ("timing", "region",       machine_region);
   set_config_int  ("timing", "speed_cap",    speed_cap);
   set_config_float("timing", "speed_factor", timing_speed_multiplier);
}

int machine_init(void)
{
   int i;

   /* Sanity check. */
   if(!file_is_loaded) {
      WARN("machine_init() called with no file loaded");
      return 1;
   }

   /* Determine machine type from the region. */
   timing_update_machine_type();

   /* Initialize each component starting with the Central Processing Unit (CPU).
      Note that the order in which this is done is very important! */
   if(cpu_init() != 0) {
      WARN("Failed to initialize the CPU core");
      machine_exit();
      return 1;
   }

   /* Map in hardware registers. */
   cpu_map_block_read_handler(0x4000, read_registers);
   cpu_map_block_write_handler(0x4000, write_registers);

   /* Initialize Picture Processing Unit (PPU). */
   if(ppu_init() != 0) {
      WARN("Failed to initialize the PPU core");
      machine_exit();
      return 1;
    }

   /* Initialize Memory Mapping Controller (MMC). */
   if(mmc_init() != 0) {
      WARN("mmc_init() failed (unsupported mapper?)");
      machine_exit();
      return 1;
   }

   /* Initialize Audio Processing Unit (APU). */
   if(apu_init() != 0) {
      WARN ("Failed to initialize the APU core");
      machine_exit();
      return 1;
   }

   /* Reset input. It has already been initialized by main(). */
   input_reset();

   /* Initialize the real-time game rewinder. */
   if(rewind_init() != 0) {
      WARN("Failed to initialize the rewinder");
      machine_exit();
      return 1;
   }

   /* Reset game clock. */
   machine_reset_game_clock();

   /* Reset counters. */
   executed_frames = 0;
   rendered_frames = 0;

   /* Install keyboard handler. */
   LOCK_VARIABLE(key_names);
   LOCK_VARIABLE(key_codes);
   LOCK_FUNCTION(keyboard_handler);
   keyboard_ucallback = keyboard_handler;

   /* Clear keyboard buffer. */
   memset(&key_names, 0, sizeof(key_names));
   memset(&key_codes, 0, sizeof(key_codes));
   key_buffer_size = 0;

   /* Return success. */
   return 0;
}

void machine_exit(void)
{
   /* Sanity check. */
   if(!file_is_loaded) {
      WARN("machine_exit() called with no file loaded");
      return;
   }

   /* Deinitialize each component in the reverse order in which they were initialized. */
   rewind_exit ();

   apu_exit ();
   /* mmc_exit (); */
   ppu_exit ();
   cpu_exit ();

   /* Clear any stale memory used by the rewinder. */
   rewind_clear();

   /* Remove keyboard handler. */
   keyboard_ucallback = NULL;

   log_printf("Executed frames: %d (%d rendered).", executed_frames, rendered_frames);
}

void machine_reset(void)
{
   /* Note: This should have the same order as machine_init(). */
   cpu_reset();
   ppu_reset();
   mmc_reset();
   apu_reset();
   input_reset();
}

/* Main virtual machine loop. Executes a single full frame and returns. Note that
   due to how the PPU works, this may execute more than a frame, but it is
   guaranteed to always generate at least one frame per call */
void machine_main(void)
{
   int i;
   BOOL redraw;

   /* Check if a frame timer interrupt has occured. */
   if(frame_interrupt) {
      /* Sync timing variables. */
      timing_fps = actual_fps_count;
      timing_hertz = virtual_fps_count;
      timing_audio_fps = audio_fps;

      actual_fps_count = 0;
      virtual_fps_count = 0;
      audio_fps = 0;

      /* Clear interrupt flag so it doesn't fire again. */
      frame_interrupt = FALSE;
   }

   /* Process general input. */
   for(i = 0; i < key_buffer_size; i++) {
      int name, scancode;

      name = key_names[i];
      scancode = key_codes[i];

      switch(scancode) {
         /* ESC - Enter GUI. */
         case KEY_ESC:  {
            if(!gui_is_active)
               want_gui = TRUE;
   
	    break;
        }

        default:
           break;
      }

      /* Pass it on to other modules. */
      gui_handle_keypress(name, scancode);
      input_handle_keypress(name, scancode);
      video_handle_keypress(name, scancode);
   }

   /* Clear keyboard buffer. */
   machine_clear_key_buffer();

   /* Handle NetPlay. It doesn't really matter where this occurs, just that it *does*
      occur on a regular enough basis. Putting it here makes it run independantly
      of the timing code below this point. */
   if(netplay_mode)
      netplay_process();

   /* Handle fast forward. */
   if(key[KEY_TILDE] && !(input_mode & INPUT_MODE_CHAT)) {
      if(!timing_fast_forward) {
	  /* Enter fast forward mode. */
	  timing_fast_forward = TRUE;
	  timing_update_timing();
      }
   }
   else if(timing_fast_forward) {
      /* Exit fast forward mode. */
      timing_fast_forward = FALSE;
      timing_update_timing();
   }

   /* Decrement the frame counter. It keeps a running count of how many frames have
      passed in the timers, but have not been executed. In other words, if it is
      greater than one, we are running too slow, so we simply skip drawing for the
      extra frames to try and speed the emulation up. */
   frame_count--;
   if(frame_count > 0) {
      /* This frame will be executed, but not drawn. */
      redraw = FALSE;
   }
   else {
      /* This frame will be executed, and drawn. */
      redraw = TRUE;

      /* We only need to handle speed throttling if we're drawing, since if we're
         frame skipping we just want to run as much emulated code as possible to
         catch up instead, so throttling would be anti-productive. */
      if(speed_cap) {
         /* Wait for a tick from the throttle timer. Freeing unused CPU time during
            this phase will reduce the CPU usage of the program below 100% if any
            spare CPU power is left-over. In either case, we need to at least yield
            the timeslice to play nice with multitasking environments. */
         while(throttle_counter == 0) {
            if(cpu_usage == CPU_USAGE_NORMAL) {
               /* Yield timeslice only. */
               rest(0);
            }
	    else if(cpu_usage == CPU_USAGE_PASSIVE) {
               /* Free timeslice. Note that on some operating systems, the task
                  scheduler may take as much as 11ms to finish. */
               rest(1);
            }
	 }
      }

      /* Get all currently pending frames into the frame counter. */
      frame_count = throttle_counter;

      /* We use subtract here to avoid losing ticks if the timer interrupt fires
         between this and the last statement. */
      throttle_counter -= frame_count;

      /* Enforce frame skip setting if it is not set to auto. */
      if((frame_skip != -1) && (frame_count > frame_skip))
         frame_count = frame_skip;
   }

   /* Handle real-time game rewinding. This essentially replaces the remainder of the
      emulation loop with a simple save state load. With save states getting loaded
      often enough, it appears as though gameplay is going backwards in time. */
   if(rewind_is_enabled()) {
      /* We only want to enable rewinding in normal gameplay mode. */
      if(input_mode & INPUT_MODE_PLAY) {
         if(key[KEY_BACKSLASH]) {
	    if(!rewind_load_snapshot()) {
               apu_sync_update();
               ppu_sync_update();

               /* Skip the remainder of this frame. */
               return;
            }
         }
         else {
            /* If we aren't rewinding, then just save a snapshot of this frame. */
            rewind_save_snapshot();
         }
      }
   }

   /* At this point, we've performed general input processing, synchronized the frame
      timing and collected the neccessary information for frame skipping. So all we
      have left to do is emulate a single frame and return. */
   executed_frames++;
   virtual_fps_count++;

   /* Adjust the timing and game clocks. */
   timing_clock_delta += 1000 / timing_get_base_frame_rate();
   timing_clock += timing_clock_delta;
   game_clock_milliseconds += timing_clock_delta;
   timing_clock_delta -= floor(timing_clock_delta);

   /* This isn't the most elegant way of doing this, but it is the easiest. */
   while(game_clock_milliseconds >= 1000) {
      game_clock_milliseconds -= 1000;
      game_clock_seconds++;
   }
   while(game_clock_seconds >= 60) {
      game_clock_seconds -= 60;
      game_clock_minutes++;
   }
   while(game_clock_minutes >= 60) {
      game_clock_minutes -= 60;
      game_clock_hours++;
   }
   while(game_clock_hours >= 24) {
      game_clock_hours -= 24;
      game_clock_days++;
   }

   /* Game input processing is handled here. This is a bit different from general input
      processing, which runs as often as possible. Game input processing expects to
      only occur once per frame, locked to the machine's frame rate. */
   input_process();

   /* Check if we are frame skipping, or not. */
   if(redraw) {
      /* This frame will be drawn. */
      rendered_frames++;
      actual_fps_count++;

      /* Enable PPU rendering. */
      ppu_set_option(PPU_OPTION_ENABLE_RENDERING, TRUE);
   }
   else {
      /* Disable PPU rendering for frame skipping. This only affects visual output
         (i.e buffer writes), not emulation. This serves more as a hint, it is not
         guaranteed to be honored by the PPU, especially if it is currently mid-frame. */
      ppu_set_option(PPU_OPTION_ENABLE_RENDERING, FALSE);
   }

   /* For NSF playback, we use a simplified pipeline that does not synchronize the PPU.
      The NSF routines handle drawing the frame buffer instead, and the value of frame_lock
      is neither set nor checked. However, frame skipping is still honored. */
   if(nsf_is_loaded) {
      int line, total_lines;

      nsf_start_frame();

      /* Get the total number of lines per frame and cache it for speed. */
      total_lines = PPU_TOTAL_LINES;

      /* For each scanline, call the NSF playback handler nsf_execute(), predict APU IRQs,
         run the CPU for the scanline's length and synchronize the APU. */
      for(line = 0; line < total_lines; line++) {
         nsf_execute(SCANLINE_CLOCKS);

         apu_predict_irqs(SCANLINE_CLOCKS);
         cpu_execute(SCANLINE_CLOCKS);

         apu_sync_update();
      }

      nsf_end_frame();
   }
   else {
      /* Execute a scanline at a time, waiting for the PPU to complete a frame. Note that this
         just means the PPU completes a single frame, it does not account for when the frame
         was completed or how long the PPU continues running afterwards. */
      while(!frame_lock) {
         apu_predict_irqs(SCANLINE_CLOCKS);

         if(mmc_predict_asynchronous_irqs)
            mmc_predict_asynchronous_irqs(SCANLINE_CLOCKS);

         ppu_predict_interrupts(SCANLINE_CLOCKS, PPU_PREDICT_ALL);

         cpu_execute(SCANLINE_CLOCKS);
 
         apu_sync_update();
         ppu_sync_update();
      }

      /* Clear frame lock. */
      frame_lock = FALSE;
   }

   /* If CPU usage is not set to aggressive, yield the timeslice. */
   if(cpu_usage != CPU_USAGE_AGGRESSIVE)
      rest(0);
}

/* Pauses the emulation, both timing and audio output. */
void machine_pause(void)
{
  /* Suspend timers. */
  suspend_timing();

  /* Suspend audio. */
  audio_suspend();
}

/* Resumes the emulation. */
void machine_resume(void)
{
  /* Start timers. */
  resume_timing();

  /* Start audio. */
  audio_resume();
}

void machine_save_state(PACKFILE* file, const int version)
{
   const UINT32* delta = (UINT32*)&timing_clock_delta;

   pack_iputl(timing_clock, file);
   pack_iputl(*delta, file);

   pack_iputw(game_clock_milliseconds, file);
   pack_putc(game_clock_seconds, file);
   pack_putc(game_clock_hours, file);
   pack_putc(game_clock_minutes, file);
   pack_iputw(game_clock_days, file);
}

void machine_load_state(PACKFILE* file, const int version)
{
   UINT32 delta;
   float* delta_f;

   timing_clock = pack_igetl(file);
   /* Ugh, this is ugly. */
   delta = pack_igetl(file);
   delta_f = (float*)&delta;
   timing_clock_delta = *delta_f;

   game_clock_milliseconds = pack_igetw(file);
   game_clock_seconds = pack_getc(file);
   game_clock_hours = pack_getc(file);
   game_clock_minutes = pack_getc(file);
   game_clock_days = pack_igetw(file);
}

/* This is similar to Allegro's clear_keybuf(), but clears our custom keyboard buffer.
   This is called by the GUI before it closes in order to prevent the emulation loop from
   immediately re-opening it again. */
void machine_clear_key_buffer(void)
{
   key_buffer_size = 0;
}

/* This just clears the game clock. */
void machine_reset_game_clock(void)
{
   game_clock_milliseconds = 0;
   game_clock_seconds = 0;
   game_clock_minutes = 0;
   game_clock_hours = 0;
   game_clock_days = 0;
}

/* These are like machine_pause() and machine_resume(), except that they don't touch audio.
   Calling both in succession effectively resets the timing system, causing any changes
   made to the timing configuration to be immediately reloaded. */
void suspend_timing(void)
{
   /* Remove timers. */
   remove_int(fps_timer);
   remove_int(throttle_timer);

   /* Reset variables. */
   actual_fps_count = 0;
   virtual_fps_count = 0;
   frame_count = 1;
   frame_interrupt = FALSE;
   throttle_counter = 0;
}

void resume_timing(void)
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
   timer_ticks = ROUND( (REAL)timer_ticks_1_hz / timing_get_frame_rate() );

   /* Lock memory. As far as I know, this is only needed under DOS. */
   LOCK_VARIABLE(frame_interrupt);
   LOCK_VARIABLE(throttle_Counter);
   LOCK_FUNCTION(fps_timer);
   LOCK_FUNCTION(throttle_timer);

   /* Install timers. */
   install_int_ex(fps_timer, timer_ticks_1_hz);
   install_int_ex(throttle_timer, timer_ticks);
}

/* -------------------------------------------------------------------------------- */

/* Allegro timer routines. */
static void fps_timer(void)
{
   frame_interrupt = TRUE;
}
END_OF_STATIC_FUNCTION(fps_timer);

static void throttle_timer(void)
{
  throttle_counter++;
}
END_OF_STATIC_FUNCTION(throttle_timer);

static int keyboard_handler(int key, int* scancode)
{
   if(key_buffer_size >= KEY_BUFFER_MAX)
      return key;

   key_names[key_buffer_size] = key;
   key_codes[key_buffer_size] = *scancode;
   key_buffer_size++;

   return key;
}
END_OF_STATIC_FUNCTION(keyboard_handler);

/* Memory-mapped I/O handlers. These trap the $4000-$47FF range for hardware registers,
   which is used for the APU, PPU and input controller. */
static UINT8 read_registers(const UINT16 address)
{
   if(address == 0x4014)
      return ppu_read(address);
   else if(address <= 0x4015)
      return apu_read(address);
   else if((address == 0x4016) || (address == 0x4017))
      return input_read(address);

   return 0x00;
}

static void write_registers(const UINT16 address, const UINT8 data)
{
   if(address == 0x4014) {
      ppu_write (address, value);
   }
   else if(address <= 0x4017) {
      apu_write(address, value);

      if((address == 0x4016) || (address == 0x4017))
         input_write(address, value);
   }
}
