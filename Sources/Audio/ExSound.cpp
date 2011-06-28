/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.

   Sourcer.cpp: Manager for multiple sound sources. */

#include "Common.hpp"
#include "Sourcer.hpp"

namespace Sound {
namespace Sourcer {

// Define some macros to make our code cleaner while still giving decent performance.
#define FirstSource    sources[0]

#define SourceLoop     for(int source = 0; source < totalSources; source++)
#define CurrentSource  sources[source]

// const-correctness is a pain!
#define ConstSource(x) ((const Sound::Interface*)(x))

Interface::Interface(void)
{
   totalSources = 0;

   // Clear output.
   output = 0;
}

void Interface::clearSources(void)
{
   memset(sources, 0, sizeof(sources));
   totalSources = 0;
}

void Interface::attachSource(Sound::Interface* interface)
{
   RT_ASSERT(interface);

   SourceLoop {
      if(CurrentSource == interface) {
         // The source is already attached (this shouldn't happen).
         WARN_GENERIC();
         return;
      }
   }

   if((totalSources + 1) > MaximumSources) {
      // Source limit has been reached (this shouldn't happen).
     WARN_GENERIC();
     return;
   }

   sources[totalSources] = interface;
   totalSources++;
}

int Interface::getSources(void) const
{
   return totalSources;
}

int Interface::getMaximumSources(void) const
{
   return MaximumSources;
}

void Interface::reset(void)
{
   SourceLoop
      CurrentSource->reset();

   // Clear output;
   output = 0;
}

uint8 Interface::read(uint16 address) const
{
   if(totalSources == 0)
      return 0x00;
   else if(totalSources == 1)
      return ConstSource(FirstSource)->read(address);
   else {
      uint8 value = 0x00;
      SourceLoop
         value |= ConstSource(CurrentSource)->read(address);

      return value;
   }
}

void Interface::write(uint16 address, uint8 value)
{
   if(totalSources == 0)
      return;
   else if(totalSources == 1)
      FirstSource->write(address, value);
   else {
      SourceLoop
         CurrentSource->write(address, value);
   }
}

void Interface::process(cpu_time_t cycles)
{
   if(totalSources == 0)
      return;
   else if(totalSources == 1)
      FirstSource->process(cycles);
   else {
      SourceLoop
         CurrentSource->process(cycles);
   }
}

void Interface::save(PACKFILE* file, int version) const
{
   RT_ASSERT(file);

   SourceLoop
      ConstSource(CurrentSource)->save(file, version);
}

void Interface::load(PACKFILE* file, int version)
{
   RT_ASSERT(file);

   SourceLoop
      CurrentSource->load(file, version);
}

void Interface::mix(real input)
{
   if(totalSources == 0)
      output = input;
   else if(totalSources == 1) {
      FirstSource->mix(input);
      output = FirstSource->output;
   }
   else {
      real total = 0.0;
      SourceLoop {
         CurrentSource->mix(input);
         total += CurrentSource->output;
      }

      output = total / totalSources;
   }
}

} //namespace Sourcer
} //namespace Sound
