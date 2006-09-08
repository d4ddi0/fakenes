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

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "core.h"
#include "cpu.h"
#include "debug.h"
#include "dsp.h"
#include "log.h"
#include "timing.h"
#include "types.h"

/* Quality. */
ENUM apu_quality = APU_QUALITY_FAST;

/* Stereo mode. */
ENUM apu_stereo_mode = APU_STEREO_MODE_2;

/* Static APU context. */
static apu_t apu;

/* Internal function prototypes (defined at bottom). */
static void set_freq (REAL);
static INLINE void build_luts (int);
static INLINE REAL get_sample (ENUM);
static INLINE void process (void);

/* Macro to convert generated samples to normalized samples. */
#define APU_TO_OUTPUT(value)     (value / 32767.0)
#define APU_TO_OUTPUT_24(value)  (value / 8388607.0)
                                             
/* --- Lookup tables. --- */

#define DECAY_LUT_SIZE     16
#define VBL_LUT_SIZE       32
#define TRILENGTH_LUT_SIZE 128

static int decay_lut[DECAY_LUT_SIZE];
static int vbl_lut[VBL_LUT_SIZE];
static int trilength_lut[TRILENGTH_LUT_SIZE];

/* vblank length table used for squares, triangle, noise */
static const int vbl_length[] = {
   5, 127, 10,  1, 19,  2, 40,  3, 80, 4, 30,  5,  7, 6, 13, 7, 6, 8, 12,
   9,  24, 10, 48, 11, 96, 12, 36, 13, 8, 14, 16, 15
};

/* frequency limit of square channels */
static const int freq_limit[] = {
   0x3ff, 0x555, 0x666, 0x71c, 0x787, 0x7c1, 0x7e0, 0x7f0
};

