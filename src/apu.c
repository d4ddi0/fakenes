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
#include "papu.h"
#include "types.h"

/* TODO: Would like more use of the below macro where possible.  Also would
   like the function reodered into public and private sections.  Alot of
   namespace issues need to be cleaned up with the public stuff as well
   (everything prefixed with APU_ or apu_ at minimum), and the individual
   ExSound header files need a huge clean-up, too. */

#ifdef DEBUG
#define DEBUG_PRINTF(msg)  printf (msg)
#else
#define DEBUG_PRINTF(msg)
#endif

/* pointer to active APU */
static apu_t *apu = NULL;

/* look up table madness */
static int decay_lut[16];
static int vbl_lut[32];
static int trilength_lut[128];

/* vblank length table used for rectangles, triangle, noise */
static const int vbl_length[] = {
   5, 127, 10,  1, 19,  2, 40,  3, 80, 4, 30,  5,  7, 6, 13, 7, 6, 8, 12,
   9,  24, 10, 48, 11, 96, 12, 36, 13, 8, 14, 16, 15
};

/* frequency limit of rectangle channels */
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

/* ratios of pos/neg pulse for rectangle waves */
static const int duty_lut[] = {
   2, 4, 8, 12
};

// for $4017:bit7 by T.Yano
static int apu_cnt_rate = 5;

void apu_setcontext (apu_t *src_apu)
{
   RT_ASSERT(src_apu);

   apu = src_apu;
}

apu_t *apu_getcontext (void)
{
   RT_ASSERT(apu);

   return (apu);
}

/*
** Simple queue routines
*/
#define APU_QEMPTY() (apu->q_head == apu->q_tail)
#define EX_QEMPTY()  (apu->ex_q_head == apu->ex_q_tail)

#include "apu_ex.h"

static INLINE void apu_enqueue (apudata_t *d)
{
   RT_ASSERT(d);
   RT_ASSERT(apu);

   apu->queue[apu->q_head] = *d;
   apu->q_head = ((apu->q_head + 1) & APUQUEUE_MASK);

   if (APU_QEMPTY())
      DEBUG_PRINTF("apu: queue overflow\n");      
}

static INLINE apudata_t *apu_dequeue(void)
{
   int loc;

   RT_ASSERT(apu);
                
   if (APU_QEMPTY())
      DEBUG_PRINTF("apu: queue empty\n");

   loc = apu->q_tail;
   apu->q_tail = ((apu->q_tail + 1) & APUQUEUE_MASK);

   return (&apu->queue[loc]);
}

static INLINE void ex_enqueue(apudata_t *d)
{
   RT_ASSERT(d);
   RT_ASSERT(apu);

   apu->ex_queue[apu->ex_q_head] = *d;
   apu->ex_q_head = ((apu->ex_q_head + 1) & APUQUEUE_MASK);

   if (EX_QEMPTY())
      DEBUG_PRINTF("ex_apu: queue overflow\n");      
}

static INLINE apudata_t *ex_dequeue(void)
{
   int loc;

   RT_ASSERT(apu);

   if (EX_QEMPTY())
      DEBUG_PRINTF("ex_apu: queue empty\n");

   loc = apu->ex_q_tail;
   apu->ex_q_tail = ((apu->ex_q_tail + 1) & APUQUEUE_MASK);

   return (&apu->ex_queue[loc]);
}

void apu_setchan (int chan, int enable)
{
   RT_ASSERT(apu);

   apu->mix_enable[chan] = enable;
}

/* emulation of the 15-bit shift register the
** NES uses to generate pseudo-random series
** for the white noise channel
*/
static INLINE int shift_register15 (int xor_tap)
{
   static int sreg = 0x4000;
   int bit0, tap, bit14;

   bit0 = (sreg & 1);
   tap = ((sreg & xor_tap) ? 1 : 0);
   bit14 = (bit0 ^ tap);
   sreg >>= 1;
   sreg |= (bit14 << 14);

   return ((bit0 ^ 1));
}

/* RECTANGLE WAVE
   ==============

   reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
   reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
   reg2: 8 bits of freq
   reg3: 0-2=high freq, 7-4=vbl length counter */

#define APU_RECTANGLE_OUTPUT()   ((chan->output_vol) << 8)

