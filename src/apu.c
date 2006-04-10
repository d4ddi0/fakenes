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

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "cpu.h"
#include "debug.h"
#include "dsp.h"
#include "timing.h"
#include "types.h"

/* TODO: Split ExSound (MMC5/VRC6) channels into DSP channels, instead of
   letting them do their own mixing.  This requires converting their
   secondary renderers to the APU's signed 32-bit format, an easy task. */

/* Stereo mode. */
ENUM apu_stereo_mode = APU_STEREO_MODE_2;

/* Static APU context. */
static apu_t apu;

/* Internal function prototypes (defined at bottom). */
static void set_params (REAL, REAL);
static void build_luts (int);
static void regwrite (UINT32, UINT8);
static void write_cur (UINT16, UINT8);
static void sync_apu_register (void);

/* Macro to convert generated samples to normalized samples. */
#define APU_TO_OUTPUT(value)  (value / 32767.0f)
                                             
/* --- Lookup tables. --- */

static int decay_lut[16];
static int vbl_lut[32];
static int trilength_lut[128];

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
static const int dmc_clocks[] = {
   428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 85, 72,
    54
};

/* ratios of pos/neg pulse for square waves */
static const int duty_lut[] = {
   2, 4, 8, 12
};

/* --- Queue routines. --- */

#define APU_QEMPTY()    (apu.q_head == apu.q_tail)
#define APU_EX_QEMPTY() (apu.ex_q_head == apu.ex_q_tail)

// ExSound headers
#include "apu/mmc5.h"
#include "apu/vrc6.h"

static INLINE void apu_enqueue (apudata_t *d)
{
   RT_ASSERT(d);

   apu.queue[apu.q_head] = *d;
   apu.q_head = ((apu.q_head + 1) & APUQUEUE_MASK);

   if (APU_QEMPTY())
      DEBUG_PRINTF("apu: queue overflow\n");      
}

static INLINE apudata_t *apu_dequeue (void)
{
   int loc;

   if (APU_QEMPTY())
      DEBUG_PRINTF("apu: queue empty\n");

   loc = apu.q_tail;
   apu.q_tail = ((apu.q_tail + 1) & APUQUEUE_MASK);

   return (&apu.queue[loc]);
}

static INLINE void apu_ex_enqueue (apudata_t *d)
{
   RT_ASSERT(d);

   apu.ex_queue[apu.ex_q_head] = *d;
   apu.ex_q_head = ((apu.ex_q_head + 1) & APUQUEUE_MASK);

   if (APU_EX_QEMPTY())
      DEBUG_PRINTF("ex_apu: queue overflow\n");      
}

static INLINE apudata_t *apu_ex_dequeue (void)
{
   int loc;

   if (APU_EX_QEMPTY())
      DEBUG_PRINTF("ex_apu: queue empty\n");

   loc = apu.ex_q_tail;
   apu.ex_q_tail = ((apu.ex_q_tail + 1) & APUQUEUE_MASK);

   return (&apu.ex_queue[loc]);
}

/* --- Sound generators. --- */

