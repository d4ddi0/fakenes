/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _SOUND__MMC5_HPP
#define _SOUND__MMC5_HPP
#include "../include/common.h"
#include "sound.hpp"

namespace Sound {
namespace MMC5 {

class Interface;

class Square : public Channel {
protected:
   friend class Interface;

   void reset (void);
   void write (uint16 address, uint8 value);
   void process (cpu_time_t cycles);

   uint8 output;

private:
   int16 timer;
   uint16 period;
   uint8 length;
   bool halt;
   uint8 volume;
   uint8 duty;
   bool clamped;

   uint8 step;
};

class PCM : public Channel {
protected:
   friend class Interface;

   void reset (void);
   void write (uint16 address, uint8 value);

   uint8 output;
};

class Interface : public Sound::Interface {
public:
   void reset (void);
   uint8 read (uint16 address);
   void write (uint16 address, uint8 value);
   void process (cpu_time_t cycles);
   void mix (void);

private:
   Square square1;
   Square square2;
   PCM pcm;
};

}  /* namespace MMC5 */
}  /* namespace Sound */

#endif   /* !_SOUND__MMC5_HPP */
