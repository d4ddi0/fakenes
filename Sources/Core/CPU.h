/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef CORE__CPU_H__INCLUDED
#define CORE__CPU_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Execution modes. These trade off performance for accuracy; Turbo is
   the fastest and most inaccurate, while Unchained runs the APU and PPU
   at every CPU clock cycle and supports advanced mapper logic. */
enum CPU_EXECUTION_MODEL {
   CPU_EXECUTION_MODEL_NORMAL = 0,
   CPU_EXECUTION_MODEL_TURBO,
   CPU_EXECUTION_MODEL_UNCHAINED,

   /* Default to a balance between speed and accuracy. */
   CPU_EXECUTION_MODEL_DEFAULT = CPU_EXECUTION_MODEL_NORMAL
};

/* Interrupt types that the CPU can generate, via cpu_set_interrupt(). */
enum CPU_INTERRUPT {
   CPU_INTERRUPT_NMI = 0,
   CPU_INTERRUPT_IRQ,
   CPU_INTERRUPT_IRQ_DMC,
   CPU_INTERRUPT_IRQ_FRAME,
   CPU_INTERRUPT_IRQ_MMC,
   CPU_INTERRUPT_IRQ_MMC_ASYNC
};

/* Block sizes (in pages), suitable for passing as the second parameter
   to the cpu_map_block_*() memory-mapping functions. Note that the
   default page granularity is 1 kB, so if that is changed in
   'Internals.h' than these will need to be updated. */
enum {
   CPU_MAP_BLOCK_1K  = 1,
   CPU_MAP_BLOCK_2K  = 2,
   CPU_MAP_BLOCK_4K  = 4,
   CPU_MAP_BLOCK_8K  = 8,
   CPU_MAP_BLOCK_16K = 16,
   CPU_MAP_BLOCK_32K = 32,
   CPU_MAP_BLOCK_64K = 64,

   /* Alias to map all of memory at once. */
   CPU_MAP_ALL = CPU_MAP_BLOCK_64K
};

/* Data types used for execution times. */
typedef UINT32 cpu_time_t;
typedef INT32 cpu_rtime_t;

/* These macros define the format of CPU read and write handlers for
   special-cased memory-mapped I/O. */
#define CPU_READ_HANDLER(_NAME)		UINT8 (*_NAME)(const UINT16 address)
#define CPU_WRITE_HANDLER(_NAME)	void (*_NAME)(const UINT16 address, const UINT8 data)

extern void cpu_load_config(void);
extern void cpu_save_config(void);
extern int cpu_init(void);
extern void cpu_exit(void);
extern void cpu_reset(void);
extern CPU_EXECUTION_MODEL cpu_get_execution_model(void);
extern void cpu_set_execution_model(const CPU_EXECUTION_MODEL model);
extern void cpu_update(void);
extern UINT8 cpu_read(const UINT16 address);
extern void cpu_write(const UINT16 address, const UINT8 data);
extern cpu_time_t cpu_execute(const cpu_time_t time);
extern void cpu_set_interrupt(const CPU_INTERRUPT type, const cpu_time_t time);
extern void cpu_clear_interrupt(const CPU_INTERRUPT type);
extern cpu_time_t cpu_get_time(void);
extern cpu_time_t cpu_get_time_elapsed(cpu_time_t* time);
extern void cpu_burn(const cpu_time_t time);
extern void cpu_load_state(PACKFILE *file, const int version);
extern void cpu_save_state(PACKFILE *file, const int version);
extern void cpu_enable_sram(void);
extern void cpu_disable_sram(void);
extern void cpu_map_block_address(const UINT16 address, const int pages, UINT8* data);
extern void cpu_map_block_read_address(const UINT16 address, const int pages, const UINT8* data);
extern void cpu_map_block_write_address(const UINT16 address, const int pages, UINT8* data);
extern void cpu_map_block_read_handler(const UINT16 address, const int pages, CPU_READ_HANDLER(handler));
extern void cpu_map_block_write_handler(const UINT16 address, const int pages, CPU_WRITE_HANDLER(handler));
extern void cpu_map_block_rom(const UINT16 address, const int pages, const int rom_page);
extern void cpu_unmap_block(const UINT16 address, const int pages);
extern void cpu_unmap_block_read(const UINT16 address, const int pages);
extern void cpu_unmap_block_write(const UINT16 address, const int pages);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CORE__CPU_H__INCLUDED */
