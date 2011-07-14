/* FakeNES - A free, portable, Open Source NES emulator.

   timing.c: Helper functions for the timing system.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include "apu.h"
#include "common.h"
#include "core.h"
#include "gui.h"
#include "load.h"
#include "machine.h"
#include "nsf.h"
#include "rom.h"
#include "timing.h"
#include "types.h"

REAL timing_get_timing_scale(void)
{
   /* This gets the speed ratio relative to the normal clock rate, as
      affected by any speed modifiers.

      < 1.0 = slower
        1.0 = normal
      > 1.0 = faster
      */

   /* This function should NOT be called by anything except timing_get_speed(). */
   REAL scale = 1.0;
   scale *= timing_speed_multiplier;
   if(timing_half_speed)
      scale /= 2.0;
   if(timing_fast_forward)
      scale *= 2.0;

   return scale;
}

/* This returns the frame rate without any speed modifiers applied. */
REAL timing_get_base_frame_rate(void)
{
   return PPU_FRAME_RATE;
}

REAL timing_get_frame_rate(void)
{
   REAL rate = timing_get_base_frame_rate();

   /* Modifications are only allowed in the INDIRECT timing mode. */
   if(timing_mode == TIMING_MODE_INDIRECT) {
      if(machine_timing == MACHINE_TIMING_SMOOTH) {
         /* Approximate to make it easier for the host timers(OS portion) to handle the rate. */
         rate = floor(rate);
      }

      /* Apply any other speed modifiers. */
      rate *= timing_get_timing_scale();
   }

   return rate;
}

REAL timing_get_frequency(void)
{
   /* This function returns the frequency of the master clock.  Not the ideal frequency defined in the constants at the top,  
      but the one that we're actually emulating at.  TIMING_SMOOTH affects this.

      Do NOT use this function for anything emulation related, use it to affect output only please. */

   return (timing_get_frame_rate() * PPU_FRAME_CLOCKS) * PPU_CLOCK_DIVIDER;
}

void timing_update_timing(void)
{
   /* Cycle timers to match new emulation speeds. */
   suspend_timing();
   resume_timing();

   /* Rebuild cycle table. */
   FN2A03_Rebuild_Cycle_Table();

   /* Update the APU's mixer. */
   apu_update();

   /* Update the NSF player. */
   if(nsf_is_loaded)
      nsf_update_timing();

   cpu_update();
}

void timing_update_machine_type(void)
{
   /* This function resyncs machine_type to the value of machine_region. */

   switch(machine_region) {
      case MACHINE_REGION_AUTOMATIC: {
         if(rom_is_loaded) {
            /* Try to determine a suitable machine type by searching for
               country codes in the ROM's filename. */
            if(ustrstr(global_rom.filename, "(E)")) {
               /* Europe. */
               machine_type = MACHINE_TYPE_PAL;
            }
            else {
               /* Default to NTSC. */
               machine_type = MACHINE_TYPE_NTSC;
            }
         }
         else {
            /* Default to NTSC. */
            machine_type = MACHINE_TYPE_NTSC;
         }

         break;
      }

      case MACHINE_REGION_NTSC: {
         /* NTSC (~60 Hz). */
         machine_type = MACHINE_TYPE_NTSC;
         break;
      }

      case MACHINE_REGION_PAL: {
         /* PAL (~50 Hz). */
         machine_type = MACHINE_TYPE_PAL;
         break;
      }
   }

   timing_update_timing();
}

/* Note: Unlike timing_update_timing(), mode changes should NOT affect anything outside of emulation. */
void timing_update_mode(void)
{
   /* Rebuild cycle table. */
   FN2A03_Rebuild_Cycle_Table();

   /* Update the APU's mixer. */
   apu_update();
}
