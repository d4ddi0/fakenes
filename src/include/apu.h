/* Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)

   This program is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU Library General 
   Public License as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
   Library General Public License for more details.  To obtain a 
   copy of the GNU Library General Public License, write to the Free 
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Any permitted reproduction of these routines, in whole or in part,
   must bear this legend.

   Heavily modified for FakeNES by randilyn.
   Portions (c) 2001-2006 FakeNES Team. */

#ifndef APU_H_INCLUDED
#define APU_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "core.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Ports. */
#define APU_WRA0  0x4000
#define APU_WRA1  0x4001
#define APU_WRA2  0x4002
#define APU_WRA3  0x4003
#define APU_WRB0  0x4004
#define APU_WRB1  0x4005
#define APU_WRB2  0x4006
#define APU_WRB3  0x4007
#define APU_WRC0  0x4008
#define APU_WRC2  0x400a
#define APU_WRC3  0x400b
#define APU_WRD0  0x400c
#define APU_WRD2  0x400e
#define APU_WRD3  0x400f
#define APU_WRE0  0x4010
#define APU_WRE1  0x4011
#define APU_WRE2  0x4012
#define APU_WRE3  0x4013
#define APU_RWF0  0x4015
#define APU_WRG0  0x4017

#define APU_REGA  APU_WRA0
#define APU_REGZ  APU_WRG0
#define APU_REGS  ((APU_REGZ - APU_REGA) + 1)

#define APU_BASEFREQ_NTSC  (1.89e9 / 88 / 12)
#define APU_BASEFREQ_PAL   (26601712.5 / 15)

enum
{
   APU_CHANNEL_SQUARE_1 = 0,
   APU_CHANNEL_SQUARE_2,
   APU_CHANNEL_TRIANGLE,
   APU_CHANNEL_NOISE,
   APU_CHANNEL_DMC,
   APU_CHANNEL_EXTRA_1,
   APU_CHANNEL_EXTRA_2,
   APU_CHANNEL_EXTRA_3,
   APU_CHANNELS
};

/* --- 2A03 support. --- */

typedef struct apu_chan_s
{
   UINT8 regs[4];

   BOOL enabled;

   REAL output;
   REAL last_sample;

   REAL phaseacc;
   int freq;
   BOOL fixed_envelope;
   BOOL holdnote;
   int volume;

   int adder;
   int duty_flip;
   int vbl_length;
   int linear_length;

   /* Sweep. */
   int sweep_phase;
   int sweep_delay;
   BOOL sweep_on;
   int sweep_shifts;
   int sweep_length;
   BOOL sweep_inc;

   /* Envelope. */
   int env_phase;
   int env_delay;
   int env_vol;

   /* Square. */
   int freq_limit;   /* this may not be necessary in the future */
   BOOL sweep_complement;  /* difference between square wave channels. */

   /* Triangle. */
   BOOL counter_started;

   /* Noise. */
   int xor_tap;

   /* DMC. */
   UINT16 address;
   UINT16 dma_length;
   UINT8 cur_byte;
   int sample_bits;
   int counter;
   UINT8 shift_reg;
   BOOL silence;
   BOOL looping;
   BOOL irq_gen;
   BOOL irq_occurred;

   /* DMC reloading. */
   UINT16 cached_address;
   UINT16 cached_dmalength;

} apu_chan_t;

typedef struct apu_apusound_s
{
   apu_chan_t square[2];
   apu_chan_t triangle;
   apu_chan_t noise;
   apu_chan_t dmc;

} APU_APUSOUND;

/* --- VRC6 Sound support. --- */

typedef struct apu_vrc6s_chan_s
{
   UINT32 cps;
   INT32 cycles;

   UINT32 spd;

   UINT8 regs[3];
   UINT8 update;
   UINT8 adr;

   /* For saw wave. */
   UINT32 output;

} apu_vrc6s_chan_t;

typedef struct _APU_VRC6SOUND
{
   apu_vrc6s_chan_t square[2];
   apu_vrc6s_chan_t saw;

} APU_VRC6SOUND;

/* --- MMC5 Sound support. --- */

typedef struct _apu_mmc5s_chan_s
{                       
   UINT32 cps;
   INT32 cycles;
   INT32 sweepphase;
   INT32 envphase;

   UINT32 spd;
   UINT32 envspd;
   UINT32 sweepspd;

   UINT32 length;
   UINT32 freq;
   UINT32 release;

   UINT8 regs[4];
   UINT8 update;
   UINT8 key;
   UINT8 adr;
   UINT8 envadr;
   UINT8 duty;

   /* For digital audio. */
   INT32 output;

} apu_mmc5s_chan_t;

typedef struct _APU_MMC5SOUND
{
   apu_mmc5s_chan_t square[2];
   apu_mmc5s_chan_t da;

} APU_MMC5SOUND;

/* ExSound interface. */
typedef struct _APU_EXSOUND
{
   const UINT8 *id;
   void (*reset) (void);                  /* reset */
   void (*update) (void);                 /* update */
   REAL (*process) (ENUM);                /* process channel */
   void (*write) (UINT16, UINT8);         /* write to a port */
   void (*save_state) (PACKFILE *, int);  /* save state */
   void (*load_state) (PACKFILE *, int);  /* load state */

} APU_EXSOUND;

typedef struct apu_s
{
   APU_APUSOUND  apus;
   APU_MMC5SOUND mmc5s;
   APU_VRC6SOUND vrc6s;

   UINT8 regs[APU_REGS];

   FLAGS enable_reg;

   /* For ExSound. */
   const APU_EXSOUND *exsound;

   REAL cycle_rate;

   // for $4017:bit7 by T.Yano
   int cnt_rate;
   cpu_time_t frame_counter;
   BOOL frame_irq_gen;
   BOOL frame_irq_occurred;

   struct
   {
      BOOL can_process;
      REAL base_frequency;
      REAL mixing_frequency;
      cpu_time_t clock_counter;
      REAL accumulators[APU_CHANNELS];
      REAL sample_cache[APU_CHANNELS];
      REAL accumulated_samples;
      REAL max_samples;

   } mixer;

} apu_t;

/* Function prototypes */
extern void apu_load_config (void);
extern void apu_save_config (void);
extern int apu_init (void);
extern void apu_exit (void);
extern void apu_reset (void);
extern void apu_update (void);
extern void apu_start_frame (void);
extern void apu_end_frame (void);
extern void apu_set_exsound (ENUM);
extern UINT8 apu_read (UINT16);
extern void apu_write (UINT16, UINT8);
extern void apu_save_state (PACKFILE *, int);
extern void apu_load_state (PACKFILE *, int);

extern ENUM apu_quality;
extern ENUM apu_stereo_mode;

enum
{
   APU_QUALITY_FAST = 0,
   APU_QUALITY_ACCURATE,
   APU_QUALITY_INTERPOLATED
};

enum
{
   APU_STEREO_MODE_1 = 1,
   APU_STEREO_MODE_2,
   APU_STEREO_MODE_3,
   APU_STEREO_MODE_4,
};

enum
{
   APU_EXSOUND_NONE = 0,
   APU_EXSOUND_MMC5,
   APU_EXSOUND_VRC6
};

#ifdef __cplusplus
}
#endif   /* __cplusplus */
#endif   /* !APU_H_INCLUDED */
