/* FakeNES - A free, portable, Open Source NES emulator.

   main.h: Declarations for main loop.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL want_exit;
extern BOOL want_gui;

extern void main_load_config(void);
extern void main_save_config(void);

#ifdef __cplusplus
}
#endif
#endif   /* !MAIN_H_INCLUDED */

