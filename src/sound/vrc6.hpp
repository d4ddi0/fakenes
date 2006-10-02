/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _SOUND__VRC6_HPP
#define _SOUND__VRC6_HPP
#include "../include/common.h"
#include "sound.hpp"

namespace Sound {
namespace VRC6 {

class Interface;

class Square : public Channel {
protected:
   friend class Interface;

   void reset (void);
   void write (uint16 address, uint8 value);
   void process (cpu_time_t cycles);

   uint8 output;

private:
   bool enabled;
   int16 timer;
   uint16 period;
   uint8 volume;
   uint8 duty;
   bool force;

   uint8 step;
};

class Saw : public Channel {
protected:
   friend class Interface;

   void reset (void);
   void write (uint16 address, uint8 value);
   void process (cpu_time_t cycles);

   uint8 output;

private:
   bool enabled;
   int16 timer;
   uint16 period;
   uint8 rate;

   uint8 step;
   uint8 volume;
};

class Interface : public Sound::Interface {
public:
   void reset (void);
   void write (uint16 address, uint8 value);
   void process (cpu_time_t cycles);
   void mix (void);

private:
   Square square1;
   Square square2;
   Saw saw;
};

}  /* namespace VRC6 */
}  /* namespace Sound */

#endif   /* !_SOUND__VRC6_HPP */
