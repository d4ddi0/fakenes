

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


#ifdef POSIX

extern char * confdir;

extern char * sramdir;

#endif


static UINT8 cpu_sram [8192];


static char *get_sram_filename (char *buffer, const char *rom_filename, int buffer_size)
{
#ifdef POSIX

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

    memset (cpu_ram, NULL, sizeof (cpu_ram));

    memset (cpu_sram, NULL, sizeof (cpu_sram));

    /* Trainer support - copy the trainer into the memory map */
    if ((global_rom.control_byte_1 & ROM_CTRL_TRAINER))
    {
        memcpy (cpu_sram + 0x1000, global_rom.trainer, 512);
    }


    cpu_active_pc = &cpu_context.PC.word;


    if (global_rom.sram_flag)
    {
        sram_load (global_rom.filename);
    }


    cpu_context.TrapBadOps = TRUE;


    memset(dummy_read, 0, sizeof(dummy_read));
    memset(dummy_write, 0, sizeof(dummy_write));


    return (0);
}


UINT8 cpu_read_direct_safeguard(UINT16 address)
{
    return cpu_block_2k_read_address [address >> 11] [address];
}


void cpu_write_direct_safeguard(UINT16 address, UINT8 value)
{
    cpu_block_2k_write_address [address >> 11] [address] = value;
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


void cpu_free_prg_rom (const ROM *rom)
{
    if (rom -> prg_rom) free (rom -> prg_rom);
}


UINT8 * cpu_get_prg_rom_pages (ROM *rom)
{
    int num_pages = rom -> prg_rom_pages;

    /* Compute a mask used to wrap invalid PRG ROM page numbers.
     *  As PRG ROM banking uses a 8k page size, this mask is based
     *  on a 8k page size.
     */
    if (((num_pages * 2 - 1) & (num_pages - 1)) == (num_pages - 1))
    /* compute mask for even power of two */
    {
        rom -> prg_rom_page_overflow_premask = (num_pages * 2) - 1;
        rom -> prg_rom_page_overflow_mask =
         rom -> prg_rom_page_overflow_premask;
    }
    else
    /* compute mask */
    {
        int i;

        /* compute the largest even power of 2 less than
           PRG ROM page count, and use that to compute the mask */
        for (i = 0; (num_pages >> (i + 1)) > 0; i++);

        rom -> prg_rom_page_overflow_premask = ((1 << (i + 1)) * 2) - 1;
        rom -> prg_rom_page_overflow_mask = ((1 << i) * 2) - 1;
    }

    /* 16k PRG ROM page size */
    rom -> prg_rom = malloc (num_pages * 0x4000);

    return rom -> prg_rom;
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


    FN2A03_Reset (&cpu_context);
}


void cpu_interrupt (int type)
{
    switch (type)
    {
        case CPU_INTERRUPT_IRQ_SINGLE_SHOT:

            FN2A03_Interrupt (&cpu_context, FN2A03_INT_IRQ_SINGLE_SHOT);


            break;


        case CPU_INTERRUPT_NMI:

            FN2A03_Interrupt (&cpu_context, FN2A03_INT_NMI);


            break;


        default:

            FN2A03_Interrupt (&cpu_context,
                FN2A03_INT_IRQ_SOURCE(type - CPU_INTERRUPT_IRQ_SOURCE(0)));


            break;
    }
}


void cpu_clear_interrupt (int type)
{
    switch (type)
    {
        case CPU_INTERRUPT_IRQ_SINGLE_SHOT:
        case CPU_INTERRUPT_NMI:

            break;


        default:

            FN2A03_Clear_Interrupt (&cpu_context,
                FN2A03_INT_IRQ_SOURCE(type - CPU_INTERRUPT_IRQ_SOURCE(0)));


            break;

    }
}


static int scanline_start_cycle;

void cpu_start_new_scanline (void)
{
    scanline_start_cycle = cpu_context.Cycles + cpu_context.ICount;
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
