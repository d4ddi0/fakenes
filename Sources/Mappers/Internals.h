/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef MAPPERS__INTERNALS_H__INCLUDED
#define MAPPERS__INTERNALS_H__INCLUDED
#include "Common/Global.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MMC_PSEUDO_CLOCKS_PER_SCANLINE 114
#define MMC_LAST_PSEUDO_SCANLINE       342

/* These are defined in 16k pages. */
#define MMC_FIRST_ROM_BLOCK   0
#define MMC_LAST_ROM_BLOCK    (ROM_PRG_ROM_PAGES - 1)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MAPPERS__INTERNALS_H__INCLUDED */
