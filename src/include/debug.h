#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED
#include <allegro.h>
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Warning macros to help with debugging. */
#define WARN(message)   \
   (allegro_message ("WARNING\n\n" message "\n\nat line %d of %s",   \
      __LINE__, __FILE__))

#define WARN_GENERIC()  WARN("Possible code fault")

#define WARN_BREAK(message) { \
   WARN(message); \
   return;  \
}

#define WARN_BREAK_GENERIC()  WARN_BREAK("Possible code fault")

#define RT_ASSERT(cond) {  \
   if (!(cond)) { \
      WARN("Runtime assertion error"); \
      exit (-1);  \
   }  \
}

#ifdef __cplusplus
}
#endif
#endif   /* !DEBUG_H_INCLUDED */
