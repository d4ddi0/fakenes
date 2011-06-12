/* FakeNES - A free, portable, Open Source NES emulator.

   load.h: Declarations for the file loader.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef LOAD_H_INCLUDED
#define LOAD_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL nsf_is_loaded, rom_is_loaded;
extern BOOL file_is_loaded;

extern const UCHAR* load_file(const UCHAR *filename);
extern void close_file(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !LOAD_H_INCLUDED */
