

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


#include <allegro.h>
#include "misc.h"

#include "rom.h"

#include "core.h"


#define CPU_INTERRUPT_IRQ   0

#define CPU_INTERRUPT_NMI   1


UINT8 cpu_ram [65536];

void sram_load (const char *rom_filename);
void sram_save (const char *rom_filename);

int cpu_init (void);

void cpu_memmap_init(void);

void cpu_exit (void);


void cpu_reset (void);


void cpu_interrupt (int);

void cpu_execute (int);


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


static INLINE void cpu_set_read_address_2k (UINT16 block_start, UINT8 *address)
{
    /* ignore low bits */
    block_start &= ~((2 << 10) - 1);

    cpu_block_2k_read_address [block_start >> 11] = address;
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

    cpu_block_2k_write_address [block_start >> 11] = address;
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
    if (rom_block >= (ROM_PRG_ROM_PAGES * 2))
    {
        rom_block &= ROM_PRG_ROM_PAGE_OVERFLOW_MASK;
    }

    cpu_set_read_address_8k (block_start, ROM_PRG_ROM + (rom_block << 13));
}


static INLINE void cpu_set_read_address_16k_rom_block (UINT16 block_start, int rom_block)
{
    if (rom_block >= ROM_PRG_ROM_PAGES)
    {
        rom_block &= ROM_PRG_ROM_PAGE_OVERFLOW_MASK / 2;
    }

    cpu_set_read_address_16k (block_start, ROM_PRG_ROM + (rom_block << 14));
}


static INLINE void cpu_set_read_address_32k_rom_block (UINT16 block_start, int rom_block)
{
    rom_block <<= 1;

    cpu_set_read_address_16k_rom_block (block_start, rom_block);
    cpu_set_read_address_16k_rom_block (block_start + 0x4000, rom_block + 1);
}


/* ----- M6502 Routines ----- */


static INLINE byte Op6502 (word Addr)
{
    // printf ("Op6502 at $%04x.\n", Addr);

    if (cpu_block_2k_read_address [Addr >> 11] != 0)
    {
        return cpu_block_2k_read_address [Addr >> 11] [Addr & 0x7FF];
    }
    else
    {
        return cpu_block_2k_read_handler [Addr >> 11] (Addr);
    }
}


static INLINE byte Rd6502 (word Addr)
{
    // printf ("Rd6502 at $%04x.\n", Addr);

    if (cpu_block_2k_read_address [Addr >> 11] != 0)
    {
        return cpu_block_2k_read_address [Addr >> 11] [Addr & 0x7FF];
    }
    else
    {
        return cpu_block_2k_read_handler [Addr >> 11] (Addr);
    }
}


static INLINE void Wr6502 (word Addr, byte Value)
{
    // printf ("Wr6502 at $%04x ($%02x).\n", Addr, Value);

    if (cpu_block_2k_write_address [Addr >> 11] != 0)
    {
        cpu_block_2k_write_address [Addr >> 11] [Addr & 0x7FF] = Value;
    }
    else
    {
        cpu_block_2k_write_handler [Addr >> 11] (Addr, Value);
    }
}


static INLINE byte Rd6502Stack (byte S)
{
    return (cpu_ram [0x100 + S]);
}


static INLINE void Wr6502Stack (byte S, byte Value)
{
    cpu_ram [0x100 + S] = Value;
}


static INLINE UINT8 cpu_read (UINT16 address)
{
    return (Rd6502 (address));
}


static INLINE void cpu_write (UINT16 address, UINT8 value)
{
    Wr6502 (address, value);
}


void cpu_start_new_scanline (void);

void cpu_consume_cycles (int cycles);

int cpu_get_cycles_line (void);
int cpu_get_cycles (int);

#ifdef __cplusplus
}
#endif

#endif /* ! __CPU_H__ */
