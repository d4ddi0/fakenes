/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "APU.h"
#include "ExSound.hpp"
#include "Local.hpp"
#include "VRC6.hpp"

namespace ExSound {
namespace VRC6 {

void Square::reset(void)
{
   enabled = false;
   timer = 0;
   period = 0;
   volume = 0;
   duty = 0;
   force = false;

   step = 0;
   output = 0;
}

void Square::write(const uint16 address, const uint8 value)
{
   switch(address) {
      case 0x9000:
      case 0xA000: {
         regs[0] = value;

         volume = value & 0x0F;
         duty = (value >> 4) & 0x07;
         force = LOAD_BOOLEAN(value & 0x80);

         if(force)
            output = volume;

         break;
      }

      case 0x9001:
      case 0xA001: {
         regs[1] = value;

         period &= ~0xFF;
         period |= value;

         break;
      }

      case 0x9002:
      case 0xA002: {
         regs[2] = value;

         enabled = LOAD_BOOLEAN(value & 0x80);
         if(!enabled)
            output = 0;

         period &= ~0xF00;
         period |= (value & 0x0F) << 8;

         break;
      }

      default:
         break;
   }
}

void Square::process(const cpu_time_t cycles)
{
   if(timer > 0) {
      timer -= cycles;
      if(timer > 0)
         return;
   }

   timer += period + 1;

   if(enabled &&
      (force || (step <= duty)))
      output = volume;
   else
      output = 0;

   if (++step > 15)
      step = 0;
}

void Square::load(FILE_CONTEXT* file, const int version)
{
   RT_ASSERT(file);

   for(int index = 0; index < 3; index++)
      write((0x9000 + index), file->read_byte(file)); // should work for both

   timer = file->read_word(file);
   step = file->read_byte(file);
   output = file->read_byte(file);
}

void Square::save(FILE_CONTEXT* file, const int version) const
{
   RT_ASSERT(file);

   for(int index = 0; index < 3; index++)
      file->write_byte(file, regs[index]);

   file->write_word(file, timer);
   file->write_byte(file, step);
   file->write_byte(file, output);
}

void Saw::reset(void)
{
   enabled = false;
   timer = 0;
   period = 0;
   rate = 0;

   step = 0;
   volume = 0;
   output = 0;
}

void Saw::write(const uint16 address, const uint8 value)
{
   switch(address) {
      case 0xB000: {
         regs[0] = value;

         rate = value & 0x3F;
         break;
      }

      case 0xB001: {
         regs[1] = value;

         period &= ~0xFF;
         period |= value;

         break;
      }

      case 0xB002: {
         regs[2] = value;

         enabled = LOAD_BOOLEAN(value & 0x80);
         if(!enabled)
            output = 0;

         period &= ~0xF00;
         period |= (value & 0x0F) << 8;

         break;
      }

      default:
         break;
   }
}

void Saw::process(const cpu_time_t cycles)
{
   if(timer > 0) {
      timer -= cycles;
      if(timer > 0)
         return;
   }

   timer += (period + 1) << 1;

   if(step == 6) {
      step = 0;
      volume = 0;
      return;
   }
   else
      step++;

   if(!enabled) {
      output = 0;
      return;
   }

   volume += rate;
   output = volume >> 3;
}

void Saw::load(FILE_CONTEXT* file, const int version)
{
   RT_ASSERT(file);

   for(int index = 0; index < 3; index++)
      write((0xB000 + index), file->read_byte(file));

   timer = file->read_word(file);
   step = file->read_byte(file);
   volume = file->read_byte(file);
   output = file->read_byte(file);
}

void Saw::save(FILE_CONTEXT* file, const int version) const
{
   RT_ASSERT(file);

   for(int index = 0; index < 3; index++)
      file->write_byte(file, regs[index]);

   file->write_word(file, timer);
   file->write_byte(file, step);
   file->write_byte(file, volume);
   file->write_byte(file, output);
}

void Interface::reset(void)
{
   square1.reset();
   square2.reset();
   saw.reset();

   output = 0;
}

void Interface::write(const uint16 address, const uint8 value)
{
   switch(address) {
      case 0x9000:
      case 0x9001:
      case 0x9002: {
         square1.write(address, value);
         break;
      }

      case 0xA000:
      case 0xA001:
      case 0xA002: {
         square2.write(address, value);
         break;
      }

      case 0xB000:
      case 0xB001:
      case 0xB002: {
         saw.write(address, value);
         break;
      }

      default:
         break;
   }
}

void Interface::process(const cpu_time_t cycles)
{
   square1.process(cycles);
   square2.process(cycles);
   saw.process(cycles);
}

void Interface::load(FILE_CONTEXT* file, const int version)
{
   RT_ASSERT(file);

   square1.load(file, version);
   square2.load(file, version);
   saw.load(file, version);
}

void Interface::save(FILE_CONTEXT* file, const int version) const
{
   RT_ASSERT(file);

   square1.save(file, version);
   square2.save(file, version);
   saw.save(file, version);
}

void Interface::mix(const real input)
{
   uint8 total = 0;

   if (apu_options.enable_extra_1)
      total += square1.output;
   if (apu_options.enable_extra_2)
      total += square2.output;

   if (apu_options.enable_extra_3)
      total += saw.output;

   /* I'm going to assume that the VRC6's mixer consists merely of adders along with a 6 bit DAC.
      This would make the maximum capacity of the VRC6's mixer 6 bits, with the values 0-61 being consumed by the above 
      accumulations and the rest (values 62-63) being consumed by headroom. */
   const real constantTotal = total / 63.0;
   output = (input + constantTotal) / 2.0;
}

} //namespace VRC6
} //namespace ExSound
