

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

misc.h: Global miscellaneous definitions.

Copyright (c) 2003, Randy McDowell.
Copyright (c) 2003, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef MISC_H_INCLUDED

#define MISC_H_INCLUDED


#include <stdio.h>


typedef unsigned char UINT8;

typedef signed char INT8;


#ifdef POSIX


#include <config.h>


#if SIZEOF_SHORT_INT == 2

typedef unsigned short int UINT16;

typedef signed short int INT16;

#elif SIZEOF_INT == 2

typedef unsigned int UINT16;

typedef signed int INT16;

#else

#error No 16-bit type could be found.

#endif


#if SIZEOF_INT == 4

typedef unsigned int UINT32;

typedef signed int INT32;

#elif SIZEOF_LONG_INT == 4

typedef unsigned long int UINT32;

typedef signed long int INT32;

#else

#error "No 32-bit type could be found."

#endif


#else


/* Use defaults. */

typedef unsigned short int UINT16;

typedef signed short int INT16;


typedef unsigned int UINT32;

typedef signed int INT32;


#endif /* POSIX */


typedef union
{
    struct
    {
#ifdef LSB_FIRST

        UINT8 low, high;
#else

        UINT8 high, low;
#endif
    }
    bytes;


    UINT16 word;

} PAIR;


#undef  TRUE

#define TRUE    1


#undef  FALSE

#define FALSE   (! TRUE)


/* NIL has replaced NULL. */

#define NIL     0


FILE * log_file;


#endif /* ! MISC_H_INCLUDED */
