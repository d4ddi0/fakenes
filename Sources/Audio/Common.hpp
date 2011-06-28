/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

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
