/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include "../include/common.h"
#include "sound.hpp"

namespace Sound {

/* These are just stubs for when a sound driver doesn't explicitly implement
   a particular method. */

Channel::Channel(void)
{
}

Channel::~Channel(void)
{
}

void Channel::reset(void)
{
}

uint8 Channel::read(uint16 address)
{
   return 0x00;
}

void Channel::write(uint16 address, uint8 value)
{
}

void Channel::process(cpu_time_t cycles)
{
}

void Channel::save(PACKFILE* file, int version)
{
   RT_ASSERT(file);
}

void Channel::load(PACKFILE* file, int version)
{
   RT_ASSERT(file);
}

Interface::Interface(void)
{
}

Interface::~Interface(void)
{
}

void Interface::reset(void)
{
   output = 0.0;
}

uint8 Interface::read(uint16 address)
{
   return 0x00;
}

void Interface::write(uint16 address, uint8 value)
{
}

void Interface::process(cpu_time_t cycles)
{
}

void Interface::save(PACKFILE* file, int version)
{
   RT_ASSERT(file);
}

void Interface::load(PACKFILE* file, int version)
{
   RT_ASSERT(file);
}

void Interface::mix(real input)
{
   output = input;
}

} //namespace Sound
