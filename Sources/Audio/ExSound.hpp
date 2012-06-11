/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Audio__ExSound_hpp__included
#define Audio__ExSound_hpp__included
#include "Local.hpp"

namespace ExSound {

// These are just templates - they do nothing unless you derive from them and replace their virtual members.
class Channel {
public:
   Channel(void) { }
   virtual ~Channel(void) { }

   virtual void reset(void) { }
   virtual uint8 read(const uint16 address) const { return 0x00; }
   virtual void write(const uint16 address, const uint8 value) { }
   virtual void process(const cpu_time_t cycles) { }
   virtual void load(FILE_CONTEXT* file, const int version) { }
   virtual void save(FILE_CONTEXT* file, const int version) const { }
};

class Interface {
public:
   Interface(void) { };
   virtual ~Interface(void) { }

   virtual void reset(void) { }
   virtual uint8 read(const uint16 address) const { return 0x00; }
   virtual void write(const uint16 address, const uint8 value) { }
   virtual void process(const cpu_time_t cycles) { }
   virtual void load(FILE_CONTEXT* file, const int version) { }
   virtual void save(FILE_CONTEXT* file, const int version) const { }
   virtual void mix(const real input) { output = input; }

   real output;
};

namespace Sourcer {

static const int MaximumSources = 8;

class Interface : public ExSound::Interface {
public:
   Interface(void);

   void clearSources(void);
   void attachSource(ExSound::Interface* interface);
   int getSources(void) const;
   int getMaximumSources(void) const;

   void reset(void);
   uint8 read(uint16 address) const;
   void write(uint16 address, uint8 value);
   void process(cpu_time_t cycles);
   void load(FILE_CONTEXT* file, int version);
   void save(FILE_CONTEXT* file, int version) const;
   void mix(real input);

private:
   ExSound::Interface* sources[MaximumSources];
   int totalSources;
};

} //namespace Sourcer
} //namespace ExSound

#endif // !Audio__ExSound_hpp__included
