

/*

FakeNES - A portable, open-source NES emulator.

cpu.h: Declarations for the CPU emulation.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __CPU_H__
#define __CPU_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


#define CPU_INTERRUPT_NMI   0

#define CPU_INTERRUPT_IRQ   1


int cpu_init (void);

void cpu_exit (void);


void cpu_reset (void);


void cpu_interrupt (int);

int cpu_execute (int);


UINT8 cpu_read (UINT16 address);

void cpu_write (UINT16 address, UINT8 value);


int cpu_get_pc (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __CPU_H__ */
