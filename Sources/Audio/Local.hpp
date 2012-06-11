/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Audio__Local_hpp__included
#define Audio__Local_hpp__included
#if defined(USE_ALLEGRO)
#   include <allegro.h>
#endif
#if defined(USE_SDL)
#   include <SDL/SDL.h>
#endif
#if defined(USE_OPENAL)
#   include <AL/al.h>
#   include <AL/alc.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "Common/Debug.h"
#include "Common/Global.h"
#include "Common/Inline.h"
#include "Common/Math.h"
#include "Common/Types.h"
#include "Core/CPU.h"
#include "Platform/Config.h"
#include "Platform/File.h"
#include "Platform/Log.h"
#include "System/Machine.h"
#include "System/Timing.h"
#include "Toolkit/Unicode.h"

// Resolve Windows API conflicts.
#if defined(SYSTEM_WINDOWS)
#   undef interface
#endif

/* "FYI: If I see it correctly, AL_INVALID is not mentioned in the 1.1 spec, so
   I've moved it to a "deprecated" section of the <AL/al.h> header (to be on
   the safe side). I'll remove any traces of it from the Linux implementation,
   and I guess the other implementations should follow..." */
#if defined(USE_OPENAL)
#   undef AL_INVALID
#   define AL_INVALID 0
#endif

#endif // !Audio__Local_hpp__included
