#ifndef __MISC_H__
#define __MISC_H__


#define UINT8   unsigned char

#define UINT16  unsigned short int

#define UINT32  unsigned long int


#define INT8    signed char

#define INT16   signed short int

#define INT32   signed long int


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

#undef  FALSE


#define TRUE    1

#define FALSE   (! TRUE)


/* Stupid Mingw32. */

#undef NULL

#define NULL    0


#endif /* ! __MISC_H__ */
