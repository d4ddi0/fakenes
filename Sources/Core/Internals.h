/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef CORE__INTERNALS_H__INCLUDED
#define CORE__INTERNALS_H__INCLUDED
#include "Common/Global.h"
#include "Common/Inline.h"
#include "Common/Types.h"
#include "Core/CPU.h"
#ifdef __cplusplus
extern "C" {
#endif

/* This macro helps us create a large number of arrays in a clean manner. */
#define CPU__ARRAY(_TYPE, _NAME, _SIZE)	_TYPE _NAME[(_SIZE)]

/* Memory map. */
#define CPU__MAP_SIZE		(0xFFFF + 1)
#define CPU__MAP_PAGE_SIZE	1024
#define CPU__MAP_PAGE_MASK	(CPU__MAP_PAGE_SIZE - 1)
#define CPU__MAP_PAGES		(CPU__MAP_SIZE / CPU__MAP_PAGE_SIZE)

/* Memory-mapped I/O is handled in one of two ways: Either via address
   mapping from virtual addresses to native addresses, or by installing
   read and write handler functions which can handle special-case
   scenarios (e.g hardware port access) but are slower. */
#define CPU__READ_ADDRESS_SIZE	CPU__MAP_PAGES
#define CPU__READ_HANDLER_SIZE	CPU__MAP_PAGES
#define CPU__READ_PATCH_SIZE	CPU__MAP_SIZE
#define CPU__WRITE_ADDRESS_SIZE	CPU__MAP_PAGES
#define CPU__WRITE_HANDLER_SIZE	CPU__MAP_PAGES

extern CPU__ARRAY( const UINT8*,      cpu__read_address,  CPU__READ_ADDRESS_SIZE  );
extern CPU__ARRAY( CPU_READ_HANDLER,  cpu__read_handler,  CPU__READ_HANDLER_SIZE  );
extern CPU__ARRAY( INT8,              cpu__read_patch,    CPU__READ_PATCH_SIZE    );
extern CPU__ARRAY( UINT8*,            cpu__write_address, CPU__WRITE_ADDRESS_SIZE );
extern CPU__ARRAY( CPU_WRITE_HANDLER, cpu__write_handler, CPU__WRITE_HANDLER_SIZE );

/* cpu__work_ram:
    This array contains the contents of work RAM (WRAM, or just RAM),
    which is general-purpose memory in the system of which the contents
    are lost at each power cycle, and also decays after reset. The NES
    has 2 KiB of work RAM located at $0000.

   cpu__save_ram:
    This array contains the contents of save RAM (SRAM), which is usually
    located in the upper memory space of the cartridge, near ROM. It is
    saved and restored between sessions, making the contents static.
    Not all games have or tolerate the presence of SRAM. Those that do
    generally place 8 KiB of it at $6000. */
#define CPU__WORK_RAM_SIZE	2048
#define CPU__SAVE_RAM_SIZE	8192

extern CPU__ARRAY( UINT8, cpu__work_ram,  CPU__WORK_RAM_SIZE );
extern CPU__ARRAY( UINT8, cpu__save_ram,  CPU__SAVE_RAM_SIZE );

/* Fast memory access routines. These avoid the function call overhead
   associated with cpu_read() and cpu_write(), by getting embedded
   directly into the caller routine. */
EXPRESS_FUNCTION UINT8 cpu__fast_read(const UINT16 address)
{
   const int page = address / CPU__MAP_PAGE_SIZE;

   if(cpu__read_handler[page]) {
      /* Use read handler. */
      CPU_READ_HANDLER handler = cpu__read_handler[page];
      return handler(address);
   }
   else {
      /* No read handler. */
      const UINT8* read = cpu__read_address[page];
      return read[address & CPU__MAP_PAGE_MASK] + cpu__read_patch[address];
   }
}

EXPRESS_FUNCTION void cpu__fast_write(const UINT16 address, const UINT8 data)
{
   const int page = address / CPU__MAP_PAGE_SIZE;

   if(cpu__write_handler[page]) {
      /* Use write handler. */
      CPU_WRITE_HANDLER handler = cpu__write_handler[page];
      handler(address, data);
   }
   else {
      /* No write handler. */
      UINT8* write = cpu__write_address[page];
      write[address & CPU__MAP_PAGE_MASK] = data;  
   }
}

/* Unpaged memory access routines. Code may use these to bypass
   memory-mapped I/O for performance. This is used for example for
   zero page accesses by the core. */
EXPRESS_FUNCTION UINT8 cpu__fast_ram_read(const UINT16 address)
{
   return cpu__work_ram[address] + cpu__read_patch[address];
}

EXPRESS_FUNCTION void cpu__fast_ram_write(const UINT16 address, const UINT8 data)
{
   cpu__work_ram[address] = data;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CORE__INTERNALS_H__INCLUDED */
