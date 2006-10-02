/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.

   mmc5.cpp: MMC5 sound hardware emulation by randilyn. */

#include "../include/common.h"
#include "sound.hpp"
#include "mmc5.hpp"
             
namespace Sound {
namespace MMC5 {

// TODO: Support envelope generator(?)

static const uint8 length_lut[32] = {
   0x0A, 0x14, 0x28, 0x50, 0xA0, 0x3C, 0x0E, 0x1A, 0x0C, 0x18, 0x30, 0x60,
   0xC0, 0x48, 0x10, 0x20, 0xFE, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
   0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E
};

/* Pulse sequences for each step 0-7 of each duty cycle 0-3 on the square
   wave channels. */
static const uint8 square_duty_lut[4][8] = {
   { 0x0, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
   { 0x0, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0 },
   { 0x0, 0xF, 0xF, 0xF, 0xF, 0x0, 0x0, 0x0 },
   { 0xF, 0x0, 0x0, 0xF, 0xF, 0xF, 0xF, 0xF }
};

void Square::reset (void)
{
   timer = 0;
   period = 0;
   length = 0;
   halt = false;
   volume = 0;
   duty = 0;
   clamped = false;

   step = 0;
   output = 0;
}

void Square::write (uint16 address, uint8 value)
{
   switch (address)
   {
      case 0x5000:
      case 0x5004:
      {
         volume = (value & 0x0f);
         duty = (value >> 6);
         halt = (value & 0x20);

         break;
      }

      case 0x5002:
      case 0x5006:
      {
         period &= ~0xff;
         period |= value;

         break;
      }

      case 0x5003:
      case 0x5007:
      {
         period &= ~0x700;
         period |= ((value & 0x07) << 8);

         if (!clamped)
            length = length_lut[(value >> 3)];

         // Reset sequencer(?)
         step = 0;

         break;
      }

      default:
         break;
   }
}

void Square::process (cpu_time_t cycles)
{
   if (timer > 0)
   {
      timer -= cycles;
      if (timer > 0)
         return;
   }

   timer += ((period + 2) << 1);

   if (length > 0)
      output = (volume & square_duty_lut[duty][step]);
   else
      output = 0;

   step++;
   if (step > 7)
      step = 0;
}

void PCM::reset (void)
{
   output = 0;
}

void PCM::write (uint16 address, uint8 value)
{
   switch (address)
   {
      case 0x5011:
      {
         output = value;
         break;
      }

      default:
         break;
   }
}

void Interface::reset (void)
{
   square1.reset ();
   square2.reset ();
   pcm.reset ();

   output = 0;
}

uint8 Interface::read (uint16 address)
{
   uint8 value = 0;

   switch (address)
   {
      case 0x5015:
      {
         if (square1.length > 0)
            value |= 0x01;
         if (square2.length > 0)
            value |= 0x02;

         break;
      }

      default:
         break;
   }

   return (value);
}

void Interface::write (uint16 address, uint8 value)
{
   switch (address)
   {
      case 0x5000:
      case 0x5002:
      case 0x5003:
      {
         square1.write (address, value);
         break;
      }

      case 0x5004:
      case 0x5006:
      case 0x5007:
      {
         square2.write (address, value);
         break;
      }

      case 0x5011:
      {
         pcm.write (address, value);
         break;
      }

      case 0x5015:
      {
         square1.clamped = (value & 0x01);
         if (square1.clamped)
            square1.length = 0;

         square2.clamped = (value & 0x02);
         if (square2.clamped)
            square2.length = 0;

         break;
      }

      default:
         break;
   }
}

void Interface::process (cpu_time_t cycles)
{
   if (apu_options.enable_extra_1)
      square1.process (cycles);
   if (apu_options.enable_extra_2)
      square2.process (cycles);
}

void Interface::mix (void)
{
   output = 0.0;

   if (apu_options.enable_extra_1)
      output += (square1.output / 15.0);
   if (apu_options.enable_extra_2)
      output += (square2.output / 15.0);

   if (apu_options.enable_extra_3)
      output += (pcm.output / 255.0);

   output /= 3.0;
}

}  /* namespace MMC5 */
}  /* namespace Sound */