static INLINE int apu_rectangle (rectangle_t *chan)
{
   int output;
   float total;
   float sample_weight;

   RT_ASSERT(chan);

   if ((!chan->enabled) || (chan->vbl_length <= 0))
      return (APU_RECTANGLE_OUTPUT());

   /* vbl length counter */
   if (!chan->holdnote)
      chan->vbl_length -= apu_cnt_rate;

   /* envelope decay at a rate of (env_delay + 1) / 240 secs */
   chan->env_phase -= (4 * apu_cnt_rate); /* 240/60 */

   while (chan->env_phase < 0)
   {
      chan->env_phase += chan->env_delay;

      if (chan->holdnote)
         chan->env_vol = ((chan->env_vol + 1) & 0x0f);
      else if (chan->env_vol < 0x0f)
         chan->env_vol++;
   }

   /* TODO: using a table of max frequencies is not technically
   ** clean, but it is fast and (or should be) accurate 
   */
   if ((chan->freq < 8) ||
       ((!chan->sweep_inc) && (chan->freq > chan->freq_limit)))
   {
      return (APU_RECTANGLE_OUTPUT());
   }

   /* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */
   if (chan->sweep_on && chan->sweep_shifts)
   {
      chan->sweep_phase -= (2 * apu_cnt_rate);  /* 120/60 */

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

   if (chan->fixed_envelope)
      output = (chan->volume << 8); /* fixed volume */
   else
      output = ((chan->env_vol ^ 0x0f) << 8);

   sample_weight = chan->phaseacc;

   if (sample_weight > apu->cycle_rate)
      sample_weight = apu->cycle_rate;

   total = ((chan->adder < chan->duty_flip) ? sample_weight :
      -sample_weight);

   chan->phaseacc -= apu->cycle_rate;  /* # of cycles per sample */

   while (chan->phaseacc < 0)
   {
      chan->phaseacc += APU_TO_FIXED((chan->freq + 1));
      chan->adder = ((chan->adder + 1) & 0x0f);

      sample_weight = APU_TO_FIXED((chan->freq + 1));

      if (chan->phaseacc > 0)
         sample_weight -= chan->phaseacc;

      total += ((chan->adder < chan->duty_flip) ? sample_weight :
         -sample_weight);
   }

   /* TODO: Clean up this line. */
   chan->output_vol = (int)floor (output * total / apu->cycle_rate + 0.5);

   return (APU_RECTANGLE_OUTPUT());
}

/* TRIANGLE WAVE
   =============

   reg0: 7=holdnote, 6-0=linear length counter
   reg2: low 8 bits of frequency
   reg3: 7-3=length counter, 2-0=high 3 bits of frequency */

#define APU_TRIANGLE_OUTPUT() (((chan->output_vol * 21) >> 4) << 8)

static INLINE int apu_triangle (triangle_t *chan)
{
   static float val;
   float sample_weight, total, prev_val;

   RT_ASSERT(chan);

   if ((!chan->enabled) || (chan->vbl_length <= 0))
      return (APU_TRIANGLE_OUTPUT());

   if (chan->counter_started)
   {
      if (chan->linear_length > 0)
         chan->linear_length -= (4 * apu_cnt_rate);   /* 240/60 */

      if ((chan->vbl_length > 0) && (!chan->holdnote))
         chan->vbl_length -= apu_cnt_rate;
   }
   else if ((!chan->holdnote) && chan->write_latency)
   {
      if (--chan->write_latency == 0)
         chan->counter_started = TRUE;
   }

   if ((chan->linear_length <= 0) || (chan->freq < APU_TO_FIXED(4)))
   {
      /* inaudible */
      return (APU_TRIANGLE_OUTPUT());
   }

   /* TODO: All of the following could use a major clean-up. */

   if (chan->ideal_triangle)
   {
      total = 0;
      sample_weight = 0;
      prev_val = val;

      if (chan->adder)
         val -= ((float)apu->cycle_rate / chan->freq);
      else
         val += ((float)apu->cycle_rate / chan->freq);

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

      chan->output_vol = (int)floor (total * 256 + 0.5);
   }
   else  /* !ideal_triangle */
   {
      sample_weight = chan->phaseacc;

      if (sample_weight > apu->cycle_rate)
         sample_weight = apu->cycle_rate;

      total = ((((chan->adder & 0x10) ? 0x1f : 0) ^ chan->adder) *
         sample_weight);

      chan->phaseacc -= apu->cycle_rate;  /* # of cycles per sample */

      while (chan->phaseacc < 0)
      {
         chan->phaseacc += chan->freq;
         chan->adder = ((chan->adder + 1) & 0x1f);

         sample_weight = chan->freq;

         if (chan->phaseacc > 0)
            sample_weight -= chan->phaseacc;

         total += ((((chan->adder & 0x10) ? 0x1f : 0) ^ chan->adder) *
            sample_weight);
      }

      chan->output_vol = (int)floor (total * 512 / apu->cycle_rate + 0.5);
   }

   return (APU_TRIANGLE_OUTPUT());
}


/* WHITE NOISE CHANNEL
   ===================

   reg0: 0-3=volume, 4=envelope, 5=hold
   reg2: 7=small(93 byte) sample,3-0=freq lookup
   reg3: 7-4=vbl length counter */

#define APU_NOISE_OUTPUT() (((chan->output_vol * 13) >> 4) << 8)

static INLINE int apu_noise (noise_t *chan)
{
   int outvol;

   static int noise_bit;
   float total;
   float sample_weight;

   RT_ASSERT(chan);

   if ((!chan->enabled) || (chan->vbl_length <= 0))
      return (APU_NOISE_OUTPUT());

   /* vbl length counter */
   if (!chan->holdnote)
      chan->vbl_length -= apu_cnt_rate;

   /* envelope decay at a rate of (env_delay + 1) / 240 secs */
   chan->env_phase -= (4 * apu_cnt_rate); /* 240/60 */

   while (chan->env_phase < 0)
   {
      chan->env_phase += chan->env_delay;

      if (chan->holdnote)
         chan->env_vol = ((chan->env_vol + 1) & 0x0f);
      else if (chan->env_vol < 0x0f)
         chan->env_vol++;
   }

   if (chan->fixed_envelope)
      outvol = (chan->volume << 8); /* fixed volume */
   else
      outvol = ((chan->env_vol ^ 0x0f) << 8);

   sample_weight = chan->phaseacc;

   if (sample_weight > apu->cycle_rate)
      sample_weight = apu->cycle_rate;

   total = (noise_bit ? sample_weight : -sample_weight);

   chan->phaseacc -= apu->cycle_rate;  /* # of cycles per sample */

   while (chan->phaseacc < 0)
   {
      chan->phaseacc += chan->freq;

      noise_bit = shift_register15 (chan->xor_tap);

      sample_weight = chan->freq;

      if (chan->phaseacc > 0)
         sample_weight -= chan->phaseacc;

      total += (noise_bit ? sample_weight : -sample_weight);
   }

   /* TODO: Clean up this line. */
   chan->output_vol = (int)floor (outvol * total / apu->cycle_rate + 0.5);

   return (APU_NOISE_OUTPUT());
}

static INLINE void apu_dmcreload (dmc_t *chan)
{
   RT_ASSERT(chan);

   chan->address = chan->cached_addr;
   chan->dma_length = chan->cached_dmalength;
   chan->irq_occurred = FALSE;
}

/* DELTA MODULATION CHANNEL
   =========================

   reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
   reg1: output dc level, 6 bits unsigned
   reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
   reg3: length, (value * 16) + 1 */

#define APU_DMC_OUTPUT()   (((chan->output_vol * 13) >> 4) << 8)

static INLINE int apu_dmc (dmc_t *chan)
{
   float total;
   float sample_weight;
   int delta_bit;

   RT_ASSERT(chan);

   /* TODO: Alot of this could use a clean-up. */

   /* only process when channel is alive */
   if (chan->dma_length == 0)
   {
      chan->output_vol = (chan->regs[1] << 8);

      return (APU_DMC_OUTPUT());
   }

   sample_weight = chan->phaseacc;

   if (sample_weight > apu->cycle_rate)
      sample_weight = apu->cycle_rate;

   total = ((chan->regs[1] << 8) * sample_weight);

   chan->phaseacc -= apu->cycle_rate;  /* # of cycles per sample */
   
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
            apu_dmcreload (chan);
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

   chan->output_vol = (int)floor (total / apu->cycle_rate + 0.5);

   return (APU_DMC_OUTPUT());
}

static void apu_regwrite (UINT32 address, UINT8 value)
{  
   int chan;

   RT_ASSERT(apu);

   /* TODO: This needs a major clean-up.  Some stuff here (such as ? TRUE :
            FALSE statements) just aren't neccessary.  Also, I noticed that
            alot of times calculations are repeated more than once for no
            good reason.  The result should be buffered instead.  I would
            also like to be able to wrap alot of this to 87 columns or so
            like the rest of the code. */

   switch (address)
   {
      case APU_WRA0: /* rectangles */
      case APU_WRB0:
      {
         chan = ((address & 4) ? 1 : 0);

         apu->apus.rectangle[chan].regs[0] = value;
   
         apu->apus.rectangle[chan].volume = (value & 0x0f);
         apu->apus.rectangle[chan].env_delay = decay_lut[(value & 0x0f)];
         apu->apus.rectangle[chan].holdnote = ((value & 0x20) ? TRUE : FALSE);
         apu->apus.rectangle[chan].fixed_envelope = ((value & 0x10) ? TRUE : FALSE);
         apu->apus.rectangle[chan].duty_flip = duty_lut[(value >> 6)];

         break;
      }

      case APU_WRA1:
      case APU_WRB1:
      {
         chan = ((address & 4) ? 1 : 0);

         apu->apus.rectangle[chan].regs[1] = value;
         apu->apus.rectangle[chan].sweep_on = ((value & 0x80) ? TRUE : FALSE);
         apu->apus.rectangle[chan].sweep_shifts = (value & 7);
         apu->apus.rectangle[chan].sweep_delay = decay_lut[((value >> 4) & 7)];
         apu->apus.rectangle[chan].sweep_inc = ((value & 0x08) ? TRUE : FALSE);
         apu->apus.rectangle[chan].freq_limit = freq_limit[(value & 7)];

         break;
      }

      case APU_WRA2:
      case APU_WRB2:
      {
         chan = ((address & 4) ? 1 : 0);

         apu->apus.rectangle[chan].regs[2] = value;
         apu->apus.rectangle[chan].freq = ((apu->apus.rectangle[chan].freq & ~0xff) | value);

         break;
      }

      case APU_WRA3:
      case APU_WRB3:
      {
         chan = ((address & 4) ? 1 : 0);

         apu->apus.rectangle[chan].regs[3] = value;
   
         apu->apus.rectangle[chan].vbl_length = vbl_lut[(value >> 3)];
         apu->apus.rectangle[chan].env_vol = 0;
         apu->apus.rectangle[chan].freq = (((value & 7) << 8) | (apu->apus.rectangle[chan].freq & 0xff));
         apu->apus.rectangle[chan].adder = 0;

         if (apu->enable_reg & (1 << chan))
            apu->apus.rectangle[chan].enabled = TRUE;

         break;
      }

      case APU_WRC0: /* triangle */
      {
         apu->apus.triangle.regs[0] = value;
         apu->apus.triangle.holdnote = ((value & 0x80) ? TRUE : FALSE);
   
         if ((!apu->apus.triangle.counter_started) &&
             (apu->apus.triangle.vbl_length > 0))
         {
            apu->apus.triangle.linear_length =
               trilength_lut[(value & 0x7f)];
         }

         break;
      }

      case APU_WRC2:
      {
         apu->apus.triangle.regs[1] = value;
         apu->apus.triangle.freq = APU_TO_FIXED(((((apu->apus.triangle.regs[2] & 7) << 8) + value) + 1));

         break;
      }

      case APU_WRC3:
      {
         apu->apus.triangle.regs[2] = value;
     
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
         apu->apus.triangle.write_latency = (int)(228 / APU_FROM_FIXED(apu->cycle_rate));
   
         apu->apus.triangle.freq = APU_TO_FIXED(((((value & 7) << 8) + apu->apus.triangle.regs[1]) + 1));
         apu->apus.triangle.vbl_length = vbl_lut[(value >> 3)];
         apu->apus.triangle.counter_started = FALSE;
         apu->apus.triangle.linear_length = trilength_lut[(apu->apus.triangle.regs[0] & 0x7f)];

         if (apu->enable_reg & 0x04)
            apu->apus.triangle.enabled = TRUE;

         break;
      }
      
      case APU_WRD0: /* noise */
      {
         apu->apus.noise.regs[0] = value;
         apu->apus.noise.env_delay = decay_lut[(value & 0x0f)];
         apu->apus.noise.holdnote = ((value & 0x20) ? TRUE : FALSE);
         apu->apus.noise.fixed_envelope = ((value & 0x10) ? TRUE : FALSE);
         apu->apus.noise.volume = (value & 0x0f);

         break;
      }

      case APU_WRD2:
      {
         apu->apus.noise.regs[1] = value;
         apu->apus.noise.freq = APU_TO_FIXED(noise_freq[(value & 0x0f)]);
         apu->apus.noise.xor_tap = ((value & 0x80) ? 0x40 : 0x02);

         break;
      }

      case APU_WRD3:
      {
         apu->apus.noise.regs[2] = value;
   
         apu->apus.noise.vbl_length = vbl_lut[(value >> 3)];
         apu->apus.noise.env_vol = 0;  /* reset envelope */

         if (apu->enable_reg & 0x08)
            apu->apus.noise.enabled = TRUE;

         break;
      }

      case APU_WRE0: /* DMC */
      {
         apu->apus.dmc.regs[0] = value;
   
         apu->apus.dmc.freq = APU_TO_FIXED(dmc_clocks[(value & 0x0f)]);
         apu->apus.dmc.looping = ((value & 0x40) ? TRUE : FALSE);
   
         if (value & 0x80)
         {
            apu->apus.dmc.irq_gen = TRUE;
         }
         else
         {
            apu->apus.dmc.irq_gen = FALSE;
            apu->apus.dmc.irq_occurred = FALSE;
         }

         break;
      }

      case APU_WRE1: /* 7-bit DAC */
      {
         /* add the _delta_ between written value and
         ** current output level of the volume reg
         */
         value &= 0x7f; /* bit 7 ignored */
         apu->apus.dmc.regs[1] = value;

         break;
      }

      case APU_WRE2:
      {
         apu->apus.dmc.regs[2] = value;
         apu->apus.dmc.cached_addr = (0xc000 + (UINT16)(value << 6));

         break;
      }

      case APU_WRE3:
      {
         apu->apus.dmc.regs[3] = value;
         apu->apus.dmc.cached_dmalength = (((value << 4) + 1) << 3);

         break;
      }

      case APU_SMASK:
      {
         /* bodge for timestamp queue */
         apu->apus.dmc.enabled = ((value & 0x10) ? TRUE : FALSE);
   
         apu->enable_reg = value;
   
         for (chan = 0; chan < 2; chan++)
         {
            if ((value & (1 << chan)) == 0)
            {
               apu->apus.rectangle[chan].enabled = FALSE;
               apu->apus.rectangle[chan].vbl_length = 0;
            }
         }
   
         if ((value & 0x04) == 0)
         {
            apu->apus.triangle.enabled = FALSE;
            apu->apus.triangle.vbl_length = 0;
            apu->apus.triangle.linear_length = 0;
            apu->apus.triangle.counter_started = FALSE;
            apu->apus.triangle.write_latency = 0;
         }
   
         if ((value & 0x08) == 0)
         {
            apu->apus.noise.enabled = FALSE;
            apu->apus.noise.vbl_length = 0;
         }
   
         if (value & 0x10)
         {
            if (apu->apus.dmc.dma_length == 0)
               apu_dmcreload (&apu->apus.dmc);
         }
         else
         {
            apu->apus.dmc.dma_length = 0;
            apu->apus.dmc.irq_occurred = FALSE;
         }

         break;
      }

      case 0x4009:   /* unused, but they get hit in some mem-clear loops */
      case 0x400D:
         break;
   
      case 0x4017:
      {
         if (value & 0x80)
            apu_cnt_rate = 4;
         else
            apu_cnt_rate = 5;

         break;
      }
   
      default:
         break;
   }
}

/* Read from $4000-$4017 */
UINT8 apu_read (UINT32 address)
{
   UINT8 value;

   RT_ASSERT(apu);

   switch (address)
   {
      case APU_SMASK:
      {
         value = 0;

         /* Return 1 in 0-5 bit pos if a channel is playing */

         if (apu->apus.rectangle[0].enabled_cur &&
             (apu->apus.rectangle[0].vbl_length_cur > 0))
         {
            value |= 0x01;
         }

         if (apu->apus.rectangle[1].enabled_cur &&
             (apu->apus.rectangle[1].vbl_length_cur > 0))
         {
            value |= 0x02;
         }

         if (apu->apus.triangle.enabled_cur &&
             (apu->apus.triangle.vbl_length_cur > 0))
         {
            value |= 0x04;
         }

         if (apu->apus.noise.enabled_cur &&
             (apu->apus.noise.vbl_length_cur > 0))
         {
            value |= 0x08;
         }

         /* bodge for timestamp queue */
         if (apu->apus.dmc.enabled_cur)
            value |= 0x10;

         if (apu->apus.dmc.irq_occurred_cur)
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

UINT8 ex_read (UINT32 address)
{
   RT_ASSERT(apu);

   if (apu->ex_chip & 4)
   {
      return (FDSSoundRead (address));
   }
   else if (apu->ex_chip & 16)
   {
      apudata_t d;

      d.timestamp = cpu_get_cycles (FALSE);
      d.address = (address + 0x10000);

      ex_enqueue (&d);
   }

   return (0);
}

void apu_write (UINT32 address, UINT8 value)
{
   apudata_t d;

   RT_ASSERT(apu);

   /* TODO: Clean this up as is seen fit.  Looks pretty OK to me, but I
      don't know how others would feel about the case grouping. */

   switch (address)
   {
      case 0x4015:
      {
         /* bodge for timestamp queue */
         apu->apus.dmc.enabled = ((value & 0x10) ? TRUE : FALSE);

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

void apu_write_cur (UINT32 address, UINT8 value)
{
   /* for sync read $4015 */
   int chan;

   RT_ASSERT(apu);

   /* TODO: Clean this horrible mess up.  It was far worse before I got to
      it, but I am still not satisfied with the code quality. */

   switch (address)
   {
      case APU_WRA0:
      case APU_WRB0:
      {
         chan = ((address & 4) ? 1 : 0);

         apu->apus.rectangle[chan].holdnote_cur = ((value & 0x20) ? TRUE : FALSE);

         break;
      }

      case APU_WRA3:
      case APU_WRB3:
      {
         chan = ((address & 4) ? 1 : 0);

         apu->apus.rectangle[chan].vbl_length_cur = (vbl_length[(value >> 3)] * 5);

         if (apu->enable_reg_cur & (1 << chan))
            apu->apus.rectangle[chan].enabled_cur = TRUE;

         break;
      }

      case APU_WRC0:
      {
         apu->apus.triangle.holdnote_cur = ((value & 0x80) ? TRUE : FALSE);

         break;
      }

      case APU_WRC3:
      {
         apu->apus.triangle.vbl_length_cur = (vbl_length[(value >> 3)] * 5);

         if (apu->enable_reg_cur & 0x04)
            apu->apus.triangle.enabled_cur = TRUE;

         apu->apus.triangle.counter_started_cur = TRUE;

         break;
      }

      case APU_WRD0:
      {
         apu->apus.noise.holdnote_cur = ((value & 0x20) ? TRUE : FALSE);

         break;
      }

      case APU_WRD3:
      {
         apu->apus.noise.vbl_length_cur = (vbl_length[(value >> 3)] * 5);

         if (apu->enable_reg_cur & 0x08)
            apu->apus.noise.enabled_cur = TRUE;

         break;
      }

      case APU_WRE0:
      {
         apu->apus.dmc.freq_cur = dmc_clocks[(value & 0x0f)];
         apu->apus.dmc.phaseacc_cur = 0;
         apu->apus.dmc.looping_cur = ((value & 0x40) ? TRUE : FALSE);

         if (value & 0x80)
         {
            apu->apus.dmc.irq_gen_cur = TRUE;
         }
         else
         {
            apu->apus.dmc.irq_gen_cur = FALSE;
            apu->apus.dmc.irq_occurred_cur = FALSE;
         }

         break;
      }

      case APU_WRE3:
      {
         apu->apus.dmc.cached_dmalength_cur = ((value << 4) + 1);

         break;
      }

      case APU_SMASK:
      {
         apu->enable_reg_cur = value;

         for (chan = 0; chan < 2; chan++)
         {
            if ((value & (1 << chan)) == 0)
            {
               apu->apus.rectangle[chan].enabled_cur = FALSE;
               apu->apus.rectangle[chan].vbl_length_cur = 0;
            }
         }

         if ((value & 0x04) == 0)
         {
            apu->apus.triangle.enabled_cur = FALSE;
            apu->apus.triangle.vbl_length_cur = 0;
            apu->apus.triangle.counter_started_cur = FALSE;
         }

         if ((value & 0x08) == 0)
         {
            apu->apus.noise.enabled_cur = FALSE;
            apu->apus.noise.vbl_length_cur = 0;
         }

         if (value & 0x10)
         {
            if (apu->apus.dmc.dma_length_cur == 0)
               apu->apus.dmc.dma_length_cur = apu->apus.dmc.cached_dmalength_cur;

            apu->apus.dmc.enabled_cur = TRUE;
         }
         else
         {
            apu->apus.dmc.dma_length_cur = 0;
            apu->apus.dmc.enabled_cur = FALSE;
            apu->apus.dmc.irq_occurred_cur = FALSE;
         }

         break;
      }
   }
}

void ex_write (UINT32 address, UINT8 value)
{
   apudata_t d;

   RT_ASSERT(apu);

   d.timestamp = cpu_get_cycles (FALSE);
   d.address = address;
   d.value = value;

   ex_enqueue (&d);

   if (apu->ex_chip & 4)
      FDSSoundWriteCurrent (address, value);
}

void apu_getpcmdata (void **data, int *num_samples, int *sample_bits)
{
   RT_ASSERT(apu);

   *data = apu->buffer;
   *num_samples = apu->num_samples;
   *sample_bits = apu->sample_bits;
}

void apu_process (void *buffer, int num_samples, int dither)
{
   apudata_t *d;
   int elapsed_cycles;
   static int prev_sample = 0;
   int next_sample, accum;

   RT_ASSERT(apu);

   /* grab it, keep it local for speed */
   elapsed_cycles = apu->elapsed_cycles;

   if (buffer)
   {
      apu->buffer = buffer; 

      while (num_samples--)
      {
         while ((!APU_QEMPTY()) &&
                (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
         {
            d = apu_dequeue ();
            apu_regwrite (d->address, d->value);
         }

         while ((!EX_QEMPTY()) &&
                (apu->ex_queue[apu->ex_q_tail].timestamp <= elapsed_cycles))
         {
            d = ex_dequeue();

            if (apu->ex_chip & 1)
               VRC6SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 2)
               OPLLSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 4)
               FDSSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 8)
               MMC5SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 16)
            {
               if (d->address & 0x10000)
                  N106SoundRead (d->address & 0xffff);
               else
                  N106SoundWrite (d->address, d->value);
            }
            else if (apu->ex_chip & 32)
               PSGSoundWrite (d->address, d->value);
         }

         elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);

         accum = 0;

         if (apu->mix_enable[0])
            accum += apu_rectangle (&apu->apus.rectangle[0]);
         if (apu->mix_enable[1])
            accum += apu_rectangle (&apu->apus.rectangle[1]);
         if (apu->mix_enable[2])
            accum += apu_triangle (&apu->apus.triangle);
         if (apu->mix_enable[3])
            accum += apu_noise (&apu->apus.noise);
         if (apu->mix_enable[4])
            accum += apu_dmc (&apu->apus.dmc);

         // if (apu->ext && apu->mix_enable[5])
         //    accum += apu->ext->process ();

         if (apu->mix_enable[5])
         {
            if (apu->ex_chip & 1)
               accum += VRC6SoundRender ();
            else if (apu->ex_chip & 2)
               accum += OPLLSoundRender ();
            else if (apu->ex_chip & 4)
               accum += FDSSoundRender ();
            else if (apu->ex_chip & 8)
               accum += MMC5SoundRender ();
            else if (apu->ex_chip & 16)
               accum += N106SoundRender ();
            else if (apu->ex_chip & 32)
               accum += PSGSoundRender ();
         }

         /* do any filtering */
         next_sample = accum;

         if (apu->filter_list & PAPU_FILTER_LOW_PASS_MODE_1)
         {
            accum += prev_sample;
            accum >>= 1;
         }
         else if (apu->filter_list & PAPU_FILTER_LOW_PASS_MODE_2)
         {
            next_sample = accum = (((accum + accum + accum) + prev_sample) >> 2);
         }
         else if (apu->filter_list & PAPU_FILTER_LOW_PASS_MODE_3)
         {
            accum += prev_sample;
            next_sample = accum >>= 1;
         }

         if (apu->filter_list & PAPU_FILTER_HIGH_PASS)
         {
            accum -= prev_sample;
            accum <<= 2;
         }

         if (apu->filter_list & PAPU_FILTER_DELTA_SIGMA_FILTER)
         {
            /* Delta-Sigma filter by Siloh. */

            int old_accum;

            old_accum = accum;
            accum = ((accum + accum + accum + prev_sample) >> 2);
            next_sample = accum;
            accum += (accum - old_accum);
            accum -= (int)ROUND(((rand () / ((float)RAND_MAX - 1)) * (accum
               * 0.01)));
         }

         prev_sample = next_sample;

         /* prevent clipping */
         if (accum > 0x7fffff)
            accum = 0x7fffff;
         else if (accum < -0x800000)
            accum = -0x800000;

         if (audio_unsigned_samples)
            accum ^= 0x800000;

         if (apu->sample_bits == 16)
         {
            UINT16 *buf = buffer;

            if (dither)
               accum ^= ((accum & 0x000080) << 1);

            /* store sample and increment base pointer */
            *buf++ = (accum >> 8);

            /* save changes back to typeless buffer */
            buffer = buf;
         }
         else
         {
            UINT8 *buf = buffer;

            if (dither)
               accum ^= ((accum & 0x008000) << 1);

            *buf++ = (accum >> 16);

            buffer = buf;
         }
      }
   }
   else  /* buffer */
   {
      while (num_samples--)
      {
         while ((!APU_QEMPTY()) &&
                (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
         {
            d = apu_dequeue ();
            apu_regwrite (d->address, d->value);
         }

         while ((!EX_QEMPTY()) &&
                (apu->ex_queue[apu->ex_q_tail].timestamp <= elapsed_cycles))
         {
            d = ex_dequeue();

            if (apu->ex_chip & 1)
               VRC6SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 2)
               OPLLSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 4)
               FDSSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 8)
               MMC5SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 16)
            {
               if (d->address & 0x10000)
                  N106SoundRead (d->address & 0xffff);
               else
                  N106SoundWrite (d->address, d->value);
            }
            else if (apu->ex_chip & 32)
               PSGSoundWrite (d->address, d->value);
         }

         elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);
      }
   }

   /* resync cycle counter */
   apu->elapsed_cycles = cpu_get_cycles (FALSE);
}

void apu_process_stereo (void *buffer, int num_samples, int dither, int
   style, int flip, int surround)
{
   apudata_t *d;
   int elapsed_cycles;
   int scrap1, scrap2;
   int accum_centre, accum_left, accum_right;
   int old_accum_left, old_accum_right;
   int next_sample_left, next_sample_right;
   static int prev_sample_left = 0, prev_sample_right = 0;

   RT_ASSERT(apu);

   /* grab it, keep it local for speed */
   elapsed_cycles = apu->elapsed_cycles;

   if (buffer)
   {
      apu->buffer = buffer; 

      while (num_samples--)
      {
         while ((!APU_QEMPTY()) &&
                (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
         {
            d = apu_dequeue ();
            apu_regwrite (d->address, d->value);
         }

         while ((!EX_QEMPTY()) &&
                (apu->ex_queue[apu->ex_q_tail].timestamp <= elapsed_cycles))
         {
            d = ex_dequeue();

            if(apu->ex_chip & 1)
               VRC6SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 2)
               OPLLSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 4)
               FDSSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 8)
               MMC5SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 16)
            {
               if (d->address & 0x10000)
                  N106SoundRead (d->address & 0xffff);
               else
                  N106SoundWrite (d->address, d->value);
            }
            else if (apu->ex_chip & 32)
               PSGSoundWrite (d->address, d->value);
         }

         elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);

         accum_centre = accum_left = accum_right = 0;

         switch (style)
         {
            case 1:  /* FakeNES classic. */
            {
               if (apu->mix_enable[0])
                  accum_left += apu_rectangle (&apu->apus.rectangle[0]);
               if (apu->mix_enable[1])
                  accum_right += apu_rectangle (&apu->apus.rectangle[1]);
               if (apu->mix_enable[2])
                  accum_left += apu_triangle (&apu->apus.triangle);
               if (apu->mix_enable[3])
                  accum_right += apu_noise (&apu->apus.noise);
               if (apu->mix_enable[4])
                  accum_centre += apu_dmc (&apu->apus.dmc);

               break;
            }

            case 2:  /* FakeNES enhanced. (may/2002) */
            {
               /* Centers the triangle. */

               if (apu->mix_enable[0])
                  accum_left += apu_rectangle (&apu->apus.rectangle[0]);
               if (apu->mix_enable[1])
                  accum_right += apu_rectangle (&apu->apus.rectangle[1]);
   
               if (apu->mix_enable[2])
                  accum_centre += apu_triangle (&apu->apus.triangle);
               if (apu->mix_enable[3])
                  accum_centre += apu_noise (&apu->apus.noise);
               if (apu->mix_enable[4])
                  accum_centre += apu_dmc (&apu->apus.dmc);

               break;
            }

            case 3:  /* Real NES. */
            {
               if (apu->mix_enable[0])
                  accum_left += apu_rectangle (&apu->apus.rectangle[0]);
               if (apu->mix_enable[1])
                  accum_left += apu_rectangle (&apu->apus.rectangle[1]);
               if (apu->mix_enable[2])
                  accum_right += apu_triangle (&apu->apus.triangle);
               if (apu->mix_enable[3])
                  accum_right += apu_noise (&apu->apus.noise);
               if (apu->mix_enable[4])
                  accum_right += apu_dmc (&apu->apus.dmc);

               break;
            }

            default: /* Dumb mix. */
            {
               if (apu->mix_enable[0])
                  accum_left += apu_rectangle (&apu->apus.rectangle[0]);
               if (apu->mix_enable[1])
                  accum_left += apu_rectangle (&apu->apus.rectangle[1]);
               if (apu->mix_enable[2])
                  accum_left += apu_triangle (&apu->apus.triangle);
               if (apu->mix_enable[3])
                  accum_left += apu_noise (&apu->apus.noise);
               if (apu->mix_enable[4])
                  accum_left += apu_dmc (&apu->apus.dmc);

               accum_left >>= 1;
               accum_right = accum_left;

               break;
            }
         }

         // if (apu->ext && apu->mix_enable[5])
         //    accum += apu->ext->process ();

         if (apu->mix_enable[5])
         {
            if (apu->ex_chip & 1)
               accum_centre += VRC6SoundRender ();
            else if (apu->ex_chip & 2)
               accum_centre += OPLLSoundRender ();
            else if (apu->ex_chip & 4)
               accum_centre += FDSSoundRender ();
            else if (apu->ex_chip & 8)
               accum_centre += MMC5SoundRender ();
            else if (apu->ex_chip & 16)
               accum_centre += N106SoundRender ();
            else if (apu->ex_chip & 32)
               accum_centre += PSGSoundRender ();
         }

         accum_left += (accum_centre >> 1);
         accum_right += (accum_centre >> 1);

         /* do any filtering */
         next_sample_left = accum_left;
         next_sample_right = accum_right;

         if (apu->filter_list & PAPU_FILTER_LOW_PASS_MODE_1)
         {
            accum_left += prev_sample_left;
            accum_left >>= 1;
            accum_right += prev_sample_right;
            accum_right >>= 1;
         }
         else if (apu->filter_list & PAPU_FILTER_LOW_PASS_MODE_2)
         {
            accum_left = (((accum_left + accum_left + accum_left) + prev_sample_left) >> 2);
            accum_right = (((accum_right + accum_right + accum_right) + prev_sample_right) >> 2);
         }
         else if (apu->filter_list & PAPU_FILTER_LOW_PASS_MODE_3)
         {
            accum_left += prev_sample_left;
            next_sample_left = accum_left >>= 1;
            accum_right += prev_sample_right;
            next_sample_right = accum_right >>= 1;
         }

         if (apu->filter_list & PAPU_FILTER_HIGH_PASS)
         {
            accum_left -= prev_sample_left;
            accum_left <<= 2;
            accum_right -= prev_sample_right;
            accum_right <<= 2;
         }

         if (apu->filter_list & PAPU_FILTER_DELTA_SIGMA_FILTER)
         {
            /* Delta-Sigma filter by Siloh. */

            int old_accum_left, old_accum_right;

            old_accum_left = accum_left;
            accum_left = ((accum_left + accum_left + accum_left +
               prev_sample_left) >> 2);
            next_sample_left = accum_left;
            accum_left += (accum_left - old_accum_left);
            accum_left -= (int)ROUND(((rand () / ((float)RAND_MAX - 1)) *
               (accum_left * 0.01)));

            old_accum_right = accum_right;
            accum_right = ((accum_right + accum_right + accum_right +
               prev_sample_right) >> 2);
            next_sample_right = accum_right;
            accum_right += (accum_right - old_accum_right);
            accum_right -= (int)ROUND(((rand () / ((float)RAND_MAX - 1)) *
               (accum_right * 0.01)));
         }

         prev_sample_left = next_sample_left;
         prev_sample_right = next_sample_right;

         /* surround sound */
         switch (surround)
         {
            case 1:
            {
               accum_left = ((accum_left + accum_right) >> 1);
               accum_right = ~accum_left;

               break;
            }

            case 2:
            {
               accum_right = ~accum_right;

               break;
            }

            case 3:
            {
               /* thanks to kode54 */
               old_accum_left = accum_left;
   
               scrap1 = ((accum_left + accum_right) >> 1);
   
               scrap2 = (accum_right - scrap1);
               accum_left -= scrap2;
   
               accum_right -= (old_accum_left - scrap1);

               break;
            }
         }

         if (surround != 1)
         {
             /* stereo blending */
             old_accum_left = accum_left;
             old_accum_right = accum_right;
    
             accum_left += (old_accum_right >> 1);
             accum_right += (old_accum_left >> 1);
         }

         /* prevent clipping */
         if (accum_left > 0x7fffff)
            accum_left = 0x7fffff;
         else if (accum_left < -0x800000)
            accum_left = -0x800000;

         if (accum_right > 0x7fffff)
            accum_right = 0x7fffff;
         else if (accum_right < -0x800000)
            accum_right = -0x800000;
            
         /* reverse stereo */
         if (flip)
         {
            old_accum_left = accum_left;

            accum_left = accum_right;
            accum_right = old_accum_left;
         }

         if (audio_unsigned_samples)
         {
            accum_left ^= 0x800000;
            accum_right ^= 0x800000;
         }

         if (apu->sample_bits == 16)
         {
            UINT16 *buf = buffer;

            if (dither)
            {
               accum_left ^= ((accum_left & 0x000080) << 1);
               accum_right ^= ((accum_right & 0x000080) << 1);
            }

            *buf++ = (accum_left >> 8);
            *buf++ = (accum_right >> 8);

            buffer = buf;
         }
         else
         {
            UINT8 *buf = buffer;

            if (dither)
            {
               accum_left ^= ((accum_left & 0x008000) << 1);
               accum_right ^= ((accum_right & 0x008000) << 1);
            }

            *buf++ = (accum_left >> 16);
            *buf++ = (accum_right >> 16);

            buffer = buf;
         }
      }
   }
   else  /* buffer. */
   {
      while (num_samples--)
      {
         while ((!APU_QEMPTY()) &&
                (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
         {
            d = apu_dequeue ();
            apu_regwrite (d->address, d->value);
         }

         while ((!EX_QEMPTY()) &&
                (apu->ex_queue[apu->ex_q_tail].timestamp <= elapsed_cycles))
         {
            d = ex_dequeue ();

            if (apu->ex_chip & 1)
               VRC6SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 2)
               OPLLSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 4)
               FDSSoundWrite (d->address, d->value);
            else if (apu->ex_chip & 8)
               MMC5SoundWrite (d->address, d->value);
            else if (apu->ex_chip & 16)
            {
               if (d->address & 0x10000)
                  N106SoundRead (d->address & 0xffff);
               else
                  N106SoundWrite (d->address, d->value);
            }
            else if (apu->ex_chip & 32)
               PSGSoundWrite (d->address, d->value);
         }

         elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);
      }
   }

   /* resync cycle counter */
   apu->elapsed_cycles = cpu_get_cycles (FALSE);
}

