/*

FakeNES - A portable, Open Source NES emulator.

coreoff.c: FN2A03 structure offsets generator tool.

Copyright (c) 2004, Charles Bilyue' and Randy McDowell.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

 This file contains autogeneration of structure offsets for the
x86-specific emulation core functions for the Ricoh RP2A03G CPU,
as used in the Nintendo Famicom (Family Computer) and NES (Nintendo
Entertainment System).

*/

/* Not really needed. */
#ifndef INLINE
#define INLINE
#endif

#include <stdio.h>
#include "core.h"
#include "misc.h"

#define generate_offset(label) \
    printf("%%define O_" #label " %d\n",\
        (int) &((FN2A03 *) NULL)->label); \
    printf("%%define B_" #label " [R_Base + O_" #label "]\n")

int main()
{
 printf("%%ifndef CORE_I\n");
 printf("%%define CORE_I\n");
 printf("\n");
 printf("\n");
 generate_offset(PC);
 printf("\n");
 generate_offset(A);
 generate_offset(X);
 generate_offset(Y);
 generate_offset(S);
 printf("\n");
 generate_offset(N);
 generate_offset(V);
 generate_offset(D);
 generate_offset(I);
 generate_offset(Z);
 generate_offset(C);
 printf("\n");
 generate_offset(ICount);
 generate_offset(Cycles);
 generate_offset(IBackup);
 generate_offset(IRequest);
 generate_offset(Trap);
 generate_offset(AfterCLI);
 generate_offset(TrapBadOps);
 generate_offset(Trace);
 generate_offset(Jammed);
 printf("\n");
 printf("\n");
 printf("%%endif ;!defined(CORE_I)\n");
 printf("\n");

 return 0;
}
