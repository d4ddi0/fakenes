

/*

FakeNES - A portable, open-source NES emulator.

cpu.c: Implementation of the CPU emulation.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "cpu.h"

#include "input.h"

#include "mmc.h"

#include "papu.h"

#include "ppu.h"

#include "rom.h"


#include "core.h"

#include "misc.h"


#ifdef UNIX

extern char * confdir;

extern char * sramdir;

#endif


static UINT8 cpu_ram [65536];

static UINT8 cpu_sram [8192];


static M6502 cpu_context;


int cpu_init (void)
{
    UINT8 buffer [256];

    FILE * sram_file;


    memset (cpu_ram, NULL, sizeof (cpu_ram));

    memset (cpu_sram, NULL, sizeof (cpu_sram));


    cpu_active_pc = &cpu_context.PC.W;


    if (global_rom.sram_flag)
    {

#ifdef UNIX

        if (sramdir != NULL)
        {
            strcpy (buffer, sramdir);
    
            strcat (buffer, "/");
    
            strcat (buffer, get_filename (global_rom.filename));
    
    
            replace_extension
                (buffer, buffer, "sav", sizeof (buffer));
        }
	
#else

        replace_extension (buffer,
            global_rom.filename, "sav", sizeof (buffer));

#endif


        if (exists (buffer))
        {
            sram_file = fopen (buffer, "rb");
    

            if (sram_file)
            {
                fread (cpu_sram, 1, sizeof (cpu_sram), sram_file);
    
                fclose (sram_file);
            }
        }
    }


    cpu_context.TrapBadOps = TRUE;


    return (0);
}


void cpu_exit (void)
{
    UINT8 buffer [256];

    FILE * sram_file;


#ifdef DEBUG

    FILE * dump_file;

    int address;


    dump_file = fopen ("cpudump.ram", "wb");


    if (dump_file)
    {
        for (address = 0; address < 65536; address ++)
        {
            fputc (cpu_read (address), dump_file);
        }


        fclose (dump_file);
    }

#endif


    if (global_rom.sram_flag)
    {

#ifdef UNIX

        if (sramdir != NULL)
        {
            strcpy (buffer, sramdir);

            strcat (buffer, "/");

            strcat (buffer, get_filename (global_rom.filename));


            replace_extension
                (buffer, buffer, "sav", sizeof (buffer));
        }
	
#else

        replace_extension (buffer,
            global_rom.filename, "sav", sizeof (buffer));

#endif

        sram_file = fopen (buffer, "wb");


        if (sram_file)
        {
            fwrite (cpu_sram, 1, sizeof (cpu_sram), sram_file);

            fclose (sram_file);
        }
    }
}


void cpu_reset (void)
{
    Reset6502 (&cpu_context);
}


void cpu_interrupt (int type)
{
    switch (type)
    {
        case CPU_INTERRUPT_IRQ:

            Int6502 (&cpu_context, INT_IRQ);


            break;


        case CPU_INTERRUPT_NMI:

            Int6502 (&cpu_context, INT_NMI);


            break;


        default:


            break;
    }
}


int cpu_execute (int cycles)
{
    int count = 0;


    cpu_context.ICount += cycles;

    Run6502 (&cpu_context);


    return (count);
}


UINT8 cpu_read (UINT16 address)
{
    return (Rd6502 (address));
}


void cpu_write (UINT16 address, UINT8 value)
{
    Wr6502 (address, value);
}


/* ----- M6502 Routines ----- */


byte Op6502 (word Addr)
{
    // printf ("Op6502 at $%04x.\n", Addr);

    if (Addr >= 0x8000)
    {
        return (READ_ROM ((Addr - 0x8000)));
    }
    else if (Addr < 0x2000)
    {
        return (cpu_ram [Addr & 0x7ff]);
    }
    else if ((Addr >= 0x6000) && (Addr < 0x8000))
    {
        if (! mmc_disable_sram)
        {
            return (cpu_sram [Addr - 0x6000]);
        }
        else
        {
            return (0);
        }
    }
    else /* if (Addr < 0x6000) */
    {
        return (cpu_ram [Addr]);
    }
}


byte Rd6502 (word Addr)
{
    // printf ("Rd6502 at $%04x.\n", Addr);

    if (Addr < 0x2000)
    {
        return (cpu_ram [Addr & 0x7ff]);
    }
    else if (Addr >= 0x8000)
    {
        return (READ_ROM ((Addr - 0x8000)));
    }
    else if ((Addr < 0x4000) || (Addr == 0x4014))
    {
        return (ppu_read (Addr));
    }
    else if (Addr <= 0x4015)
    {
        return (papu_read (Addr));
    }
    else if ((Addr == 0x4016) || (Addr == 0x4017))
    {
        return (input_read (Addr));
    }
    else if ((Addr >= 0x6000) && (Addr < 0x8000))
    {
        if (! mmc_disable_sram)
        {
            return (cpu_sram [Addr - 0x6000]);
        }
        else
        {
            return (0);
        }
    }
    else
    {
        return (cpu_ram [Addr]);
    }
}


void Wr6502 (word Addr, byte Value)
{
    // printf ("Wr6502 at $%04x ($%02x).\n", Addr, Value);

    if (Addr < 0x2000)
    {
        cpu_ram [Addr & 0x7ff] = Value;
    }
    else if ((Addr < 0x4000) || (Addr == 0x4014))
    {
        ppu_write (Addr, Value);
    }
    else if (Addr <= 4015)
    {
        papu_write (Addr, Value);
    }
    else if ((Addr == 0x4016) || (Addr == 0x4017))
    {
        input_write (Addr, Value);
    }
    else if (Addr >= 0x8000)
    {
        if (mmc_write)
        {
            mmc_write (Addr, Value);
        }
    }
    else if ((Addr >= 0x6000) && (Addr < 0x8000))
    {
        if (! mmc_disable_sram)
        {
            cpu_sram [Addr - 0x6000] = Value;
        }
    }
    else
    {
        cpu_ram [Addr] = Value;
    }
}


/* Not needed. */

byte Loop6502 (M6502 * R)
{
    return (INT_NONE);
}