/* noise frequency lookup table */
static const int noise_freq[] = {
   4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

/* DMC transfer freqs */
static const int dmc_clocks_ntsc[] = {
   428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72,
   54
};

static const int dmc_clocks_pal[] = {
   397, 353, 315, 297, 265, 235, 209, 198, 176, 148, 131, 118, 98, 78, 66,
   50
};

/* ratios of pos/neg pulse for square waves */
static const int duty_lut[] = {
   2, 4, 8, 12
};

/* Frame IRQ start clocks for NTSC and PAL. */
static const cpu_time_t frame_irq_start_ntsc = 29830u + 1;

/*
<kevtris> divide out by the difference in clock frequency
<kevtris> i.e. PALfreq/NTSCfreq* NTSCcounterValue
<kevtris> and that will be it +/- 1 or 2 counts

Should be: ~36947.373246227290526216786397872 PAL clocks
*/
static const cpu_time_t frame_irq_start_pal = 36947u + 1;

// ExSound headers
#include "apu/mmc5.h"
#include "apu/vrc6.h"

/* --- Sound generators. --- */

static INLINE REAL apu_envelope (apu_chan_t *chan)
{
   REAL output;

   RT_ASSERT(chan);

   /* envelope decay at a rate of (env_delay + 1) / 240 secs */
   chan->env_phase -= (4 * apu.cnt_rate); /* 240/60 */

   while (chan->env_phase < 0)
   {
      /* MAX() kludge here to fix a possible lock-up in some games. */
      chan->env_phase += MAX(chan->env_delay, 1);

      if (chan->holdnote)
         chan->env_vol = ((chan->env_vol + 1) & 0x0f);
      else if (chan->env_vol < 0x0f)
         chan->env_vol++;
   }

   if (chan->fixed_envelope)
      output = (chan->volume << 8); /* fixed volume */
   else
      output = ((chan->env_vol ^ 0x0f) << 8);

   return (output);
}

/* SQUARE WAVE CHANNELS
   Waveform generator
   ====================

   reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
   reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
   reg2: 8 bits of freq
   reg3: 0-2=high freq, 7-4=vbl length counter */

static INLINE REAL apu_square (apu_chan_t *chan)
{
   REAL total, sample_weight, output;

   RT_ASSERT(chan);

   if (!chan->enabled || (chan->vbl_length <= 0))
      return (chan->output);

   /* length counter */
   if (!chan->holdnote)
      chan->vbl_length -= apu.cnt_rate;

   /* TODO: using a table of max frequencies is not technically
   ** clean, but it is fast and (or should be) accurate 
   */
   if ((chan->freq < 8) ||
       (!chan->sweep_inc && (chan->freq > chan->freq_limit)))
   {
      return (chan->output);
   }

   /* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */
   if (chan->sweep_on && chan->sweep_shifts)
   {
      chan->sweep_phase -= (2 * apu.cnt_rate);  /* 120/60 */

      while (chan->sweep_phase < 0)
      {
         /* MAX() kludge here to fix a possible lock-up in some games. */
         chan->sweep_phase += MAX(chan->sweep_delay, 1);

         if (chan->sweep_inc)
         {
            /* ramp up */
            if (chan->sweep_complement)
               chan->freq += ~(chan->freq >> chan->sweep_shifts);
            else
               chan->freq -= (chan->freq >> chan->sweep_shifts);
         }
         else  
         {
            /* ramp down */
            chan->freq += (chan->freq >> chan->sweep_shifts);
         }
      }
   }

   /* Envelope unit. */
   output = apu_envelope (chan);

   sample_weight = chan->phaseacc;

   if (sample_weight > apu.cycle_rate)
      sample_weight = apu.cycle_rate;

   /* Positive/negative pulse. */
   total = ((chan->adder < chan->duty_flip) ? sample_weight :
      -sample_weight);

   chan->phaseacc -= apu.cycle_rate;  /* # of cycles per sample */

   while (chan->phaseacc < 0)
   {
      chan->phaseacc += MAX(chan->freq, 1);

      chan->adder = ((chan->adder + 1) & 0x0f);

      sample_weight = chan->freq;

      if (chan->phaseacc > 0)
         sample_weight -= chan->phaseacc;

      total += ((chan->adder < chan->duty_flip) ? sample_weight :
         -sample_weight);
   }

   output = ((output * total) / apu.cycle_rate);

   chan->output = APU_TO_OUTPUT(output);

   return (chan->output);
}

/* TRIANGLE WAVE CHANNEL
   Waveform generator
   =============

   reg0: 7=holdnote, 6-0=linear length counter
   reg2: low 8 bits of frequency
   reg3: 7-3=length counter, 2-0=high 3 bits of frequency */

static INLINE REAL apu_triangle (apu_chan_t *chan)
{
   static REAL val = 0, prev_val = 0;
   REAL total, sample_weight, output;
                                    
   RT_ASSERT(chan);

   if (!chan->enabled || (chan->vbl_length <= 0))
      return (chan->output);
                        
   if (chan->counter_started)
   {
      if (chan->linear_length > 0)
         chan->linear_length -= (4 * apu.cnt_rate);   /* 240/60 */

      if ((chan->vbl_length > 0) && !chan->holdnote)
         chan->vbl_length -= apu.cnt_rate;
   }

   if ((chan->linear_length <= 0) || (chan->freq < 4))
   {
      /* inaudible */
      return (chan->output);
   }

   /* TODO: All of the following could use a major clean-up. */

   total = 0;
   sample_weight = 0;
   prev_val = val;

   if (chan->adder)
      val -= (apu.cycle_rate / chan->freq);
   else
      val += (apu.cycle_rate / chan->freq);

   while ((val < -8) || (val >= 8))
   {
      if (val < -8)
      {
         total += ((prev_val + -8) * (prev_val - -8));
         sample_weight += (prev_val - -8);
         prev_val = -8;
         val = (-16 - val);
         chan->adder = 0;
      }

      if (val >= 8)
      {
         total += ((prev_val + 8) * (8 - prev_val));
         sample_weight += (8 - prev_val);
         prev_val = 8;
         val = (16 - val);
         chan->adder = 1;
      }
   }

   if (chan->adder)
   {
      total += ((prev_val + val) * (prev_val - val));
      sample_weight += (prev_val - val);
   }
   else
   {
      total += ((prev_val + val) * (val - prev_val));
      sample_weight += (val - prev_val);
   }

   total /= sample_weight;

   output = (total * 256.0);
   output = ((output * 21.0) / 16.0);

   chan->output = APU_TO_OUTPUT(output);

   return (chan->output);
}

/* WHITE NOISE CHANNEL
   Waveform generator
   ===================

   reg0: 0-3=volume, 4=envelope, 5=hold
   reg2: 7=small(93 byte) sample,3-0=freq lookup
   reg3: 7-4=vbl length counter */

static INLINE REAL apu_noise (apu_chan_t *chan)
{
   static int noise_bit = 0;
   REAL total, sample_weight, output;

   RT_ASSERT(chan);

   if (!chan->enabled || (chan->vbl_length <= 0))
      return (chan->output);

   /* vbl length counter */
   if (!chan->holdnote)
      chan->vbl_length -= apu.cnt_rate;

   /* Envelope unit. */
   output = apu_envelope (chan);

   sample_weight = chan->phaseacc;

   if (sample_weight > apu.cycle_rate)
      sample_weight = apu.cycle_rate;

   total = (noise_bit ? sample_weight : -sample_weight);

   chan->phaseacc -= apu.cycle_rate;  /* # of cycles per sample */

   while (chan->phaseacc < 0)
   {
      static int sreg = 0x4000;
      int bit0, tap, bit14;

      /* MAX() kludge here to fix a possible lock-up in some games. */
      chan->phaseacc += MAX(chan->freq, 1);

      bit0 = (sreg & 1);
      tap = ((sreg & chan->xor_tap) ? 1 : 0);
      bit14 = (bit0 ^ tap);
      sreg >>= 1;
      sreg |= (bit14 << 14);
      noise_bit = (bit0 ^ 1);

      sample_weight = chan->freq;

      if (chan->phaseacc > 0)
         sample_weight -= chan->phaseacc;

      total += (noise_bit ? sample_weight : -sample_weight);
   }

   output = ((output * total) / apu.cycle_rate);
   output = ((output * 13.0) / 16.0);

   chan->output = APU_TO_OUTPUT(output);

   return (chan->output);
}

/* DELTA MODULATION CHANNEL
   Waveform generator
   =========================

   reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
   reg1: output dc level, 6 bits unsigned
   reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
   reg3: length, (value * 16) + 1 */

static INLINE void apu_dmc_reload (apu_chan_t *chan)
{
   RT_ASSERT(chan);

   chan->address = chan->cached_address;
   chan->dma_length = chan->cached_dmalength;
}

static INLINE REAL apu_do_dmc (apu_chan_t *chan)
{
   REAL sample = 0;

   RT_ASSERT(chan);

   if (!chan->enabled)
      return (chan->last_sample);

   /* DMA reader. */

   /*
   When the sample buffer is in an empty state and the bytes counter is
   non-zero...
   */

   if ((chan->dma_length > 0) && (chan->sample_bits == 0))
   {
      /* Fill sample buffer. */

      /* DMCDMABuf=X6502_DMR(0x8000+DMCAddress); */
      chan->cur_byte = cpu_read ((0x8000 + chan->address));

      /*
      When the DMA reader accesses a byte of memory, the CPU is suspended
      for 4 clock cycles.
      */
      cpu_consume_cycles (4);

      /* DMCAddress=(DMCAddress+1)&0x7fff; */
      chan->address = ((chan->address + 1) & 0x7fff);

      chan->sample_bits = 8;

      if (--chan->dma_length == 0)
      {
         /* if loop bit set, we're cool to retrigger sample */
         if (chan->looping)
         {
            /*
            The bytes counter is decremented;
            if it becomes zero and the loop flag is set, the sample is restarted
            */

            apu_dmc_reload (chan);
         }
         else
         {
            /* check to see if we should generate an irq */
            if (chan->irq_gen && !chan->irq_occurred)
            {
               /*
               if the bytes counter becomes zero and the interrupt enabled
               flag is set, the interrupt flag is set.
               */

               chan->irq_occurred = TRUE;
               cpu_interrupt (CPU_INTERRUPT_IRQ_DMC);
            }

            /* The DMC seems to become disabled after the length counter
               reaches 0, regardless of whether or not new values are
               written to the length counter afterwards.  The channel must
               be restarted via a write to $4015 with bit 4 set.

               Fixes Crystalis, quite possibly other games.

               - randi
               */

            chan->enabled = FALSE;
         }
      }
   }

   /* Output unit. */

   if (chan->counter == 0)
   {
      /* Start a new cycle. */

      /* Reload counter. */
      chan->counter = 8;

      if (chan->sample_bits > 0)
      {
         /* Sample buffer contains data. */

         /* Clear silence flag. */
         chan->silence = FALSE;

         /* Empty sample buffer into the shift register. */

         chan->shift_reg = chan->cur_byte;

         chan->cur_byte = 0;
         chan->sample_bits = 0;
      }        
      else
      {
         /* Set silence flag. */
         chan->silence = TRUE;
      }
   }
                       
   if (!chan->silence)
   {
      BOOL bit0;

      /*
      If the silence flag is clear, bit 0 of the shift register is applied to
      the DAC counter: If bit 0 is clear and the counter is greater than 1, the
      counter is decremented by 2, otherwise if bit 0 is set and the counter is less
      than 126, the counter is incremented by 2.
      */

      bit0 = TRUE_OR_FALSE(chan->shift_reg & 1);

      if (!bit0 && (chan->regs[1] > 1))
      {
         /* positive delta */
         chan->regs[1] -= 2;
      }
      else if (bit0 && (chan->regs[1] < 126))
      {
         /* negative delta */
         chan->regs[1] += 2;
      }

      sample = (chan->regs[1] << 8);
   }

   /* Clock shift register. */
   chan->shift_reg >>= 1;

   /* Decrement counter. */
   chan->counter--;

   return (sample);
}

static INLINE REAL apu_dmc (apu_chan_t *chan)
{
   REAL total, sample_weight, output;

   RT_ASSERT(chan);

   if (!chan->enabled)
      return (chan->output);

   if (chan->silence)
   {
      /* Channel is silenced. */
      total = 0;
   }
   else
   {
      sample_weight = chan->phaseacc;
   
      if (sample_weight > apu.cycle_rate)
         sample_weight = apu.cycle_rate;
   
      total = ((chan->regs[1] << 8) * sample_weight);
   }

   chan->phaseacc -= apu.cycle_rate;  /* # of cycles per sample */
   
   while (chan->phaseacc < 0)
   {
      REAL sample;

      /* MAX() kludge here to fix a possible lock-up in some games. */
      chan->phaseacc += MAX(chan->freq, 1);

      sample = apu_do_dmc (chan);

      if (sample)
      {
         sample_weight = chan->freq;
   
         if (chan->phaseacc > 0)
            sample_weight -= chan->phaseacc;
   
         total += (sample * sample_weight);

         chan->last_sample = sample;
      }
   }

   output = (total / apu.cycle_rate);
   output = ((output * 13.0) / 16.0);

   chan->output = APU_TO_OUTPUT(output);

   return (chan->output);
}

static INLINE void apu_update_frame_counter (void)
{
   cpu_time_t start;

   /* This function simply updates the frame counter for frame IRQs.  It
      must be called exactly once per cycle.

      Note that the frame counter isn't used for frame sequencing - for
      that, we use 'apu.cnt_rate' along with phase accumulators. */

   apu.frame_counter++;

   start = ((machine_type == MACHINE_TYPE_NTSC) ? frame_irq_start_ntsc :
      frame_irq_start_pal);

   /* check to see if we should generate an irq */
   if ((apu.frame_counter >= start) &&
       (apu.frame_irq_gen && !apu.frame_irq_occurred))
   {
      apu.frame_irq_occurred = TRUE;
      cpu_interrupt (CPU_INTERRUPT_IRQ_FRAME);
   }
}                  

void apu_load_config (void)
{
   /* Like other components, the APU is both an interface and an emulation.
      However, apu_init() and apu_exit() should only be called during
      emulation (e.g, when a ROM is loaded/unloaded).  To load the
      configuration, uses these two functions instead. */

   /* Load configuration. */

   apu_quality     = get_config_int ("apu", "quality",     apu_quality);
   apu_stereo_mode = get_config_int ("apu", "stereo_mode", apu_stereo_mode);

   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_SQUARE_1, get_config_int ("apu", "enable_square_1", TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_SQUARE_2, get_config_int ("apu", "enable_square_2", TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_TRIANGLE, get_config_int ("apu", "enable_triangle", TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_NOISE,    get_config_int ("apu", "enable_noise",    TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_DMC,      get_config_int ("apu", "enable_dmc",      TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_EXTRA_1,  get_config_int ("apu", "enable_extra_1",  TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_EXTRA_2,  get_config_int ("apu", "enable_extra_2",  TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_EXTRA_3,  get_config_int ("apu", "enable_extra_3",  TRUE));

   // for ExSound
   APU_LogTableInitialize ();

   /* Disable ExSound. */
   apu_set_exsound (APU_EXSOUND_NONE);
}

void apu_save_config (void)
{
   /* Save configuration. */

   set_config_int ("apu", "quality",     apu_quality);
   set_config_int ("apu", "stereo_mode", apu_stereo_mode);

   set_config_int ("apu", "enable_square_1", dsp_get_channel_enabled (APU_CHANNEL_SQUARE_1));
   set_config_int ("apu", "enable_square_2", dsp_get_channel_enabled (APU_CHANNEL_SQUARE_2));
   set_config_int ("apu", "enable_triangle", dsp_get_channel_enabled (APU_CHANNEL_TRIANGLE));
   set_config_int ("apu", "enable_noise",    dsp_get_channel_enabled (APU_CHANNEL_NOISE));
   set_config_int ("apu", "enable_dmc",      dsp_get_channel_enabled (APU_CHANNEL_DMC));
   set_config_int ("apu", "enable_extra_1",  dsp_get_channel_enabled (APU_CHANNEL_EXTRA_1));
   set_config_int ("apu", "enable_extra_2",  dsp_get_channel_enabled (APU_CHANNEL_EXTRA_2));
   set_config_int ("apu", "enable_extra_3",  dsp_get_channel_enabled (APU_CHANNEL_EXTRA_3));
}

int apu_init (void)
{
   /* Reset APU. */
   apu_reset ();

   /* Reset frame counter. */
   apu.frame_counter = 0;

   /* Return success. */
   return (0);
}

void apu_exit (void)
{
   /* Do nothing. */
}

void apu_reset (void)
{
   /* Initializes/resets emulated sound hardware, creates waveforms/voices
      */

   const APU_EXSOUND *exsound;
   cpu_time_t frame_counter;
   UINT16 address;

   /* Save ExSound interface. */
   exsound = apu.exsound;

   /* Save frame counter, since it is not cleared by a soft reset (a hard
      reset always implies a call to apu_init(), which clears it). */
   frame_counter = apu.frame_counter;

   /* Clear context. */
   memset (&apu, 0, sizeof (apu));

   /* set the stupid flag to tell difference between two squares */

   apu.apus.square[0].sweep_complement = TRUE;
   apu.apus.square[1].sweep_complement = FALSE;

   /* Restore ExSound interface. */
   apu.exsound = exsound;

   if (apu.exsound && apu.exsound->reset)
      apu.exsound->reset ();

   /* Clear all registers. */

   for (address = APU_REGA; address <= APU_REGZ; address++)
      apu_write (address, 0);

   /* Restore frame counter. */
   apu.frame_counter = frame_counter;

   /* Initialize everything else. */
   apu_update ();
}

void apu_update (void)
{
   /* Updates the APU to external changes without resetting it, since that
      might cause problems in a currently running game. */

   /* Set sample rate. */
   set_freq (audio_sample_rate);

   /* Sync ExSound to changes. */

   if (apu.exsound && apu.exsound->update)
      apu.exsound->update ();

   /* Deinitialize DSP. */
   dsp_exit ();

   /* Open DSP buffer. */
   if (dsp_open (audio_buffer_frame_size_samples, APU_CHANNELS) != 0)
      WARN("Failed to open DSP buffer");

   /* Set up channel map. */

   switch (apu_stereo_mode)
   {
      case APU_STEREO_MODE_1:
      {
         /* FakeNES classic. */

         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0, -1.0);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0, +1.0);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0, -1.0);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0, +1.0);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_1,  1.0, -1.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_2,  1.0, +1.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_3,  1.0, +0.0);

         break;
      }

      case APU_STEREO_MODE_2:
      {
         /* FakeNES enhanced. (may/2002) */
         /* Centers the triangle and noise. */

         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0, -1.0);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0, +1.0);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_1,  1.0, -0.5);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_2,  1.0, +0.5);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_3,  1.0, +0.0);

         break;
      }

      case APU_STEREO_MODE_3:
      {
         /* Real NES. */

         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0, -1.0);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0, -1.0);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0, +1.0);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0, +1.0);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0, +1.0);

         /* What should the behavior here be? */
         dsp_set_channel_params (APU_CHANNEL_EXTRA_1,  1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_2,  1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_3,  1.0, +0.0);

         break;
      }

      case FALSE:
      case APU_STEREO_MODE_4:
      {
         /* Mono       (apu_stereo_mode == FALSE)
            Stereo Mix (apu_stereo_mode == APU_STEREO_MODE_4) */
                                                                   
         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_1,  1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_2,  1.0, +0.0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA_3,  1.0, +0.0);

         break;
      }

      default:
         WARN_GENERIC();
   }
}

