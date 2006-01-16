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
   must bear this legend. */

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

/* length of generated noise */
#define APU_NOISE_32K   0x7fff
#define APU_NOISE_93    93

#define APU_BASEFREQ    1789772.5

/* to/from 16.16 fixed point */
#define APU_TO_FIXED    itofix
#define APU_FROM_FIXED  fixtoi

// ----------------------------------------------------------------------------
// APU Sound struct

/* channel structures */
/* As much data as possible is precalculated,
** to keep the sample processing as lean as possible
*/
 
typedef struct rectangle_s
{
   int regs[4];

   int enabled;
   
   int phaseacc;
   int freq;
   int output_vol;
   int fixed_envelope;
   int holdnote;
   int volume;

   int sweep_phase;
   int sweep_delay;
   int sweep_on;
   int sweep_shifts;
   int sweep_length;
   int sweep_inc;

   /* this may not be necessary in the future */
   int freq_limit;

   /* rectangle 0 uses a complement addition for sweep
   ** increases, while rectangle 1 uses subtraction
   */
   int sweep_complement;

   int env_phase;
   int env_delay;
   int env_vol;

   int vbl_length;
   int adder;
   int duty_flip;

   /* for sync read $4105 */
   int enabled_cur;
   int holdnote_cur;
   int vbl_length_cur;

   int smooth_envelope;
   int smooth_sweep;

} rectangle_t;

typedef struct triangle_s
{
   int regs[3];

   int enabled;

   int freq;
   int phaseacc;
   int output_vol;

   int adder;

   int holdnote;
   int counter_started;
   /* quasi-hack */
   int write_latency;

   int vbl_length;
   int linear_length;

   /* for sync read $4105 */
   int enabled_cur;
   int holdnote_cur;
   int counter_started_cur;
   int vbl_length_cur;

   /* less compatibility, clearer sound if enabled */
   int ideal_triangle;

} triangle_t;

typedef struct noise_s
{
   int regs[3];

   int enabled;

   int freq;
   int phaseacc;
   int output_vol;

   int env_phase;
   int env_delay;
   int env_vol;
   int fixed_envelope;
   int holdnote;

   int volume;

   int vbl_length;

   int xor_tap;

   /* for sync read $4105 */
   int enabled_cur;
   int holdnote_cur;
   int vbl_length_cur;

} noise_t;

typedef struct dmc_s
{
   int regs[4];

   /* bodge for timestamp queue */
   int enabled;
   
   int freq;
   int phaseacc;
   int output_vol;

   int address;
   int cached_addr;
   int dma_length;
   int cached_dmalength;
   int cur_byte;

   int looping;
   int irq_gen;
   int irq_occurred;

   /* for sync read $4105 and DPCM IRQ */
   int freq_cur;
   int phaseacc_cur;
   int dma_length_cur;
   int cached_dmalength_cur;
   int enabled_cur;
   int looping_cur;
   int irq_gen_cur;
   int irq_occurred_cur;

} dmc_t;

typedef struct apusound_s
{
   rectangle_t rectangle[2];
   triangle_t triangle;
   noise_t noise;
   dmc_t dmc;

} APUSOUND;

// ----------------------------------------------------------------------------
// VRC6 Sound struct

typedef struct
{
   int cps;
   int cycles;
   int spd;
   int regs[3];
   int update;
   int adr;
   int mute;

} VRC6_SQUARE;

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

} VRC6_SAW;

typedef struct
{
	VRC6_SQUARE square[2];
	VRC6_SAW saw;
   int mastervolume;

} VRC6SOUND;

// ----------------------------------------------------------------------------
// APU Sound struct

typedef struct
{
   int pg_phase;
   int pg_spd;
   int vib_cycles;
   int input;
   int eg_phase;
   int eg_sl;
   int eg_arr;
   int eg_drr;
   int eg_rrr;
   int pg_vib;
   int *sintblp;
   int tl;
   int eg_mode;
   int eg_type;
   int su_type;
   int eg_ar;
   int eg_dr;
   int eg_rr;
   int eg_ks;
   int eg_am;

} OPLL_OP;

typedef struct
{
   int cps;
   int spd;
   int cycles;
   int adr;
   int adrmask;
   int *table;
   int output;

} OPLL_LFO;

typedef struct
{
   int cps;
   int cycles;
   int fbbuf[2];
   int output;
	OPLL_OP op[2];
   int mastervolume;
   int tone[8];
   int key;
   int toneno;
   int freql;
   int freqh;
   int fb;
   int update;

} OPLL_CH;

typedef struct
{
	OPLL_CH ch[6];
	OPLL_LFO lfo[2];
   int mastervolume;
   int usertone[8];
   int adr;
   int rhythmc;
   int toneupdate;

} OPLLSOUND;

// ----------------------------------------------------------------------------
// FDS Sound struct

