#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED
#include "misc.h"
#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Move stuff from misc.h into here and get this file included by
   everything. */

#undef TRUE
#define TRUE   1
#undef FALSE
#define FALSE  0

#undef NULL
#define NULL   0

#define ROUND(x)  (x + 0.5)

#ifdef __cplusplus
}
#endif
#endif   /* !COMMON_H_INCLUDED */