void apu_start_frame (void)
{
   /* Start DSP buffer fill. */
   dsp_start ();

   apu.mixer.can_process = TRUE;
}

void apu_end_frame (void)
{
   /* Flush all pending data. */
   process ();

   apu.mixer.can_process = FALSE;

   /* End DSP buffer fill. */
   dsp_end ();

   if (audio_enable_output)
   {
      void *buffer;

      buffer = audio_get_buffer ();
      if (!buffer)
         WARN_BREAK_GENERIC();
   
      /* Process DSP buffer into audio buffer. */
      dsp_render (buffer, (apu_stereo_mode ? 2 : 1), audio_sample_size,
         audio_unsigned_samples);
   
      audio_free_buffer ();
   }
}

void apu_set_exsound (ENUM exsound)
{
   switch (exsound)
   {
      case APU_EXSOUND_NONE:
      {
         apu.exsound = NULL;

         break;
      }

      case APU_EXSOUND_MMC5:
      {
         apu.exsound = &apu_mmc5s;

         break;
      }

      case APU_EXSOUND_VRC6:
      {
         apu.exsound = &apu_vrc6s;

         break;
      }

      default:
         WARN_GENERIC();
   }

   // for ExSound
   APU_LogTableInitialize ();

   if (apu.exsound && apu.exsound->reset)
      apu.exsound->reset ();
}

