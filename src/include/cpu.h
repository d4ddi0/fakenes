

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

cpu.h: Declarations for the CPU abstraction.

Copyright (c) 2003, Randy McDowell.
Copyright (c) 2003, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __CPU_H__
#define __CPU_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <allegro.h>
#include "misc.h"

#include "rom.h"

#include "core.h"


#define INLINE_WITH_MACROS     /* Use macros instead of      */
                               /* inline functions for stack */
                               /* and zero page handlers     */

#define CPU_INTERRUPT_NONE  0
#define CPU_INTERRUPT_NMI   1

#define CPU_INTERRUPT_IRQ_BASE          2

/* Maskable IRQ, cleared after an IRQ is acknowledged */
#define CPU_INTERRUPT_IRQ_SINGLE_SHOT   2
#define CPU_INTERRUPT_IRQ               CPU_INTERRUPT_IRQ_SINGLE_SHOT

#define CPU_INTERRUPT_IRQ_SOURCE(x)     (CPU_INTERRUPT_IRQ_BASE + 1 + (x))

/* pAPU Delta Modulation Code IRQ */
#define CPU_INTERRUPT_IRQ_DMC           CPU_INTERRUPT_IRQ_SOURCE(0)
/* Frame IRQ */
#define CPU_INTERRUPT_IRQ_FRAME         CPU_INTERRUPT_IRQ_SOURCE(1)
/* MMC-specific IRQ */
#define CPU_INTERRUPT_IRQ_MMC           CPU_INTERRUPT_IRQ_SOURCE(2)

#define CPU_INTERRUPT_IRQ_SOURCE_MAX    FN2A03_INT_IRQ_SOURCE_MAX

UINT8 cpu_ram [65536];

FN2A03 cpu_context;


INT8 cpu_patch_table [65536];


void patches_load (const char * rom_filename);

void patches_save (const char * rom_filename);


/* Number of used entries in cpu_patch_info. */

int cpu_patch_count;


/* Format of cpu_patch_info. */

/* Designed primarily for use with Game Genie. */

typedef struct _CPU_PATCH
{
    int active;


    UINT8 * title;


    UINT16 address;


    UINT8 value;

    UINT8 match_value;


    int enabled;

} CPU_PATCH;


/* Try to keep it fast - 15 patches limit. */

#define MAX_PATCHES     15


CPU_PATCH cpu_patch_info [MAX_PATCHES];


void sram_load (const char *rom_filename);
void sram_save (const char *rom_filename);

int cpu_init (void);

void cpu_memmap_init(void);

void cpu_exit (void);


void cpu_reset (void);


void cpu_interrupt (int);
void cpu_clear_interrupt (int);

static INLINE void cpu_execute (int cycles)
{
    cpu_context.ICount += cycles;

    FN2A03_Run (&cpu_context);
}


UINT16 * cpu_active_pc;


void cpu_free_prg_rom (const ROM *rom);
UINT8 * cpu_get_prg_rom_pages (ROM *rom);

void enable_sram(void);
void disable_sram(void);


UINT8 dummy_read [(8 << 10)];
UINT8 dummy_write [(8 << 10)];


UINT8 *cpu_block_2k_read_address [64 / 2];
UINT8 *cpu_block_2k_write_address [64 / 2];
UINT8 (*cpu_block_2k_read_handler [64 / 2]) (UINT16 address);
void (*cpu_block_2k_write_handler [64 / 2]) (UINT16 address, UINT8 data);

UINT8 cpu_read_direct_safeguard(UINT16 address);
void cpu_write_direct_safeguard(UINT16 address, UINT8 value);


static INLINE UINT8 FN2A03_Read (UINT16 Addr);


static INLINE void cpu_set_read_address_2k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((2 << 10) - 1);

    cpu_block_2k_read_address [block_start >> 11] = address - block_start;
    cpu_block_2k_read_handler [block_start >> 11] =
        cpu_read_direct_safeguard;


    /* Check if we are using patches. */

    if (cpu_patch_count > 0)
    {
        int index;


        for (index = 0; index < cpu_patch_count; index ++)
        {
            if (cpu_patch_info [index].enabled &&
               (cpu_patch_info [index].address < (block_start + 0x800)) && (cpu_patch_info [index].address >= block_start))
            {
                UINT8 value;


                if (cpu_patch_info [index].active)
                {
                    /* Disable patch. */
    
                    cpu_patch_info [index].active = FALSE;
    
    
                    cpu_patch_table [cpu_patch_info [index].address] = 0;
                }


                value = FN2A03_Read (cpu_patch_info [index].address);


                if (value == cpu_patch_info [index].match_value)
                {
                    /* Enable patch. */

                    cpu_patch_info [index].active = TRUE;


                    cpu_patch_table [cpu_patch_info [index].address] = (cpu_patch_info [index].value - value);
                }
            }
        }
    }
}


