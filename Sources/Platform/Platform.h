/* FakeNES - A free, portable, Open Source NES emulator.

   load.h: Declarations for the file loader.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef PLATFORM__PLATFORM_H__INCLUDED
#define PLATFORM__PLATFORM_H__INCLUDED
#include "Common/Global.h"
#ifdef __cplusplus
extern "C" {
#endif

extern int platform_init(void);
extern void platform_exit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !PLATFORM__PLATFORM_H__INCLUDED */