UINT8 apu_read (UINT16 address)
{
   UINT8 value = 0;

   if (apu.mixer.can_process)
      process ();

   switch (address)
   {
      case APU_RWF0: /* $4015 */
      {
         /* Status register. */

         /*
         $4015   if-d nt21   DMC IRQ, frame IRQ, length counter statuses
         */

         const apu_chan_t *chan;

         /* Return 1 in 0-5 bit pos if a channel is playing */

         /* Square 0. */

         chan = &apu.apus.square[0];

         if (chan->enabled && (chan->vbl_length > 0))
            value |= 0x01;

         /* Square 1. */

         chan = &apu.apus.square[1];

         if (chan->enabled && (chan->vbl_length > 0))
            value |= 0x02;

         /* Triangle. */

         chan = &apu.apus.triangle;

         if (chan->enabled && (chan->vbl_length > 0))
            value |= 0x04;

         /* Noise. */

         chan = &apu.apus.noise;

         if (chan->enabled && (chan->vbl_length > 0))
            value |= 0x08;

         /* DMC. */

         chan = &apu.apus.dmc;

         if (chan->enabled && (chan->dma_length > 0))
            value |= 0x10;

         if (chan->irq_occurred)
            value |= 0x80;

         /* Frame IRQ. */

         if (apu.frame_irq_occurred)
            value |= 0x40;

         /* kev says reads from $4015 reset the frame counter, so... */

         /* Reset frame counter. */
         apu.frame_counter = 0;

         /* Clear frame IRQ. */
         apu.frame_irq_occurred = FALSE;
         cpu_clear_interrupt (CPU_INTERRUPT_IRQ_FRAME);

         break;
      }

      default:
      {
         value = (address >> 8); /* heavy capacitance on data bus */

         break;
      }
   }

   return (value);
}

