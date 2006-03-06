/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   cpu.c: Implementation of the CPU abstraction.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "core.h"
#include "cpu.h"
#include "crc32.h"
#include "input.h"
#include "mmc.h"
#include "papu.h"
#include "ppu.h"
#include "rom.h"
#include "save.h"
#include "types.h"


UINT8 cpu_sram [SRAM_SIZE];


int cpu_init (void)
{
    memset (cpu_ram, NIL, sizeof (cpu_ram));

    memset (cpu_sram, NIL, sizeof (cpu_sram));


    /* Trainer support - copy the trainer into the memory map */
    if ((global_rom.control_byte_1 & ROM_CTRL_TRAINER))
    {
        memcpy (cpu_sram + 0x1000, global_rom.trainer, 512);
    }


    cpu_active_pc = &cpu_context.PC.word;


    if (global_rom.sram_flag)
    {
        load_sram ();
    }


    cpu_context.TrapBadOps = TRUE;


    memset(dummy_read, 0, sizeof(dummy_read));
    memset(dummy_write, 0, sizeof(dummy_write));


    return (0);
}


UINT8 cpu_read_direct_safeguard(UINT16 address)
{
    return cpu_block_2k_read_address [address >> 11] [address] + cpu_patch_table [address];
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


    /* Load patches from patch table. */

    if (rom_is_loaded)
    {
        load_patches ();
    }


    memset (cpu_patch_table, 0, sizeof (cpu_patch_table));


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


    if ((global_rom.control_byte_1 & ROM_CTRL_TRAINER))
    {
        /* compute CRC32 for trainer */
        global_rom.trainer_crc32 =
            crc32_calculate (global_rom.trainer, ROM_TRAINER_SIZE);
    }
    else
    {
        global_rom.trainer_crc32 = 0;
    }


    /* compute CRC32 for PRG ROM */
    global_rom.prg_rom_crc32 = crc32_calculate (global_rom.prg_rom,
        (global_rom.prg_rom_pages * 0x4000));


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


    /* Save SRAM. */

    save_sram ();


    /* Save patches. */

    save_patches ();
}


void cpu_free_prg_rom (const ROM *rom)
{
    if (rom -> prg_rom) free (rom -> prg_rom);
}


UINT8 * cpu_get_prg_rom_pages (ROM *rom)
{
    int num_pages = rom -> prg_rom_pages;
    int copycount, missing, count, next, pages_mirror_size;

    /* Compute a mask used to wrap invalid PRG ROM page numbers.
     *  As PRG ROM uses a 16k page size, this mask is based
     *  on a 16k page size.
     */
    if (((num_pages * 2 - 1) & (num_pages - 1)) == (num_pages - 1))
    /* compute mask for even power of two */
    {
        pages_mirror_size = num_pages;
    }
    else
    /* compute mask */
    {
        int i;

        /* compute the smallest even power of 2 greater than
           PRG ROM page count, and use that to compute the mask */
        for (i = 0; (num_pages >> i) > 0; i++);

        pages_mirror_size = (1 << i);
    }

    rom -> prg_rom_page_overflow_mask = pages_mirror_size - 1;


    /* identify-map all the present pages */
    for (copycount = 0; copycount < num_pages; copycount++)
    {
        rom -> prg_rom_page_lookup [copycount] = copycount;
    }


    /* mirror-map all the not-present pages */
    for (next = num_pages, missing = pages_mirror_size - num_pages,
        count = 1; missing; count <<= 1, missing >>= 1)
    {
        if (missing & 1)
        {
            for (copycount = count; copycount; copycount--, next++)
            {
                rom -> prg_rom_page_lookup[next] =
                    rom -> prg_rom_page_lookup[next - count];
            }
        }
    }


    /* 16k PRG ROM page size */
    rom -> prg_rom = malloc (num_pages * 0x4000);

    if (rom -> prg_rom != NIL)
    {
        /* initialize to a known value for areas not present in image */
        memset (rom -> prg_rom, 0xFF, (num_pages * 0x4000));
    }

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
    /* Save SRAM. */

    save_sram ();


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
    scanline_start_cycle = cpu_context.ICount;
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
        cpu_context.ICount -= cpu_context.Cycles;
        cpu_context.Cycles = 0;
    }


    return (cycles / CYCLE_LENGTH);
}


void cpu_save_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    /* Save CPU registers */
    pack_iputw (cpu_context.PC.word, file_chunk);

    pack_putc (cpu_context.A, file_chunk);
    pack_putc (cpu_context.X, file_chunk);
    pack_putc (cpu_context.Y, file_chunk);
    pack_putc (cpu_context.S, file_chunk);
    pack_putc (FN2A03_Pack_Flags(&cpu_context), file_chunk);

    /* Save cycle counters */
    pack_iputl (cpu_context.ICount, file_chunk);
    pack_iputl (cpu_context.Cycles, file_chunk);

    /* Save IRQ state */
    pack_iputl (cpu_context.IRequest, file_chunk);

    /* Save execution state */
    pack_putc (cpu_context.Jammed, file_chunk);


    /* Save NES internal RAM */
    pack_fwrite (cpu_ram, 0x800, file_chunk);

    /* Save cartridge expansion RAM */
    pack_fwrite (cpu_sram, 0x2000, file_chunk);


    pack_fclose_chunk (file_chunk);
}


void cpu_load_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;

    UINT8 P;


    file_chunk = pack_fopen_chunk (file, FALSE);


    /* Restore CPU registers */
    cpu_context.PC.word = pack_igetw (file_chunk);

    cpu_context.A = pack_getc (file_chunk);
    cpu_context.X = pack_getc (file_chunk);
    cpu_context.Y = pack_getc (file_chunk);
    cpu_context.S = pack_getc (file_chunk);
    P = pack_getc (file_chunk);
    FN2A03_Unpack_Flags(&cpu_context, P);

    /* Restore cycle counters */
    cpu_context.ICount = pack_igetl (file_chunk);
    cpu_context.Cycles = pack_igetl (file_chunk);

    /* Restore IRQ state */
    cpu_context.IRequest = pack_igetl (file_chunk);

    /* Restore execution state */
    cpu_context.Jammed = pack_getc (file_chunk);


    /* Restore NES internal RAM */
    pack_fread (cpu_ram, 0x800, file_chunk);

    /* Restore cartridge expansion RAM */
    pack_fread (cpu_sram, 0x2000, file_chunk);


    pack_fclose_chunk (file_chunk);
}
