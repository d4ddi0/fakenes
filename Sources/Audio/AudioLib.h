/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef AUDIO__AUDIOLIB_H__INCLUDED
#define AUDIO__AUDIOLIB_H__INCLUDED
#include "Common/Global.h"
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
#endif /* __cplusplus */
#endif /* !AUDIO__AUDIOLIB_H__INCLUDED */
