

#ifndef __DATA_H__
#define __DATA_H__


#include "data2.h"


DATAFILE * data;


#define DATA_ARROW_SPRITE   \
    ((BITMAP *) data [ARROW_SPRITE].dat)

#define DATA_GUN_SPRITE   \
    ((BITMAP *) data [GUN_SPRITE].dat)


#define DATA_DEFAULT_PALETTE    \
    ((RGB *) data [DEFAULT_PALETTE].dat)

#define DATA_GRAYSCALE_PALETTE     \
    ((RGB *) data [GRAYSCALE_PALETTE].dat)

#define DATA_GNUBOY_PALETTE     \
    ((RGB *) data [GNUBOY_PALETTE].dat)

#define DATA_NESTER_PALETTE     \
    ((RGB *) data [NESTER_PALETTE].dat)

#define DATA_NESTICLE_PALETTE   \
    ((RGB *) data [NESTICLE_PALETTE].dat)


#define DATA_LARGE_FONT     \
    ((FONT *) data [LARGE_FONT].dat)

#define DATA_SMALL_FONT     \
    ((FONT *) data [SMALL_FONT].dat)


#endif /* ! __DATA_H__ */
