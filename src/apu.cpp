/* FakeNES - A free, portable, Open Source NES emulator.

   apu.cpp: Implementation of the APU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include "apu.h"
#include "apu_int.h"
#include "audio.h"
#include "audio_int.h"
#include "common.h"
#include "core.h"
#include "cpu.h"
#include "debug.h"
#include "log.h"
#include "timing.h"
#include "types.h"

#include "sound/sound.hpp"
#include "sound/mmc5.hpp"
#include "sound/vrc6.hpp"

// TODO: DMC emulation rethinking and cleanup.
// TODO: DMC IRQ prediction(requires the above, otherwise it would be a mess to implement).

// Global options.
apu_options_t apu_options = {
   TRUE,                       // Enable processing
   APU_EMULATION_HIGH_QUALITY, // Emulation quality/performance tradeoff
   FALSE,                      // Stereo output mode
   FALSE,                      // Swap stereo channels
   1.0,                        // Global volume

   // Enable filters:
   FALSE, // Normalize output

   // Enable channels:
   TRUE, //    Square 1
   TRUE, //    Square 2
   TRUE, //    Triangle
   TRUE, //    Noise
   TRUE, //    DMC
   TRUE, //    Extra 1
   TRUE, //    Extra 2
   TRUE, //    Extra 3
};

// Static APU context.
static APU apu;

// ExSound contexts.
static Sound::Interface* exsound = null;
static Sound::MMC5::Interface MMC5;
static Sound::VRC6::Interface VRC6;

// Internal function prototypes (defined at bottom).
static void process(void);
static void mix(void);
static void enqueue(real&);

// Channel indices.
enum {
   APU_CHANNEL_SQUARE_1 = 0,
   APU_CHANNEL_SQUARE_2,
   APU_CHANNEL_TRIANGLE,
   APU_CHANNEL_NOISE,
   APU_CHANNEL_DMC,
   APU_CHANNEL_EXTRA_1,
   APU_CHANNEL_EXTRA_2,
   APU_CHANNEL_EXTRA_3,
   APU_CHANNELS,
};

// --- Lookup tables. ---
static const uint8 length_lut[32] = {
   0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A,
   0x0E, 0x0C, 0x1A, 0x0E, 0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
   0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

static const uint16 noise_period_lut_ntsc[16] = {
   0x004, 0x008, 0x010, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0CA, 0x0FE,
   0x17C, 0x1FC, 0x2FA, 0x3F8, 0x7F2, 0xFE4
};

static const uint16 noise_period_lut_pal[16] = {
   0x004, 0x007, 0x00E, 0x01E, 0x03C, 0x058, 0x076, 0x094, 0x0BC, 0x0EC,
   0x162, 0x1D8, 0x2C4, 0x3B0, 0x762, 0xEC2
};

static const uint16 dmc_period_lut_ntsc[16] = {
   0x1AC, 0x17C, 0x154, 0x140, 0x11E, 0x0FE, 0x0E2, 0x0D6, 0x0BE, 0x0A0,
   0x08E, 0x080, 0x06A, 0x054, 0x048, 0x036
};

static const uint16 dmc_period_lut_pal[16] = {
   0x18E, 0x162, 0x13C, 0x12A, 0x114, 0x0EC, 0x0D2, 0x0C6, 0x0B0, 0x094,
   0x084, 0x076, 0x062, 0x04E, 0x042, 0x032
};

/* Periods for the frame sequencer, 0=4step, 1=5step.  The periods are
   represented as delays from the current step to the next step.

   In 4-step mode, the first period is loaded immediately.  For 5-step mode,
   the first period is loaded after the sequencer has been clocked once. */
static const uint16 frame_sequencer_period_lut_ntsc[2][5] = {
   { 0x1D23, 0x1D20, 0x1D22, 0x1D22, 0x1D22 },
   { 0x1D22, 0x1D20, 0x1D22, 0x1D22, 0x1D1C }
};

static const uint16 frame_sequencer_period_lut_pal[2][5] = {
   { 0x207B, 0x207A, 0x2078, 0x207A, 0x207A },
   { 0x207A, 0x207A, 0x2078, 0x207A, 0x207A }
};

