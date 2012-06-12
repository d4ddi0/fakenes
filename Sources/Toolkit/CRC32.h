/* FakeNES - A portable, Open Source NES and Famicom emulator.
 * Copyright © 2011-2012 Digital Carat Group
 *
 * This is free software. See 'License.txt' for additional copyright and
 * licensing information. You must read and accept the license prior to any
 * modification or use of this software.
 */
#ifndef TOOLKIT__CRC32_H__INCLUDED
#define TOOLKIT__CRC32_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern UINT32	crc32_start(void);
extern void	crc32_end(UINT32* crc32);
extern void	crc32_update(UINT32* crc32, const UINT8 data);
extern UINT32	calculate_crc32(const void* buffer, const SIZE size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !TOOLKIT__CRC32_H__INCLUDED */
