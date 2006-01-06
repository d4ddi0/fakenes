/*

FakeNES - A portable, Open Source NES emulator.

coreoff.c: FN2A03 structure offsets generator tool.

Copyright (c) 2001-2006, Charles Bilyue' and Randy McDowell.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

 This file contains autogeneration of structure offsets for the
x86-specific emulation core functions for the Ricoh RP2A03G CPU,
as used in the Nintendo Famicom (Family Computer) and NES (Nintendo
Entertainment System).

*/

#define ALLEGRO_USE_CONSOLE
#include <allegro.h>
#include <stdio.h>
#include "core.h"
#include "misc.h"

#ifndef OUTPUT
#error OUTPUT not defined.
#endif
static FILE *output;

#define generate_offset(label) \
    fprintf(output, "%%define O_" #label " %d\n", \
        (int) &((FN2A03 *) NULL)->label); \
    fprintf(output, "%%define B_" #label " [R_Base + O_" #label "]\n")

int main (void)
{
    allegro_init ();

    output = fopen (OUTPUT, "w");
    if (!output) {
        printf ("Could not open output: %s.\n", OUTPUT);
        return (1);
    }

    fprintf (output, "%%ifndef CORE_I\n");
    fprintf (output, "%%define CORE_I\n");
    fprintf (output, "\n\n");
    generate_offset (PC);
    fprintf (output, "\n");
    generate_offset (A);
    generate_offset (X);
    generate_offset (Y);
    generate_offset (S);
    fprintf (output, "\n");
    generate_offset (N);
    generate_offset (V);
    generate_offset (D);
    generate_offset (I);
    generate_offset (Z);
    generate_offset (C);
    fprintf (output, "\n");
    generate_offset (ICount);
    generate_offset (Cycles);
    generate_offset (IBackup);
    generate_offset (IRequest);
    generate_offset (Trap);
    generate_offset (AfterCLI);
    generate_offset (TrapBadOps);
    generate_offset (Trace);
    generate_offset (Jammed);
    fprintf (output, "\n\n");
    fprintf (output, "%%endif ;!defined(CORE_I)\n");
    fprintf (output, "\n");
    fclose (output);

    return (0);
}
END_OF_MAIN ()
