

/*

FakeNES - A portable, Open Source NES emulator.

crc32.h: Declarations for the CRC32 calculation.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/

#ifndef __CRC32_H__
#define __CRC32_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


UINT32 crc32_calculate (const UINT8 *buffer, unsigned len);


#ifdef __cplusplus
}
#endif

#endif /* ! __CRC32_H__ */
