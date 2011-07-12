/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Video__HQX_hpp__included
#define Video__HQX_hpp__included
#include "Common/Global.h"
#include "Local.hpp"

extern void hq2x(unsigned char* pIn, unsigned char* pOut, int Xres, int Yres, int BpL);
extern void hq3x(unsigned char* pIn, unsigned char* pOut, int Xres, int Yres, int BpL);
extern void hq4x(unsigned char* pIn, unsigned char* pOut, int Xres, int Yres, int BpL);

#endif // !Video__HQX_hpp__included
