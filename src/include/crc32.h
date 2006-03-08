/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   crc32.c: Declarations for CRC32 calculation routines.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef CRC32_H_INCLUDED
#define CRC32_H_INCLUDED
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif

unsigned long crc32 (const unsigned char *, unsigned long);

#ifdef __cplusplus
}
#endif
#endif   /* !CRC32_H_INCLUDED */
