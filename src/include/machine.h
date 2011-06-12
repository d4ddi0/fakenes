/* FakeNES - A free, portable, Open Source NES emulator.

   machine.h: Declarations for the virtual machine.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef MACHINE_H_INCLUDED
#define MACHINE_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

enum {
   CPU_USAGE_PASSIVE = 0,
   CPU_USAGE_NORMAL,
   CPU_USAGE_AGGRESSIVE,
};

enum {
   MACHINE_MEDIA_ROM = 0,
   MACHINE_MEDIA_NSF,
};

enum {
   MACHINE_REGION_AUTOMATIC = -1,
   MACHINE_REGION_NTSC,
   MACHINE_REGION_PAL,
};

enum {
   MACHINE_TIMING_ACCURATE = 0,
   MACHINE_TIMING_SMOOTH,
};

enum {
   MACHINE_TYPE_NTSC = 0,
   MACHINE_TYPE_PAL,
};

enum {
   /* Direct timing bypasses all speed modifiers(the timings used always match the real thing). */
   TIMING_MODE_DIRECT,
   /* Indirect timing takes all speed modifiers, etc. into account. */
   TIMING_MODE_INDIRECT,
};

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

extern UINT32 timing_clock;

extern UINT16 game_clock_milliseconds;
extern UINT8 game_clock_seconds;
extern UINT8 game_clock_minutes;
extern UINT8 game_clock_hours;
extern UINT16 game_clock_days;

extern BOOL frame_lock;

extern void machine_load_config(void);
extern void machine_save_config(void);
extern int machine_init(void);
extern void machine_exit(void);
extern void machine_reset(void);
extern void machine_main(void);
extern void machine_pause(void);
extern void machine_resume(void);
extern void machine_save_state(PACKFILE* file, const int version);
extern void machine_load_state(PACKFILE* file, const int version);
extern void machine_clear_key_buffer(void);
extern void suspend_timing(void);
extern void resume_timing(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MACHINE_H_INCLUDED */
