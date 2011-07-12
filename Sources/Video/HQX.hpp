/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Video__HQX_hpp__included
#define Video__HQX_hpp__included
#include "Common/Global.h"
#include "Common/Types.h"
#include "Local.hpp"

extern void HQX_Initialize();
extern void hq2x(unsigned char* pIn, unsigned char* pOut, const int Xres, const int Yres, const int BpL);
extern void hq3x(unsigned char* pIn, unsigned char* pOut, const int Xres, const int Yres, const int BpL);
extern void hq4x(unsigned char* pIn, unsigned char* pOut, const int Xres, const int Yres, const int BpL);

// We should probably extern these since they are assembly.
extern "C" {

extern UINT32 HQX_LUT16to32[65536];
extern UINT32 HQX_RGBtoYUV[65536];

extern void hq2x_x86(unsigned char*, unsigned char*, UINT32, UINT32, UINT32);
extern void hq3x_x86(unsigned char*, unsigned char*, UINT32, UINT32, UINT32);
extern void hq4x_x86(unsigned char*, unsigned char*, UINT32, UINT32, UINT32);

} // extern "C"

#endif // !Video__HQX_hpp__included