void apu_write (UINT16 address, UINT8 value)
{  
   int index;
   apu_chan_t *chan;

   if (apu.mixer.can_process)
      process ();

   switch (address)
   {
      case APU_WRA0: /* $4000 */
      case APU_WRB0: /* $4004 */
      {
         /* Square Wave channels, register set 1. */

         /*
         $4000/4 ddle nnnn   duty, loop env/disable length, env disable, vol/env
         period
         */

         /* Determine which channel to use.
            $4000 - Square wave 1(0)
            $4004 - Square wave 2(1) */
         index = ((address & 4) ? 1 : 0);

         chan = &apu.apus.square[index];

         chan->regs[0] = value;

         chan->volume = (value & 0x0f);
         chan->env_delay = decay_lut[(value & 0x0f)];
         chan->holdnote = TRUE_OR_FALSE(value & 0x20);
         chan->fixed_envelope = TRUE_OR_FALSE(value & 0x10);
         chan->duty_flip = duty_lut[(value >> 6)];

         break;
      }

      case APU_WRA1: /* $4001 */
      case APU_WRB1: /* $4005 */
      {
         /* Square Wave channels, register set 2. */

         /*
         $4001/5 eppp nsss   enable sweep, period, negative, shift
         */

         index = ((address & 4) ? 1 : 0);

         chan = &apu.apus.square[index];

         chan->regs[1] = value;

         chan->sweep_on = TRUE_OR_FALSE(value & 0x80);
         chan->sweep_shifts = (value & 7);
         chan->sweep_delay = decay_lut[((value >> 4) & 7)];
         chan->sweep_inc = TRUE_OR_FALSE(value & 0x08);
         chan->freq_limit = freq_limit[(value & 7)];

         break;
      }

      case APU_WRA2: /* $4002 */
      case APU_WRB2: /* $4006 */
      {
         /* Square Wave channels, register set 3. */

         /*
         $4002/6 pppp pppp   period low
         */

         index = ((address & 4) ? 1 : 0);

         chan = &apu.apus.square[index];

         chan->regs[2] = value;

         chan->freq = ((chan->freq & ~0xff) | value);

         break;
      }

      case APU_WRA3: /* $4003 */
      case APU_WRB3: /* $4007 */
      {
         /* Square Wave channels, register set 4. */

         /*
         $4003/7 llll lppp   length index, period high
         */

         index = ((address & 4) ? 1 : 0);

         chan = &apu.apus.square[index];

         chan->regs[3] = value;
   
         chan->vbl_length = vbl_lut[(value >> 3)];
         chan->env_vol = 0;
         chan->freq = (((value & 7) << 8) | (chan->freq & 0xff));
         chan->adder = 0;

         if (apu.enable_reg & (1 << index))
            chan->enabled = TRUE;

         break;
      }

      case APU_WRC0: /* $4008 */
      {
         /* Triangle wave channel, register 1. */

         /*
         $4008   clll llll   control, linear counter load
         */

         chan = &apu.apus.triangle;

         chan->regs[0] = value;

         chan->holdnote = TRUE_OR_FALSE(value & 0x80);
   
         if ((!chan->counter_started) && (chan->vbl_length > 0))
            chan->linear_length = trilength_lut[(value & 0x7f)];

         break;
      }

      case APU_WRC2: /* $400A */
      {
         /* Triangle wave channel, register 2. */

         /*
         $400A   pppp pppp   period low
         */

         chan = &apu.apus.triangle;

         chan->regs[1] = value;

         chan->freq = ((((chan->regs[2] & 7) << 8) + value) + 1);

         break;
      }

      case APU_WRC3: /* $400B */
      {
         /* Triangle wave channel, register 3. */

         /*
         $400A   pppp pppp   period low
         */

         chan = &apu.apus.triangle;

         chan->regs[2] = value;
     
         chan->freq = ((((value & 7) << 8) + chan->regs[1]) + 1);
         chan->vbl_length = vbl_lut[(value >> 3)];
         chan->counter_started = FALSE;
         chan->linear_length = trilength_lut[(chan->regs[0] & 0x7f)];

         if (apu.enable_reg & 0x04)
            chan->enabled = TRUE;

         break;
      }
      
      case APU_WRD0: /* $400C */
      {
         /* White noise channel, register 1. */

         /*
         $400C   --le nnnn   loop env/disable length, env disable, vol/env period
         */

         chan = &apu.apus.noise;

         chan->regs[0] = value;

         chan->env_delay = decay_lut[(value & 0x0f)];
         chan->holdnote = TRUE_OR_FALSE(value & 0x20);
         chan->fixed_envelope = TRUE_OR_FALSE(value & 0x10);
         chan->volume = (value & 0x0f);

         break;
      }

      case APU_WRD2: /* $400E */
      {
         /* White noise channel, register 2. */

         /*
         $400E   s--- pppp   short mode, period index
         */

         chan = &apu.apus.noise;

         chan->regs[1] = value;
         chan->freq = noise_freq[(value & 0x0f)];
         chan->xor_tap = ((value & 0x80) ? 0x40 : 0x02);

         break;
      }

      case APU_WRD3: /* $400F */
      {
         /* White noise channel, register 3. */

         /*
         $400F   llll l---   length index
         */

         chan = &apu.apus.noise;

         chan->regs[2] = value;
   
         chan->vbl_length = vbl_lut[(value >> 3)];
         chan->env_vol = 0;  /* reset envelope */

         if (apu.enable_reg & 0x08)
            chan->enabled = TRUE;

         break;
      }

      case APU_WRE0: /* $4010 */
      {
         /* Delta modulation channel, register 1. */

         /*
         $4010   il-- ffff   IRQ enable, loop, frequency index
         */

         chan = &apu.apus.dmc;

         chan->regs[0] = value;

         if (machine_type == MACHINE_TYPE_NTSC)
            chan->freq = dmc_clocks_ntsc[(value & 0x0f)];
         else
            chan->freq = dmc_clocks_pal[(value & 0x0f)];

         chan->looping = TRUE_OR_FALSE(value & 0x40);
         chan->irq_gen = TRUE_OR_FALSE(value & 0x80);
   
         if (!chan->irq_gen)
         {
            chan->irq_gen = FALSE;
            chan->irq_occurred = FALSE;
            cpu_clear_interrupt (CPU_INTERRUPT_IRQ_DMC);
         }

         break;
      }

      case APU_WRE1: /* $4011 */
      {
         /* Delta modulation channel, register 2. */
         
         /*
         $4011   -ddd dddd   DAC
         */

         chan = &apu.apus.dmc;

         /* add the _delta_ between written value and current output level
            of the volume reg */
         value &= 0x7f; /* bit 7 ignored */

         chan->regs[1] = value;

         break;
      }

      case APU_WRE2: /* $4012 */
      {
         /* Delta modulation channel, register 3. */
         
         /*
         $4012   aaaa aaaa   sample address
         */

         chan = &apu.apus.dmc;

         chan->regs[2] = value;

         /* DMCAddress=0x4000+(DMCAddressLatch<<6); */
         chan->cached_address = (0x4000 + (value << 6));

         if (chan->dma_length == 0)
            apu_dmc_reload (chan);

         break;
      }

      case APU_WRE3: /* $4013 */
      {
         /* Delta modulation channel, register 4. */
         
         /*
         $4013   llll llll   sample length
         */

         chan = &apu.apus.dmc;

         chan->regs[3] = value;

         /* DMCSize=(DMCSizeLatch<<4)+1; */
         chan->cached_dmalength = ((value << 4) + 1);

         if (chan->dma_length == 0)
            apu_dmc_reload (chan);

         break;
      }

      case APU_RWF0: /* $4015 */
      {
         /* Common register set 1. */

         apu.enable_reg = value;

         /* Squares. */

         for (index = 0; index < 2; index++)
         {
            chan = &apu.apus.square[index];

            if ((value & (1 << index)) == 0)
            {
               /* Channel is disabled. */

               chan->enabled = FALSE;
               chan->vbl_length = 0;
            }
         }

         /* Triangle. */

         chan = &apu.apus.triangle;

         if (!(value & 0x04))
         {
            /* Channel is disabled. */

            chan->enabled = FALSE;
            chan->vbl_length = 0;
            chan->linear_length = 0;
            chan->counter_started = FALSE;
         }

         /* Noise. */

         chan = &apu.apus.noise;

         if (!(value & 0x08))
         {
            /* Channel is disabled. */

            chan->enabled = FALSE;
            chan->vbl_length = 0;
         }

         /* DMC. */

         chan = &apu.apus.dmc;

         /*
         ---d xxxx

         If d is set and the DMC's DMA reader has no more sample bytes to fetch, the DMC
         sample is restarted. If d is clear then the DMA reader's sample bytes remaining
         is set to 0.
         */

         chan->enabled = TRUE_OR_FALSE(value & 0x10);

         if (chan->enabled)
         {
            /* Channel is enabled - check for a reload. */

            if (chan->dma_length == 0)
               apu_dmc_reload (chan);
         }
         else
         {
            /* Channel is disabled. */

            chan->dma_length = 0;
         }

         /*
         When $4015 is written to, the channels' length counter enable flags are set,
         the DMC is possibly started or stopped, and the DMC's IRQ occurred flag is
         cleared.
         */
         chan->irq_occurred = FALSE;
         cpu_clear_interrupt (CPU_INTERRUPT_IRQ_DMC);

         break;
      }

      case APU_WRG0: /* $4017 */
      {
         /* Common register set 2. */

         /*
         $4017   fd-- ----   5-frame cycle, disable frame interrupt
         */

         apu.cnt_rate = ((value & 0x80) ? 4 : 5);

         /*
         <_Q> setting $4017.6 or $4017.7 will turn off frame IRQs
         <_Q> setting $4017.7 puts it into the 5-step sequence
         <_Q> which does not generate interrupts
         */

         if (value & 0xc0)
            apu.frame_irq_gen = FALSE;
         else
            apu.frame_irq_gen = TRUE;

         /* Reset frame counter. */
         apu.frame_counter = 0;

         /* Clear frame IRQ. */
         apu.frame_irq_occurred = FALSE;
         cpu_clear_interrupt (CPU_INTERRUPT_IRQ_FRAME);

         break;
      }
   
      default:
         break;
   }

   if ((address >= APU_WRA0) && (address <= APU_WRG0))
   {
      /* For state saving. */
      apu.regs[(address - APU_WRA0)] = value;
   }

   if (apu.exsound && apu.exsound->write)
      apu.exsound->write (address, value);
}

