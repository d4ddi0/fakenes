/* FakeNES - A free, portable, Open Source NES emulator.

   crc.c: Implementation of CRC calculation routines.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include "common.h"
#include "crc.h"
#include "debug.h"
#include "types.h"

/* --- CRC32 routines. --- */

static BOOL crc32_initialized = FALSE;
static const unsigned long crc32_seed = 0xffffffff;
static unsigned long crc32_table[256];

static INLINE void crc32_init (void)
{
   int index;

   for (index = 0; index < 256; index++)
   {
      unsigned long value = index;
      int bit;

      for (bit = 0; bit < 8; bit++)
      {
         if (value & 1)
            value = ((value >> 1) ^ 0xedb88320);
         else
            value >>= 1;
      }

      crc32_table[index] = value;
   }

   crc32_initialized = TRUE;
}

static INLINE unsigned long crc32_start (void)
{
   return (crc32_seed);
}

static INLINE unsigned long crc32_end (unsigned long crc)
{
   return ((crc ^ crc32_seed));
}

static INLINE unsigned long crc32_update (unsigned char value, unsigned long
   crc)
{
   return ((crc32_table[((crc ^ value) & 0xff)] ^ ((crc >> 8) &
      0x00ffffff)));
}

unsigned long build_crc32 (const unsigned char *buffer, unsigned long size)
{
   unsigned long crc;
   unsigned long offset;

   RT_ASSERT(buffer);

   if (!crc32_initialized)
      crc32_init ();

   crc = crc32_start ();

   for (offset = 0; offset < size; offset++)
      crc = crc32_update (buffer[offset], crc);

   crc = crc32_end (crc);

   return (crc);
}
