

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


static UINT8 cpu_sram [8192];


static M6502 cpu_context;


static char *get_sram_filename (char *buffer, const char *rom_filename, int buffer_size)
{
#ifdef UNIX

    if (sramdir != NULL)
    {
        strcpy (buffer, sramdir);
    
        strcat (buffer, "/");

        strcat (buffer, get_filename (rom_filename));
    
    
        replace_extension
            (buffer, buffer, "sav", buffer_size);
    }
    else
    {
        replace_extension (buffer,
            rom_filename, "sav", buffer_size);
    }
	
#else

    replace_extension (buffer,
        rom_filename, "sav", buffer_size);

#endif

    return buffer;

}


void sram_load (const char *rom_filename)
{
    UINT8 buffer [256];

    FILE * sram_file;


    get_sram_filename(buffer, rom_filename, sizeof(buffer));

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


void sram_save (const char *rom_filename)
{
    UINT8 buffer [256];

    FILE * sram_file;


    get_sram_filename(buffer, rom_filename, sizeof(buffer));

    sram_file = fopen (buffer, "wb");


    if (sram_file)
    {
        fwrite (cpu_sram, 1, sizeof (cpu_sram), sram_file);

        fclose (sram_file);
    }
}


int cpu_init (void)
{

    memset (cpu_ram, 0xFF, sizeof (cpu_ram));

    memset (cpu_sram, NULL, sizeof (cpu_sram));


    cpu_active_pc = &cpu_context.PC.W;


    if (global_rom.sram_flag)
    {
        sram_load (global_rom.filename);
    }


    cpu_context.TrapBadOps = TRUE;


    memset(dummy_read, 0, sizeof(dummy_read));
    memset(dummy_write, 0, sizeof(dummy_write));


    return (0);
}


UINT8 ppu_read_2000_3FFF (UINT16 address)
{
    return ppu_read(address);
}

UINT8 ppu_read_4000_47FF (UINT16 address)
{
    if (address == 0x4014)
        return (ppu_read(address));
    else if (address <= 0x4015)
    {
        return (papu_read (address));
    }
    else if ((address == 0x4016) || (address == 0x4017))
    {
        return (input_read (address));
    }

    return (0);
}

void ppu_write_2000_3FFF (UINT16 address, UINT8 value)
{
    ppu_write(address, value);
}

void ppu_write_4000_47FF (UINT16 address, UINT8 value)
{
    if (address == 0x4014)
    {
        ppu_write (address, value);
    }
    else if (address <= 0x4015)
    {
        papu_write (address, value);
    }
    else if ((address == 0x4016) || (address == 0x4017))
    {
        input_write (address, value);
    }
}

void cpu_memmap_init (void)
{
    int index;

    /* Start with a clean memory map */
    for (index = 0; index < (64 << 10); index += (2 << 10))
    {
        cpu_set_read_address_2k (index, dummy_read);
        cpu_set_write_address_2k (index, dummy_write);
    }

    /* Map in RAM */
    for (index = 0; index < (8 << 10); index += (2 << 10))
    {
        cpu_set_read_address_2k (index, cpu_ram);
        cpu_set_write_address_2k (index, cpu_ram);
    }

    /* Map in SRAM */
    enable_sram();

    /* Map in registers */
    cpu_set_read_handler_8k (0x2000, ppu_read_2000_3FFF);
    cpu_set_write_handler_8k (0x2000, ppu_write_2000_3FFF);

    cpu_set_read_handler_2k (0x4000, ppu_read_4000_47FF);
    cpu_set_write_handler_2k (0x4000, ppu_write_4000_47FF);
}


void cpu_exit (void)
{

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
        sram_save (global_rom.filename);
    }
}


void enable_sram(void)
{
    cpu_set_read_address_8k (0x6000, cpu_sram);
    cpu_set_write_address_8k (0x6000, cpu_sram);
}


void disable_sram(void)
{
    cpu_set_read_address_8k (0x6000, dummy_read);
    cpu_set_write_address_8k (0x6000, dummy_write);
}


void cpu_reset (void)
{

    if (global_rom.sram_flag)
    {
        sram_save (global_rom.filename);
    }


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


    cpu_context.IPeriod = cycles;
    cpu_context.ICount += cycles;

    Run6502 (&cpu_context);


    return (count);
}


static int scanline_start_cycle;

void cpu_start_new_scanline (void)
{
    scanline_start_cycle = cpu_context.Cycles + cpu_context.ICount;
}


void cpu_consume_cycles (int cycles)
{
    cpu_context.ICount -= cycles * CYCLE_LENGTH;
    cpu_context.Cycles += cycles * CYCLE_LENGTH;
}


int cpu_get_cycles_line (void)
{
    return (cpu_context.Cycles - scanline_start_cycle) / CYCLE_LENGTH;
}


int cpu_get_cycles (int reset)
{
    int cycles = cpu_context.Cycles;


    if (reset)
    {
        cpu_context.Cycles = 0;
    }


    return (cycles / CYCLE_LENGTH);
}


/* Not needed. */

byte Loop6502 (M6502 * R)
{
    return (INT_NONE);
}
