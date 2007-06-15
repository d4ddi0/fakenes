/* FakeNES - A free, portable, Open Source NES emulator.

   audiolib.h: Declarations for the audio library.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef AUDIOLIB_H_INCLUDED
#define AUDIOLIB_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern int audiolib_init(void);
extern void audiolib_exit(void);
extern int audiolib_open_stream(void);
extern void audiolib_close_stream(void);
extern void *audiolib_get_buffer(void* buffer);
extern void audiolib_free_buffer(void* buffer);
extern void audiolib_suspend(void);
extern void audiolib_resume(void);

#ifdef __cplusplus
}
#endif
#endif /* !AUDIOLIB_H_INCLUDED */