static INLINE void cpu_set_read_address_4k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((4 << 10) - 1);

    cpu_set_read_address_2k (block_start, address);
    cpu_set_read_address_2k (block_start + (2 << 10), address + (2 << 10));
}


static INLINE void cpu_set_read_address_8k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((8 << 10) - 1);

    cpu_set_read_address_4k (block_start, address);
    cpu_set_read_address_4k (block_start + (4 << 10), address + (4 << 10));
}


static INLINE void cpu_set_read_address_16k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((16 << 10) - 1);

    cpu_set_read_address_8k (block_start, address);
    cpu_set_read_address_8k (block_start + (8 << 10), address + (8 << 10));
}


static INLINE void cpu_set_read_address_32k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((32 << 10) - 1);

    cpu_set_read_address_16k (block_start * 2, address);
    cpu_set_read_address_16k (block_start + (16 << 10), address + (16 << 10));
}


static INLINE void cpu_set_write_address_2k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((2 << 10) - 1);

    cpu_block_2k_write_address [block_start >> 11] = address - block_start;
    cpu_block_2k_write_handler [block_start >> 11] =
        cpu_write_direct_safeguard;
}


static INLINE void cpu_set_write_address_4k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((4 << 10) - 1);

    cpu_set_write_address_2k (block_start, address);
    cpu_set_write_address_2k (block_start + (2 << 10), address + (2 << 10));
}


static INLINE void cpu_set_write_address_8k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((8 << 10) - 1);

    cpu_set_write_address_4k (block_start, address);
    cpu_set_write_address_4k (block_start + (4 << 10), address + (4 << 10));
}


static INLINE void cpu_set_write_address_16k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((16 << 10) - 1);

    cpu_set_write_address_8k (block_start, address);
    cpu_set_write_address_8k (block_start + (8 << 10), address + (8 << 10));
}


static INLINE void cpu_set_write_address_32k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((32 << 10) - 1);

    cpu_set_write_address_16k (block_start * 2, address);
    cpu_set_write_address_16k (block_start + (16 << 10), address + (16 << 10));
}


static INLINE void cpu_set_read_handler_2k (UINT16 block_start, UINT8 (*handler) (UINT16 address))
{
    /* ignore low bits */
    block_start &= ~((2 << 10) - 1);

    cpu_block_2k_read_address [block_start >> 11] = 0;
    cpu_block_2k_read_handler [block_start >> 11] = handler;
}


static INLINE void cpu_set_read_handler_4k (UINT16 block_start, UINT8 (*handler) (UINT16 address))
{
    /* ignore low bits */
    block_start &= ~((4 << 10) - 1);

    cpu_set_read_handler_2k (block_start, handler);
    cpu_set_read_handler_2k (block_start + (2 << 10), handler);
}


static INLINE void cpu_set_read_handler_8k (UINT16 block_start, UINT8 (*handler) (UINT16 address))
{
    /* ignore low bits */
    block_start &= ~((8 << 10) - 1);

    cpu_set_read_handler_4k (block_start, handler);
    cpu_set_read_handler_4k (block_start + (4 << 10), handler);
}


static INLINE void cpu_set_read_handler_16k (UINT16 block_start, UINT8 (*handler) (UINT16 address))
{
    /* ignore low bits */
    block_start &= ~((16 << 10) - 1);

    cpu_set_read_handler_8k (block_start, handler);
    cpu_set_read_handler_8k (block_start + (8 << 10), handler);
}


static INLINE void cpu_set_read_handler_32k (UINT16 block_start, UINT8 (*handler) (UINT16 address))
{
    /* ignore low bits */
    block_start &= ~((32 << 10) - 1);

    cpu_set_read_handler_16k (block_start, handler);
    cpu_set_read_handler_16k (block_start + (16 << 10), handler);
}


static INLINE void cpu_set_write_handler_2k (UINT16 block_start, void (*handler) (UINT16 address, UINT8 value))
{
    /* ignore low bits */
    block_start &= ~((2 << 10) - 1);

    cpu_block_2k_write_address [block_start >> 11] = 0;
    cpu_block_2k_write_handler [block_start >> 11] = handler;
}


static INLINE void cpu_set_write_handler_4k (UINT16 block_start, void (*handler) (UINT16 address, UINT8 value))
{
    /* ignore low bits */
    block_start &= ~((4 << 10) - 1);

    cpu_set_write_handler_2k (block_start, handler);
    cpu_set_write_handler_2k (block_start + (2 << 10), handler);
}


static INLINE void cpu_set_write_handler_8k (UINT16 block_start, void (*handler) (UINT16 address, UINT8 value))
{
    /* ignore low bits */
    block_start &= ~((8 << 10) - 1);

    cpu_set_write_handler_4k (block_start, handler);
    cpu_set_write_handler_4k (block_start + (4 << 10), handler);
}


