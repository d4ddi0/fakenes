/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   crc32.c: CRC32 calculation routines.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include "common.h"
#include "crc32.h"
#include "debug.h"
#include "types.h"

static BOOL initialized = FALSE;
static const unsigned long crc32_seed = 0xffffffff;
static unsigned long crc32_table[256];

static void crc32_init (void)
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

   initialized = TRUE;
}

static INLINE unsigned long crc32_update (unsigned char value, unsigned long
   crc32)
{
   return ((crc32_table[((crc32 ^ value) & 0xff)] ^ ((crc32 >> 8) &
      0x00ffffff)));
}

static INLINE unsigned long crc32_start (void)
{
   return (crc32_seed);
}

static INLINE unsigned long crc32_end (unsigned long crc32)
{
   return ((crc32 ^ crc32_seed));
}

unsigned long crc32 (const unsigned char *buffer, unsigned long size)
{
   unsigned long crc32;
   unsigned long offset;

   RT_ASSERT(buffer);

   if (!initialized)
      crc32_init ();

   crc32 = crc32_start ();

   for (offset = 0; offset < size; offset++)
      crc32 = crc32_update (buffer[offset], crc32);

   crc32 = crc32_end (crc32);

   return (crc32);
}
