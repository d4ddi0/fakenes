/* FakeNES - A free, portable, Open Source NES emulator.

   common.h: Global common definitions.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED
#include <allegro.h>
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

#undef TRUE
#define TRUE   1
#undef FALSE
#define FALSE  0

#undef NULL
#define NULL   0

#define ROUND(x)  (x + 0.5)

#define M_PI      3.14159265358979323846

#define EPSILON   (1.0f / 256.0f)

/* TODO: Remove all references to NIL and correct compiler warnings. */
#define NIL    0

int disable_gui;  /* From main.c, should probably be moved into
                     gui.c/gui.h and be made BOOL. */

static INLINE int fix (int value, int base, int limit)
{
   if (value < base)
      value = base;
   if (value > limit)
      value = limit;

   return (value);
}

static INLINE REAL fixf (REAL value, REAL base, REAL limit)
{
   if (value < base)
      value = base;
   if (value > limit)
      value = limit;

   return (value);
}

#ifdef __cplusplus
}
#endif
#endif   /* !COMMON_H_INCLUDED */
