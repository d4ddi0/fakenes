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

#ifndef APU_INT_H_INCLUDED
#define APU_INT_H_INCLUDED
#include "common.h"
#include "core.h"
#include "types.h"

#define APU_REGS 24

// Maximum number of channels to send to the DSP (mono = 1, stereo = 2).
#define APU_MIXER_MAX_CHANNELS 2

typedef struct apu_envelope_s {
   uint8 timer;               // save
   uint8 period;              // do not save
   uint8 counter;             // save
   bool fixed;                // do not save
   uint8 fixed_volume;        // do not save
   bool dirty;                // save

} apu_envelope_t;

typedef struct apu_sweep_s {
   bool enabled;              // do not save
   uint8 timer;               // save
   uint8 period;              // do not save
   uint8 shifts;              // do not save
   bool invert;               // do not save
   bool increment;            // do not save
   bool dirty;                // save
             
} apu_sweep_t;

typedef struct apu_chan_s {
   // General.
   uint8 output;                 // save
   uint8 volume;                 // save for squares, noise, and dmc
   bool looping;                 // do not save
   bool silence;                 // save for squares, noise, and dmc

   // Timer.
   int16 timer;                  // save
   uint16 period;                // save for squares

   // Length counter (all except dmc).
   uint8 length;                 // save
   bool length_disable;          // do not save

   // Envelope generator (square/noise).
   apu_envelope_t envelope;

   // Sweep unit (squares). */
   apu_sweep_t sweep;

   // Sequencer (squares/triangle).
   uint8 sequence_step;          // save

   // Linear counter (triangle).
   uint8 linear_length;          // save
   bool halt_counter;            // save
   uint8 cached_linear_length;   // do not save

   // Square.
   uint8 duty_cycle;             // do not save

   // Noise.
   uint16 xor_tap;               // do not save
   uint16 shift16;               // save

   // DMC.
   bool enabled;                 // save
   uint16 address;               // save
   uint16 dma_length;            // save
   uint8 cur_byte;               // save
   uint8 sample_bits;            // save
   uint8 counter;                // save
   uint8 shift_reg;              // save
   bool irq_gen;                 // do not save
   bool irq_occurred;            // save
   uint16 cached_address;        // do not save
   uint16 cached_dmalength;      // do not save

} apu_chan_t;

typedef struct apu_s {
   apu_chan_t square[2];
   apu_chan_t triangle;
   apu_chan_t noise;
   apu_chan_t dmc;

   // Delta value for timers.
   cpu_time_t timer_delta;

   // Mixer.
   struct {
      int channels;
      cpu_time_t delta_cycles;
      real inputs[APU_MIXER_MAX_CHANNELS];
      real accumulators[APU_MIXER_MAX_CHANNELS];
      real sample_cache[APU_MIXER_MAX_CHANNELS];
      real filter[APU_MIXER_MAX_CHANNELS];
      real accumulated_samples;
      real max_samples;

   } mixer;

   // State.
   uint8 regs[APU_REGS];            // save

   // Timestamp of the last call to process().
   cpu_time_t clock_counter;

   // Frame sequencer & frame IRQs.
   int16 sequence_counter;          // save
   uint8 sequence_step;             // save
   uint8 sequence_steps;            // do not save
   bool frame_irq_gen;              // do not save
   bool frame_irq_occurred;         // save

   // IRQ prediction.
   cpu_time_t prediction_timestamp; // save
   cpu_time_t prediction_cycles;    // save

} apu_t;

#endif //!APU_INT_H_INCLUDED
