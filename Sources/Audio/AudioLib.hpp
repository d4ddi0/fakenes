/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Audio__AudioLib_hpp__included
#define Audio__AudioLib_hpp__included
#include "Local.hpp"

extern int audiolib_init(void);
extern void audiolib_exit(void);
extern int audiolib_open_stream(void);
extern void audiolib_close_stream(void);
extern void *audiolib_get_buffer(void* buffer);
extern void audiolib_free_buffer(void* buffer);
extern void audiolib_suspend(void);
extern void audiolib_resume(void);

#endif // !Audio__AudioLib_hpp__included