void apu_save_state (PACKFILE *file, int version)
{
   int index;

   RT_ASSERT(file);

   /* Save registers. */

   for (index = 0; index < APU_REGS; index++)
      pack_putc (apu.regs[index], file);

   /* Save frame counter. */
   pack_iputl (apu.frame_counter, file);

   if (apu.exsound)
   {
      /* Write ExSound ID. */
      pack_fwrite (apu.exsound->id, 8, file);
      
      if (apu.exsound->save_state)
         apu.exsound->save_state (file, version);
   }
   else
   {
      /* No ExSound hardware present. */
      pack_fwrite ("NONE\0\0\0\0", 8, file);
   }
}

void apu_load_state (PACKFILE *file, int version)
{              
   int index;
   UINT8 signature[8];

   RT_ASSERT(file);

   /* Load registers. */

   for (index = 0; index < APU_REGS; index++)
      apu_write ((APU_REGA + index), pack_getc (file));

   /* Load frame counter. */
   apu.frame_counter = pack_igetl (file);

   /* Load ExSound ID. */
   /* Will be set to NONE if no ExSound hardware is present. */
   pack_fread (signature, 8, file);

   if (apu.exsound && apu.exsound->load_state)
      apu.exsound->load_state (file, version);

   /* Synchronize the APU mixer's clock with the CPU's cycle counter. */
   apu.mixer.clock_counter = cpu_get_cycles (FALSE);
}

