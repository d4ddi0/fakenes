/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef PLATFORM__LOAD_H__INCLUDED
#define PLATFORM__LOAD_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL nsf_is_loaded, rom_is_loaded;
extern BOOL file_is_loaded;

extern const UDATA* load_file(const UDATA *filename);
extern void close_file(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !PLATFORM__LOAD_H__INCLUDED */
