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

#define TRUE_OR_FALSE(x)   (x ? TRUE : FALSE)

#undef NULL
#define NULL   0

#define ROUND(x)  (x + 0.5)

#define M_PI      3.14159265358979323846

#define EPSILON   (1.0f / 256.0f)

/* TODO: Remove all references to NIL and correct compiler warnings. */
#define NIL    0

int disable_gui;  /* From main.c, should probably be moved into
                     gui.c/gui.h and be made BOOL. */

int saved_argc;
char **saved_argv;   /* Saved from main(), needed for ALUT. */

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

#define RAND32_MAX   0xffffffff

static INLINE unsigned rand32 (void)
{
   unsigned value;

   /* Kludge for rand() only returning at most 32767 (0x7fff) on some
      systems (such as Windows). */

   value = ((rand () & 0x7fff) << 17);
   value |= ((rand () & 0x7fff) << 2);
   value |= (rand () & 0x4);
   value &= RAND32_MAX;

   return (value);
}

#ifdef __cplusplus
}
#endif
#endif   /* !COMMON_H_INCLUDED */
