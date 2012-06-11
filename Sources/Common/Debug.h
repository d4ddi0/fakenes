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
#define WARN(_MESSAGE) { \
   const char* _format = "WARNING\n\n" _MESSAGE "\n\nat line %d of %s"; \
   allegro_message(_format, __LINE__, __FILE__); \
   \
   UTF_STRING* _converted = create_utf_string_from_c_string(UNICODE_FORMAT_FASTEST, _format); \
   log_printf(_converted, __LINE__, __FILE__); \
   delete_utf_string(_converted); \
}

#define WARN_GENERIC() \
   WARN("***Possible code fault***\nPlease report this to the developers.")

#define WARN_BREAK(_MESSAGE) { \
   WARN(_MESSAGE); \
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

#define SAFEGUARD(_CONDITION) RT_ASSERT((_CONDITION))

#ifdef __cplusplus
#   define Warning		WARN
#   define GenericWarning	WARN_GENERIC
#   define Safeguard		SAFEGUARD
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !COMMON__DEBUG_H__INCLUDED */
