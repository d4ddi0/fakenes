/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Audio__APUAtoms_hpp__included
#define Audio__APUAtoms_hpp__included
#include "Local.hpp"

// Maximum number of channels to send to the DSP (mono = 1, stereo = 2).
static const enum_type APU_MIXER_MAX_CHANNELS = 2;

class APUEnvelope {
public:
   uint8 timer;
   uint8 period;
   uint8 counter;
   bool fixed;
   uint8 fixed_volume;
   bool dirty;
};

class APUSweep {
public:
   bool enabled;
   uint8 timer;
   uint8 period;
   uint8 shifts;
   bool invert;
   bool increment;               // do not save
   bool dirty;
};

class APUChannel {
public:
   // General.
   uint8 output;
   uint8 volume;                 // do not save for triangle
   bool looping;
   bool silence;                 // do not save for triangle

   // Timer.
   int16 timer;
   uint16 period;
};

class APUWaveformChannel : public APUChannel {
public:
   // Length counter (all except dmc).
   uint8 length;
   bool length_disable;
};

class APUSquare : public APUWaveformChannel {
public:
   // Envelope generator (square/noise).
   APUEnvelope envelope;
   // Sweep unit. */
   APUSweep sweep;

   // Sequencer (squares/triangle).
   uint8 sequence_step;

   // Square wave generator.
   uint8 duty_cycle;
};

class APUTriangle : public APUWaveformChannel {
public:
    // Sequencer (squares/triangle).
   uint8 sequence_step;

  // Linear counter.
   uint8 linear_length;
   bool halt_counter;
   uint8 cached_linear_length;
};

class APUNoise : public APUWaveformChannel {
public:
   // Envelope generator (squares/noise).
   APUEnvelope envelope;

   // Noise generator.
   uint16 xor_tap;
   uint16 shift16;
};

class APUDMC : public APUChannel {
public:
   // Memory reader.
   uint16 address;
   uint16 dma_length;
   uint16 cached_address;
   uint16 cached_dmalength;

   // Sample buffer.
   uint8 cur_byte;
   uint8 sample_bits;

   // Output unit.
   uint8 counter;
   uint8 shift_reg;

   // IRQ generator.
   bool irq_gen;
   bool irq_occurred;
};

// Mixer environment for the low pass filter.
typedef struct _APULPFilter {
   real filter_sample;

} APULPFilter;

// Mixer environment for the DC blocking filter.
typedef struct _APUDCFilter {
   real timer;
   real filter_sample;
   real next_filter_sample;
   real stepTime;
   real weightPerStep;
   real curStep;

} APUDCFilter;

class APU {
public:
   // State detection.
   int initializing;
   bool synchronizing;

   // Timestamp of the last call to process().
   cpu_time_t clock_counter;
   // Buffer to hold unused clocks to avoid losing cycles.
   cpu_time_t clock_buffer;

   // IRQ prediction.
   cpu_time_t prediction_timestamp;
   cpu_time_t prediction_cycles;

   // Frame sequencer & frame IRQs.
   int16 sequence_counter;
   uint8 sequence_step;
   uint8 sequence_steps;
   bool frame_irq_gen;
   bool frame_irq_occurred;

   // Sound generators.
   APUSquare square[2];
   APUTriangle triangle;
   APUNoise noise;
   APUDMC dmc;

   // Delta value for timers.
   cpu_time_t timer_delta;          // do not save

   // Mixer.
   struct {                         // do not save any of this
      int channels;
      cpu_time_t delta_cycles;
      real inputs[APU_MIXER_MAX_CHANNELS];
      real accumulators[APU_MIXER_MAX_CHANNELS];
      real sample_cache[APU_MIXER_MAX_CHANNELS];
      real accumulated_samples;
      real max_samples;

      // Filtering environments.
      APULPFilter lpEnv[APU_MIXER_MAX_CHANNELS];
      APUDCFilter dcEnv[APU_MIXER_MAX_CHANNELS];

   } mixer;
};

#endif // !Audio__APUAtoms_hpp__included
