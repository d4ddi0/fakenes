/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   timing.h: Declarations for the timing system.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef TIMING_H_INCLUDED
#define TIMING_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

ENUM machine_region;
ENUM machine_type;
ENUM cpu_usage;
int frame_skip_min;
int frame_skip_max;
int timing_fps;
int timing_hertz;
int timing_audio_fps;
BOOL timing_half_speed;

int machine_init (void);
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

#ifdef __cplusplus
}
#endif
#endif   /* !TIMING_H_INCLUDED */

