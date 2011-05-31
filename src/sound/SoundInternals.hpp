/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _SOUND__SOUNDINTERNALS_HPP
#define _SOUND__SOUNDINTERNALS_HPP
#include "Common.hpp"

namespace Sound {

// These are just templates - they do nothing unless you derive from them and replace their virtual members.
class Channel {
public:
   Channel(void) { }
   virtual ~Channel(void) { }

   virtual void reset(void) { }
   virtual uint8 read(const uint16 address) const { return 0x00; }
   virtual void write(const uint16 address, const uint8 value) { }
   virtual void process(const cpu_time_t cycles) { }
   virtual void save(PACKFILE* file, const int version) const { }
   virtual void load(PACKFILE* file, const int version) { }
};

class Interface {
public:
   Interface(void) { };
   virtual ~Interface(void) { }

   virtual void reset(void) { }
   virtual uint8 read(const uint16 address) const { return 0x00; }
   virtual void write(const uint16 address, const uint8 value) { }
   virtual void process(const cpu_time_t cycles) { }
   virtual void save(PACKFILE* file, const int version) const { }
   virtual void load(PACKFILE* file, const int version) { }
   virtual void mix(const real input) { output = input; }

   real output;
};

} //namespace Sound

#endif //!_SOUND__SOUNDINTERNALS_HPP
