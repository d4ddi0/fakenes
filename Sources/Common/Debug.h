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
#include <string.h>
#include "Common/Global.h"
#include "Platform/Log.h"
#include "Toolkit/Unicode.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef EANBLE_DEBUG
#   define DEBUG_PRINTF log_printf
#else
#   define DEBUG_PRINTF
#endif

/* Warning macros to help with debugging. */
#define WARN(message) { \
   const char* format = "WARNING\n\n" message "\n\nat line %d of %s"; \
   allegro_message(format, __LINE__, __FILE__); \
   \
   UTF_STRING* converted = create_utf_string_from_data(UNICODE_FORMAT_FASTEST, \
      (const UTF_DATA*)format, UNICODE_FORMAT_ASCII, strlen(format)); \
   \
   log_printf(converted, __LINE__, __FILE__); \
   delete_utf_string(converted); \
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
#   define Warning		WARN
#   define GenericWarning	WARN_GENERIC
#   define Safeguard		SAFEGUARD
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !COMMON__DEBUG_H__INCLUDED */
