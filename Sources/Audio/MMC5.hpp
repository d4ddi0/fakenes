/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Audio__MMC5_hpp__included
#define Audio__MMC5_hpp__included
#include "ExSound.hpp"
#include "Local.hpp"

namespace ExSound {
namespace MMC5 {

class Interface;

class Square : public Channel {
protected:
   friend class Interface;

   void reset(void);
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void load(FILE_CONTEXT* file, const int version);
   void save(FILE_CONTEXT* file, const int version) const;

   void update_120hz(void);
   void update_240hz(void);

   uint8 output;  // save

private:
   uint8 regs[4]; // save

   int16 timer;   // save
   uint16 period; // do not save
   uint8 length;  // save
   bool halt;     // do not save
   uint8 volume;  // save
   uint8 duty;    // do not save
   bool clamped;  // do not save

   uint8 step;    // save

   struct {
      uint8 timer;        // save
      uint8 period;       // do not save
      bool fixed;         // do not save
      uint8 fixed_volume; // do not save
      bool reset;         // save

      uint8 counter;      // save

   } envelope;
};

class PCM : public Channel {
protected:
   friend class Interface;

   void reset(void);
   void write(uint16 address, uint8 value);
   void load(FILE_CONTEXT* file, int version);
   void save(FILE_CONTEXT* file, int version) const;

   uint8 output;  // save
};

class Interface : public ExSound::Interface {
public:
   void reset(void);
   uint8 read(const uint16 address) const;
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void load(FILE_CONTEXT* file, const int version);
   void save(FILE_CONTEXT* file, const int version) const;
   void mix(const real input);

private:
   Square square1;
   Square square2;
   PCM pcm;

   int32 timer;   // save
   bool flip;     // save
};

} //namespace MMC5
} //namespace ExSound

#endif // !Audio__MMC5_hpp__included