/* set the filter type */
void apu_setfilterlist (int filter_list)
{
   RT_ASSERT(apu);

   apu->filter_list = filter_list;
}

// apu_reset_apus() added by T.Yano
void apu_reset_apus (APUSOUND *apus)
{
   int i;
   int mode;

   RT_ASSERT(apus);

   /* TODO: This could use a light clean-up and functionality check. */

   // Reset rectangles
   for (i = 0; i < 2; i++)
      memset (&apus->rectangle[i], 0, sizeof (apus->rectangle[i]));

   apus->rectangle[0].sweep_complement = TRUE;
   apus->rectangle[1].sweep_complement = FALSE;

   mode = apus->triangle.ideal_triangle;

   memset (&apus->triangle, 0, sizeof (apus->triangle));

   apus->triangle.ideal_triangle = mode;

   memset (&apus->noise, 0, sizeof (apus->noise));
   memset (&apus->dmc, 0, sizeof (apus->dmc));
}

void apu_reset (void)
{
   UINT32 address;

   RT_ASSERT(apu);

   /* TODO: This needs a huge clean-up and functionality check. */

   apu->elapsed_cycles = 0;

   memset (&apu->queue, 0, (APUQUEUE_SIZE * sizeof (apudata_t)));

   apu->q_head = apu->q_tail = 0;

   memset(&apu->ex_queue, 0, (APUQUEUE_SIZE * sizeof (apudata_t)));

   apu->ex_q_head = apu->ex_q_tail = 0;
   apu->ex_chip = 0;

   // added by T.Yano
   apu_reset_apus (&apu->apus);

   apu->enable_reg = 0;
   apu->enable_reg_cur = 0;

   /* use to avoid bugs =) */
   for (address = 0x4000; address <= 0x4013; address++)
   {
      apu_regwrite (address, 0);
      apu_write_cur (address, 0);
   }

   apu_regwrite (0x4015, 0);
   apu_write_cur (0x4015, 0);
                
   if (apu->ext)
      apu->ext->reset();

   // for ExSound
   LogTableInitialize ();

   FDSSoundReset ();
   FDSSoundVolume (1);

   PSGSoundReset ();
   PSGSoundVolume (1);

   N106SoundReset ();
   N106SoundVolume (1);

   VRC6SoundReset ();
   VRC6SoundVolume (1);

   OPLLSoundReset ();
   OPLLSoundVolume (1);

   MMC5SoundReset ();
   MMC5SoundVolume (1);

   // for $4017:bit7 by T.Yano
   apu_cnt_rate = 5;
}

