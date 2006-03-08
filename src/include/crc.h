/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   crc.c: Declarations for CRC calculation routines.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef CRC_H_INCLUDED
#define CRC_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

unsigned long build_crc32 (const unsigned char *, unsigned long);

#ifdef __cplusplus
}
#endif
#endif   /* !CRC_H_INCLUDED */
