/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef COMMON__COMMON_H__INCLUDED
#define COMMON__COMMON_H__INCLUDED
/* Allegro defines some things such as INLINE. However, in the future we won't use it. */
#include <allegro.h>
#ifdef __cplusplus
extern "C" {
#endif

/* For legacy reasons, these are forced to 0 and 1 for when they are saved to a file.
   However, these days LOAD_BOOLEAN and SAVE_BOOLEAN should be used instead. */
#undef TRUE
#undef FALSE
#define TRUE  1
#define FALSE 0

/* This resolves a boolean condition to an integral value. */
#define TRUE_OR_FALSE(_CONDITION) ( (_CONDITION) ? TRUE : FALSE )
#define ZERO_OR_ONE(_CONDITION) ( (_CONDITION) ? 1 : 0 )

#define LOAD_BOOLEAN(_INTEGER) ( TRUE_OR_FALSE(_INTEGER) )
#define SAVE_BOOLEAN(_BOOLEAN) ( ZERO_OR_ONE(_BOOLEAN) )

/* Usually NULL is defined as (void*)0, however it is often more useful for it to just
   equal zero, since it is compatible with pointers either way. */
#undef NULL
#define NULL 0

/* For GCC, we can force it to always inline. Without this, it would refuse to inline
   functions which are too large. Note that normally Allegro defines INLINE, so we
   have to undefine it before redefining it. */
#ifdef __GNUC__
#   undef INLINE
#   define INLINE __attribute__((always_inline)) inline
#else
#   ifndef INLINE
#      define INLINE inline
#   endif
#endif

/* The LINEAR macro is used for when a function is only ever called once. */
#define LINEAR INLINE
/* The QUICK macro serves as a shortcut for a function that must run as fast as
   possible. Generally this equates to a static inline function. */
#define QUICK static INLINE

#ifdef __cplusplus
} // extern "C"

#define true  (TRUE)
#define false (FALSE)

#define null (NULL)

/* Note that the 'inline' keyword should never be used, as it can cause problems due to
   the way macro expansion is handled with INLINE. */
#define forceinline INLINE

#define linear (LINEAR)
#define quick (QUICK)

#endif /* __cplusplus */
#endif /* !COMMON__COMMON_H__INCLUDED */