static INLINE void cpu_set_write_handler_16k (UINT16 block_start, void (*handler) (UINT16 address, UINT8 value))
{
    /* ignore low bits */
    block_start &= ~((16 << 10) - 1);

    cpu_set_write_handler_8k (block_start, handler);
    cpu_set_write_handler_8k (block_start + (8 << 10), handler);
}


static INLINE void cpu_set_write_handler_32k (UINT16 block_start, void (*handler) (UINT16 address, UINT8 value))
{
    /* ignore low bits */
    block_start &= ~((32 << 10) - 1);

    cpu_set_write_handler_16k (block_start, handler);
    cpu_set_write_handler_16k (block_start + (16 << 10), handler);
}


/* ----- ROM-specific banking ----- */


static INLINE void cpu_set_read_address_8k_rom_block (UINT16 block_start, int rom_block)
{
    rom_block = (rom_block & 1) + ROM_PRG_ROM_PAGE_LOOKUP
        [(rom_block / 2) & ROM_PRG_ROM_PAGE_OVERFLOW_MASK] * 2;

    cpu_set_read_address_8k (block_start, ROM_PRG_ROM + (rom_block << 13));
}


static INLINE void cpu_set_read_address_16k_rom_block (UINT16 block_start, int rom_block)
{
    rom_block = ROM_PRG_ROM_PAGE_LOOKUP
        [rom_block & ROM_PRG_ROM_PAGE_OVERFLOW_MASK];

    cpu_set_read_address_16k (block_start, ROM_PRG_ROM + (rom_block << 14));
}


static INLINE void cpu_set_read_address_32k_rom_block (UINT16 block_start, int rom_block)
{
    rom_block <<= 1;

    cpu_set_read_address_16k_rom_block (block_start, rom_block);
    cpu_set_read_address_16k_rom_block (block_start + 0x4000, rom_block + 1);
}


/* ----- FN2A03 Routines ----- */


static INLINE UINT8 FN2A03_Fetch (UINT16 Addr)
{
    // printf ("FN2A03_Fetch at $%04x.\n", Addr);

    if (cpu_block_2k_read_address [Addr >> 11] != 0)
    {
        return cpu_block_2k_read_address [Addr >> 11] [Addr] + cpu_patch_table [Addr];
    }
    else
    {
        return cpu_block_2k_read_handler [Addr >> 11] (Addr);
    }
}


static INLINE UINT8 FN2A03_Read (UINT16 Addr)
{
    // printf ("FN2A03_Read at $%04x.\n", Addr);

    if (cpu_block_2k_read_address [Addr >> 11] != 0)
    {
        return cpu_block_2k_read_address [Addr >> 11] [Addr] + cpu_patch_table [Addr];
    }
    else
    {
        return cpu_block_2k_read_handler [Addr >> 11] (Addr);
    }
}


static INLINE void FN2A03_Write (UINT16 Addr, UINT8 Value)
{
    // printf ("FN2A03_Write at $%04x ($%02x).\n", Addr, Value);

    if (cpu_block_2k_write_address [Addr >> 11] != 0)
    {
        cpu_block_2k_write_address [Addr >> 11] [Addr] = Value;
    }
    else
    {
        cpu_block_2k_write_handler [Addr >> 11] (Addr, Value);
    }
}


#ifndef INLINE_WITH_MACROS


static INLINE UINT8 FN2A03_Read_Stack (UINT8 S)
{
    return (cpu_ram [0x100 + S]);
}


static INLINE void FN2A03_Write_Stack (UINT8 S, UINT8 Value)
{
    cpu_ram [0x100 + S] = Value;
}


static INLINE UINT8 FN2A03_Read_ZP (UINT8 S)
{
    return (cpu_ram [S]);
}


static INLINE void FN2A03_Write_ZP (UINT8 S, UINT8 Value)
{
    cpu_ram [S] = Value;
}


#else   /* ! INLINE_WITH_MACROS */


#define FN2A03_Read_Stack(S)        (cpu_ram [0x100 + (UINT8) (S)])

#define FN2A03_Write_Stack(S,Value) (cpu_ram [0x100 + (UINT8) (S)] = Value)


#define FN2A03_Read_ZP(S)           (cpu_ram [(UINT8) (S)])

#define FN2A03_Write_ZP(S,Value)    (cpu_ram [(UINT8) (S)] = Value)


#endif



static INLINE UINT8 cpu_read (UINT16 address)
{
    return (FN2A03_Read (address));
}


static INLINE void cpu_write (UINT16 address, UINT8 value)
{
    FN2A03_Write (address, value);
}


void cpu_start_new_scanline (void);

static INLINE void cpu_consume_cycles (int cycles)
{
    FN2A03_consume_cycles (&cpu_context, cycles);
}


int cpu_get_cycles_line (void);
int cpu_get_cycles (int);


void cpu_save_state (PACKFILE *, int);

void cpu_load_state (PACKFILE *, int);


#ifdef __cplusplus
}
#endif

#endif /* ! __CPU_H__ */
