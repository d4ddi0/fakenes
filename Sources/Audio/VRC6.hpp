/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Audio__VRC6_hpp__included                                
#define Audio__VRC6_hpp__included
#include "ExSound.hpp"
#include "Local.hpp"

namespace ExSound {
namespace VRC6 {

class Interface;

class Square : public Channel {
protected:
   friend class Interface;

   void reset(void);
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void load(FILE_CONTEXT* file, const int version);
   void save(FILE_CONTEXT* file, const int version) const;

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
   void load(FILE_CONTEXT* file, const int version);
   void save(FILE_CONTEXT* file, const int version) const;

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

class Interface : public ExSound::Interface {
public:
   void reset(void);
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void load(FILE_CONTEXT* file, const int version);
   void save(FILE_CONTEXT* file, const int version) const;
   void mix(const real input);

private:
   Square square1;
   Square square2;
   Saw saw;
};
                        
} //namespace VRC6
} //namespace ExSound

#endif // !Audio__VRC6_hpp__included
