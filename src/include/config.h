/* FakeNES - A free, portable, Open Source NES emulator.

   config.h: Declarations for configuration wrapper.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif

void load_config (void);
void save_config (void);

#ifdef __cplusplus
}
#endif
#endif   /* !CONFIG_H_INCLUDED */

