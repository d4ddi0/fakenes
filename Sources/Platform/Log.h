/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef PLATFORM__LOG_H__INCLUDED
#define PLATFORM__LOG_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void log_open(const char*);
extern void log_close(void);
extern void log_printf(const UDATA*, ...);
extern UDATA* get_log_text(void);
extern void console_clear(void);
extern void console_printf(const UDATA*, ...);
extern UDATA* get_console_text(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !PLATFORM__LOG_H__INCLUDED */
