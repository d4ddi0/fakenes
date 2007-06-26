/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _SOUND__SOUND_HPP
#define _SOUND__SOUND_HPP
#include "../include/common.h"
#include "../include/apu.h"
#include "../include/cpu.h"

namespace Sound {

// These are just templates - they do nothing unless you derive from them and replace their virtual members.
class Channel {
public:
   Channel(void) { }
   virtual ~Channel(void) { }

   virtual void reset(void) { }
   virtual uint8 read(uint16 address) { return 0x00; }
   virtual void write(uint16 address, uint8 value) { }
   virtual void process(cpu_time_t cycles) { }
   virtual void save(PACKFILE* file, int version) { }
   virtual void load(PACKFILE* file, int version) { }
};

class Interface {
public:
   Interface(void) { };
   virtual ~Interface(void) { }

   virtual void reset(void) { }
   virtual uint8 read(uint16 address) { return 0x00; }
   virtual void write(uint16 address, uint8 value) { }
   virtual void process(cpu_time_t cycles) { }
   virtual void save(PACKFILE* file, int version) { }
   virtual void load(PACKFILE* file, int version) { }
   virtual void mix(real input) { output = input; }
         
   real output;
};

} //namespace Sound

#endif //!_SOUND__SOUND_HPP