static INLINE REAL apu_envelope (apu_chan_t *chan)
{
   REAL output;

   RT_ASSERT(chan);

   /* envelope decay at a rate of (env_delay + 1) / 240 secs */
   chan->env_phase -= (4 * apu.cnt_rate); /* 240/60 */

   while (chan->env_phase < 0)
   {
      chan->env_phase += chan->env_delay;

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
   REAL output;
   REAL sample_weight;
   REAL total;

   RT_ASSERT(chan);

   if (!chan->enabled ||
       (chan->vbl_length <= 0))
   {
      /* Channel is disabled or vbl length counter is negative or zero -
         return last known output level. */
      return (chan->output);
   }

   /* vbl length counter */
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
         chan->sweep_phase += chan->sweep_delay;

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
      chan->phaseacc += (chan->freq + 1);
      chan->adder = ((chan->adder + 1) & 0x0f);

      sample_weight = (chan->freq + 1);

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
   static REAL val;
   REAL sample_weight, total, prev_val;
   REAL output;

   RT_ASSERT(chan);

   if (!chan->enabled ||
       (chan->vbl_length <= 0))
   {
      return (chan->output);
   }

   if (chan->counter_started)
   {
      if (chan->linear_length > 0)
         chan->linear_length -= (4 * apu.cnt_rate);   /* 240/60 */

      if ((chan->vbl_length > 0) && !chan->holdnote)
         chan->vbl_length -= apu.cnt_rate;
   }
   else if (!chan->holdnote && chan->write_latency)
   {
      chan->write_latency--;

      /* Using EPSILON for zero range comparison is much more accurate than
         casting it to an integer and then comparing it directly to zero. */
      if ((chan->write_latency >= (0 - EPSILON)) ||
          (chan->write_latency <= (0 + EPSILON)))
      {
         chan->write_latency = 0;
         chan->counter_started = TRUE;
      }
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

   output = (total * 256.0f);
   output = ((output * 21.0f) / 16.0f);

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
   REAL output;

   static int noise_bit;
   REAL total;
   REAL sample_weight;

   RT_ASSERT(chan);

   if (!chan->enabled ||
       (chan->vbl_length <= 0))
   {
      return (chan->output);
   }

   /* vbl length counter */
   if (!chan->holdnote)
      chan->vbl_length -= apu.cnt_rate;

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

      chan->phaseacc += chan->freq;

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
   output = ((output * 13.0f) / 16.0f);

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

static INLINE REAL apu_dmc (apu_chan_t *chan)
{
   REAL total;
   REAL sample_weight;
   int delta_bit;
   REAL output;

   RT_ASSERT(chan);

   /* TODO: Alot of this could use a clean-up. */

   /* only process when channel is alive */
   if (chan->dma_length == 0)
   {
      chan->output = APU_TO_OUTPUT((chan->regs[1] << 8));
      return (chan->output);
   }

   sample_weight = chan->phaseacc;

   if (sample_weight > apu.cycle_rate)
      sample_weight = apu.cycle_rate;

   total = ((chan->regs[1] << 8) * sample_weight);

   chan->phaseacc -= apu.cycle_rate;  /* # of cycles per sample */
   
   while (chan->phaseacc < 0)
   {
      chan->phaseacc += chan->freq;
      
      if ((chan->dma_length & 7) == 0)
      {
         chan->cur_byte = cpu_read (chan->address);
         
         /* steal a cycle from CPU */
         cpu_consume_cycles (1);

         if (chan->address == 0xffff)
            chan->address = 0x8000;
         else
            chan->address++;
      }

      if (--chan->dma_length == 0)
      {
         /* if loop bit set, we're cool to retrigger sample */
         if (chan->looping)
         {
            /* Reload. */
            chan->address = chan->cached_addr;
            chan->dma_length = chan->cached_dmalength;
            chan->irq_occurred = FALSE;
         }
         else
         {
            /* check to see if we should generate an irq */
            if (chan->irq_gen)
            {
               chan->irq_occurred = TRUE;
               cpu_interrupt (CPU_INTERRUPT_IRQ);
            }

            /* bodge for timestamp queue */
            sample_weight = (chan->freq - chan->phaseacc);

            total += ((chan->regs[1] << 8) * sample_weight);

            while (chan->phaseacc < 0)
               chan->phaseacc += chan->freq;

            chan->enabled = FALSE;

            break;
         }
      }

      delta_bit = ((chan->dma_length & 7) ^ 7);

      if (chan->cur_byte & (1 << delta_bit))
      {
         /* positive delta */
         if (chan->regs[1] < 0x7d)
            chan->regs[1] += 2;
      }
      else            
      {
         /* negative delta */
         if (chan->regs[1] > 1)
            chan->regs[1] -= 2;
      }

      sample_weight = chan->freq;

      if (chan->phaseacc > 0)
         sample_weight -= chan->phaseacc;

      total += ((chan->regs[1] << 8) * sample_weight);
   }

   output = (total / apu.cycle_rate);
   output = ((output * 13.0f) / 16.0f);

   chan->output = APU_TO_OUTPUT(output);

   return (chan->output);
}

int apu_init (void)
{
   /* Initialize DSP. */
   dsp_init ();

   /* Load configuration. */

   apu_stereo_mode = get_config_int ("apu", "stereo_mode", apu_stereo_mode);

   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_SQUARE_1, get_config_int ("apu", "enable_square_1", TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_SQUARE_2, get_config_int ("apu", "enable_square_2", TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_TRIANGLE, get_config_int ("apu", "enable_triangle", TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_NOISE,    get_config_int ("apu", "enable_noise",    TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_DMC,      get_config_int ("apu", "enable_dmc",      TRUE));
   DSP_ENABLE_CHANNEL_EX(APU_CHANNEL_EXTRA,    get_config_int ("apu", "enable_extra",    TRUE));

   /* Initialize everything else. */
   apu_update ();

   /* Reset APU. */
   apu_reset ();

   /* Return success. */
   return (0);
}

void apu_exit (void)
{
   /* Deinitialize DSP. */
   dsp_exit ();

   /* Save configuration. */

   set_config_int ("apu", "stereo_mode", apu_stereo_mode);

   set_config_int ("apu", "enable_square_1", dsp_get_channel_enabled (APU_CHANNEL_SQUARE_1));
   set_config_int ("apu", "enable_square_2", dsp_get_channel_enabled (APU_CHANNEL_SQUARE_2));
   set_config_int ("apu", "enable_triangle", dsp_get_channel_enabled (APU_CHANNEL_TRIANGLE));
   set_config_int ("apu", "enable_noise",    dsp_get_channel_enabled (APU_CHANNEL_NOISE));
   set_config_int ("apu", "enable_dmc",      dsp_get_channel_enabled (APU_CHANNEL_DMC));
   set_config_int ("apu", "enable_extra",    dsp_get_channel_enabled (APU_CHANNEL_EXTRA));
}  

void apu_reset (void)
{
   /* Initializes/resets emulated sound hardware, creates waveforms/voices
      */

   REAL sample_rate, refresh_rate;

   /* Save parameters. */

   sample_rate = apu.sample_rate;
   refresh_rate = apu.refresh_rate;

   /* Clear context. */
   memset (&apu, 0, sizeof (apu));

   /* Clear queues. */
             
   memset (&apu.queue,    0, (APUQUEUE_SIZE * sizeof (apudata_t)));
   memset (&apu.ex_queue, 0, (APUQUEUE_SIZE * sizeof (apudata_t)));

   /* set the stupid flag to tell difference between two squares */

   apu.apus.square[0].sweep_complement = TRUE;
   apu.apus.square[1].sweep_complement = FALSE;

   /* Restore parameters. */
   set_params (sample_rate, refresh_rate);

   // for ExSound
   APU_LogTableInitialize ();

   APU_MMC5SoundReset ();
   APU_MMC5SoundVolume (1);
   APU_VRC6SoundReset ();
   APU_VRC6SoundVolume (1);

   // for $4017:bit7 by T.Yano
   apu.cnt_rate = 5;
}

void apu_update (void)
{
   /* Updates the APU to external changes without resetting it, since that
      might cause problems in a currently running game. */

   /* Set parameters. */
   set_params (audio_sample_rate, timing_get_speed ());

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

         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0f, -1.0f);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0f, +1.0f);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0f, -1.0f);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0f, +1.0f);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA,    1.0f, 0);

         break;
      }

      case APU_STEREO_MODE_2:
      {
         /* FakeNES enhanced. (may/2002) */
         /* Centers the triangle and noise. */

         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0f, -1.0f);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0f, +1.0f);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA,    1.0f, 0);

         break;
      }

      case APU_STEREO_MODE_3:
      {
         /* Real NES. */

         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0f, -1.0f);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0f, -1.0f);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0f, +1.0f);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0f, +1.0f);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0f, +1.0f);

         /* What should the behavior of ExSound be in this mode? */
         dsp_set_channel_params (APU_CHANNEL_EXTRA, 1.0f, 0);

         break;
      }

      case FALSE:
      case APU_STEREO_MODE_4:
      {
         /* Mono       (apu_stereo_mode == FALSE)
            Stereo Mix (apu_stereo_mode == APU_STEREO_MODE_4) */
                                                                   
         dsp_set_channel_params (APU_CHANNEL_SQUARE_1, 1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_SQUARE_2, 1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_TRIANGLE, 1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_NOISE,    1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_DMC,      1.0f, 0);
         dsp_set_channel_params (APU_CHANNEL_EXTRA,    1.0f, 0);

         break;
      }

      default:
         WARN_GENERIC();
   }
}

