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
   int regs[4];

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

/* --- VRC6 support. --- */

typedef struct
{
   int cps;
   int cycles;
   int spd;
   int regs[3];
   int update;
   int adr;
   int mute;

} APU_VRC6_SQUARE;

typedef struct
{
   int cps;
   int cycles;
   int spd;
   int output;
   int regs[3];
   int update;
   int adr;
   int mute;

} APU_VRC6_SAW;

typedef struct
{
   APU_VRC6_SQUARE square[2];
   APU_VRC6_SAW saw;
   int mastervolume;

} APU_VRC6SOUND;

/* --- MMC5 support. --- */

typedef struct
{
   int cps;
   int cycles;
   int sweepphase;
   int envphase;

   int spd;
   int envspd;
   int sweepspd;

   int length;
   int freq;
   int mastervolume;
   int release;

   int regs[4];
   int update;
   int key;
   int adr;
   int envadr;
   int duty;
   int mute;

} APU_MMC5_SQUARE;

typedef struct
{
   int output;
   int key;
   int mute;

} APU_MMC5_DA;

typedef struct
{
   APU_MMC5_SQUARE square[2];
   APU_MMC5_DA da;

} APU_MMC5SOUND;

/* APU queue structure */
#define APUQUEUE_SIZE   4096
#define APUQUEUE_MASK   (APUQUEUE_SIZE - 1)

/* apu ring buffer member */
typedef struct apudata_s
{
   unsigned timestamp;
   UINT16 address;
   UINT8 value;

} apudata_t;

typedef struct apu_s
{
   APU_APUSOUND  apus;
   APU_MMC5SOUND mmc5;
   APU_VRC6SOUND vrc6s;

   FLAGS enable_reg;
   FLAGS enable_reg_cur;

   apudata_t queue[APUQUEUE_SIZE];
   int q_head, q_tail;

   // for ExSound
   apudata_t ex_queue[APUQUEUE_SIZE];
   int ex_q_head, ex_q_tail;
   ENUM exsound;

   REAL elapsed_cycles;
   REAL cycle_rate;

   REAL sample_rate;
   REAL refresh_rate;

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
   APU_CHANNEL_EXTRA,
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
