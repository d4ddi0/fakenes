/* FakeNES - A free, portable, Open Source NES emulator.

   timing.h: Declarations for the timing system.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef TIMING_H_INCLUDED
#define TIMING_H_INCLUDED
#include "audio.h"
#include "apu.h"
#include "common.h"
#include "gui.h"
#include "rom.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

ENUM machine_region;
ENUM machine_type;

ENUM cpu_usage;

BOOL speed_cap;
int frame_skip;
int timing_fps;
int timing_hertz;
int timing_audio_fps;

REAL timing_speed_multiplier;
BOOL timing_half_speed;

unsigned timing_clock;

int frames_to_execute;

int machine_init (void);
void machine_exit (void);
void machine_reset (void);

void suspend_timing (void);
void resume_timing (void);

enum
{
   MACHINE_REGION_AUTOMATIC,
   MACHINE_REGION_NTSC,
   MACHINE_REGION_PAL
};

enum
{
   MACHINE_TYPE_NTSC,
   MACHINE_TYPE_PAL
};

enum
{
   CPU_USAGE_PASSIVE,
   CPU_USAGE_NORMAL,
   CPU_USAGE_AGGRESSIVE
};

#define SCANLINE_CLOCKS       341
#define RENDER_CLOCKS         256
#define HBLANK_CLOCKS         (SCANLINE_CLOCKS - RENDER_CLOCKS)
#define HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP (320 - 256)
#define TOTAL_LINES_NTSC      262
#define TOTAL_LINES_PAL       312
#define FIRST_DISPLAYED_LINE  0
#define LAST_DISPLAYED_LINE   239
#define FIRST_VBLANK_LINE     240

static INLINE int timing_get_speed (void)
{
   REAL speed;

   speed = (machine_type == MACHINE_TYPE_NTSC ? 60.0f : 50.0f);
   speed *= timing_speed_multiplier;
   if (timing_half_speed)
      speed /= 2.0f;

   return (ROUND(speed));
}

static INLINE void timing_update_speed (void)
{
   if (!gui_is_active)
   {
      /* Cycle timers to match new emulation speeds. */
      suspend_timing ();
      resume_timing ();
   }

   /* Cycle audio to match new emulation speeds. */
   audio_exit ();
   audio_init ();

   if (rom_is_loaded)
      apu_update ();
}

static INLINE void timing_update_machine_type (void)
{
   /* This function resyncs machine_type to the value of machine_region. */

   switch (machine_region)
   {
      case MACHINE_REGION_AUTOMATIC:
      {
         if (rom_is_loaded)
         {
            /* Try to determine a suitable machine type by searching for
               country codes in the ROM's filename. */

            if (ustrstr (global_rom.filename, "(E)"))
            {
               /* Europe. */
               machine_type = MACHINE_TYPE_PAL;
            }
            else
            {
               /* Default to NTSC. */
               machine_type = MACHINE_TYPE_NTSC;
            }
         }
         else  
         {
            /* Default to NTSC. */
            machine_type = MACHINE_TYPE_NTSC;
         }

         break;
      }

      case MACHINE_REGION_NTSC:
      {
         /* NTSC (60 Hz). */
         machine_type = MACHINE_TYPE_NTSC;

         break;
      }

      case MACHINE_REGION_PAL:
      {
         /* PAL (50 Hz). */
         machine_type = MACHINE_TYPE_PAL;

         break;
      }
   }

   timing_update_speed ();
}

#ifdef __cplusplus
}
#endif
#endif   /* !TIMING_H_INCLUDED */
