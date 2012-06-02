/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef COMMON__MATH_H__INCLUDED
#define COMMON__MATH_H__INCLUDED
#include "Common/Global.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Note that the C version of this macro does not cast to integer. */
#define ROUND(_VALUE) ( (_VALUE) + 0.5 )

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* <+KittyCat> $ grep EPSILON include/3dobject.h
   <+KittyCat> #define EPSILON (1.0f/1024.0f) */
#define EPSILON ( 1.0 / 1024.0 )

#define MIN2(_A, _B)     ( ((_A) < (_B)) ? (_A) : (_B) )
#define MAX2(_A, _B)     ( ((_A) < (_B)) ? (_A) : (_B) )

#define MIN3(_A, _B, _C) ( MIN2( (_A), MIN2((_B), (_C)) ) )
#define MAX3(_A, _B, _C) ( MAX2( (_A), MAX2((_B), (_C)) ) )

/* Macro to compare 2 REALs. */
#define COMPARE_TWO_REALS(a, b)  \
   ( TRUE_OR_FALSE( ((a) >= ((b) - EPSILON)) && ((a) <= ((b) + EPSILON)) ) )

#ifdef __cplusplus
} // extern "C"

#define Epsilon (EPSILON)

template<typename TYPE>
constant_function TYPE Round(const TYPE value) {
   return (TYPE)ROUND(value);
}

template<typename TYPE>
constant_function TYPE Minimum(const TYPE a, const TYPE b) {
   return MIN2(a, b);
}

template<typename TYPE>
constant_function TYPE Maximum(const TYPE a, const TYPE b) {
   return MAX2(a, b);
}

#endif /* __cplusplus */
#endif /* !COMMON__MATH_H__INCLUDED */
