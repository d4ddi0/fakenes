#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED
#include <string.h>
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Move type stuff out of misc.h into here, and get this file included
   by everything. */

/* Extended data types. */
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
