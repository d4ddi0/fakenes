/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef SYSTEM__MAIN_H__INCLUDED
#define SYSTEM__MAIN_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL want_exit;
extern BOOL want_gui;

extern void main_load_config(void);
extern void main_save_config(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !SYSTEM__MAIN_H__INCLUDED */

