/* FakeNES - A free, portable, Open Source NES emulator.

   version.h: Global version definitions.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#define VERSION_MAJOR   0
#define VERSION_MINOR   60
#define VERSION         0x060

/* Version tag can be overriden by Makefiles. */
#ifndef VERSION_TAG
#define VERSION_TAG     "CVS"
#endif

#define VERSION_STRING  "0.6.0 (" VERSION_TAG ")"

#ifdef __cplusplus
}
#endif
#endif   /* !VERSION_H_INCLUDED */
