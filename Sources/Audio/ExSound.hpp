/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef Audio__ExSound_hpp__included
#define Audio__ExSound_hpp__includde
#include "Common.hpp"

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

#endif // !Audio__ExSound_hpp__included
