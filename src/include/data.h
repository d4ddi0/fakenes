

#ifndef __DATA_H__
#define __DATA_H__


#include "data2.h"


DATAFILE * data;


#define DATA_ARROW_SPRITE   \
    ((BITMAP *) data [ARROW_SPRITE].dat)

#define DATA_GUN_SPRITE   \
    ((BITMAP *) data [GUN_SPRITE].dat)


#define DATA_GB_PALETTE     \
    ((RGB *) data [GB_PALETTE].dat)

#define DATA_NES_PALETTE    \
    ((RGB *) data [NES_PALETTE].dat)


#define DATA_LARGE_FONT     \
    ((FONT *) data [LARGE_FONT].dat)

#define DATA_SMALL_FONT     \
    ((FONT *) data [SMALL_FONT].dat)


#endif /* ! __DATA_H__ */
