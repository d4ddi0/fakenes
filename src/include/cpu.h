/* FakeNES - A free, portable, Open Source NES emulator.

   cpu.h: Declarations for the CPU abstraction.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "core.h"
#include "rom.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Use macros instead of inline functions for stack
   and zero-page handlers. */
#define INLINE_WITH_MACROS

#define CPU_INTERRUPT_NONE              0
#define CPU_INTERRUPT_NMI               1
#define CPU_INTERRUPT_IRQ_BASE          2

/* Maskable IRQ, cleared after an IRQ is acknowledged. */
#define CPU_INTERRUPT_IRQ_SINGLE_SHOT   2
#define CPU_INTERRUPT_IRQ           CPU_INTERRUPT_IRQ_SINGLE_SHOT
#define CPU_INTERRUPT_IRQ_SOURCE(x) (CPU_INTERRUPT_IRQ_BASE + 1 + (x))
/* pAPU Delta Modulation Channel IRQ. */
#define CPU_INTERRUPT_IRQ_DMC           CPU_INTERRUPT_IRQ_SOURCE(0)
/* Frame IRQ. */
#define CPU_INTERRUPT_IRQ_FRAME         CPU_INTERRUPT_IRQ_SOURCE(1)
/* MMC-specific IRQ. */
#define CPU_INTERRUPT_IRQ_MMC           CPU_INTERRUPT_IRQ_SOURCE(2)
#define CPU_INTERRUPT_IRQ_SOURCE_MAX    FN2A03_INT_IRQ_SOURCE_MAX

#define CPU_RAM_SIZE    65536
#define CPU_SRAM_SIZE   8192

UINT8 cpu_ram[CPU_RAM_SIZE];
UINT8 cpu_sram[CPU_SRAM_SIZE];

FN2A03 cpu_context;

INT8 cpu_patch_table [CPU_RAM_SIZE];

/* Number of used entries in cpu_patch_info. */
int cpu_patch_count;

/* Format of cpu_patch_info. */
/* Designed primarily for use with Game Genie. */

typedef struct _CPU_PATCH
{
   BOOL active;
   USTRING title;
   UINT16 address;
   UINT8 value;
   UINT8 match_value;
   BOOL enabled;

} CPU_PATCH;

/* Try to keep it fast - 15 patches limit. */
#define CPU_MAX_PATCHES 15

CPU_PATCH cpu_patch_info[CPU_MAX_PATCHES];

int cpu_init (void);
void cpu_memmap_init (void);
void cpu_exit (void);
void cpu_reset (void);

void cpu_interrupt (int);
void cpu_clear_interrupt (int);

UINT16 *cpu_active_pc;

void cpu_free_prg_rom (const ROM *);
UINT8 *cpu_get_prg_rom_pages (ROM *);

void cpu_enable_sram (void);
void cpu_disable_sram (void);

UINT8 dummy_read[(8 << 10)];
UINT8 dummy_write[(8 << 10)];

#define MAX_BLOCKS   (64 / 2)

UINT8 *cpu_block_2k_read_address[MAX_BLOCKS];
UINT8 *cpu_block_2k_write_address[MAX_BLOCKS];

UINT8 (*cpu_block_2k_read_handler[MAX_BLOCKS]) (UINT16);
void  (*cpu_block_2k_write_handler[MAX_BLOCKS]) (UINT16, UINT8);

UINT8 cpu_read_direct_safeguard (UINT16);
void cpu_write_direct_safeguard (UINT16, UINT8);

void cpu_start_new_scanline (void);

cpu_time_t cpu_get_cycles_line (void);
cpu_time_t cpu_get_cycles (BOOL);

void cpu_save_state (PACKFILE *, int);
void cpu_load_state (PACKFILE *, int);

#include "cpu_in.h"

#ifdef __cplusplus
}
#endif
#endif    /* !CPU_H_INCLUDED */
