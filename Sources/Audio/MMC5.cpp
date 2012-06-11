/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "APU.h"
#include "ExSound.hpp"
#include "Local.hpp"
#include "MMC5.hpp"

namespace ExSound {
namespace MMC5 {

// --- Lookup tables. ---
static const uint8 length_lut[32] = {
   0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A,
   0x0E, 0x0C, 0x1A, 0x0E, 0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
   0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

/* Pulse sequences for each step 0-7 of each duty cycle 0-3 on the square
   wave channels. */
static const uint8 square_duty_lut[4][8] = {
   { 0x0, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
   { 0x0, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0 },
   { 0x0, 0xF, 0xF, 0xF, 0xF, 0x0, 0x0, 0x0 },
   { 0xF, 0x0, 0x0, 0xF, 0xF, 0xF, 0xF, 0xF }
};

void Square::reset(void)
{
   memset(regs, 0x00, sizeof(regs));

   timer = 0;
   period = 0;
   length = 0;
   halt = false;
   volume = 0;
   duty = 0;
   clamped = false;

   envelope.timer = 0;
   envelope.period = 0;
   envelope.fixed = false;
   envelope.fixed_volume = 0;
   envelope.reset = false;

   envelope.counter = 0;

   step = 0;
   output = 0;
}

void Square::write(const uint16 address, const uint8 value)
{
   switch(address) {
      case 0x5000:
      case 0x5004: {
         regs[0] = value;

         volume = value & 0x0F;
         halt = LOAD_BOOLEAN(value & 0x20);
         duty = value >> 6;

         envelope.period = (value & 0x0F) + 1;
         envelope.fixed = LOAD_BOOLEAN(value & 0x10);
         envelope.fixed_volume = value & 0x0F;

         break;        
      }

      case 0x5001:
      case 0x5005: {
         regs[1] = value;  // unused placeholder
         break;
      }

      case 0x5002:
      case 0x5006: {
         regs[2] = value;

         period &= ~0xFF;
         period |= value;

         break;
      }

      case 0x5003:
      case 0x5007: {
         regs[3] = value;

         period &= ~0x700;
         period |= (value & 0x07) << 8;

         if(!clamped)
            length = length_lut[value >> 3];

         // Reset envelope generator.
         envelope.reset = true;
         // Reset sequencer(?)
         step = 0;

         break;
      }

      default:
         break;
   }
}

void Square::process(const cpu_time_t cycles)
{
   if (timer > 0) {
      timer -= cycles;
      if(timer > 0)
         return;
   }

   timer += (period + 2) << 1;

   if(length > 0)
      output = volume & square_duty_lut[duty][step];
   else
      output = 0;

   if (++step > 7)
      step = 0;
}

void Square::load(FILE_CONTEXT* file, const int version)
{
   RT_ASSERT(file);

   for(int index = 0; index < 4; index++)
      write((0x5000 + index), file->read_byte(file)); // should work for both

   timer = file->read_word(file);
   length = file->read_byte(file);
   volume = file->read_byte(file);
   step = file->read_byte(file);
   output = file->read_byte(file);

   envelope.timer = file->read_byte(file);
   envelope.reset = file->read_boolean(file);
   envelope.counter = file->read_byte(file);
}

void Square::save(FILE_CONTEXT* file, const int version) const
{
   RT_ASSERT(file);

   for(int index = 0; index < 4; index++)
      file->write_byte(file, regs[index]);

   file->write_word(file, timer);
   file->write_byte(file, length);
   file->write_byte(file, volume);
   file->write_byte(file, step);
   file->write_byte(file, output);

   file->write_byte(file, envelope.timer);
   file->write_boolean(file, envelope.reset);
   file->write_byte(file, envelope.counter);
}

void Square::update_120hz(void)
{
   if((length > 0) && !halt)
      length--;
}

void Square::update_240hz(void)
{
   if(envelope.reset) {
      envelope.reset = false;
      envelope.timer = 0;
      envelope.counter = 0xF;
      return;
   }

   if(envelope.timer > 0) {
      envelope.timer--;
      if(envelope.timer > 0)
         return;
   }

   envelope.timer += envelope.period;

   if(envelope.counter > 0)
      envelope.counter--;
   else if(halt)
      envelope.counter = 0xF;

   volume = envelope.fixed ? envelope.fixed_volume : envelope.counter;
}

void PCM::reset (void)
{
   output = 0;
}

void PCM::write(const uint16 address, const uint8 value)
{
   switch (address) {
      case 0x5011: {
         output = value;
         break;
      }

      default:
         break;
   }
}

void PCM::load(FILE_CONTEXT* file, const int version)
{
   RT_ASSERT(file);

   output = file->read_byte(file);
}

void PCM::save(FILE_CONTEXT* file, const int version) const
{
   RT_ASSERT(file);

   file->write_byte(file, output);
}

void Interface::reset(void)
{
   square1.reset();
   square2.reset();
   pcm.reset();

   timer = 0;
   flip = false;

   output = 0;
}

uint8 Interface::read(const uint16 address) const
{
   uint8 value = 0x00;

   switch(address) {
      case 0x5015: {
         if(square1.length > 0)
            value |= 0x01;
         if(square2.length > 0)
            value |= 0x02;

         break;
      }

      default:
         break;
   }

   return value;
}

void Interface::write(const uint16 address, const uint8 value)
{
   switch(address) {
      case 0x5000:
      case 0x5001:
      case 0x5002:
      case 0x5003: {
         square1.write(address, value);
         break;
      }

      case 0x5004:
      case 0x5005:
      case 0x5006:
      case 0x5007: {
         square2.write(address, value);
         break;
      }

      case 0x5011: {
         pcm.write(address, value);
         break;
      }

      case 0x5015: {
         square1.clamped = LOAD_BOOLEAN(~value & 0x01);
         if(square1.clamped)
            square1.length = 0;

         square2.clamped = LOAD_BOOLEAN(~value & 0x02);
         if(square2.clamped)
            square2.length = 0;

         break;
      }

      default:
         break;
   }
}

void Interface::process(const cpu_time_t cycles)
{
   if(timer > 0)
      timer -= cycles;
   if(timer <= 0) {
      /* This should actually be 7457.5 - but close enough.
         Effective rate: ~240Hz (239.996...) */
      if(flip)
         timer += 7458;
      else
         timer += 7457;

      square1.update_240hz();
      square2.update_240hz();

      if(flip) {
         flip = false;

         square1.update_120hz();
         square2.update_120hz();
      }
      else
         flip = true;
   }

   square1.process(cycles);
   square2.process(cycles);
}

void Interface::load(FILE_CONTEXT* file, const int version)
{
   RT_ASSERT(file);

   timer = file->read_long(file);
   flip = file->read_boolean(file);

   square1.load(file, version);
   square2.load(file, version);
   pcm.load(file, version);
}

void Interface::save(FILE_CONTEXT* file, const int version) const
{
   RT_ASSERT(file);

   file->write_long(file, timer);
   file->write_boolean(file, flip);

   square1.save(file, version);
   square2.save(file, version);
   pcm.save(file, version);
}

void Interface::mix(const real input)
{
   /* Not much is known about MMC5 mixing but common sense will tell that the squares are probably mixed together and then
      passed to an 8 bit DAC, and the PCM control register is simply connected directly to the DAC.
      If this is really the case, then clearly the square waves cannot be used in conjunction with the PCM as they would
      overwrite the PCM value currently stored in the DAC (similar to the limitations of the Famicom's DMC DAC).
      But any other setup would produce square waves that are far too quiet due to the headroom introduced by the PCM 
      channel...
      For our purposes, we'll just mix the squares together then combine them with the PCM, which should give reasonable
      volume levels for both the square waves and PCM. */
   uint8 squares_out = 0;
   if(apu_options.enable_extra_1)
      squares_out += square1.output;
   if(apu_options.enable_extra_2)
      squares_out += square2.output;

   uint8 pcm_out = 0;
   if(apu_options.enable_extra_3)
      pcm_out += pcm.output;

   uint16 total = (squares_out << 3) + pcm_out;

   // ((15+15)*8)+255 = 495
   const real constantTotal = total / 495.0;
   output = (input + constantTotal) / 2.0;
}

} //namespace MMC5
} //namespace ExSound