void apu_build_luts (int num_samples)
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

static void apu_setactive (apu_t *active)
{
   RT_ASSERT(active);

   apu = active;
}

void apu_setparams (int sample_rate, float refresh_rate, int sample_bits)
{
   apu->sample_rate = sample_rate;
   apu->refresh_rate = refresh_rate;
   apu->sample_bits = sample_bits;

   apu->num_samples = (sample_rate / refresh_rate);

   /* TODO: Clean up the following line.  I'm not quite sure which part
      of it is calculation and which is the fixed point transformation, but
      Allegro's ftofix() macro should be used instead. */

   /* turn into fixed point! */
   apu->cycle_rate = (INT32)(APU_BASEFREQ * 65536.0 / sample_rate);

   /* build various lookup tables for apu */
   apu_build_luts (apu->num_samples);
}

void apu_setmode (int item, int mode)
{
   RT_ASSERT(apu);

   switch (item)
   {
      case APUMODE_IDEAL_TRIANGLE:
      {
         apu->apus.triangle.ideal_triangle = mode;

         break;
      }

      case APUMODE_SMOOTH_ENVELOPE:
      {
         apu->apus.rectangle[0].smooth_envelope = mode;
         apu->apus.rectangle[1].smooth_envelope = mode;

         break;
      }

      case APUMODE_SMOOTH_SWEEP:
      {
         apu->apus.rectangle[0].smooth_sweep = mode;
         apu->apus.rectangle[1].smooth_sweep = mode;

         break;
      }

      default:
         break;
   }
}

