/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef COMMON__TYPES_H__INCLUDED
#define COMMON__TYPES_H__INCLUDED
/* Allegro defines some types. However, in the future we won't use it. */
#include <allegro.h>
#ifdef ALLEGRO_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <winalleg.h>
#endif
#ifdef C99_TYPES
#include <stdint.h>
#endif
#include <stddef.h> /* For size_t. */
#include <stdlib.h> /* For size_t. */
#include <string.h>
#include "Common/Global.h"
#include "Common/Inline.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Base types. */
#ifdef C99_TYPES
   typedef uint8_t fakenes_uint8_t;
   typedef int8_t fakenes_int8_t;
#else
   typedef unsigned char fakenes_uint8_t;
   typedef signed char fakenes_int8_t;
#endif

#ifdef C99_TYPES
   typedef uint16_t fakenes_uint16_t;
   typedef int16_t fakenes_int16_t;
#elif SIZEOF_SHORT == 2
   typedef unsigned short fakenes_uint16_t;
   typedef signed short fakenes_int16_t;
#elif SIZEOF_INT == 2
   typedef unsigned int fakenes_uint16_t;
   typedef signed int fakenes_int16_t;
#else                             
#  error No 16-bit type could be found.
#endif

#ifdef C99_TYPES
   typedef uint32_t fakenes_uint32_t;
   typedef int32_t fakenes_int32_t;
#elif SIZEOF_INT == 4
   typedef unsigned int fakenes_uint32_t;
   typedef signed int fakenes_int32_t;
#elif SIZEOF_LONG == 4
   typedef unsigned long fakenes_uint32_t;
   typedef signed long fakenes_int32_t;
#else
#  error No 32-bit type could be found.
#endif

#ifdef C99_TYPES
   typedef uint64_t fakenes_uint64_t;
   typedef int64_t fakenes_int64_t;
#elif SIZEOF_INT == 8
   typedef unsigned int fakenes_uint64_t;
   typedef signed int fakenes_int64_t;
#elif SIZEOF_LONG == 8
   typedef unsigned long fakenes_uint64_t;
   typedef signed long fakenes_int64_t;
#elif SIZEOF_LONG_LONG == 8
   typedef unsigned long long fakenes_uint64_t;
   typedef signed long long fakenes_int64_t;
#else
#  error No 64-bit type could be found.
#endif

typedef signed char     fakenes_bool_t;   /* Boolean value. */
typedef char   		fakenes_char_t;   /* ASCII character. */
typedef unsigned char 	fakenes_uchar_t;  /* Extended character. */
typedef int             fakenes_enum_t;   /* Enumeration index. */
typedef unsigned        fakenes_flags_t;  /* Flags. */
typedef fakenes_flags_t fakenes_list_t;   /* List of flags. */
typedef double          fakenes_real_t;   /* Real number. */
typedef size_t		fakenes_size_t;   /* Size or offset. */
typedef unsigned int    fakenes_uint_t;   /* Unsigned integer, unsized. */

/* Pair data type for CPU core. */
typedef union
{
   struct
   {
#ifdef LSB_FIRST
      fakenes_uint8_t low, high;
#else
      fakenes_uint8_t high, low;
#endif
   } bytes;

   fakenes_uint16_t word;

} fakenes_pair_t;

/* Shorthand aliases. */
/* typedef where possible, otherwise #define. */
typedef fakenes_uint_t UINT;

typedef fakenes_uint8_t UINT8;
typedef fakenes_int8_t INT8;
typedef fakenes_uint16_t UINT16;
typedef fakenes_int16_t INT16;

#ifdef SYSTEM_WINDOWS
   /* Override Win32 typedefs. */
#  define UINT32 fakenes_uint32_t
#  define INT32 fakenes_int32_t
#  define UINT64 fakenes_uint64_t
#  define INT64 fakenes_int64_t
#  define BOOL fakenes_bool_t
#  define SIZE fakenes_size_t
#else
   typedef fakenes_uint32_t UINT32;
   typedef fakenes_int32_t INT32;
   typedef fakenes_uint64_t UINT64;
   typedef fakenes_int64_t INT64;
   typedef fakenes_bool_t BOOL;
   typedef fakenes_size_t SIZE;
#endif

typedef fakenes_char_t CHAR;
typedef fakenes_uchar_t UCHAR;
typedef fakenes_enum_t ENUM;
typedef fakenes_flags_t FLAGS;
typedef fakenes_list_t LIST;
typedef fakenes_pair_t PAIR;
typedef fakenes_real_t REAL;

#define LOAD_BOOLEAN(_INTEGER) ( TRUE_OR_FALSE(_INTEGER) )
#define SAVE_BOOLEAN(_BOOLEAN) ( ZERO_OR_ONE(_BOOLEAN) )

/* List access macros. */
#define LIST_ADD(_LIST, _FLAGS)       	( (_LIST) |= (_FLAGS) )
#define LIST_REMOVE(_LIST, _FLAGS)    	( (_LIST) &= ~(_FLAGS) )
#define LIST_TOGGLE(_LIST, _FLAGS)    	( (_LIST) ^= (_FLAGS) )
#define LIST_COMPARE(_LIST, _FLAGS)	( TRUE_OR_FALSE( (_LIST) & (_FLAGS) ) )

#define LIST_SET(_LIST, _FLAGS, _SET) { \
   LIST_REMOVE( (_LIST), (_FLAGS) ); \
   if( (_SET) ) \
      LIST_ADD( (_LIST), (_FLAGS) ); \
}

EXPRESS_FUNCTION CONSTANT_FUNCTION int fix(int value, const int base, const int limit) {
   if(value < base)
      value = base;
   if(value > limit)
      value = limit;

   return value;
}

EXPRESS_FUNCTION CONSTANT_FUNCTION REAL fixf(REAL value, const REAL base, const REAL limit) {
   if(value < base)
      value = base;
   if(value > limit)
      value = limit;

   return value;
}

#ifdef __cplusplus
} // extern "C"

/* Lowercase C++ style aliases. This generally only includes the integer data types,
   abstract floating point data type (real), and the pair data type. Almost
   everything else, such as bool, is already provided by C++. */

typedef fakenes_uint8_t uint8;
typedef fakenes_int8_t int8;
typedef fakenes_uint16_t uint16;
typedef fakenes_int16_t int16;
typedef fakenes_uint32_t uint32;
typedef fakenes_int32_t int32;

typedef fakenes_pair_t byte_pair;
typedef fakenes_enum_t enum_type;
typedef fakenes_real_t real;
/* Sizes in the C++ portions of the program are refered to as size_type.
   This is because 'size' is often used for variable names. */
typedef fakenes_size_t size_type;
typedef fakenes_uint_t uint;

// This should be used instead of fix/fixf() when possible.
template<typename TYPE>
constant_function TYPE Clamp(TYPE value, const TYPE minimum, const TYPE maximum) {
   if( value < minimum )
      value = minimum;
   if( value > maximum )
      value = maximum;

   return value;
}

#endif /* __cplusplus */
#endif /* !COMMON__TYPES_H__INCLUDED */
