/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef Audio__VRC6_hpp__included                                
#define Audio__VRC6_hpp__included
#include "Common.hpp"

namespace Sound {
namespace VRC6 {

class Interface;

class Square : public Channel {
protected:
   friend class Interface;

   void reset(void);
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void save(PACKFILE* file, const int version) const;
   void load(PACKFILE* file, const int version);

   uint8 output;  // save

private:
   uint8 regs[3]; // save

   bool enabled;  // do not save
   int16 timer;   // save
   uint16 period; // do not save
   uint8 volume;  // do not save
   uint8 duty;    // do not save
   bool force;    // do not save

   uint8 step;    // save
};

class Saw : public Channel {
protected:
   friend class Interface;

   void reset(void);
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void save(PACKFILE* file, const int version) const;
   void load(PACKFILE* file, const int version);

   uint8 output;  // save

private:
   uint8 regs[3]; // save

   bool enabled;  // do not save
   int16 timer;   // save
   uint16 period; // do not save
   uint8 rate;    // do not save

   uint8 step;    // save
   uint8 volume;  // save
};

class Interface : public Sound::Interface {
public:
   void reset(void);
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void save(PACKFILE* file, const int version) const;
   void load(PACKFILE* file, const int version);
   void mix(const real input);

private:
   Square square1;
   Square square2;
   Saw saw;
};
                        
} //namespace VRC6
} //namespace Sound

#endif // !Audio__VRC6_hpp__included