// Pulse sequences for each step 0-7 of each duty cycle 0-3 on the square wave channels.
static const uint8 square_duty_lut[4][8] = {
   { 0x0, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
   { 0x0, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0 },
   { 0x0, 0xF, 0xF, 0xF, 0xF, 0x0, 0x0, 0x0 },
   { 0xF, 0x0, 0x0, 0xF, 0xF, 0xF, 0xF, 0xF }
};

// Output sequence for each of the triangle's 32 steps.
static const uint8 triangle_lut[32] = {
   0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2,
   0x1, 0x0, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB,
   0xC, 0xD, 0xE, 0xF
};

// Mixer tables.
static real square_table[31];
static real tnd_table[203];

// Maximum Triangle+Noise+DMC output of the mixer - for prenormalization.
#define MAX_TND (163.67 / (24329.0 / (3 * 15 + 2 * 15 + 127) + 100))

// Update flags for sound generators.
enum {
   UPDATE_ENVELOPE = (1 << 0), // Update envelope generator.
   UPDATE_LINEAR   = (1 << 1), // Update linear [length] counter.
   UPDATE_LENGTH   = (1 << 2), // Update length counter.
   UPDATE_SWEEP    = (1 << 3), // Update sweep unit.
   UPDATE_OUTPUT   = (1 << 4), // Update output.

   // 4-step mode.
   UPDATE_120HZ = (UPDATE_LENGTH | UPDATE_SWEEP),
   UPDATE_240HZ = (UPDATE_ENVELOPE | UPDATE_LINEAR),

   // 5-step mode.
   UPDATE_96HZ = (UPDATE_LENGTH | UPDATE_SWEEP),
   UPDATE_192HZ = (UPDATE_ENVELOPE | UPDATE_LINEAR),
};

// --- Sound generators. ---

// Envelope generator for squares and noise
static void apu_envelope(APUChannel& chan, APUEnvelope& env)
{
   /* When clocked by the frame sequencer, one of two actions occurs: if there was a
      write to the fourth channel register since the last clock, the counter is set
      to 15 and the divider is reset, otherwise the divider is clocked. */
   if(env.dirty) {
      env.dirty = false;

      // Reset counter.
      env.counter = 0xF;
      // Reset timer.
      env.timer = 0;

      return;
   }

   if(env.timer > 0) {
      env.timer--;
      if(env.timer > 0)
         return;
   }

   env.timer += env.period;

   /* When the divider outputs a clock, one of two actions occurs: If the
      counter is non-zero, it is decremented, otherwise if the loop flag is
      set, the counter is loaded with 15. */
   if(env.counter > 0)
      env.counter--;
   else if(chan.looping)
      env.counter = 0xF;

   /* The envelope unit's volume output depends on the constant volume flag: if
      set, the envelope parameter directly sets the volume, otherwise the
      counter's value is the current volume. */
   chan.volume = (env.fixed ? env.fixed_volume : env.counter);
}

static void apu_save_envelope(APUEnvelope& env, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   pack_putc(env.timer, file);
   pack_putc(env.counter, file);
   pack_putc((env.dirty ? 1 : 0), file);
}

static void apu_load_envelope(APUEnvelope& env, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   env.timer = pack_getc(file);
   env.counter = pack_getc(file);
   env.dirty = true_or_false(pack_getc(file));
}

// Sweep unit for squares.
static linear void apu_sweep(APUChannel& chan, APUSweep& sweep)
{
   if(sweep.timer > 0) {
      sweep.timer--;
      if(sweep.timer > 0)
         return;
   }

   if(sweep.dirty) {
      sweep.dirty = false;

      // Reset timer.
      sweep.timer = 0;
   }
   else
      sweep.timer += sweep.period;

   if(chan.silence) {
      // Clear stale silence flag.
      chan.silence = false;
   }

   if(chan.period < 8) {
      // Inaudible.
      chan.silence = true;
      return;
   }

   int delta = (chan.period >> sweep.shifts);
   if(sweep.invert) {
      delta = ~delta;
      if(sweep.increment)
         delta++;
   }

   delta += chan.period;

   if((delta > 0x7FF) && !sweep.invert)
      chan.silence = true;
   else if(sweep.enabled && (sweep.shifts > 0))
      chan.period = delta;
}

// Length counter for squares, triangle, and noise
static void apu_update_length_counter(APUWaveformChannel& chan)
{
   if((chan.length > 0) && !chan.looping)
      chan.length--;
}

// Linear counter for triangle
static linear void apu_update_linear_counter(APUTriangle& chan)
{
   /* When clocked by the frame sequencer, the following actions occur in order:

          1) If halt flag is set, set counter to reload value, otherwise if counter
          is non-zero, decrement it.

          2) If control flag is clear, clear halt flag. */
   if(chan.halt_counter)
      chan.linear_length = chan.cached_linear_length;
   else if(chan.linear_length > 0)
      chan.linear_length--;

   if(chan.halt_counter && !chan.looping)
      chan.halt_counter = false;          
}

static linear void apu_update_square(APUSquare& chan, FLAGS update_flags)
{
   if(update_flags & UPDATE_ENVELOPE)
      apu_envelope(chan, chan.envelope);

   if(update_flags & UPDATE_SWEEP)
      apu_sweep(chan, chan.sweep);

   if(update_flags & UPDATE_LENGTH)
      apu_update_length_counter(chan);

   if(update_flags & UPDATE_OUTPUT) {
      if(chan.timer > 0) {
         chan.timer -= apu.timer_delta;
         if(chan.timer > 0)
            return;
      }

      /* The timer's period is the 12-bit value (%HHHL.LLLLLLL0) formed by
         timer high and timer low, plus two. */
      chan.timer += ((chan.period + 2) << 1);

      if((chan.length > 0) && !chan.silence)
         chan.output = (chan.volume & square_duty_lut[chan.duty_cycle][chan.sequence_step]);
      else
         chan.output = 0;

      if(++chan.sequence_step > 7)
         chan.sequence_step = 0;
   }
}

static linear void apu_save_square(APUSquare& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   pack_iputw(chan.timer, file);
   pack_iputw(chan.period, file);
   pack_putc(chan.length, file);
   pack_putc(chan.sequence_step, file);
   pack_putc(chan.volume, file);
   pack_putc((chan.silence ? 1 : 0), file);
   pack_putc(chan.output, file);

   // Envelope generator.
   apu_save_envelope(chan.envelope, file, version);
   // Sweep unit.
   pack_putc(chan.sweep.timer, file);
   pack_putc((chan.sweep.dirty ? 1 : 0), file);
}

static linear void apu_load_square(APUSquare& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   chan.timer = pack_igetw(file);
   chan.period = pack_igetw(file);
   chan.length = pack_getc(file);
   chan.sequence_step = pack_getc(file);
   chan.volume = pack_getc(file);
   chan.silence = true_or_false(pack_getc(file));
   chan.output = pack_getc(file);

   // Envelope generator.
   apu_load_envelope(chan.envelope, file, version);
   // Sweep unit.
   chan.sweep.timer = pack_getc(file);
   chan.sweep.dirty = true_or_false(pack_getc(file));
}

static linear void apu_update_triangle(APUTriangle& chan, FLAGS update_flags)
{
   if(update_flags & UPDATE_LENGTH)
      apu_update_length_counter(chan);

   if(update_flags & UPDATE_LINEAR)
      apu_update_linear_counter(chan);

   if(update_flags & UPDATE_OUTPUT) {
      if(chan.timer > 0) {
         chan.timer -= apu.timer_delta;
         if(chan.timer > 0)
            return;
      }

      /* The timer's period is the 11-bit value (%HHH.LLLLLLLL) formed by
         timer high and timer low, plus one. */
      chan.timer += (chan.period + 1);

      if((!(chan.length > 0)) ||
         (!(chan.linear_length > 0)))
         return;

      /* When the timer generates a clock and the Length Counter and Linear Counter both
         have a non-zero count, the sequencer is clocked. */
      if(chan.period < 2) {
         // Inaudible.
         chan.output = 0;
      }
      else
         chan.output = triangle_lut[chan.sequence_step];

      if(++chan.sequence_step > 31)
         chan.sequence_step = 0;
   }
}

static linear void apu_save_triangle(APUTriangle& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   pack_iputw(chan.timer, file);
   pack_putc(chan.length, file);
   pack_putc(chan.linear_length, file);
   pack_putc((chan.halt_counter ? 1 : 0), file);
   pack_putc(chan.sequence_step, file);
   pack_putc(chan.output, file);
}

static linear void apu_load_triangle(APUTriangle& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   chan.timer = pack_igetw(file);
   chan.length = pack_getc(file);
   chan.linear_length = pack_getc(file);
   chan.halt_counter = true_or_false(pack_getc(file));
   chan.sequence_step = pack_getc(file);
   chan.output = pack_getc(file);
}

static linear void apu_update_noise(APUNoise& chan, FLAGS update_flags)
{
   if(update_flags & UPDATE_ENVELOPE)
      apu_envelope(chan, chan.envelope);

   if(update_flags & UPDATE_LENGTH)
      apu_update_length_counter(chan);

   if(update_flags & UPDATE_OUTPUT) {
      if(chan.timer > 0) {
         chan.timer -= apu.timer_delta;
         if(chan.timer > 0)
            return;
      }

      chan.timer += chan.period;

      uint16 bit0 = (chan.shift16 & 0x01);
      const unsigned tap = ((chan.shift16 & chan.xor_tap) ? 1 : 0);
      const uint16 bit15 = (bit0 ^ tap);

      chan.shift16 |= (bit15 << 15);
      chan.shift16 >>= 1;

      bit0 = (chan.shift16 & 0x01);

      if(!bit0 &&
         ((chan.length > 0) && !chan.silence))
         chan.output = chan.volume;
      else
         chan.output = 0;
   }
}

static linear void apu_save_noise(APUNoise& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   pack_iputw(chan.timer, file);
   pack_putc(chan.length, file);
   pack_iputw(chan.shift16, file);
   pack_putc(chan.volume, file);
   pack_putc((chan.silence ? 1 : 0), file);
   pack_putc(chan.output, file);

   // Envelope generator.
   apu_save_envelope(chan.envelope, file, version);
}

static linear void apu_load_noise(APUNoise& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   chan.timer = pack_igetw(file);
   chan.length = pack_getc(file);
   chan.shift16 = pack_igetw(file);
   chan.volume = pack_getc(file);
   chan.silence = true_or_false(pack_getc(file));
   chan.output = pack_getc(file);

   // Envelope generator.
   apu_load_envelope(chan.envelope, file, version);
}

static void apu_reload_dmc(APUDMC& chan)
{
   chan.address = chan.cached_address;
   chan.dma_length = chan.cached_dmalength;
}

static linear void apu_update_dmc(APUDMC& chan)
{
   // --- Timer ---
   /* The DMC's timer has a selectable period that is loaded from an
      internal table during writes to $4010.

      The output from the divider is used for clocking both the counter and
      the shift register of the output unit.

      The DMA reader is supposedly cycle-aware, though according to the
      "unreliable behavior" section of Blargg's doc, may not always be. */
   if(chan.timer > 0) {
      chan.timer -= apu.timer_delta;
      if(chan.timer > 0)
         return;
   }

   chan.timer += chan.period;

   // --- DMA reader ---
   /* When the sample buffer is in an empty state and the bytes counter is
      non-zero... */
   if((chan.dma_length > 0) && (chan.sample_bits == 0)) {
      // Fill sample buffer.
      // DMCDMABuf=X6502_DMR(0x8000+DMCAddress);
      chan.cur_byte = cpu_read(0x8000 + chan.address);
      // Auto-increment address.
      // DMCAddress=(DMCAddress+1)&0x7FFF;
      chan.address = ((chan.address + 1) & 0x7FFF);
      // Mark sample buffer as filled.
      chan.sample_bits = 8;

      /* When the DMA reader accesses a byte of memory, the CPU is suspended
         for 4 clock cycles. */
      cpu_consume_cycles(4);

      if (--chan.dma_length == 0) {
         // if loop bit set, we're cool to retrigger sample
         if (chan.looping) {
            /* The bytes counter is decremented;
               if it becomes zero and the loop flag is set, the sample is restarted */
            apu_reload_dmc(chan);
         }
         else {
            // check to see if we should generate an irq
            if(chan.irq_gen && !chan.irq_occurred) {
               /* if the bytes counter becomes zero and the interrupt enabled
                  flag is set, the interrupt flag is set. */
               chan.irq_occurred = true;
               cpu_interrupt(CPU_INTERRUPT_IRQ_DMC);
            }

            /* The DMC seems to become disabled after the length counter
               reaches 0, regardless of whether or not new values are
               written to the length counter afterwards.  The channel must
               be restarted via a write to $4015 with bit 4 set.

               Fixes Crystalis, quite possibly other games.
               - randi */
            chan.enabled = false;
         }
      }
   }

   // --- Output unit ---
   if(chan.counter == 0) {
      // -- Start a new cycle.
      // Reload counter.
      chan.counter = 8;

      if(chan.sample_bits > 0) {
         // -- Sample buffer contains data.
        // Clear silence flag.
         chan.silence = false;

         // Empty sample buffer into the shift register.
         chan.shift_reg = chan.cur_byte;
         chan.cur_byte = 0;
         chan.sample_bits = 0;
      }
      else {
         // Set silence flag.
         chan.silence = true;
         // Clear output.
         chan.output = 0;
      }
   }

   if(!chan.silence) {
      /* If the silence flag is clear, bit 0 of the shift register is applied to
         the DAC counter: If bit 0 is clear and the counter is greater than 1, the
         counter is decremented by 2, otherwise if bit 0 is set and the counter is less
         than 126, the counter is incremented by 2. */
      const bool bit0 = true_or_false(chan.shift_reg & 1);

      if(!bit0 && (chan.volume > 1)) {
         // positive delta
         chan.volume -= 2;
      }
      else if(bit0 && (chan.volume < 126)) {
         // negative delta
         chan.volume += 2;
      }

      chan.output = (chan.volume & 0x7F);
   }

   // Clock shift register.
   chan.shift_reg >>= 1;
   // Decrement counter.
   chan.counter--;
}

static linear void apu_save_dmc(APUDMC& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   pack_putc((chan.enabled ? 1 : 0), file);
   pack_iputw(chan.timer, file);
   pack_iputw(chan.address, file);
   pack_iputw(chan.dma_length, file);
   pack_putc(chan.cur_byte, file);
   pack_putc(chan.sample_bits, file);
   pack_putc(chan.counter, file);
   pack_putc(chan.shift_reg, file);
   pack_putc(chan.volume, file);
   pack_putc((chan.silence ? 1 : 0), file);
   pack_putc(chan.output, file);
   pack_putc((chan.irq_occurred ? 1 : 0), file);
}

static linear void apu_load_dmc(APUDMC& chan, PACKFILE* file, int version)
{
   RT_ASSERT(file);

   chan.enabled = true_or_false(pack_getc(file));
   chan.timer = pack_igetw(file);
   chan.address = pack_igetw(file);
   chan.dma_length = pack_igetw(file);
   chan.cur_byte = pack_getc(file);
   chan.sample_bits = pack_getc(file);
   chan.counter = pack_getc(file);
   chan.shift_reg = pack_getc(file);
   chan.volume = pack_getc(file);
   chan.silence = true_or_false(pack_getc(file));
   chan.output = pack_getc(file);
   chan.irq_occurred = true_or_false(pack_getc(file));
}

static void apu_update_channels(FLAGS update_flags)
{
   if(apu_options.enable_square_1)
      apu_update_square(apu.square[0], update_flags);
   if(apu_options.enable_square_2)
      apu_update_square(apu.square[1], update_flags);

   if(apu_options.enable_triangle)
      apu_update_triangle(apu.triangle, update_flags);

   if(apu_options.enable_noise)
      apu_update_noise(apu.noise, update_flags);

   if(apu_options.enable_dmc &&
      (update_flags & UPDATE_OUTPUT))
      apu_update_dmc(apu.dmc);
}

static void apu_reload_sequence_counter(void)
{
   const int mode = ((apu.sequence_steps == 5) ? 1 : 0);

   if (machine_type == MACHINE_TYPE_NTSC)
      apu.sequence_counter += frame_sequencer_period_lut_ntsc[mode][(apu.sequence_step - 1)];
   else
      apu.sequence_counter += frame_sequencer_period_lut_pal[mode][(apu.sequence_step - 1)];
}

static void apu_update_frame_sequencer(void)
{
   if(apu.sequence_counter > 0) {
      apu.sequence_counter -= apu.timer_delta;
      if(apu.sequence_counter > 0)
         return;
   }

   apu_reload_sequence_counter();

   /* Frame sequencer operation:
      4-step mode:
         Envelope units and triangle length counter update at ~240Hz.
         Sweep units and length counters update at ~120Hz.
      5-step mode:
         Envelope units and triangle length counter update at ~192Hz.
         Sweep units and length counters update at ~96Hz. */
   switch(apu.sequence_step) {
      case 1:
      case 3: {
         /* Frame sequencer steps 1 and 3
            4-step mode: Updates envelope units and triangle length counter only
            5-step mode: Updates everything */
         if (apu.sequence_steps == 5)
            apu_update_channels(UPDATE_192HZ | UPDATE_96HZ);
         else
            apu_update_channels(UPDATE_240HZ);

         break;
      }

      case 2:
      case 4: {
         /* Frame sequencer steps 2 and 4
            4-step mode: Updates everything
            5-step mode: Updates envelope units and triangle length counter only */
         if (apu.sequence_steps == 5)
            apu_update_channels(UPDATE_192HZ);
         else
            apu_update_channels(UPDATE_240HZ | UPDATE_120HZ);

         break;
      }

      case 5: {
         /* Frame sequencer step 5 (5-step mode only)
            Nothing updated. */
         break;
      }

      default:
         WARN_GENERIC();
   }

   /* check to see if we should generate an irq
      Note that IRQs are not generated in 5step mode */
   if((apu.sequence_steps == 4) && 
      (apu.sequence_step == 4) &&
      (apu.frame_irq_gen && !apu.frame_irq_occurred)) {
      apu.frame_irq_occurred = true;
      // This part is now handled by apu_predict_frame_irq() instead
      // cpu_interrupt (CPU_INTERRUPT_IRQ_FRAME);
   }

   if(++apu.sequence_step > apu.sequence_steps)
      apu.sequence_step = 1;
}

static void apu_reset_frame_sequencer(void)
{
   // Reset sequencer.
   apu.sequence_counter = 0;
   apu.sequence_step = 1;

   // Clear frame IRQ.
   apu.frame_irq_occurred = false;
   cpu_clear_interrupt(CPU_INTERRUPT_IRQ_FRAME);

   /* If the mode flag is clear, the 4-step sequence is selected, otherwise the
      5-step sequence is selected and the sequencer is immediately clocked once. */
   if(apu.sequence_steps == 5) {
      // -- 5-step mode
      /* Setting this to 1 will cause it to become 0 at the beginning of the next APU cycle, clocking the frame sequencer
         and reloading the counter for us (see apu_update_frame_sequencer() above) - so no need to do that here. */
      apu.sequence_counter = 1;  // update on next CPU cycle
   }
   else {
      // -- 4-step mode
      // Otherwise, we skip clocking the frame sequencer and just reload the counter manually.
      apu_reload_sequence_counter();
   }
}

static linear void apu_predict_frame_irq(cpu_time_t cycles)
{
   /* This function predicts when the APU's frame IRQ will occur and queues
      it in the CPU core to trigger as close to that moment as possible.

      We must be very careful to back up any variables we modify in this
      function, since we don't want to affect the APU's actual state - only
      get a rough(more accurate than not) idea of when the IRQ will occur. */
   if(!apu.frame_irq_gen)
      return;

   const int16 saved_sequence_counter = apu.sequence_counter;
   const uint8 saved_sequence_step = apu.sequence_step;

   /* Since all CPU core cycle counts are specified in PPU cycles, we need
      to convert those counts to a format that the APU can use... */
   cycles /= CYCLE_LENGTH;

   /* Now we simply simulate emulating the frame sequencer cycle-by-cycle
      (up to a minimum and maximum of 'cycles') keeping track of what
      virtual cycle we are on for calls to cpu_queue_interrupt(). */
   for(cpu_time_t offset = 0; offset < cycles; offset++) {
      if(apu.sequence_counter > 0) {
         apu.sequence_counter--;
         if(apu.sequence_counter > 0)
            continue;
      }

      apu_reload_sequence_counter();

      /* check to see if we should generate an irq
         Note that IRQs are not generated in 5step mode */
      if((apu.sequence_steps == 4) &&
         (apu.sequence_step == 4)) {
         /* TODO: Unqueue the interrupt if the ability to generate frame IRQs is lost (either via the disable bit in the 
            control register, or switching to 5step mode).  Currently, the interrupt is(incorrectly) left in the queue. */
         cpu_queue_interrupt(CPU_INTERRUPT_IRQ_FRAME, (apu.prediction_timestamp + (offset * CYCLE_LENGTH)));
      }

      if(++apu.sequence_step > apu.sequence_steps)
         apu.sequence_step = 1;
   }

   apu.sequence_counter = saved_sequence_counter;
   apu.sequence_step = saved_sequence_step;
}

static void apu_repredict_frame_irq(void)
{
   /* Normally, the IRQ predictor is only called once per scanline.

      This function repredicts the frame IRQ when a mid-scanline change
      occurs (such as the frame sequencer being reset).

      This is probably not needed (since a frame IRQ cannot occur so
      suddenly after a reset?) but we emulate it anyway "just in case". */
   if(!apu.frame_irq_gen)
      return;

   const cpu_time_t cycles = cpu_get_cycles();

   cpu_time_t cycles_remaining = (cycles - apu.prediction_timestamp);
   if(cycles_remaining <= 0)
      return;

   if (cycles_remaining > apu.prediction_cycles)
      cycles_remaining = apu.prediction_cycles;

   apu_predict_frame_irq(cycles_remaining);
}

void apu_load_config(void)
{
   /* Like other components, the APU is both an interface and an emulation.
      However, apu_init() and apu_exit() should only be called during
      emulation (e.g, when a ROM is loaded/unloaded).  To load the
      configuration, uses these two functions instead. */

   // Load configuration.
   apu_options.enabled       = TRUE_OR_FALSE(get_config_int("apu", "enabled", apu_options.enabled));
   apu_options.emulation     = get_config_int("apu", "emulation", apu_options.emulation);
   apu_options.stereo        = TRUE_OR_FALSE(get_config_int("apu", "stereo", apu_options.stereo));
   apu_options.swap_channels = TRUE_OR_FALSE(get_config_int("apu", "swap_channels", apu_options.swap_channels));
   apu_options.volume        = get_config_float("apu", "volume", apu_options.volume);

   apu_options.normalize = TRUE_OR_FALSE(get_config_int("apu", "normalize", apu_options.normalize));

   apu_options.enable_square_1 = TRUE_OR_FALSE(get_config_int("apu", "enable_square_1", apu_options.enable_square_1));
   apu_options.enable_square_2 = TRUE_OR_FALSE(get_config_int("apu", "enable_square_2", apu_options.enable_square_2));
   apu_options.enable_triangle = TRUE_OR_FALSE(get_config_int("apu", "enable_triangle", apu_options.enable_triangle));
   apu_options.enable_noise    = TRUE_OR_FALSE(get_config_int("apu", "enable_noise",    apu_options.enable_noise));
   apu_options.enable_dmc      = TRUE_OR_FALSE(get_config_int("apu", "enable_dmc",      apu_options.enable_dmc));
   apu_options.enable_extra_1  = TRUE_OR_FALSE(get_config_int("apu", "enable_extra_1",  apu_options.enable_extra_1));
   apu_options.enable_extra_2  = TRUE_OR_FALSE(get_config_int("apu", "enable_extra_2",  apu_options.enable_extra_2));
   apu_options.enable_extra_3  = TRUE_OR_FALSE(get_config_int("apu", "enable_extra_3",  apu_options.enable_extra_3));

   // Build mixer tables.
   for(int n = 0; n < 31; n++)
      square_table[n] = 95.52 / (8128.0 / n + 100);
   for(int n = 0; n < 203; n++)
      tnd_table[n] = 163.67 / (24329.0 / n + 100);

   // Disable ExSound.
   apu_set_exsound(APU_EXSOUND_NONE);
}

void apu_save_config(void)
{
   // Save configuration.
   set_config_int("apu", "enabled", (apu_options.enabled ? 1 : 0));
   set_config_int("apu", "emulation", apu_options.emulation);
   set_config_int("apu", "stereo", (apu_options.stereo ? 1 : 0));
   set_config_int("apu", "swap_channels", (apu_options.swap_channels ? 1 : 0));
   set_config_float("apu", "volume", apu_options.volume);

   set_config_int("apu", "normalize", (apu_options.normalize ? 1 : 0));

   set_config_int("apu", "enable_square_1", (apu_options.enable_square_1 ? 1 : 0));
   set_config_int("apu", "enable_square_2", (apu_options.enable_square_2 ? 1 : 0));
   set_config_int("apu", "enable_triangle", (apu_options.enable_triangle ? 1 : 0));
   set_config_int("apu", "enable_noise",    (apu_options.enable_noise    ? 1 : 0));
   set_config_int("apu", "enable_dmc",      (apu_options.enable_dmc      ? 1 : 0));
   set_config_int("apu", "enable_extra_1",  (apu_options.enable_extra_1  ? 1 : 0));
   set_config_int("apu", "enable_extra_2",  (apu_options.enable_extra_2  ? 1 : 0));
   set_config_int("apu", "enable_extra_3",  (apu_options.enable_extra_3  ? 1 : 0));
}

int apu_init(void)
{
   // Seems to be the default...
   apu.sequence_steps = 4;

   // Reset APU.
   apu_reset();
   // Reset frame sequencer.
   apu_reset_frame_sequencer();

   // Return success.
   return 0;
}

void apu_exit(void)
{
   // Do nothing.
}

void apu_reset(void)
{
   // -- Initializes/resets emulated sound hardware, creates waveforms/voices

   /* Save frame sequencer state, since it is not cleared by a soft reset (a
      hard reset always implies a call to apu_init(), which clears it). */
   const int16 sequence_counter = apu.sequence_counter;
   const uint8 sequence_step = apu.sequence_step;

   // Clear context.
   memset(&apu, 0, sizeof(apu));

   // set the stupid flag to tell difference between two squares
   apu.square[0].sweep.increment = false;
   apu.square[1].sweep.increment = true;

   /* On power-up, the shift register is loaded with the value 1. */
   apu.noise.shift16 = 1;

   // Clear all registers.
   for (uint16 address = 0x4000; address <= 0x4017; address++)
      apu_write(address, 0x00);

   // Restore frame sequencer state.
   apu.sequence_counter = sequence_counter;
   apu.sequence_step = sequence_step;

   // Reset ExSound.
   if(exsound)
      exsound->reset();

   // Initialize everything else.
   apu_update();
}

void apu_update(void)
{
   /* Updates the APU to external changes without resetting it, since that
      might cause problems in a currently running game. */

   /* Number of samples to be held in the APU mixer accumulators before
      being divided and sent to the DSP.

      This should be directly synchronized with the code execution rate to
      avoid overflowing the sample buffer. */
   apu.mixer.max_samples = ((timing_get_frequency() / CYCLE_LENGTH) / audio_sample_rate);

   // Determine number of channels to mix.
   apu.mixer.channels = (apu_options.stereo ? 2 : 1);
}

void apu_set_exsound(ENUM type)
{
   switch(type) {
      case APU_EXSOUND_NONE: {
         exsound = null;
         break;
      }

      case APU_EXSOUND_MMC5: {
         exsound = &MMC5;
         break;
      }

      case APU_EXSOUND_VRC6: {
         exsound = &VRC6;
         break;
      }

      default:
         WARN_GENERIC();
   }

   // Reset it just in case.
   if(exsound)
      exsound->reset();
}

UINT8 apu_read(UINT16 address)
{
   uint8 value = 0x00;

   // Sync state.
   process();

   switch(address) {
      case 0x4015: {
         /* Status register.
            $4015   if-d nt21   DMC IRQ, frame IRQ, length counter statuses */

         // Return 1 in 0-5 bit pos if a channel is playing

         // Square 0.
         if(apu.square[0].length > 0)
            value |= 0x01;
         // Square 1.
         if(apu.square[1].length > 0)
            value |= 0x02;

         // Triangle.
         if(apu.triangle.length > 0)
            value |= 0x04;

         // Noise.
         if(apu.noise.length > 0)
            value |= 0x08;

         // DMC.
         if(apu.dmc.enabled & (apu.dmc.dma_length > 0))
            value |= 0x10;
         if(apu.dmc.irq_occurred)
            value |= 0x80;

         // Frame IRQ.
         if(apu.frame_irq_occurred)
            value |= 0x40;

         // kev says reads from $4015 reset the frame counter, so...
         // Reset frame sequencer.
         apu_reset_frame_sequencer();
         apu_repredict_frame_irq();

         break;
      }

      default:
         break;
   }

   if(exsound)
      value |= exsound->read(address);

   return value;
}

void apu_write(UINT16 address, UINT8 value)
{
   // Sync state.
   process();

   switch(address) {
      case 0x4000:
      case 0x4004: {
         /* Square Wave channels, register set 1.
            $4000/4 ddle nnnn   duty, loop env/disable length, env disable, vol/env
            period */

         /* Determine which channel to use.
            $4000 - Square wave 1(0)
            $4004 - Square wave 2(1) */
         const int index = ((address & 4) ? 1 : 0);
         APUSquare& chan = apu.square[index];

         chan.volume = (value & 0x0F);
         chan.looping = true_or_false(value & 0x20);
         chan.duty_cycle = (value >> 6);

         // The divider's period is set to n + 1.
         chan.envelope.period = ((value & 0x0F) + 1);
         chan.envelope.fixed = true_or_false(value & 0x10);
         chan.envelope.fixed_volume = (value & 0x0F);

         break;
      }

      case 0x4001:
      case 0x4005: {
         /* Square Wave channels, register set 2.
            $4001/5 eppp nsss   enable sweep, period, negative, shift */
         const int index = ((address & 4) ? 1 : 0);
         APUSquare& chan = apu.square[index];

         // The divider's period is set to p + 1.
         chan.sweep.enabled = true_or_false(value & 0x80);
         chan.sweep.period = (((value >> 4) & 7) + 1);
         chan.sweep.shifts = (value & 7);
         chan.sweep.invert = true_or_false(value & 0x08);

         // Reset the sweep unit.
         chan.sweep.dirty = true;

         break;
      }

      case 0x4002:
      case 0x4006: {
         /* Square Wave channels, register set 3.
            $4002/6 pppp pppp   period low */
         const int index = ((address & 4) ? 1 : 0);
         APUSquare& chan = apu.square[index];

         chan.period = ((chan.period & ~0xFF) | value);

         break;
      }

      case 0x4003:
      case 0x4007: {
         /* Square Wave channels, register set 4.
            $4003/7 llll lppp   length index, period high */
         const int index = ((address & 4) ? 1 : 0);
         APUSquare& chan = apu.square[index];

         chan.period = (((value & 7) << 8) | (chan.period & 0xFF));

         if(!chan.length_disable)
            chan.length = length_lut[(value >> 3)];

         // Reset the envelope generator.
         chan.envelope.dirty = true;
         // When the fourth register is written to, the sequencer is restarted.
         chan.sequence_step = 0;

         break;
      }

      case 0x4008: {
         /* Triangle wave channel, register 1.
            $4008     CRRR.RRRR   Linear counter setup (write)
            bit 7     C---.----   Control flag (this bit is also the length counter halt flag)
            bits 6-0  -RRR RRRR   Counter reload value */
         APUTriangle& chan = apu.triangle;

         /* TODO: Are writes to this register really supposed to be
            affecting the linear counter immediately...? */
         chan.linear_length = (value & 0x7F);
         chan.looping = true_or_false(value & 0x80);

         chan.cached_linear_length = chan.linear_length;

         break;
      }

      case 0x400A: {
         /* Triangle wave channel, register 2.
            $400A   pppp pppp   period low */
         APUTriangle& chan = apu.triangle;
         chan.period = ((chan.period & ~0xFF) | value);

         break;
      }

      case 0x400B: {
         /* Triangle wave channel, register 3.
            $400A   pppp pppp   period low */
         APUTriangle& chan = apu.triangle;

         chan.period = (((value & 7) << 8) | (chan.period & 0xFF));

         if (!chan.length_disable)
            chan.length = length_lut[(value >> 3)];

         // When register $400B is written to, the halt flag is set.
         chan.halt_counter = true;

         break;
      }

      case 0x400C: {
         /* White noise channel, register 1.
            $400C   --le nnnn   loop env/disable length, env disable, vol/env period */
         APUNoise& chan = apu.noise;

         chan.volume = (value & 0x0F);
         chan.looping = true_or_false(value & 0x20);

         chan.envelope.period = ((value & 0x0F) + 1);
         chan.envelope.fixed = true_or_false(value & 0x10);
         chan.envelope.fixed_volume = (value & 0x0F);

         break;
      }

      case 0x400E: {
         /* White noise channel, register 2.
            $400E   s--- pppp   short mode, period index */
         APUNoise& chan = apu.noise;

         if(machine_type == MACHINE_TYPE_NTSC)
            chan.period = noise_period_lut_ntsc[value & 0x0F];
         else
            chan.period = noise_period_lut_pal[value & 0x0F];

         /* Bit 15 of the shift register is replaced with the exclusive-OR of
            bit 0 and one other bit: bit 6 if loop is set, otherwise bit 1.
            Note: xor_tap = loop, 0x40 = bit 6, 0x02 = bit 1. */
         chan.xor_tap = ((value & 0x80) ? 0x40 : 0x02);

         break;
      }

      case 0x400F: {
         // White noise channel, register 3.
         // $400F   llll l---   length index
         APUNoise& chan = apu.noise;

         if (!chan.length_disable)
            chan.length = length_lut[(value >> 3)];

         // Reset the envelope generator.
         chan.envelope.dirty = true;

         break;
      }

      case 0x4010: {
         /* Delta modulation channel, register 1.
            $4010   il-- ffff   IRQ enable, loop, frequency index */
         APUDMC& chan = apu.dmc;

         if(machine_type == MACHINE_TYPE_NTSC)
            chan.period = dmc_period_lut_ntsc[value & 0x0F];
         else
            chan.period = dmc_period_lut_pal[value & 0x0F];

         chan.looping = true_or_false(value & 0x40);
         chan.irq_gen = true_or_false(value & 0x80);

         if(!chan.irq_gen) {
            // Clear interrupt.
            chan.irq_occurred = false;
            cpu_clear_interrupt(CPU_INTERRUPT_IRQ_DMC);
         }

         break;
      }

      case 0x4011: {
         /* Delta modulation channel, register 2.
            $4011   -ddd dddd   DAC */
         APUDMC& chan = apu.dmc;

         // Mask off MSB.
         value &= 0x7F;
         // Overwrite current DAC value.
         chan.volume = value;
         chan.output = value;

         break;
      }

      case 0x4012: {
         /* Delta modulation channel, register 3.
            $4012   aaaa aaaa   sample address */
         APUDMC& chan = apu.dmc;

         // DMCAddress=0x4000+(DMCAddressLatch<<6);
         chan.cached_address = (0x4000 + (value << 6));

         if(chan.dma_length == 0)
            apu_reload_dmc(chan);

         break;
      }

      case 0x4013: {
         /* Delta modulation channel, register 4.
            $4013   llll llll   sample length */
         APUDMC& chan = apu.dmc;

         // DMCSize=(DMCSizeLatch<<4)+1;
         chan.cached_dmalength = ((value << 4) + 1);

         // Check for a reload.
         if(chan.dma_length == 0)
            apu_reload_dmc(chan);

         break;
      }

      case 0x4015: {
         /* Common register set 1.
            $4015   ---d nt21   length ctr enable: DMC, noise, triangle, pulse 2, 1 */

         // Squares.
         for(int index = 0; index < 2; index++) {
            APUSquare& chan = apu.square[index];

            chan.length_disable = true_or_false(!(value & (1 << index)));
            if(chan.length_disable)
               chan.length = 0;
         }

         // Triangle.
         apu.triangle.length_disable = true_or_false(!(value & 0x04));
         if(apu.triangle.length_disable)
            apu.triangle.length = 0;

         // Noise.
         apu.noise.length_disable = true_or_false(!(value & 0x08));
         if (apu.noise.length_disable)
            apu.noise.length = 0;

         // DMC.
         /* ---d xxxx

            If d is set and the DMC's DMA reader has no more sample bytes to fetch, the DMC
            sample is restarted. If d is clear then the DMA reader's sample bytes remaining
            is set to 0. */
         apu.dmc.enabled = true_or_false(value & 0x10);
         if(apu.dmc.enabled) {
            // Channel is enabled - check for a reload.
            if(apu.dmc.dma_length == 0)
               apu_reload_dmc(apu.dmc);
         }
         else {
            // Channel is disabled.
            apu.dmc.dma_length = 0;
         }

         /* When $4015 is written to, the channels' length counter enable flags are set,
            the DMC is possibly started or stopped, and the DMC's IRQ occurred flag is
            cleared. */
         apu.dmc.irq_occurred = false;
         cpu_clear_interrupt(CPU_INTERRUPT_IRQ_DMC);

         break;
      }

      case 0x4017: {
         /* Common register set 2.
            $4017   fd-- ----   5-frame cycle, disable frame interrupt */

         apu.sequence_steps = ((value & 0x80) ? 5 : 4);

         /* <_Q> setting $4017.6 or $4017.7 will turn off frame IRQs
            <_Q> setting $4017.7 puts it into the 5-step sequence
            <_Q> which does not generate interrupts */
         apu.frame_irq_gen = true_or_false(!(value & 0x40));

         // Reset frame sequencer.
         apu_reset_frame_sequencer();
         apu_repredict_frame_irq();

         break;
      }

      default:
         break;
   }

   // Cache register writes for state saving.
   if((address >= 0x4000) && (address <= 0x4017))
      apu.regs[(address - 0x4000)] = value;

   if(exsound)
      exsound->write(address, value);
}

void apu_predict_irqs(cpu_time_t cycles)
{
   // Save parameters for re-prediction if a mid-scanline change occurs.
   apu.prediction_timestamp = cpu_get_cycles();
   apu.prediction_cycles = cycles;

   // Sync state.
   process();

   apu_predict_frame_irq(cycles);
}

void apu_save_state(PACKFILE* file, int version)
{
   RT_ASSERT(file);

   // Sync state.
   process();

   // Save registers.
   for(int index = 0; index < APU_REGS; index++)
      pack_putc(apu.regs[index], file);

   // Save processing timestamp.
   pack_iputl(apu.clock_counter, file);

   // Save frame sequencer state.
   pack_iputw(apu.sequence_counter, file);
   pack_putc(apu.sequence_step, file);

   // Save IRQ prediction state.
   pack_iputl(apu.prediction_timestamp, file);
   pack_iputl(apu.prediction_cycles, file);

   // Save channel states.
   apu_save_square(apu.square[0], file, version);
   apu_save_square(apu.square[1], file, version);
   apu_save_triangle(apu.triangle, file, version);
   apu_save_noise(apu.noise, file, version);
   apu_save_dmc(apu.dmc, file, version);

   // Save ExSound state.
   if(exsound)
      exsound->save(file, version);
}

void apu_load_state(PACKFILE* file, int version)
{
   RT_ASSERT(file);

   // Load registers.
   for(int index = 0; index < APU_REGS; index++)
      apu_write((0x4000 + index), pack_getc (file));

   // Load processing timestamp.
   apu.clock_counter = pack_igetl(file);

   // Load frame sequencer state.
   apu.sequence_counter = pack_igetw(file);
   apu.sequence_step = pack_getc(file);

   // Load IRQ prediction state.
   apu.prediction_timestamp = pack_igetl(file);
   apu.prediction_cycles = pack_igetl(file);

   // Load channel states.
   apu_load_square(apu.square[0], file, version);
   apu_load_square(apu.square[1], file, version);
   apu_load_triangle(apu.triangle, file, version);
   apu_load_noise(apu.noise, file, version);
   apu_load_dmc(apu.dmc, file, version);

   // Load ExSound state.
   if(exsound)
      exsound->load(file, version);
}

// --- Internal functions. --- 
static void process(void)
{
   if(!apu_options.enabled) {
      // APU emulation is disabled - skip processing.
      return;
   }

   // Calculate the delta period.
   const cpu_time_t elapsed_cycles = (cpu_get_elapsed_cycles(&apu.clock_counter) / CYCLE_LENGTH);
   if(elapsed_cycles == 0) {
      // Nothing to do. 
      return;
   }

   switch(apu_options.emulation) {
      case APU_EMULATION_FAST: {
         // -- Faster, inaccurate version of the mixer.

         for(cpu_time_t count = 0; count < elapsed_cycles; count++) {
            // Buffer a cycle.
            apu.mixer.delta_cycles++;

            // Simulate accumulation.
            apu.mixer.accumulated_samples++;
            if(apu.mixer.accumulated_samples >= apu.mixer.max_samples) {
               // Set the timer delta to the # of cycles elapsed.
               apu.timer_delta = apu.mixer.delta_cycles;
               // Clear the cycle buffer.
               apu.mixer.delta_cycles = 0;

               // Update the frame sequencer.
               apu_update_frame_sequencer();
               // Update outputs.
               apu_update_channels(UPDATE_OUTPUT);

               // Update ExSound.
               if(exsound)
                  exsound->process(apu.timer_delta);

               // Mix outputs together.
               mix();

               // Fetch and buffer samples.
               for(int channel = 0; channel < apu.mixer.channels; channel++) {
                  // Fetch sample.
                  real sample = apu.mixer.inputs[channel];

                  // Cache sample for filter.
                  const real cached_sample = sample;
                  // Filter sample.
                  sample = ((sample + apu.mixer.filter[channel]) / 2.0);
                  // Update filter sample.
                  apu.mixer.filter[channel] = cached_sample;

                  // Send it to the audio queue.
                  enqueue(sample);
               }

               // Adjust counter.
               apu.mixer.accumulated_samples -= apu.mixer.max_samples;
            }
         }

         break;
      }

      case APU_EMULATION_ACCURATE: {
         // Since we'll be emulating every cycle, we'll use a timer delta of 1.
         apu.timer_delta = 1;

         for(cpu_time_t count = 0; count < elapsed_cycles; count++) {
            // Update the frame sequencer.
            apu_update_frame_sequencer();
            // ~1.79MHz update driven independantly of the frame sequencer.
            apu_update_channels(UPDATE_OUTPUT);

            // Update ExSound.
            if(exsound)
               exsound->process(apu.timer_delta);

            // Simulate accumulation.
            apu.mixer.accumulated_samples++;
            if(apu.mixer.accumulated_samples >= apu.mixer.max_samples) {
               // Mix outputs together.
               mix();

               // Fetch and buffer samples.
               for(int channel = 0; channel < apu.mixer.channels; channel++) {
                  // Fetch sample.
                  real sample = apu.mixer.inputs[channel];

                  // Cache sample for filter.
                  const real cached_sample = sample;
                  // Filter sample.
                  sample = ((sample + apu.mixer.filter[channel]) / 2.0);
                  // Update filter sample.
                  apu.mixer.filter[channel] = cached_sample;

                  // Send it to the audio queue.
                  enqueue(sample);
               }

               // Adjust counter.
               apu.mixer.accumulated_samples -= apu.mixer.max_samples;
            }
         }

         break;
      }

      case APU_EMULATION_HIGH_QUALITY: {
         // Since we'll be emulating every cycle, we'll use a timer delta of 1.
         apu.timer_delta = 1;

         for(cpu_time_t count = 0; count < elapsed_cycles; count++) {
            // Update the frame sequencer.
            apu_update_frame_sequencer();
            // Update outputs.
            apu_update_channels(UPDATE_OUTPUT);

            // Update ExSound.
            if(exsound)
               exsound->process(apu.timer_delta);

            // Mix outputs together.
            mix();

            // Gather samples.
            for(int channel = 0; channel < apu.mixer.channels; channel++) {
               // Fetch sample.
               const real sample = apu.mixer.inputs[channel];
               // Accumulate sample.
               apu.mixer.accumulators[channel] += sample;
               // Cache it so that we can split it up later if need be.
               apu.mixer.sample_cache[channel] = sample;
            }

            apu.mixer.accumulated_samples++;
            if(apu.mixer.accumulated_samples >= apu.mixer.max_samples) {
               // Determine how much of the last sample we want to keep for the next loop.
               const real residual = (apu.mixer.accumulated_samples - floor(apu.mixer.max_samples));
               // Calculate the divider for the APU:DSP frequency ratio.
               const real divider = (apu.mixer.accumulated_samples - residual);

               for(int channel = 0; channel < apu.mixer.channels; channel++) {
                  real& sample = apu.mixer.accumulators[channel];
                  // Remove residual sample portion.
                  sample -= (apu.mixer.sample_cache[channel] * residual);
                  // Divide.
                  sample /= divider;

                  // Send it to the audio queue.
                  enqueue(sample);
               }

               // Reload accumulators with residual sample portion.
               for(int channel = 0; channel < apu.mixer.channels; channel++)
                  apu.mixer.accumulators[channel] = (apu.mixer.sample_cache[channel] * residual);

               // Adjust counter.
               apu.mixer.accumulated_samples -= apu.mixer.max_samples;
            }
         }

         break;
      }

      default:
         WARN_GENERIC();
   }
}

static void mix(void)
{
   static const APUSquare& square1  = apu.square[0];
   static const APUSquare& square2  = apu.square[1];
   static const APUTriangle& triangle = apu.triangle;
   static const APUNoise& noise = apu.noise;
   static const APUDMC& dmc = apu.dmc;

   const real square_out = square_table[square1.output + square2.output];
   const real tnd_out = tnd_table[3 * triangle.output + 2 * noise.output + dmc.output];

   switch(apu.mixer.channels) {
      case 1: {
         // Mono output.
         real total = (square_out + tnd_out);   // 0...1

         if(exsound) {
            exsound->mix(total);
            total = exsound->output;
         }

         apu.mixer.inputs[0] = total;

         break;
      }

      case 2: {
         // Stereo output.
         int leftInput, rightInput;
         if(apu_options.swap_channels) {
            rightInput = 0;
            leftInput = 1;
         }
         else {
            leftInput = 0;
            rightInput = 1;
         }

         if(exsound) {
            /* In the case of cartridges with extra sound capabilities, we have to force the Famicom's sound to mono so
               that it is suitable for passing through the cartridge mixer.  While this is not the ideal solution(since it
               effectively disables stereo sound output for these games), it is the most accurate. */
           real total = (square_out + tnd_out);   // 0...1

            exsound->mix(total);
            total = exsound->output;

            // Halve the volume since we'll be outputting the same thing to both channels.
            total /= 2.0;

            apu.mixer.inputs[leftInput] = total;
            apu.mixer.inputs[rightInput] = total;
         }
         else {
            // Normalise output without damaging the relative volume levels.
            real left = (square_out * (1.0 / MAX_TND));
            real right = (tnd_out * (1.0 / MAX_TND));

            // Blend stereo image together somewhat and renormalize.
            const real saved_left = left;
            left = ((left + (right / 2.0)) / 1.5);
            right = ((right + (saved_left / 2.0)) / 1.5);

            apu.mixer.inputs[leftInput] = left;
            apu.mixer.inputs[rightInput] = right;
         }

         break;
      }

      default:
         WARN_GENERIC();
   }
}

#define HIGHPASS_FREQUENCY 16   // Hz
#define HIGHPASS_STEP_TIME 0.01 // In seconds

// TODO: Normalizer.
static void enqueue(real& sample)
{
   // DC blocking filter.
   const real saved_sample = sample;

   static real filter_sample = 0.0;
   static real next_filter_sample = 0.0;
   static real stepTime = 0.0;
   static real weightPerStep = 0.0;
   static real curStep = 0.0;

   if(stepTime > 0.0) {
      const real weight = (weightPerStep * curStep);
      sample -= ((filter_sample * (1.0 - weight)) + (next_filter_sample * weight));
      curStep++;
   }
   else
      sample -= filter_sample;

   if(stepTime > 0.0) {
      stepTime--;
      if(stepTime <= 0.0)
         filter_sample = next_filter_sample;
   }

   static real timer = 0.0;
   if(timer > 0.0)
      timer--;
   if(timer <= 0.0) {
      timer += (audio_sample_rate / HIGHPASS_FREQUENCY);
      next_filter_sample = saved_sample;
      stepTime = (audio_sample_rate * HIGHPASS_STEP_TIME);
      weightPerStep = (1.0 / stepTime);
      curStep = 0.0;
   }

   // Apply global volume
   sample *= apu_options.volume;

   // Store it in the queue.
   audio_queue_sample(sample);
}
