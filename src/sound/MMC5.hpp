/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.

   MMC5.cpp: MMC5 sound hardware emulation by randilyn. */

#ifndef _SOUND__MMC5_HPP
#define _SOUND__MMC5_HPP
#include "Common.hpp"

namespace Sound {
namespace MMC5 {

class Interface;

class Square : public Channel {
protected:
   friend class Interface;

   void reset(void);
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void save(PACKFILE* file, const int version) const;
   void load(PACKFILE* file, const int version);

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
   void save(PACKFILE* file, int version) const;
   void load(PACKFILE* file, int version);

   uint8 output;  // save
};

class Interface : public Sound::Interface {
public:
   void reset(void);
   uint8 read(const uint16 address) const;
   void write(const uint16 address, const uint8 value);
   void process(const cpu_time_t cycles);
   void save(PACKFILE* file, const int version) const;
   void load(PACKFILE* file, const int version);
   void mix(const real input);

private:
   Square square1;
   Square square2;
   PCM pcm;

   int32 timer;   // save
   bool flip;     // save
};

} //namespace MMC5
} //namespace Sound

#endif //!_SOUND__MMC5_HPP