void apu_process (void)
{
   REAL elapsed_cycles;
   int samples;

   elapsed_cycles = apu.elapsed_cycles;

   /* TODO: Fix this, it probably won't work with audio disabled. */
   samples = audio_buffer_frame_size_samples;

   if (audio_enable_output)
   {
      void *buffer;

      /* Start DSP buffer fill. */
      dsp_start ();

      while (samples--)
      {
         apudata_t *data;
         int channel;
         DSP_SAMPLE dsp_samples[APU_CHANNELS];
   
         while (!APU_QEMPTY() &&
                (apu.queue[apu.q_tail].timestamp <= elapsed_cycles))
         {
            data = apu_dequeue ();
            regwrite (data->address, data->value);
         }
   
         while (!APU_EX_QEMPTY() &&
                (apu.ex_queue[apu.ex_q_tail].timestamp <= elapsed_cycles))
         {
            data = apu_ex_dequeue ();
   
            switch (apu.exsound)
            {
               case APU_EXSOUND_NONE:
                  break;
   
               case APU_EXSOUND_MMC5:
               {
                  APU_MMC5SoundWrite (data->address, data->value);
   
                  break;
               }
   
               case APU_EXSOUND_VRC6:
               {
                  APU_VRC6SoundWrite (data->address, data->value);
   
                  break;
               }
   
               default:
                  WARN_GENERIC();
            }
         }
   
         elapsed_cycles += apu.cycle_rate;
   
         /* Clear samples. */
         memset (dsp_samples, 0, sizeof (dsp_samples));
   
         for (channel = 0; channel < APU_CHANNELS; channel++)
         {
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
   
               case APU_CHANNEL_EXTRA:
               {
                  INT32 value = 0;

                  switch (apu.exsound)
                  {
                     case APU_EXSOUND_NONE:
                        break;
   
                     case APU_EXSOUND_MMC5:
                     {
                        value = APU_MMC5SoundRender ();
   
                        break;
                     }
   
                     case APU_EXSOUND_VRC6:
                     {
                        value = APU_VRC6SoundRender ();
   
                        break;
                     }
   
                     default:
                        // We disable this since it probably means an
                        // outdated version of a save state file.
                        // WARN_GENERIC();
                        break;
                  }

                  /* Convert to DSP format. */
                  sample = APU_TO_OUTPUT(value);

                  break;
               }
   
               default:
                  WARN_GENERIC();
            }
                 
            dsp_samples[channel] = sample;
         }
   
         /* Send samples to buffer. */
         dsp_write (dsp_samples);
      }

      /* End DSP buffer fill. */
      dsp_end ();

      buffer = audio_get_buffer ();
      if (!buffer)
         WARN_BREAK_GENERIC();

      /* Process DSP buffer into audio buffer. */
      dsp_render (buffer, (apu_stereo_mode ? 2 : 1), audio_sample_size,
         audio_unsigned_samples);

      audio_free_buffer ();
   }
   else  /* audio_enable_output */
   {
      while (samples--)
      {
         apudata_t *data;
   
         while ((!APU_QEMPTY()) &&
                (apu.queue[apu.q_tail].timestamp <= elapsed_cycles))
         {
            data = apu_dequeue ();
            regwrite (data->address, data->value);
         }
   
         while ((!APU_EX_QEMPTY()) &&
                (apu.ex_queue[apu.ex_q_tail].timestamp <= elapsed_cycles))
         {
            data = apu_ex_dequeue ();
   
            switch (apu.exsound)
            {
               case APU_EXSOUND_NONE:
                  break;
   
               case APU_EXSOUND_MMC5:
               {
                  APU_MMC5SoundWrite (data->address, data->value);
   
                  break;
               }
   
               case APU_EXSOUND_VRC6:
               {
                  APU_VRC6SoundWrite (data->address, data->value);
   
                  break;
               }
   
               default:
                  WARN_GENERIC();
            }
         }
   
         elapsed_cycles += apu.cycle_rate;
      }

   }  /* !audio_enable_output */

   /* Re-sync cycle counter. */
   apu.elapsed_cycles = cpu_get_cycles (FALSE);

   /* TODO: Figure out what the hell this does. :b */
   sync_apu_register ();
}

