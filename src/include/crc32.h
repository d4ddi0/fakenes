/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   crc32.h: Declarations for the CRC32 calculation.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef CRC32_H_INCLUDED
#define CRC32_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

UINT32 crc32_calculate (const UINT8 *, unsigned);

#ifdef __cplusplus
}
#endif
#endif   /* !CRC32_H_INCLUDED */