/* Initializes emulated sound hardware, creates waveforms/voices */
apu_t *apu_create (int sample_rate, float refresh_rate, int sample_bits)
{
   apu_t *temp_apu;
   int channel;

   temp_apu = malloc (sizeof (apu_t));
   if (!temp_apu)
      return (NULL);

   /* set the stupid flag to tell difference between two rectangles */
   temp_apu->apus.rectangle[0].sweep_complement = TRUE;
   temp_apu->apus.rectangle[1].sweep_complement = FALSE;

   /* set the update routine */
   temp_apu->process = apu_process;
   temp_apu->ext = NULL;

   apu_setactive (temp_apu);

   apu_setparams (sample_rate, refresh_rate, sample_bits);
   apu_reset ();

   for (channel = 0; channel < 6; channel++)
      apu_setchan (channel, TRUE);

   apu_setmode (APUMODE_IDEAL_TRIANGLE, FALSE);

   return (temp_apu);
}

void apu_destroy (apu_t **src_apu)
{
   if (*src_apu)
   {
      if ((*src_apu)->ext)
         (*src_apu)->ext->shutdown ();

      free (*src_apu);
   }
}

void apu_setext (apu_t *src_apu, apuext_t *ext)
{
   RT_ASSERT(src_apu);

   src_apu->ext = ext;

   /* initialize it */
   if (src_apu->ext)
      src_apu->ext->init ();
}