void apu_set_exsound (ENUM exsound)
{
   apu.exsound = exsound;
}

/* Read from $4000-$4017 */
UINT8 apu_read (UINT16 address)
{
   UINT8 value;

   switch (address)
   {
      case APU_SMASK:
      {
         value = 0;

         /* Return 1 in 0-5 bit pos if a channel is playing */

         if (apu.apus.square[0].enabled_cur &&
             (apu.apus.square[0].vbl_length_cur > 0))
         {
            value |= 0x01;
         }

         if (apu.apus.square[1].enabled_cur &&
             (apu.apus.square[1].vbl_length_cur > 0))
         {
            value |= 0x02;
         }

         if (apu.apus.triangle.enabled_cur &&
             (apu.apus.triangle.vbl_length_cur > 0))
         {
            value |= 0x04;
         }

         if (apu.apus.noise.enabled_cur &&
             (apu.apus.noise.vbl_length_cur > 0))
         {
            value |= 0x08;
         }

         /* bodge for timestamp queue */
         if (apu.apus.dmc.enabled_cur)
            value |= 0x10;

         if (apu.apus.dmc.irq_occurred_cur)
            value |= 0x80;

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
   apudata_t d;

   /* TODO: Clean this up as is seen fit.  Looks pretty OK to me, but I
      don't know how others would feel about the case grouping. */

   switch (address)
   {
      case 0x4015:
      {
         /* bodge for timestamp queue */
         apu.apus.dmc.enabled = TRUE_OR_FALSE((value & 0x10));

         /* No break. */
      }

      case 0x4000: case 0x4001: case 0x4002: case 0x4003:
      case 0x4004: case 0x4005: case 0x4006: case 0x4007:
      case 0x4008: case 0x4009: case 0x400A: case 0x400B:
      case 0x400C: case 0x400D: case 0x400E: case 0x400F:
      case 0x4010: case 0x4011: case 0x4012: case 0x4013:
      case 0x4017:
      {
         d.timestamp = cpu_get_cycles (FALSE);
         d.address = address;
         d.value = value;

         apu_enqueue (&d);

         break;
      }

      default:
         break;
   }
}

void apu_ex_write (UINT16 address, UINT8 value)
{
   apudata_t data;

   data.timestamp = cpu_get_cycles (FALSE);
   data.address   = address;
   data.value     = value;

   /* Queue it up. */
   apu_ex_enqueue (&data);
}

void apu_save_state (PACKFILE *file, int version)
{
   int index;

   RT_ASSERT(file);

   /* Squares. */
   for (index = 0; index < 2; index++)
   {
      int subindex;

      for (subindex = 0; subindex < 4; subindex++)
         pack_putc (apu.apus.square[index].regs[subindex], file);
   }

   /* Triangle. */
   for (index = 0; index < 3; index++)
      pack_putc (apu.apus.triangle.regs[index], file);

   /* Noise. */
   for (index = 0; index < 3; index++)
      pack_putc (apu.apus.noise.regs[index], file);

   /* DMC. */
   for (index = 0; index < 4; index++)
      pack_putc (apu.apus.dmc.regs[index], file);

   /* ExSound. */
   pack_putc (apu.exsound, file);
}

void apu_load_state (PACKFILE *file, int version)
{
   int index;

   RT_ASSERT(file);

   for (index = 0; index < 0x16; index++)
   {
      int value;

      if (index == 0x14)
         continue;

      value = pack_getc (file);

      if ((index >= 0x10) && (index <= 0x13))
      {
         /* Write the DMC registers directly. */
         apu.apus.dmc.regs[(index - 0x10)] = value;
      }
      else
      {
         apu_write ((0x4000 + index), value);
         write_cur ((0x4000 + index), value);
      }
   }

   /* ExSound. */
   apu_set_exsound (pack_getc (file));
}

/* --- Internal functions. --- */

static void set_params (REAL sample_rate, REAL refresh_rate)
{
   int samples;

   /* Set parameters. */
   apu.sample_rate = sample_rate;
   apu.refresh_rate = refresh_rate;

   samples = (int)(sample_rate / refresh_rate);

   apu.cycle_rate = (((machine_type == MACHINE_TYPE_NTSC) ? APU_BASEFREQ_NTSC
      : APU_BASEFREQ_PAL) / sample_rate);

   /* build various lookup tables for apu */
   build_luts (samples);
}

static void build_luts (int num_samples)
{
   int i;

   /* TODO: Oh god, clean this up.  However, pay special attention to
      the math, and remember to add in parenthesis in the order that the
      compiler would normally perform precedance, so that Humans can read
      it too, not just GCC. */

   // decay_lut[], vbl_lut[], trilength_lut[] modified (x5) for $4017:bit7 by T.Yano
   /* lut used for enveloping and frequency sweeps */
   for (i = 0; i < 16; i++)
      decay_lut[i] = num_samples * (i + 1) * 5;

   /* used for note length, based on vblanks and size of audio buffer */
   for (i = 0; i < 32; i++)
      vbl_lut[i] = vbl_length[i] * num_samples * 5;

   /* triangle wave channel's linear length table */
   for (i = 0; i < 128; i++)
      trilength_lut[i] = num_samples * i * 5;
}

static void regwrite (UINT32 address, UINT8 value)
{  
   int chan;

   /* TODO: This needs a major clean-up.  Some stuff here (such as ? TRUE :
            FALSE statements) just aren't neccessary.  Also, I noticed that
            alot of times calculations are repeated more than once for no
            good reason.  The result should be buffered instead.  I would
            also like to be able to wrap alot of this to 87 columns or so
            like the rest of the code. */

   switch (address)
   {
      case APU_WRA0: /* squares */
      case APU_WRB0:
      {
         chan = ((address & 4) ? 1 : 0);

         apu.apus.square[chan].regs[0] = value;
   
         apu.apus.square[chan].volume = (value & 0x0f);
         apu.apus.square[chan].env_delay = decay_lut[(value & 0x0f)];
         apu.apus.square[chan].holdnote = TRUE_OR_FALSE((value & 0x20));
         apu.apus.square[chan].fixed_envelope = TRUE_OR_FALSE((value & 0x10));
         apu.apus.square[chan].duty_flip = duty_lut[(value >> 6)];

         break;
      }

      case APU_WRA1:
      case APU_WRB1:
      {
         chan = ((address & 4) ? 1 : 0);

         apu.apus.square[chan].regs[1] = value;
         apu.apus.square[chan].sweep_on = TRUE_OR_FALSE((value & 0x80));
         apu.apus.square[chan].sweep_shifts = (value & 7);
         apu.apus.square[chan].sweep_delay = decay_lut[((value >> 4) & 7)];
         apu.apus.square[chan].sweep_inc = TRUE_OR_FALSE((value & 0x08));
         apu.apus.square[chan].freq_limit = freq_limit[(value & 7)];

         break;
      }

      case APU_WRA2:
      case APU_WRB2:
      {
         chan = ((address & 4) ? 1 : 0);

         apu.apus.square[chan].regs[2] = value;
         apu.apus.square[chan].freq = ((apu.apus.square[chan].freq & ~0xff) | value);

         break;
      }

      case APU_WRA3:
      case APU_WRB3:
      {
         chan = ((address & 4) ? 1 : 0);

         apu.apus.square[chan].regs[3] = value;
   
         apu.apus.square[chan].vbl_length = vbl_lut[(value >> 3)];
         apu.apus.square[chan].env_vol = 0;
         apu.apus.square[chan].freq = (((value & 7) << 8) | (apu.apus.square[chan].freq & 0xff));
         apu.apus.square[chan].adder = 0;

         if (apu.enable_reg & (1 << chan))
            apu.apus.square[chan].enabled = TRUE;

         break;
      }

      case APU_WRC0: /* triangle */
      {
         apu.apus.triangle.regs[0] = value;
         apu.apus.triangle.holdnote = TRUE_OR_FALSE((value & 0x80));
   
         if ((!apu.apus.triangle.counter_started) &&
             (apu.apus.triangle.vbl_length > 0))
         {
            apu.apus.triangle.linear_length =
               trilength_lut[(value & 0x7f)];
         }

         break;
      }

      case APU_WRC2:
      {
         apu.apus.triangle.regs[1] = value;
         apu.apus.triangle.freq = ((((apu.apus.triangle.regs[2] & 7) << 8) + value) + 1);

         break;
      }

      case APU_WRC3:
      {
         apu.apus.triangle.regs[2] = value;
     
         /* this is somewhat of a hack.  there appears to be some latency on 
         ** the Real Thing between when trireg0 is written to and when the 
         ** linear length counter actually begins its countdown.  we want to 
         ** prevent the case where the program writes to the freq regs first, 
         ** then to reg 0, and the counter accidentally starts running because 
         ** of the sound queue's timestamp processing.
         **
         ** set latency to a couple hundred cycles -- should be plenty of time 
         ** for the 6502 code to do a couple of table dereferences and load up 
         ** the other triregs
         */
   
         /* 06/13/00 MPC -- seems to work OK */
         apu.apus.triangle.write_latency = (228.0f / apu.cycle_rate);
   
         apu.apus.triangle.freq = ((((value & 7) << 8) + apu.apus.triangle.regs[1]) + 1);
         apu.apus.triangle.vbl_length = vbl_lut[(value >> 3)];
         apu.apus.triangle.counter_started = FALSE;
         apu.apus.triangle.linear_length = trilength_lut[(apu.apus.triangle.regs[0] & 0x7f)];

         if (apu.enable_reg & 0x04)
            apu.apus.triangle.enabled = TRUE;

         break;
      }
      
      case APU_WRD0: /* noise */
      {
         apu.apus.noise.regs[0] = value;
         apu.apus.noise.env_delay = decay_lut[(value & 0x0f)];
         apu.apus.noise.holdnote = TRUE_OR_FALSE((value & 0x20));
         apu.apus.noise.fixed_envelope = TRUE_OR_FALSE((value & 0x10));
         apu.apus.noise.volume = (value & 0x0f);

         break;
      }

      case APU_WRD2:
      {
         apu.apus.noise.regs[1] = value;
         apu.apus.noise.freq = noise_freq[(value & 0x0f)];
         apu.apus.noise.xor_tap = ((value & 0x80) ? 0x40 : 0x02);

         break;
      }

      case APU_WRD3:
      {
         apu.apus.noise.regs[2] = value;
   
         apu.apus.noise.vbl_length = vbl_lut[(value >> 3)];
         apu.apus.noise.env_vol = 0;  /* reset envelope */

         if (apu.enable_reg & 0x08)
            apu.apus.noise.enabled = TRUE;

         break;
      }

      case APU_WRE0: /* DMC */
      {
         apu.apus.dmc.regs[0] = value;
   
         apu.apus.dmc.freq = dmc_clocks[(value & 0x0f)];
         apu.apus.dmc.looping = TRUE_OR_FALSE((value & 0x40));
   
         if (value & 0x80)
         {
            apu.apus.dmc.irq_gen = TRUE;
         }
         else
         {
            apu.apus.dmc.irq_gen = FALSE;
            apu.apus.dmc.irq_occurred = FALSE;
         }

         break;
      }

      case APU_WRE1: /* 7-bit DAC */
      {
         /* add the _delta_ between written value and
         ** current output level of the volume reg
         */
         value &= 0x7f; /* bit 7 ignored */
         apu.apus.dmc.regs[1] = value;

         break;
      }

      case APU_WRE2:
      {
         apu.apus.dmc.regs[2] = value;
         apu.apus.dmc.cached_addr = (0xc000 + (UINT16)(value << 6));

         break;
      }

      case APU_WRE3:
      {
         apu.apus.dmc.regs[3] = value;
         apu.apus.dmc.cached_dmalength = (((value << 4) + 1) << 3);

         break;
      }

      case APU_SMASK:
      {
         /* bodge for timestamp queue */
         apu.apus.dmc.enabled = TRUE_OR_FALSE((value & 0x10));
   
         apu.enable_reg = value;
   
         for (chan = 0; chan < 2; chan++)
         {
            if ((value & (1 << chan)) == 0)
            {
               apu.apus.square[chan].enabled = FALSE;
               apu.apus.square[chan].vbl_length = 0;
            }
         }
   
         if ((value & 0x04) == 0)
         {
            apu.apus.triangle.enabled = FALSE;
            apu.apus.triangle.vbl_length = 0;
            apu.apus.triangle.linear_length = 0;
            apu.apus.triangle.counter_started = FALSE;
            apu.apus.triangle.write_latency = 0;
         }
   
         if ((value & 0x08) == 0)
         {
            apu.apus.noise.enabled = FALSE;
            apu.apus.noise.vbl_length = 0;
         }
   
         if (value & 0x10)
         {
            if (apu.apus.dmc.dma_length == 0)
            {
               /* Reload. */
               apu.apus.dmc.address = apu.apus.dmc.cached_addr;
               apu.apus.dmc.dma_length = apu.apus.dmc.cached_dmalength;
               apu.apus.dmc.irq_occurred = FALSE;
            }
         }
         else
         {
            apu.apus.dmc.dma_length = 0;
            apu.apus.dmc.irq_occurred = FALSE;
         }

         break;
      }

      case 0x4009:   /* unused, but they get hit in some mem-clear loops */
      case 0x400D:
         break;
   
      case 0x4017:
      {
         if (value & 0x80)
            apu.cnt_rate = 4;
         else
            apu.cnt_rate = 5;

         break;
      }
   
      default:
         break;
   }
}

static void write_cur (UINT16 address, UINT8 value)
{
   /* for sync read $4015 */
   int chan;

   /* TODO: Clean this horrible mess up.  It was far worse before I got to
      it, but I am still not satisfied with the code quality. */

   switch (address)
   {
      case APU_WRA0:
      case APU_WRB0:
      {
         chan = ((address & 4) ? 1 : 0);

         apu.apus.square[chan].holdnote_cur = TRUE_OR_FALSE((value & 0x20));

         break;
      }

      case APU_WRA3:
      case APU_WRB3:
      {
         chan = ((address & 4) ? 1 : 0);

         apu.apus.square[chan].vbl_length_cur = (vbl_length[(value >> 3)] * 5);

         if (apu.enable_reg_cur & (1 << chan))
            apu.apus.square[chan].enabled_cur = TRUE;

         break;
      }

      case APU_WRC0:
      {
         apu.apus.triangle.holdnote_cur = TRUE_OR_FALSE((value & 0x80));

         break;
      }

      case APU_WRC3:
      {
         apu.apus.triangle.vbl_length_cur = (vbl_length[(value >> 3)] * 5);

         if (apu.enable_reg_cur & 0x04)
            apu.apus.triangle.enabled_cur = TRUE;

         apu.apus.triangle.counter_started_cur = TRUE;

         break;
      }

      case APU_WRD0:
      {
         apu.apus.noise.holdnote_cur = TRUE_OR_FALSE((value & 0x20));

         break;
      }

      case APU_WRD3:
      {
         apu.apus.noise.vbl_length_cur = (vbl_length[(value >> 3)] * 5);

         if (apu.enable_reg_cur & 0x08)
            apu.apus.noise.enabled_cur = TRUE;

         break;
      }

      case APU_WRE0:
      {
         apu.apus.dmc.freq_cur = dmc_clocks[(value & 0x0f)];
         apu.apus.dmc.phaseacc_cur = 0;
         apu.apus.dmc.looping_cur = TRUE_OR_FALSE((value & 0x40));

         if (value & 0x80)
         {
            apu.apus.dmc.irq_gen_cur = TRUE;
         }
         else
         {
            apu.apus.dmc.irq_gen_cur = FALSE;
            apu.apus.dmc.irq_occurred_cur = FALSE;
         }

         break;
      }

      case APU_WRE3:
      {
         apu.apus.dmc.cached_dmalength_cur = ((value << 4) + 1);

         break;
      }

      case APU_SMASK:
      {
         apu.enable_reg_cur = value;

         for (chan = 0; chan < 2; chan++)
         {
            if ((value & (1 << chan)) == 0)
            {
               apu.apus.square[chan].enabled_cur = FALSE;
               apu.apus.square[chan].vbl_length_cur = 0;
            }
         }

         if ((value & 0x04) == 0)
         {
            apu.apus.triangle.enabled_cur = FALSE;
            apu.apus.triangle.vbl_length_cur = 0;
            apu.apus.triangle.counter_started_cur = FALSE;
         }

         if ((value & 0x08) == 0)
         {
            apu.apus.noise.enabled_cur = FALSE;
            apu.apus.noise.vbl_length_cur = 0;
         }

         if (value & 0x10)
         {
            if (apu.apus.dmc.dma_length_cur == 0)
               apu.apus.dmc.dma_length_cur = apu.apus.dmc.cached_dmalength_cur;

            apu.apus.dmc.enabled_cur = TRUE;
         }
         else
         {
            apu.apus.dmc.dma_length_cur = 0;
            apu.apus.dmc.enabled_cur = FALSE;
            apu.apus.dmc.irq_occurred_cur = FALSE;
         }

         break;
      }
   }
}

static void sync_apu_register (void)
{
   if ((!apu.apus.square[0].holdnote_cur) &&
       (apu.apus.square[0].vbl_length_cur > 0))
   {
      apu.apus.square[0].vbl_length_cur -= apu.cnt_rate;
   }

   if ((!apu.apus.square[1].holdnote_cur) &&
       (apu.apus.square[1].vbl_length_cur > 0))
   {
      apu.apus.square[1].vbl_length_cur -= apu.cnt_rate;
   }

   if (apu.apus.triangle.counter_started_cur)
   {
      if ((apu.apus.triangle.vbl_length_cur > 0) &&
          (!apu.apus.triangle.holdnote_cur))
      {
         apu.apus.triangle.vbl_length_cur -= apu.cnt_rate;
      }
   }

   if ((!apu.apus.noise.holdnote_cur) &&
       (apu.apus.noise.vbl_length_cur > 0))
   {
      apu.apus.noise.vbl_length_cur -= apu.cnt_rate;
   }
}
