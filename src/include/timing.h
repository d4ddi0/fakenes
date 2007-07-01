/* FakeNES - A free, portable, Open Source NES emulator.

   timing.h: Declarations for the timing system.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef TIMING_H_INCLUDED
#define TIMING_H_INCLUDED
#include <math.h>
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Keep the .0 on the end of these so that any calculations involving them will usually be converted to floating point first
   thing(which prevents those pesky integer math errors, yarr!). */
#define MASTER_CLOCK_NTSC         21477272.0 /* In MHz */
#define MASTER_CLOCK_PAL          26601712.0

#define APU_CLOCK_DIVIDER_NTSC    12 /* Same as CPU */
#define APU_CLOCK_DIVIDER_PAL     16 /* Same as CPU */
#define CPU_CLOCK_DIVIDER_NTSC    12
#define CPU_CLOCK_DIVIDER_PAL     16
#define PPU_CLOCK_DIVIDER_NTSC    4  /* 3x   CPU */
#define PPU_CLOCK_DIVIDER_PAL     5  /* 3.2x CPU */

/* From master clock. */
#define APU_CLOCK_DIVIDER         ((machine_type == MACHINE_TYPE_NTSC) ? APU_CLOCK_DIVIDER_NTSC : APU_CLOCK_DIVIDER_PAL)
#define CPU_CLOCK_DIVIDER         ((machine_type == MACHINE_TYPE_NTSC) ? CPU_CLOCK_DIVIDER_NTSC : CPU_CLOCK_DIVIDER_PAL)
#define PPU_CLOCK_DIVIDER         ((machine_type == MACHINE_TYPE_NTSC) ? PPU_CLOCK_DIVIDER_NTSC : PPU_CLOCK_DIVIDER_PAL)

/* To master clock(same as above - we just use MULTIPLIER to be more symbolic of what they're for). */
#define APU_CLOCK_MULTIPLIER      APU_CLOCK_DIVIDER
#define CPU_CLOCK_MULTIPLIER      CPU_CLOCK_DIVIDER
#define PPU_CLOCK_MULTIPLIER      PPU_CLOCK_DIVIDER

#define PPU_SCANLINE_CLOCKS       341                                       /* PPU clocks per scanline */
#define PPU_RENDER_CLOCKS         256                                       /* PPU clocks per scanline(rendering portion) */
#define PPU_HBLANK_CLOCKS         (PPU_SCANLINE_CLOCKS - PPU_RENDER_CLOCKS) /* PPU clocks per scanline(HBlank portion) */
#define PPU_HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP (320 - 256)
#define PPU_TOTAL_LINES_NTSC      262                                       /* Total lines processed per frame, NTSC */
#define PPU_TOTAL_LINES_PAL       312                                       /* Total kines proccesed per frame, PAL */
#define PPU_FIRST_DISPLAYED_LINE  0                                         /* First line rendered */
#define PPU_LAST_DISPLAYED_LINE   239                                       /* Last line rendered */
#define PPU_FIRST_VBLANK_LINE     240                                       /* Line on which VBlank starts */
#define PPU_FRAME_CLOCKS_NTSC     (PPU_SCANLINE_CLOCKS * PPU_TOTAL_LINES_NTSC)
#define PPU_FRAME_CLOCKS_PAL      (PPU_SCANLINE_CLOCKS * PPU_TOTAL_LINES_PAL)
#define PPU_FRAME_RATE_NTSC       ((MASTER_CLOCK_NTSC / PPU_CLOCK_DIVIDER_NTSC) / PPU_FRAME_CLOCKS_NTSC) /* ~60 Hz */
#define PPU_FRAME_RATE_PAL        ((MASTER_CLOCK_PAL  / PPU_CLOCK_DIVIDER_PAL)  / PPU_FRAME_CLOCKS_PAL)  /* ~50 Hz */

/* Values suitable to passing to cpu_execute(), cpu_consume_cycles(), apu_predict_irqs(), etc. */
/* Uses master clock cycles. */
#define SCANLINE_CLOCKS (PPU_SCANLINE_CLOCKS * PPU_CLOCK_MULTIPLIER)
#define RENDER_CLOCKS   (PPU_RENDER_CLOCKS   * PPU_CLOCK_MULTIPLIER)
#define HBLANK_CLOCKS   (PPU_HBLANK_CLOCKS   * PPU_CLOCK_MULTIPLIER)
#define HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP (PPU_HBLANK_CLOCKS_BEFORE_VRAM_ADDRESS_FIXUP * PPU_CLOCK_MULTIPLIER)

/* Lookahead buffering for the IRQ and NMI predictors, since even a bus cycle CPU will always execute more than requested
   (due to using master clock cycles for external timing and CPU clock cycles for the start/stop state machine), and it's
   much, much worse with opcode-based CPU emulations. */
/* 8 = maximum cycle length of a 6502 opcode. */
#define PREDICTION_BUFFER_CYCLES (8 * CPU_CLOCK_MULTIPLIER)

extern ENUM machine_region;
extern ENUM machine_type;
extern ENUM machine_timing;

extern ENUM cpu_usage;

extern BOOL speed_cap;
extern int frame_skip;
extern int timing_fps;
extern int timing_hertz;
extern int timing_audio_fps;

extern ENUM timing_mode;
extern REAL timing_speed_multiplier;
extern BOOL timing_half_speed;
extern BOOL timing_fast_forward;

extern unsigned timing_clock;

extern int frames_to_execute;

extern int machine_init (void);
extern void machine_exit (void);
extern void machine_reset (void);

extern void suspend_timing (void);
extern void resume_timing (void);

enum {
   MACHINE_REGION_AUTOMATIC,
   MACHINE_REGION_NTSC,
   MACHINE_REGION_PAL,
};

enum {
   MACHINE_TYPE_NTSC,
   MACHINE_TYPE_PAL,
};

enum {
   MACHINE_TIMING_SMOOTH,
   MACHINE_TIMING_ACCURATE,
};

enum {
   CPU_USAGE_PASSIVE,
   CPU_USAGE_NORMAL,
   CPU_USAGE_AGGRESSIVE,
};

enum {
   /* Direct timing bypasses all speed modifiers(the timings used always match the real thing). */
   TIMING_MODE_DIRECT,
   /* Indirect timing takes all speed modifiers, etc. into account. */
   TIMING_MODE_INDIRECT,
};

extern REAL timing_get_timing_scale(void);
extern REAL timing_get_frame_rate(void);
extern REAL timing_get_frequency(void);
extern void timing_update_timing(void);
extern void timing_update_machine_type(void);
extern void timing_update_mode(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !TIMING_H_INCLUDED */
