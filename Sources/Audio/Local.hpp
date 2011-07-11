/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Audio__Local_hpp__included
#define Audio__Local_hpp__included
#include <allegro.h>
#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include "Common/Debug.h"
#include "Common/Global.h"
#include "Common/Math.h"
#include "Common/Types.h"
#include "Core/CPU.h"
#include "Platform/Config.h"
#include "Platform/File.h"
#include "Platform/Log.h"
#include "System/Machine.h"
#include "System/Timing.h"

// Curse you, Windows!
#ifdef ALLEGRO_WINDOWS
#undef interface
#endif

#endif // !Audio__Local_hpp__included