/* --- Internal functions. --- */

static void set_freq (REAL sample_rate)
{
   REAL freq;

   /* We directly sync the APU with the CPU.  This may not be the most
      accurate method, but it gives the best quality sound. */
   apu.mixer.base_frequency = timing_get_frequency ();

   switch (apu_quality)
   {
      case APU_QUALITY_FAST:
      {
         /* Use the output sample rate for mixing. */
         apu.mixer.mixing_frequency = sample_rate;

         break;
      }

      case APU_QUALITY_ACCURATE:
      case APU_QUALITY_INTERPOLATED:
      {
         /* Use the APU's actual frequency for mixing. */
         apu.mixer.mixing_frequency = apu.mixer.base_frequency;

         break;
      }

      default:
         WARN_GENERIC();
   }

   /* (Re)build various lookup tables to match the mixing frequency. */
   build_luts (apu.mixer.mixing_frequency);

   /* Set cycle rate accordingly to account for any differences between the
      ideal mixing frequency and the actual mixing frequency.

      We have to remember to take speed modifiers into account, since the
      base frequencies (as defined in apu.h) are constants. */

   freq = (((machine_type == MACHINE_TYPE_NTSC) ? APU_BASEFREQ_NTSC :
      APU_BASEFREQ_PAL) * timing_get_speed_ratio ());

   apu.cycle_rate = (freq / apu.mixer.mixing_frequency);

   /* Number of samples to be held in the APU mixer accumulators before
      being divided and sent to the DSP.

      With the fast mixer, this is simply how often a sample is written
      relative to the APU mixer's clock. */
   apu.mixer.max_samples = (apu.mixer.base_frequency / sample_rate);
}

static INLINE void build_luts (int num_samples)
{
   int i;

   /* TODO: Clean this up. */

   // decay_lut[], vbl_lut[], trilength_lut[] modified (x5) for $4017:bit7 by T.Yano
   /* lut used for enveloping and frequency sweeps */
   for (i = 0; i < DECAY_LUT_SIZE; i++)
      decay_lut[i] = ROUND(((REAL)num_samples * (i + 1)) * 5.0);

   /* used for note length, based on vblanks and size of audio buffer */
   for (i = 0; i < VBL_LUT_SIZE; i++)
      vbl_lut[i] = ROUND(((REAL)num_samples * vbl_length[i]) * 5.0);

   /* triangle wave channel's linear length table */
   for (i = 0; i < TRILENGTH_LUT_SIZE; i++)
      trilength_lut[i] = ROUND(((REAL)num_samples * i) * 5.0);
}

