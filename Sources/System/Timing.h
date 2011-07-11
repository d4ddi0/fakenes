/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef SYSTEM__TIMING_H__INCLUDED
#define SYSTEM__TIMING_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MASTER_CLOCK_NTSC         21477272 /* In MHz */
#define MASTER_CLOCK_PAL          26601712

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

/* Note: All exact clock times start at 1. */
#define PPU_RENDER_CLOCKS         256                                       /* PPU clocks per scanline(rendering portion) */
#define PPU_HBLANK_START          (PPU_RENDER_CLOCKS + 1)                   /* Time of HBlank start. */
#define PPU_HBLANK_CLOCKS         85                                        /* PPU clocks per scanline(HBlank portion) */
#define PPU_HBLANK_PREFETCH_START 321                                       /* When the PPU fetches the first background tiles. */
#define PPU_SCANLINE_CLOCKS       (PPU_RENDER_CLOCKS + PPU_HBLANK_CLOCKS)   /* PPU clocks per scanline */
#define PPU_FIRST_LINE            -1                                        /* First line emulated */
#define PPU_LAST_LINE_NTSC        260                                       /* Last line emulated, NTSC. */             
#define PPU_LAST_LINE_PAL         310                                       /* Last line emulated, PAL. */
#define PPU_FIRST_DISPLAYED_LINE  0                                         /* First line rendered */
#define PPU_LAST_DISPLAYED_LINE   239                                       /* Last line rendered */
#define PPU_DISPLAY_LINES         240					    /* Total number of visible lines. */
#define PPU_CLOCK_SKIP_LINE       -1                                        /* Line at which the PPU skips a clock */
#define PPU_CLOCK_SKIP_CYCLE      329                                       /* Cycle at which the PPU skips a clock */
#define PPU_IDLE_LINE             240                                       /* Line on which the PPU only idles */
#define PPU_FIRST_VBLANK_LINE     241                                       /* Line on which VBlank starts */
#define PPU_TOTAL_LINES_NTSC      ((PPU_LAST_LINE_NTSC - PPU_FIRST_LINE) + 1)  /* Total lines processed per frame, NTSC */
#define PPU_TOTAL_LINES_PAL       ((PPU_LAST_LINE_PAL - PPU_FIRST_LINE) + 1)   /* Total lines proccesed per frame, PAL */
#define PPU_FRAME_CLOCKS_NTSC     (PPU_SCANLINE_CLOCKS * PPU_TOTAL_LINES_NTSC) /* May be +/- 1 due to skipped cycle */
#define PPU_FRAME_CLOCKS_PAL      (PPU_SCANLINE_CLOCKS * PPU_TOTAL_LINES_PAL)  /* ... */

/* These macros generate floating-point values containing the ideal frame rate (in Hz).
   Very important: Don't remove the (REAL) cast or it will produce an integer result. */
#define PPU_FRAME_RATE_NTSC ((MASTER_CLOCK_NTSC / (REAL)PPU_CLOCK_DIVIDER_NTSC) / PPU_FRAME_CLOCKS_NTSC) /* ~60 Hz */
#define PPU_FRAME_RATE_PAL  ((MASTER_CLOCK_PAL  / (REAL)PPU_CLOCK_DIVIDER_PAL)  / PPU_FRAME_CLOCKS_PAL)  /* ~50 Hz */

/* Machine dependant timings. */
#define PPU_LAST_LINE    ((machine_type == MACHINE_TYPE_NTSC) ? PPU_LAST_LINE_NTSC    : PPU_LAST_LINE_PAL)
#define PPU_TOTAL_LINES  ((machine_type == MACHINE_TYPE_NTSC) ? PPU_TOTAL_LINES_NTSC  : PPU_TOTAL_LINES_PAL)
#define PPU_FRAME_CLOCKS ((machine_type == MACHINE_TYPE_NTSC) ? PPU_FRAME_CLOCKS_NTSC : PPU_FRAME_CLOCKS_PAL)
#define PPU_FRAME_RATE   ((machine_type == MACHINE_TYPE_NTSC) ? PPU_FRAME_RATE_NTSC   : PPU_FRAME_RATE_PAL)

/* Values suitable to passing to cpu_execute(), cpu_consume_cycles(), apu_predict_irqs(), etc. */
/* Uses master clock cycles. */
#define SCANLINE_CLOCKS (PPU_SCANLINE_CLOCKS * PPU_CLOCK_MULTIPLIER)
#define RENDER_CLOCKS   (PPU_RENDER_CLOCKS   * PPU_CLOCK_MULTIPLIER)
#define HBLANK_CLOCKS   (PPU_HBLANK_CLOCKS   * PPU_CLOCK_MULTIPLIER)

/* Lookahead buffering for the IRQ and NMI predictors, since the CPU will
   always execute more than requested. */
#define PREDICTION_BUFFER_CYCLES (8 * CPU_CLOCK_MULTIPLIER) /* 8 = Maximum cycle length of a 6502 opcode. */

/* -------------------------------------------------------------------------------- */

extern REAL timing_get_timing_scale(void);
extern REAL timing_get_base_frame_rate(void);
extern REAL timing_get_frame_rate(void);
extern REAL timing_get_frequency(void);
extern void timing_update_timing(void);
extern void timing_update_machine_type(void);
extern void timing_update_mode(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !SYSTEM__TIMING_H__INCLUDED */
