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

class APUEnvelope {
public:
   uint8 timer;               // save
   uint8 period;              // do not save
   uint8 counter;             // save
   bool fixed;                // do not save
   uint8 fixed_volume;        // do not save
   bool dirty;                // save
};

class APUSweep {
public:
   bool enabled;              // do not save
   uint8 timer;               // save
   uint8 period;              // do not save
   uint8 shifts;              // do not save
   bool invert;               // do not save
   bool increment;            // do not save
   bool dirty;                // save
};

class APUChannel {
public:
   // General.
   uint8 output;                 // save
   uint8 volume;                 // save for squares, noise, and dmc
   bool looping;                 // do not save
   bool silence;                 // save for squares, noise, and dmc

   // Timer.
   int16 timer;                  // save
   uint16 period;                // save for squares
};

class APUWaveformChannel : public APUChannel {
public:
   // Length counter (all except dmc).
   uint8 length;                 // save
   bool length_disable;          // do not save
};

class APUSquare : public APUWaveformChannel {
public:
   // Envelope generator (square/noise).
   APUEnvelope envelope;
   // Sweep unit (squares). */
   APUSweep sweep;

   // Sequencer (squares/triangle).
   uint8 sequence_step;          // save

   // Square.
   uint8 duty_cycle;             // do not save
};

class APUTriangle : public APUWaveformChannel {
public:
   // Sequencer (squares/triangle).
   uint8 sequence_step;          // save

   // Linear counter (triangle).
   uint8 linear_length;          // save
   bool halt_counter;            // save
   uint8 cached_linear_length;   // do not save
};

class APUNoise : public APUWaveformChannel {
public:
   // Envelope generator (square/noise).
   APUEnvelope envelope;

   // Noise.
   uint16 xor_tap;               // do not save
   uint16 shift16;               // save
};

class APUDMC : public APUChannel {
public:
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
};

class APU {
public:
   APUSquare square[2];
   APUTriangle triangle;
   APUNoise noise;
   APUDMC dmc;

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
};

#endif //!APU_INT_H_INCLUDED
