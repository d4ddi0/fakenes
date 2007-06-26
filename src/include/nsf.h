/* FakeNES - A free, portable, Open Source NES emulator.

   nsf.h: Declarations for the NSF player.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef NSF_H_INCLUDED
#define NSF_H_INCLUDED
#include "common.h"
#include "mmc.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL nsf_open(const UCHAR* filename);
extern void nsf_close(void);
extern void nsf_main(void);

extern const MMC nsf_mapper;

#ifdef __cplusplus
}
#endif
#endif   /* !NSF_H_INCLUDED */

