

#define UINT8   unsigned char

#define UINT16  unsigned short

#define UINT32  unsigned long


#undef  TRUE

#undef  FALSE


#define TRUE    1

#define FALSE   (! TRUE)


/* Stupid Mingw32. */

#undef NULL

#define NULL    0


#define CONST   const

#define INLINE  inline


#ifndef UNIX

#define USE_ZLIB

#endif
