/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef COMMON__DEBUG_H__INCLUDED
#define COMMON__DEBUG_H__INCLUDED
/* We need Allegro.h for allegro_message(). */
#include <allegro.h>
#include <stdlib.h>
#include "Common.h"
#include "Platform/Log.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#   define DEBUG_PRINTF log_printf
#else
#   define DEBUG_PRINTF
#endif

/* Warning macros to help with debugging. */
#define WARN(message) { \
   allegro_message("WARNING\n\n" message "\n\nat line %d of %s", __LINE__, __FILE__); \
   log_printf("\nWarning: " message " (line %d, %s)\n", __LINE__, __FILE__); \
}

#define WARN_GENERIC() \
   WARN("***Possible code fault***\nPlease report this to the developers.")

#define WARN_BREAK(message) { \
   WARN(message); \
   return; \
}

#define WARN_BREAK_GENERIC() \
   WARN_BREAK("***Possible code fault***\nPlease report this to the developers.")

#define RT_ASSERT(_CONDITION) { \
   if( !(_CONDITION) ) { \
      WARN("***Runtime assertion error***\nPlease report this to the developers."); \
      exit(-1); \
   } \
}

#define SAFEGUARD(_GUARD) RT_ASSERT((_GUARD))

#ifdef __cplusplus
#   define Safeguard SAFEGUARD
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !COMMON__DEBUG_H__INCLUDED */