typedef struct
{
   int wave[0x40];
   int envspd;
   int envphase;
   int envout;
   int outlvl;

   int phase;
   int spd;
   int volume;
   int sweep;

   int enable;
   int envmode;
   int xxxxx;
   int xxxxx2;

   int timer;
   int last_spd;

} FDS_FMOP;

typedef struct FDSSOUND
{
   int cps;
   int cycles;
   int mastervolume;
   int output;
   int fade;

	FDS_FMOP op[2];

   int waveaddr;
   int mute;
   int key;
   int reg[0x10];
   int reg_cur[0x10];

} FDSSOUND;

// ----------------------------------------------------------------------------
// MMC5 Sound struct

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

} MMC5_SQUARE;

typedef struct
{
   int output;
   int key;
   int mute;

} MMC5_DA;

typedef struct
{
	MMC5_SQUARE square[2];
	MMC5_DA da;

} MMC5SOUND;

// ----------------------------------------------------------------------------
// N106 Sound struct

typedef struct
{
   int logvol;
   int cycles;
   int spd;
   int phase;
   int tlen;
   int update;
   int freql;
   int freqm;
   int freqh;
   int vreg;
   int tadr;
   int nazo;
   int mute;

} N106_WM;

typedef struct
{
   int cps;
   int mastervolume;

	N106_WM ch[8];

   int addressauto;
   int address;
   int chinuse;

   int tone[0x100]; /* TONE DATA */
   int data[0x80];

} N106SOUND;

// ----------------------------------------------------------------------------
// FME7 Sound struct

typedef struct
{
   int cps;
   int cycles;
   int spd;
   int regs[3];
   int update;
   int adr;
   int mute;
   int key;

} PSG_SQUARE;

typedef struct
{
   int cps;
   int cycles;
   int spd;
   int noiserng;
   int regs[1];
   int update;
   int noiseout;

} PSG_NOISE;

typedef struct
{
   int cps;
   int cycles;
   int spd;
   int envout;
   INT8 *adr;
   int regs[3];
   int update;

} PSG_ENVELOPE;

typedef struct
{
	PSG_SQUARE square[3];
	PSG_ENVELOPE envelope;
	PSG_NOISE noise;
   int mastervolume;
   int adr;

} PSGSOUND;

// ----------------------------------------------------------------------------
// APU Sound struct

enum {
   APUMODE_IDEAL_TRIANGLE,
   APUMODE_SMOOTH_ENVELOPE,
   APUMODE_SMOOTH_SWEEP
};

typedef struct
{
   int min_range, max_range;
   UINT8 (*read_func)(UINT32 address);

} apu_memread;

typedef struct
{
   int min_range, max_range;
   void (*write_func)(UINT32 address, UINT8 value);

} apu_memwrite;

/* external sound chip stuff */
typedef struct apuext_s
{
   void (*init)(void);
   void (*shutdown)(void);
   void (*reset)(void);
   int (*process)(void);
   apu_memread *mem_read;
   apu_memwrite *mem_write;

} apuext_t;

/* APU queue structure */
#define APUQUEUE_SIZE   4096
#define APUQUEUE_MASK   (APUQUEUE_SIZE - 1)

/* apu ring buffer member */
typedef struct apudata_s
{
   int timestamp, address;
   int value;

} apudata_t;

typedef struct apu_s
{
   APUSOUND apus;
   VRC6SOUND vrc6s;
   OPLLSOUND ym2413s;
   FDSSOUND fdssound;
   MMC5SOUND mmc5;
   N106SOUND n106s;
   PSGSOUND psg;

   int enable_reg;
   int enable_reg_cur;

   apudata_t queue[APUQUEUE_SIZE];
   int q_head, q_tail;

   // for ExSound
   apudata_t ex_queue[APUQUEUE_SIZE];
   int ex_q_head, ex_q_tail;
   int ex_chip;

   int elapsed_cycles;

   void *buffer; /* pointer to output buffer */
   int num_samples;

   int mix_enable[6];
   int filter_list;

   int cycle_rate;

   int sample_rate;
   int sample_bits;
   float refresh_rate;

   void (*process)(void *buffer, int num_samples, int dither);

   /* external sound chip */
   apuext_t *ext;

} apu_t;

/* Function prototypes */
apu_t *apu_getcontext (void);
void apu_setcontext (apu_t *);
apu_t *apu_create (int, float, int);
void apu_destroy(apu_t **);
void apu_setparams(int, float, int);
void apu_process (void *, int, int);
void apu_process_stereo (void *, int, int, int, int, int);
void apu_reset (void);
void apu_setext (apu_t *, apuext_t *);
void apu_setfilterlist (int);
void apu_setchan (int, int);
void apu_setmode (int, int);
UINT8 apu_read (UINT32);
void apu_write (UINT32, UINT8);
UINT8 ex_read (UINT32);
void ex_write (UINT32, UINT8);
void apu_write_cur (UINT32, UINT8);
void sync_apu_register (void);
int sync_dmc_register (int);

#ifdef __cplusplus
}
#endif
#endif /* APU_H_INCLUDED */
