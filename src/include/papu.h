

/*

FakeNES - A portable, open-source NES emulator.

papu.h: Declarations for the APU interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __PAPU_H__
#define __PAPU_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


int papu_filter_type;


int papu_init (void);

void papu_reinit (void);

void papu_exit (void);


void papu_reset (void);


UINT8 papu_read (UINT16);

void papu_write (UINT16, UINT8);


void papu_update (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __PAPU_H__ */
