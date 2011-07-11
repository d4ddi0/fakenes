/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef SYSTEM__REWINDER_H__INCLUDED
#define SYSTEM__REWINDER_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void rewind_load_config(void);
extern void rewind_save_config(void);
extern int rewind_init(void);
extern void rewind_exit(void);
extern void rewind_clear(void);
extern BOOL rewind_save_snapshot(void);
extern BOOL rewind_load_snapshot(void);
extern BOOL rewind_is_enabled(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !SYSTEM__REWINDER_H__INCLUDED */
