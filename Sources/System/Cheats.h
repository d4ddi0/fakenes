/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef SYSTEM__CHEATS_H__INCLUDED
#define SYSTEM__CHEATS_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern int cheats_decode(const UINT8 *code, UINT16 *address, UINT8 *value, UINT8 *match_value);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !SYSTEM__CHEATS_H__INCLUDED */
