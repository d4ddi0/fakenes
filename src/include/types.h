/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   types.h: Portable type definitions.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED
#include <string.h>
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Base types. */
typedef unsigned char UINT8;
typedef signed char INT8;

#ifdef POSIX

#  include <config.h>

#  if SIZEOF_SHORT_INT == 2
      typedef unsigned short int UINT16;
      typedef signed short int INT16;
#  elif SIZEOF_INT == 2
      typedef unsigned int UINT16;
      typedef signed int INT16;
#  else
#     error No 16-bit type could be found.
#  endif

#  if SIZEOF_INT == 4
      typedef unsigned int UINT32;
      typedef signed int INT32;
#  elif SIZEOF_LONG_INT == 4
      typedef unsigned long int UINT32;
      typedef signed long int INT32;
#  else
#     error No 32-bit type could be found.
#  endif

#else /* POSIX */

/* Use defaults. */
typedef unsigned short int UINT16;
typedef signed short int INT16;
typedef unsigned int UINT32;
typedef signed int INT32;

#endif /* POSIX */

/* Extended data types. */
typedef union
{
   struct
   {
#ifdef LSB_FIRST
      UINT8 low, high;
#else
      UINT8 high, low;
#endif

   } bytes;

   UINT16 word;

} PAIR;

typedef signed char BOOL;  /* Boolean value. */
typedef int ENUM;          /* Enumeration index. */
typedef unsigned LIST;     /* List. */
typedef float REAL;        /* Real number. */
typedef char CHAR;         /* ASCII character. */
typedef char UCHAR;        /* Unicode character. */

typedef LIST FLAGS;        /* List of flags. */

/* String data types. */
#define STRING_SIZE_BASE   1024  /* Typical size. */
#define STRING_SIZE        (STRING_SIZE_BASE * sizeof (CHAR))
#define USTRING_SIZE       (STRING_SIZE_BASE * sizeof (UCHAR))
typedef CHAR STRING[STRING_SIZE_BASE];    /* ASCII string. */
typedef UCHAR USTRING[STRING_SIZE_BASE];  /* Unicode string. */

/* String clearing macros. */
#define STRING_CLEAR_SIZE(str, size)   memset (str, 0, size)
#define USTRING_CLEAR_SIZE             STRING_CLEAR_SIZE
#define STRING_CLEAR(str)  STRING_CLEAR_SIZE(str, STRING_SIZE)
#define USTRING_CLEAR(str) USTRING_CLEAR_SIZE(str, USTRING_SIZE)

#ifdef __cplusplus
}
#endif
#endif   /* !TYPES_H_INCLUDED */