void sync_apu_register ()
{
   if ((!apu->apus.rectangle[0].holdnote_cur) &&
       (apu->apus.rectangle[0].vbl_length_cur > 0))
   {
      apu->apus.rectangle[0].vbl_length_cur -= apu_cnt_rate;
   }

   if ((!apu->apus.rectangle[1].holdnote_cur) &&
       (apu->apus.rectangle[1].vbl_length_cur > 0))
   {
      apu->apus.rectangle[1].vbl_length_cur -= apu_cnt_rate;
   }

   if (apu->apus.triangle.counter_started_cur)
   {
      if ((apu->apus.triangle.vbl_length_cur > 0) &&
          (!apu->apus.triangle.holdnote_cur))
      {
         apu->apus.triangle.vbl_length_cur -= apu_cnt_rate;
      }
   }

   if ((!apu->apus.noise.holdnote_cur) &&
       (apu->apus.noise.vbl_length_cur > 0))
   {
      apu->apus.noise.vbl_length_cur -= apu_cnt_rate;
   }
}

int sync_dmc_register (int cpu_cycles)
{
   int irq_occurred = FALSE;

   apu->apus.dmc.phaseacc_cur -= cpu_cycles;

   while (apu->apus.dmc.phaseacc_cur < 0)
   {
      apu->apus.dmc.phaseacc_cur += (apu->apus.dmc.freq_cur * 8);

      if (apu->apus.dmc.dma_length_cur)
      {
        if (--apu->apus.dmc.dma_length_cur == 0)
        {
           if (apu->apus.dmc.looping_cur)
           {
              apu->apus.dmc.dma_length_cur =
               apu->apus.dmc.cached_dmalength_cur;

              apu->apus.dmc.irq_occurred_cur = FALSE;
           }
           else
           {
              apu->apus.dmc.dma_length_cur = 0;

              if (apu->apus.dmc.irq_gen_cur)
              {
                 apu->apus.dmc.irq_occurred_cur = TRUE;
                 irq_occurred = TRUE;
              }

              apu->apus.dmc.enabled_cur = FALSE;
           }
        }
      }
   }

   return (irq_occurred);
}
