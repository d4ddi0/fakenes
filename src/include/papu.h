

/*

FakeNES - A portable, open-source NES emulator.

papu.h: Declarations for the pAPU emulation.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __PAPU_H__
#define __PAPU_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


int papu_init (void);

void papu_exit (void);


void papu_reset (void);


UINT8 papu_read (UINT16 address);

void papu_write (UINT16 address, UINT8 value);


void papu_process_frame (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __PAPU_H__ */
