/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef COMMON__INLINE_H__INCLUDED
#define COMMON__INLINE_H__INCLUDED
/* Allegro defines some things such as INLINE. However, in the future we won't use it. */
#include <allegro.h>
#ifdef __cplusplus
extern "C" {
#endif

/* The INLINE macro insists that a function be inlined, but does not require it.
   It is ultimately the decision of the compiler to do so.

   The FORCE_INLINE macro forces a function to always be inlined whenever
   possible, unless optimizations are disabled. Functions beyond the
   compiler's inline limit (of code size) will still not inline.

   The EXTERN_INLINE usualy resolves to extern inline. This isn't supported by
   every compiler or standard so it may not be defined to anything. It provides
   external linkage to inline functions, even if they would normally be static.
   This can be used to pass an inline function as a template argument.

   The DISCRETE_FUNCTION macro is used for when a function is (without a doubt)
   only ever called once. It is guaranteed to always be inlined into its parent
   whenever possible. It is functionally identical to FORCE_INLINE, but helps
   with hand-optimizations. Note that the compiler may automatically inline
   functions that are only ever invoked once.

   The EXPRESS_FUNCTION macro serves as a shortcut for a micro-function that
   must run as fast as possible. It is also always implicitly static. This
   should not be used for large functions, or functions with more than a few
   arguments. If speed is not terribly critical, either INLINE or FORCE_INLINE
   should be used instead.

   The KERNEL_FUNCTION macro tells the compiler that a function is called
   very frequently, allowing it to be better optimized. It will not be inlined
   by default, as that counteracts cache optimizations. Which functions are
   suitable for kernelling will be determined by a profiler.

   The LINEAR_FUNCTION macro defines a function as being entirely flat; all of
   the functions invoked from within it will be inlined if possible. The
   function itself will not always be inlined by default.
*/
#if defined(ENABLE_DEBUG) || defined(ENABLE_PROFILE)
/* Disable all optimizations in debug or profiling mode. */
#   define INLINE
#   define FORCE_INLINE
#   define EXTERN_INLINE	extern

#   define DISCRETE_FUNCTION
#   define EXPRESS_FUNCTION	static
#   define LINEAR_FUNCTION
#   define KERNEL_FUNCTION
#else
/* Since we support C99 only, 'inline' should always work properly. */
#   ifndef INLINE
#      define INLINE inline
#   endif

#   ifdef __GNUC__
#      define FORCE_INLINE	INLINE __attribute__((always_inline))
#      define KERNEL_FUNCTION	__attribute__((hot)) 
#      define LINEAR_FUNCTION	__attribute__((flatten)) 
#   else
#      define FORCE_INLINE	INLINE
#      define KERNEL_FUNCTION
#      define LINEAR_FUNCTION
#   endif

/* We require C99 by default, so this should always work. */
#   define EXTERN_INLINE	extern inline
#   define DISCRETE_FUNCTION	FORCE_INLINE
#   define EXPRESS_FUNCTION	static FORCE_INLINE

#endif /* !ENABLE_DEBUG && !ENABLE_PROFILE */

#ifdef __cplusplus
} // extern "C"

#define force_inline		FORCE_INLINE
#define extern_inline		EXTERN_INLINE

#define discrete_function	DISCRETE_FUNCTION
#define express_function	EXPRESS_FUNCTION
#define kernel_function		KERNEL_FUNCTION
#define linear_function		LINEAR_FUNCTION

#endif /* __cplusplus */
#endif /* !COMMON__INLINE_H__INCLUDED */
