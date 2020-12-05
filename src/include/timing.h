

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

timing.h: Declarations for the timing system.

Copyright (c) 2001-2006, Randy McDowell.
Copyright (c) 2001-2006, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef TIMING_H_INCLUDED

#define TIMING_H_INCLUDED


int machine_type;


enum
{
    MACHINE_TYPE_NTSC, MACHINE_TYPE_PAL
};


int machine_init (void);

void machine_reset (void);


int frame_skip_min;

int frame_skip_max;


int timing_fps;

int timing_hertz;


int timing_audio_fps;


int timing_half_speed;


void suspend_timing (void);

void resume_timing (void);


#define SCANLINE_CLOCKS         341


#define RENDER_CLOCKS           256


#define HBLANK_CLOCKS           (SCANLINE_CLOCKS - RENDER_CLOCKS)

#define HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP    (320 - 256)


#define TOTAL_LINES_NTSC        262

#define TOTAL_LINES_PAL         312


#define FIRST_DISPLAYED_LINE    0

#define LAST_DISPLAYED_LINE     239


#define FIRST_VBLANK_LINE       240


#endif /* ! TIMING_H_INCLUDED */

