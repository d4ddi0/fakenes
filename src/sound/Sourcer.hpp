/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.

   Sourcer.cpp: Manager for multiple sound sources. */

#ifndef _SOUND__SOURCER_HPP
#define _SOUND__SOURCER_HPP
#include "Common.hpp"

namespace Sound {
namespace Sourcer {

static const int MaximumSources = 8;

class Interface : public Sound::Interface {
public:
   Interface(void);

   void clearSources(void);
   void attachSource(Sound::Interface* interface);
   int getSources(void) const;
   int getMaximumSources(void) const;

   void reset(void);
   uint8 read(uint16 address) const;
   void write(uint16 address, uint8 value);
   void process(cpu_time_t cycles);
   void save(PACKFILE* file, int version) const;
   void load(PACKFILE* file, int version);
   void mix(real input);

private:
   Sound::Interface* sources[MaximumSources];
   int totalSources;
};

} //namespace Sourcer
} //namespace Sound

#endif //!_SOUND__SOURCER_HPP
