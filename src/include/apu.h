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

   Heavily modified for FakeNES by Siloh.
   Portions (c) 2001-2006 FakeNES Team. */

#ifndef APU_H_INCLUDED
#define APU_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Ports. */
#define APU_WRA0        0x4000
#define APU_WRA1        0x4001
#define APU_WRA2        0x4002
#define APU_WRA3        0x4003
#define APU_WRB0        0x4004
#define APU_WRB1        0x4005
#define APU_WRB2        0x4006
#define APU_WRB3        0x4007
#define APU_WRC0        0x4008
#define APU_WRC2        0x400a
#define APU_WRC3        0x400b
#define APU_WRD0        0x400c
#define APU_WRD2        0x400e
#define APU_WRD3        0x400f
#define APU_WRE0        0x4010
#define APU_WRE1        0x4011
#define APU_WRE2        0x4012
#define APU_WRE3        0x4013

#define APU_SMASK       0x4015

#define APU_BASEFREQ_NTSC  (1.89e9 / 88 / 12)
#define APU_BASEFREQ_PAL   (26601712.5 / 15)

/* --- 2A03 support. --- */

typedef struct apu_chan_s
{
   UINT8 regs[4];

   BOOL enabled;

   REAL output;

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

   /* Triangle? */
   BOOL counter_started;
   REAL write_latency;   /* quasi-hack */

   /* Noise. */
   int xor_tap;

   /* DMC. */
   UINT16 address;
   UINT16 cached_addr;
   int dma_length;
   int cached_dmalength;
   UINT8 cur_byte;

   /* for sync read $4105 */
   BOOL enabled_cur;
   BOOL holdnote_cur;
   BOOL counter_started_cur;
   int vbl_length_cur;
   int freq_cur;
   REAL phaseacc_cur;

   /* DMC? */
   BOOL looping;
   BOOL irq_gen;
   BOOL irq_occurred;
   int dma_length_cur;
   int cached_dmalength_cur;
   BOOL looping_cur;
   BOOL irq_gen_cur;
   BOOL irq_occurred_cur;

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

/* APU queue structure */
#define APUQUEUE_SIZE   4096
#define APUQUEUE_MASK   (APUQUEUE_SIZE - 1)

/* apu ring buffer member */
typedef struct apudata_s
{
   int timestamp;
   UINT16 address;
   UINT8 value;

} apudata_t;

/* ExSound interface. */
typedef struct _APU_EXSOUND
{
   const UINT8 *id;
   void (*reset) (void);                  /* reset */
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

   UINT8 regs[0x16];

   FLAGS enable_reg;
   FLAGS enable_reg_cur;

   apudata_t queue[APUQUEUE_SIZE];
   int q_head, q_tail;

   // for ExSound
   apudata_t ex_queue[APUQUEUE_SIZE];
   int ex_q_head, ex_q_tail;
   const APU_EXSOUND *exsound;

   int elapsed_cycles;
   REAL cycle_rate;

   REAL sample_rate;
   REAL refresh_rate;

   // for $4017:bit7 by T.Yano
   int cnt_rate;

} apu_t;

/* Function prototypes */
int apu_init (void);
void apu_exit (void);
void apu_reset (void);
void apu_update (void);
void apu_process (void);
void apu_set_exsound (ENUM);
UINT8 apu_read (UINT16);
void apu_write (UINT16, UINT8);
void apu_ex_write (UINT16, UINT8);
void apu_save_state (PACKFILE *, int);
void apu_load_state (PACKFILE *, int);

ENUM apu_stereo_mode;

enum
{
   APU_STEREO_MODE_1 = 1,
   APU_STEREO_MODE_2,
   APU_STEREO_MODE_3,
   APU_STEREO_MODE_4,
};

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

enum
{
   APU_EXSOUND_NONE = 0,
   APU_EXSOUND_MMC5,
   APU_EXSOUND_VRC6
};

#ifdef __cplusplus
}
#endif
#endif /* APU_H_INCLUDED */