static INLINE REAL get_sample (ENUM channel)
{
   /* Helper function to fetch a sample from 'channel'. */

   REAL sample = 0;

   switch (channel)
   {
      case APU_CHANNEL_SQUARE_1:
      {
         sample = apu_square (&apu.apus.square[0]);

         break;
      }

      case APU_CHANNEL_SQUARE_2:
      {
         sample = apu_square (&apu.apus.square[1]);

         break;
      }

      case APU_CHANNEL_TRIANGLE:
      {
         sample = apu_triangle (&apu.apus.triangle);

         break;
      }

      case APU_CHANNEL_NOISE:
      {
         sample = apu_noise (&apu.apus.noise);

         break;
      }

      case APU_CHANNEL_DMC:
      {
         sample = apu_dmc (&apu.apus.dmc);

         break; 
      } 

      case APU_CHANNEL_EXTRA_1:
      case APU_CHANNEL_EXTRA_2:
      case APU_CHANNEL_EXTRA_3:
      {
         if (apu.exsound && apu.exsound->process)
            sample = apu.exsound->process (channel);

         break;
      }

      default:
         WARN_GENERIC();
   }

   return (sample);
}

static INLINE void process (void)
{
   cpu_time_t cycles, elapsed_cycles;
   cpu_time_t count;

   cycles = cpu_get_cycles (FALSE);
   elapsed_cycles = (cycles - apu.mixer.clock_counter);

   if (elapsed_cycles == 0)
   {
      /* Nothing to do. */
      return;
   }

   apu.mixer.clock_counter = cycles;

   /* Avoid re-entry. */
   apu.mixer.can_process = FALSE;

   switch (apu_quality)
   {
      case APU_QUALITY_FAST:
      {
         /* Even faster version of the mixer. */

         /* Accumulate samples:       No
            Cycle-by-cycle emulation: No
            */

         for (count = 0; count < elapsed_cycles; count++)
         {
            /* Update frame counter. */
            apu_update_frame_counter ();

            /* Simulate accumulation. */
            apu.mixer.accumulated_samples++;
            
            if (apu.mixer.accumulated_samples >= apu.mixer.max_samples)
            {
               int channel;

               /* Gather samples. */
   
               for (channel = 0; channel < APU_CHANNELS; channel++)
                  apu.mixer.accumulators[channel] = get_sample (channel);

               /* Send to DSP. */
               dsp_write (apu.mixer.accumulators);

               /* Adjust counter. */
               apu.mixer.accumulated_samples -= apu.mixer.max_samples;
            }
         }

         break;
      }

      case APU_QUALITY_ACCURATE:
      {
         /* Accumulate samples:       No
            Cycle-by-cycle emulation: Yes
            */

         for (count = 0; count < elapsed_cycles; count++)
         {
            int channel;

            /* Update frame counter. */
            apu_update_frame_counter ();

            /* Simulate accumulation. */
            apu.mixer.accumulated_samples++;
         
            if (apu.mixer.accumulated_samples >= apu.mixer.max_samples)
            {
               /* Gather samples. */

               for (channel = 0; channel < APU_CHANNELS; channel++)
                  apu.mixer.accumulators[channel] = get_sample (channel);

               /* Send to DSP. */
               dsp_write (apu.mixer.accumulators);

               /* Adjust counter. */
               apu.mixer.accumulated_samples -= apu.mixer.max_samples;
            }
            else
            {
               /* Gather and discard samples (for emulation only). */

               for (channel = 0; channel < APU_CHANNELS; channel++)
                  get_sample (channel);
            }
         }

         break;
      }

      case APU_QUALITY_INTERPOLATED:
      {
         /* Accumulate samples:       Yes
            Cycle-by-cycle emulation: Yes
            */

         for (count = 0; count < elapsed_cycles; count++)
         {
            int channel;

            /* Update frame counter. */
            apu_update_frame_counter ();
      
            for (channel = 0; channel < APU_CHANNELS; channel++)
            {
               REAL sample;
      
               sample = get_sample (channel);
      
               apu.mixer.accumulators[channel] += sample;
      
               /* Cache it so that we can split it up later if need be. */
               apu.mixer.sample_cache[channel] = sample;
            }
         
            apu.mixer.accumulated_samples++;
         
            if (apu.mixer.accumulated_samples >= apu.mixer.max_samples)
            {
               REAL residual;
               REAL divider;
      
               /* Determine how much of the last sample we want to keep for
                  the next loop. */
               residual = (apu.mixer.accumulated_samples - floor
                  (apu.mixer.max_samples));
      
               /* Calculate the divider for the APU:DSP frequency ratio. */
               divider = (apu.mixer.accumulated_samples - residual);
      
               for (channel = 0; channel < APU_CHANNELS; channel++)
               {
                  REAL *sample = &apu.mixer.accumulators[channel];
      
                  /* Remove residual sample portion. */
                  *sample -= (apu.mixer.sample_cache[channel] * residual);
      
                  /* Divide. */
                  *sample /= divider;
               }

               /* Send to DSP. */
               dsp_write (apu.mixer.accumulators);

               for (channel = 0; channel < APU_CHANNELS; channel++)
               {
                  /* Reload accumulators with residual sample protion. */
                  apu.mixer.accumulators[channel] =
                     (apu.mixer.sample_cache[channel] * residual);
               }
      
               /* Adjust counter. */
               apu.mixer.accumulated_samples -= apu.mixer.max_samples;
            }
         }

         break;
      }

      default:
         WARN_GENERIC();
   }

   apu.mixer.can_process = TRUE;
}
