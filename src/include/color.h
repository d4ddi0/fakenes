/* FakeNES - A free, portable, Open Source NES emulator.

   color.h: Fast routines for color management.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef COLOR_H_INCLUDED
#define COLOR_H_INCLUDED
#include "common.h"
#include "types.h"
#include "video.h"
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

static INLINE UINT16 color_mix_16(const UINT16 c1, const UINT16 c2)
{
   return ((((c1 & RED_CMASK_16) + (c2 & RED_CMASK_16)) >> 1) & RED_CMASK_16) |
          ((((c1 & GREEN_CMASK_16) + (c2 & GREEN_CMASK_16)) >> 1) & GREEN_CMASK_16) |
          ((((c1 & BLUE_CMASK_16) + (c2 & BLUE_CMASK_16)) >> 1) & BLUE_CMASK_16);
}

static INLINE UINT16 color_mix_16_fast(const UINT16 c1, const UINT16 c2)
{
   return (c1 + c2) >> 1;
}

static INLINE UINT16 _color_pack_16(const int r, const int g, const int b)
{
   return ((r >> RED_SHIFTS_16) << RED_CSHIFTS_16) |
          ((g >> GREEN_SHIFTS_16) << GREEN_CSHIFTS_16) |
          ((b >> BLUE_SHIFTS_16) << BLUE_CSHIFTS_16);
}

static INLINE void _color_unpack_16(const UINT16 c, int *r, int *g, int *b)
{
   *r = ((c & RED_CMASK_16) >> RED_CSHIFTS_16) << RED_SHIFTS_16;
   *g = ((c & GREEN_CMASK_16) >> GREEN_CSHIFTS_16) << GREEN_SHIFTS_16;
   *b = ((c & BLUE_CMASK_16) >> BLUE_CSHIFTS_16) << BLUE_SHIFTS_16;
}

static INLINE UINT16 color_pack_16(const int r, const int g, const int b)
{
   return video__swap_rgb ?
      _color_pack_16(b, g, r) : _color_pack_16(r, g, b);

}

static INLINE void color_unpack_16(const UINT16 c, int *r, int *g, int *b)
{
   if(video__swap_rgb)
      _color_unpack_16(c, b, g, r);
   else
      _color_unpack_16(c, r, g, b);
}

static INLINE UINT16 color_add_16(const UINT16 c1, const UINT16 c2)
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
static INLINE UINT16 color_brighten_16(const UINT16 c, const int amount)
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

static INLINE UINT16 color_darken_16(const UINT16 c, const int amount)
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

#ifdef __cplusplus
}
#endif
#endif   /* !COLOR_H_INCLUDED */
