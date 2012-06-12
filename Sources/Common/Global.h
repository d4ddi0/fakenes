/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef COMMON__GLOBAL_H__INCLUDED
#define COMMON__GLOBAL_H__INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/* For legacy reasons, these are forced to 0 and 1 for when they are saved to a file.
   However, these days LOAD_BOOLEAN and SAVE_BOOLEAN should be used instead. */
#undef TRUE
#undef FALSE
#define TRUE	1
#define FALSE	0

/* This resolves a boolean condition to an integral value. */
#define TRUE_OR_FALSE(_CONDITION)	( (_CONDITION) ? TRUE : FALSE )
#define ZERO_OR_ONE(_CONDITION)		( (_CONDITION) ? 1 : 0 )

/* Usually NULL is defined as (void*)0, however it is often more useful for it to just
   equal zero, since it is compatible with pointers either way. */
#undef NULL
#define NULL	0
#define NULLPTR (void*)0

#ifdef __GNUC__
#   define CONSTANT_FUNCTION	__attribute__((const))
#   define PURE_FUNCTION	__attribute__((pure)) 
#else
#   define CONSTANT_FUNCTION
#   define PURE_FUNCTION
#endif

#define ARRAY_BASE(_ARRAY)		(&(_ARRAY)[0])
#define ARRAY_CAST(_ARRAY, _TYPE)	(((_TYPE))ARRAY_BASE((_ARRAY)))

#ifdef __cplusplus
} // extern "C"

#define true	(TRUE)
#define false	(FALSE)
#define null	(NULL)
#define nullptr	(NULLPTR)
 
#define constant_function	CONSTANT_FUNCTION
#define pure_function		PURE_FUNCTION

#define ArrayBase		ARRAY_BASE
#define ArrayCast		ARRAY_CAST

#endif /* __cplusplus */
#endif /* !COMMON__GLOBAL_H__INCLUDED */
