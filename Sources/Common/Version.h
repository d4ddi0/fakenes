/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef COMMON__VERSION_H__INCLUDED
#define COMMON__VERSION_H__INCLUDED
#include "Common/Global.h"
#ifdef __cplusplus
extern "C" {
#endif

#define VERSION_MAJOR   1
#define VERSION_MINOR   0
#define VERSION         0x100

/* Version tag can be overriden by Makefiles. */
#ifndef VERSION_TAG
#define VERSION_TAG     "SVN"
#endif

#define VERSION_STRING  "1.0 (" VERSION_TAG ")"

#define WINDOW_TITLE_NORMAL	"FakeNES GT - Let the good times roll!"
#define WINDOW_TITLE_FILE	"FakeNES GT"

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !COMMON__VERSION_H__INCLUDED */
