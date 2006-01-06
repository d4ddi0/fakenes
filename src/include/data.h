

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

data.h: Definitions of datafile helper macros.

Copyright (c) 2001-2006, Randy McDowell.
Copyright (c) 2001-2006, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef DATA_H_INCLUDED

#define DATA_H_INCLUDED


#include "datafile.h"


#define DATA(id)                (datafile_data [DATAFILE_ ##id].dat)


#define DATA_TO_BITMAP(id)      ((BITMAP *) DATA (id))


#define DATA_TO_FONT(id)        ((FONT *) DATA (id))


#define DATA_TO_RGB(id)         ((RGB *) DATA (id))


#define DATA_TO_SAMPLE(id)      ((SAMPLE *) DATA (id))


#endif /* ! DATA_H_INCLUDED */
