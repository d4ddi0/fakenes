/* FakeNES - A free, portable, Open Source NES emulator.

   config.h: Declarations for configuration wrapper.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void load_config (void);
extern void save_config (void);
extern BOOL get_config_bool(const char* section, const char* name, BOOL default_value);
extern void set_config_bool(const char* section, const char* name, BOOL value);

#ifdef __cplusplus
}
#endif
#endif /* !CONFIG_H_INCLUDED */

