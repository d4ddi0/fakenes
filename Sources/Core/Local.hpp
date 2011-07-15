/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Core__Local_hpp__included
#define Core__Local_hpp__included
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <vector>
#include "Audio/APU.h"
#include "Common/Binary.h"
#include "Common/Debug.h"
#include "Common/Global.h"
#include "Common/Inline.h"
#include "Common/Types.h"
#include "Platform/File.h"
#include "Platform/Save.h"
#include "System/Machine.h"
#include "System/ROM.h"
#include "System/Timing.h"
#include "Video/PPU.h"

// Resolve Windows API conflicts.
#ifdef SYSTEM_WINDOWS
#undef ABSOLUTE
#endif

#endif // !Core__Local_hpp__included
