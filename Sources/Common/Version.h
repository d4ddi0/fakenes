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

#define VERSION_MAJOR   6
#define VERSION_MINOR   0
#define VERSION         0x600

/* Version tag can be overriden by Makefiles. */
#ifndef VERSION_TAG
#define VERSION_TAG     "Unstable"
#endif

#define VERSION_STRING  "6.0 (" VERSION_TAG ")"

#define WINDOW_TITLE_NORMAL	"FakeNES 6.0 \"Fireflower\" (Unstable)"
#define WINDOW_TITLE_FILE	"FakeNES"

#ifdef __cplusplus
}
#endif
#endif   /* !VERSION_H_INCLUDED */
