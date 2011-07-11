/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef PLATFORM__CONFIG_H__INCLUDED
#define PLATFORM__CONFIG_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void load_config(void);
extern void save_config(void);
extern BOOL get_config_bool(const char* section, const char* name, BOOL default_value);
extern void set_config_bool(const char* section, const char* name, BOOL value);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !PLATFORM__CONFIG_H__INCLUDED */
