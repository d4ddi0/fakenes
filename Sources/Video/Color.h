/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef VIDEO__COLOR_H__INCLUDED
#define VIDEO__COLOR_H__INCLUDED
#include "Common/Global.h"
#include "Common/Inline.h"
#include "Common/Types.h"
#include "Video/Internals.h"
#ifdef __cplusplus
extern "C" {
#endif

#define RED_MASK_16        0x1F
#define GREEN_MASK_16      0x3F
#define BLUE_MASK_16       0x1F

#define RED_SHIFTS_16      3
#define GREEN_SHIFTS_16    2
#define BLUE_SHIFTS_16     3

#define RED_CMASK_16       0xF800
#define GREEN_CMASK_16     0x07E0
#define BLUE_CMASK_16      0X001F

#define RED_CSHIFTS_16     11
#define GREEN_CSHIFTS_16   5
#define BLUE_CSHIFTS_16    0

EXPRESS_FUNCTION CONSTANT_FUNCTION UINT16 color_mix_16(const UINT16 c1, const UINT16 c2)
{
   return ((((c1 & RED_CMASK_16) + (c2 & RED_CMASK_16)) >> 1) & RED_CMASK_16) |
          ((((c1 & GREEN_CMASK_16) + (c2 & GREEN_CMASK_16)) >> 1) & GREEN_CMASK_16) |
          ((((c1 & BLUE_CMASK_16) + (c2 & BLUE_CMASK_16)) >> 1) & BLUE_CMASK_16);
}

EXPRESS_FUNCTION CONSTANT_FUNCTION UINT16 color_mix_16_fast(const UINT16 c1, const UINT16 c2)
{
   return (c1 + c2) >> 1;
}

EXPRESS_FUNCTION CONSTANT_FUNCTION UINT16 _color_pack_16(const int r, const int g, const int b)
{
   return ((r >> RED_SHIFTS_16) << RED_CSHIFTS_16) |
          ((g >> GREEN_SHIFTS_16) << GREEN_CSHIFTS_16) |
          ((b >> BLUE_SHIFTS_16) << BLUE_CSHIFTS_16);
}

EXPRESS_FUNCTION CONSTANT_FUNCTION void _color_unpack_16(const UINT16 c, int *r, int *g, int *b)
{
   *r = ((c & RED_CMASK_16) >> RED_CSHIFTS_16) << RED_SHIFTS_16;
   *g = ((c & GREEN_CMASK_16) >> GREEN_CSHIFTS_16) << GREEN_SHIFTS_16;
   *b = ((c & BLUE_CMASK_16) >> BLUE_CSHIFTS_16) << BLUE_SHIFTS_16;
}

EXPRESS_FUNCTION PURE_FUNCTION UINT16 color_pack_16(const int r, const int g, const int b)
{
   return video__swap_rgb ?
      _color_pack_16(b, g, r) : _color_pack_16(r, g, b);

}

EXPRESS_FUNCTION PURE_FUNCTION void color_unpack_16(const UINT16 c, int *r, int *g, int *b)
{
   if(video__swap_rgb)
      _color_unpack_16(c, b, g, r);
   else
      _color_unpack_16(c, r, g, b);
}

EXPRESS_FUNCTION CONSTANT_FUNCTION UINT16 color_add_16(const UINT16 c1, const UINT16 c2)
{
   const UINT16 lsb_mask = 0x821;
   const UINT16 msb_mask = 0x8410;
   UINT16 low_bits, dest, rb_sat, g_sat;

   low_bits = (c1 & lsb_mask) + (c2 & lsb_mask);
   dest = (c1 & ~lsb_mask) + (c2 & ~lsb_mask) + (low_bits & ~lsb_mask);
   dest ^= msb_mask * 2;
   rb_sat = (((dest & 0x10020) - 0x801) >> 5) & 0xF81F;
   g_sat = (((dest & 0x800) - 1) >>  6) & (0x3F <<  5);
   dest = ((dest & ~lsb_mask) + (low_bits & lsb_mask)) | rb_sat | g_sat;

   return dest;
}

/* Meh, this is slow, but oh well. */
EXPRESS_FUNCTION PURE_FUNCTION UINT16 color_brighten_16(const UINT16 c, const int amount)
{
   int r, g, b;
   color_unpack_16(c, &r, &g, &b);

   r += amount;
   g += amount;
   b += amount;
   if(r > 255) r = 255;
   if(g > 255) g = 255;
   if(b > 255) b = 255;

   return color_pack_16(r, g, b);
}

EXPRESS_FUNCTION PURE_FUNCTION UINT16 color_darken_16(const UINT16 c, const int amount)
{
   int r, g, b;
   color_unpack_16(c, &r, &g, &b);

   r -= amount;
   g -= amount;
   b -= amount;
   if(r < 0) r = 0;
   if(g < 0) g = 0;
   if(b < 0) b = 0;

   return color_pack_16(r, g, b);
}

/* Functions from 'Color.cpp'. */
extern void rgb_to_hsl(const int r, const int g, const int b, REAL *h, REAL *s, REAL *l);
extern void hsl_to_rgb(const REAL h, const REAL s, const REAL l, int* r, int* g, int* b);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !VIDEO__COLOR_H__INCLUDED */
