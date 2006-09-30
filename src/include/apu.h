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

#define APU_REGS   24

#define APU_BASEFREQ_NTSC  (1.89e9 / 88.0 / 12.0)
#define APU_BASEFREQ_PAL   (26601712.5 / 16.0)

/* Macros to convert generated samples to normalized samples. */
#define APU_TO_OUTPUT(value)     ((value) / 65535.0)
#define APU_TO_OUTPUT_24(value)  ((value) / 16777215.0)

/* Maximum number of channels to send to the DSP (mono = 1, stereo = 2). */
#define APU_MIXER_MAX_CHANNELS   2

/* --- 2A03 support. --- */

typedef struct apu_envelope_s
{
   UINT8 timer;               /* save */
   UINT8 period;              /* do not save */
   UINT8 counter;             /* save */
   BOOL fixed;                /* do not save */
   UINT8 fixed_volume;        /* do not save */
   BOOL dirty;                /* save */

} apu_envelope_t;

typedef struct apu_sweep_s
{
   BOOL enabled;              /* do not save */
   UINT8 timer;               /* save */
   UINT8 period;              /* do not save */
   UINT8 shifts;              /* do not save */
   BOOL invert;               /* do not save */
   BOOL increment;            /* do not save */
   BOOL dirty;                /* save */
             
} apu_sweep_t;

typedef struct apu_chan_s
{
   /* General. */
   UINT8 output;                 /* save */
   UINT8 volume;                 /* save for squares, noise, and dmc */
   BOOL looping;                 /* do not save */
   BOOL silence;                 /* save for squares, noise, and dmc */

   /* Timer. */
   INT16 timer;                  /* save */
   UINT16 period;                /* save for squares */

   /* Length counter (all except dmc). */
   UINT8 length;                 /* save */
   BOOL length_disable;          /* do not save */
   
   /* Envelope generator (square/noise). */
   apu_envelope_t envelope;

   /* Sweep unit (squares). */
   apu_sweep_t sweep;

   /* Sequencer (squares/triangle). */
   UINT8 sequence_step;          /* save */

   /* Linear counter (triangle). */
   UINT8 linear_length;          /* save */
   BOOL halt_counter;            /* save */
   UINT8 cached_linear_length;   /* do not save */

   /* Square. */
   UINT8 duty_cycle;            /* do not save */

   /* Noise. */
   UINT16 xor_tap;               /* do not save */
   UINT16 shift16;               /* save */

   /* DMC. */
   BOOL enabled;                 /* save */
   UINT16 address;               /* save */
   UINT16 dma_length;            /* save */
   UINT8 cur_byte;               /* save */
   UINT8 sample_bits;            /* save */
   UINT8 counter;                /* save */
   UINT8 shift_reg;              /* save */
   BOOL irq_gen;                 /* do not save */
   BOOL irq_occurred;            /* save */
   UINT16 cached_address;        /* do not save */
   UINT16 cached_dmalength;      /* do not save */

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

   /* For mixing. */
   REAL linear_output;

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

   /* For mixing. */
   REAL linear_output;

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
   void (*process) (ENUM);                /* process channel */
   REAL (*mix) (void);                    /* mix channels */
   void (*write) (UINT16, UINT8);         /* write to a port */
   void (*save_state) (PACKFILE *, int);  /* save state */
   void (*load_state) (PACKFILE *, int);  /* load state */

} APU_EXSOUND;

typedef struct apu_s
{
   APU_APUSOUND  apus;
   APU_MMC5SOUND mmc5s;
   APU_VRC6SOUND vrc6s;

   /* For ExSound. */
   const APU_EXSOUND *exsound;

   /* Frequencies. */
   REAL base_frequency;

   /* Delta value for timers. */
   int timer_delta;

   /* Mixer. */
   struct
   {            
      BOOL can_process;

      REAL sample_rate;
      int channels;

      REAL frequency;
      cpu_time_t clock_counter;
      cpu_time_t delta_cycles;

      REAL inputs[APU_MIXER_MAX_CHANNELS];
      REAL accumulators[APU_MIXER_MAX_CHANNELS];
      REAL sample_cache[APU_MIXER_MAX_CHANNELS];
      REAL accumulated_samples;
      REAL max_samples;

   } mixer;

   /* State. */
   UINT8 regs[APU_REGS];         /* save */

   /* Frame sequencer & frame IRQs. */
   INT16 sequence_counter;       /* save */
   UINT8 sequence_step;          /* save */
   UINT8 sequence_steps;         /* do not save */
   BOOL frame_irq_gen;           /* do not save */
   BOOL frame_irq_occurred;      /* save */

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

typedef struct apu_options_s
{
   BOOL enabled;
   ENUM emulation;
   BOOL stereo;

   /* Channels. */
   BOOL enable_square_1;
   BOOL enable_square_2;
   BOOL enable_triangle;
   BOOL enable_noise;
   BOOL enable_dmc;
   BOOL enable_extra_1;
   BOOL enable_extra_2;
   BOOL enable_extra_3;

} apu_options_t;

extern apu_options_t apu_options;

enum
{
   APU_EMULATION_FAST = 0,
   APU_EMULATION_ACCURATE,
   APU_EMULATION_ULTRA
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
