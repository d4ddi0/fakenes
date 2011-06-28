/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef Audio__Common_hpp__included
#define Audio__Common_hpp__included
#include <cstdlib>
#include <cstring>
#include <allegro.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include "apu.h"
#include "apu_int.h"
#include "audio.h"
#include "audio_int.h"
#include "common.h"
#include "config.h"
#include "core.h"
#include "cpu.h"
#include "debug.h"
#include "log.h"
#include "machine.h"
#include "timing.h"
#include "types.h"
#include <allegro.h>
#include <cstdlib>
#include <cstring>
#include "audio.h"
#include "audio_int.h"
#include "audiolib.h"
#include "common.h"
#include "debug.h"
#include "log.h"
#include "types.h"

#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

// Curse you, Windows!
#ifdef ALLEGRO_WINDOWS
#undef interface
#endif

#endif // !Audio__Common_hpp__included
