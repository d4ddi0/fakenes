/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef _SOUND__COMMON_HPP
#define _SOUND__COMMON_HPP
#include <cstdlib>
#include <cstring>
#include "../include/common.h"
#include "../include/apu.h"
#include "../include/cpu.h"
#include "SoundInternals.hpp"

// Curse you, Windows!
#ifdef ALLEGRO_WINDOWS
#undef interface
#endif

#endif //!_SOUND_COMMON_HPP
