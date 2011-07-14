/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Video__Local_hpp__included
#define Video__Local_hpp__included
#include <allegro.h>
#ifdef USE_ALLEGROGL
#include <alleggl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "Common/Binary.h"
#include "Common/Debug.h"
#include "Common/Global.h"
#include "Common/Inline.h"
#include "Common/Math.h"
#include "Common/Types.h"
#include "Audio/APU.h"
#include "Audio/Audio.h"
#include "Core/CPU.h"
#include "GUI/GUI.h"
#include "Platform/Load.h"
#include "Platform/Log.h"
#include "Platform/Platform.h"
#include "System/Input.h"
#include "System/Mapper.h"
#include "System/Machine.h"
#include "System/ROM.h"
#include "System/Timing.h"

#endif // !Video__Local_hpp__included
