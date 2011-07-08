/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "Core/Core.hpp"
#include "Core/CPU.h"
#include "Core/CPUAtoms.hpp"
#include "Core/Internals.h"
#include "Core/Local.hpp"
#include "Core/Patch.h"

using namespace std;

namespace {

// Fail-safe to make sure cpu_init() has been called.
bool initialized = false;

// Current execution model. See 'CPU.h' for details.
CPU_EXECUTION_MODEL executionModel = CPU_EXECUTION_MODEL_DEFAULT;

/* This table maps symbolic CPU interrupt types (e.g MMC) to
   generic NMI or IRQ interrupts in the core. */
typedef map<CPU_INTERRUPT,COREInterruptType> InterruptTableType;
InterruptTableType interruptTable;

// Miscellaneous.
const uint16 SRAMBlockAddress = 0x6000;
const int SRAMBlockSize = CPU_MAP_BLOCK_8K;
const uint16 SRAMTrainerOffset = 0x1000;

const uint16 WRAMBlockAddress = 0x0000;
const int WRAMBlockSize = CPU_MAP_BLOCK_2K;

} // namespace anonymous

// Memory-map.
CPU__ARRAY( const UINT8*, cpu__read_address,  CPU__READ_ADDRESS_SIZE  );
CPU__ARRAY( void*,        cpu__read_handler,  CPU__READ_HANDLER_SIZE  );
CPU__ARRAY( INT8,         cpu__read_patch,    CPU__READ_PATCH_SIZE    );
CPU__ARRAY( UINT8*,       cpu__write_address, CPU__WRITE_ADDRESS_SIZE );
CPU__ARRAY( void*,        cpu__write_handler, CPU__WRITE_HANDLER_SIZE );

// Internal and external (cartridge) memory.
CPU__ARRAY( UINT8, cpu__save_ram,  CPU__SAVE_RAM_SIZE );
CPU__ARRAY( UINT8, cpu__work_ram,  CPU__WORK_RAM_SIZE );

// Function prototypes.
static UINT8 DummyRead(const UINT16 address);
static void DummyWrite(const UINT16 address, const UINT8 data);

// ----------------------------------------------------------------------

void cpu_load_config(void)
{
   executionModel = get_config_int("system", "execution_model", CPU_EXECUTION_MODEL_DEFAULT);
   switch(executionModel) {
      case CPU_EXECUTION_MODEL_NORMAL:
      case CPU_EXECUTION_MODEL_TURBO:
      case CPU_EXECUTION_MODEL_UNCHAINED:
         break;

      default: {
         WARN("Invalid execution model specified, falling back to the default.");
         executionModel = CPU_EXECUTION_MODEL_DEFAULT:
      }
   }
}

void cpu_save_config(void)
{
   set_config_int("system", "execution_model", executionModel);
}

int cpu_init(void)
{
   if(initialized) {
      WARN("cpu_init() called without a matching call to cpu_exit().");
      return 1;
   }

   // Clear arrays.
   memset(cpu__read_address,   0, CPU__READ_ADDRESS_SIZE);
   memset(cpu__read_handler,   0, CPU__READ_HANDLER_SIZE);
   memset(cpu__read_patch,     0, CPU__READ_PATCH_SIZE);
   memset(cpu__write_address , 0, CPU__WRITE_ADDRESS_SIZE);
   memset(cpu__write_handler,  0, CPU__WRITE_HANDLER_SIZE);

   memset(cpu__save_ram, 0, CPU__SAVE_RAM_SIZE);
   memset(cpu__work_ram, 0, CPU__WORK_RAM_SIZE);

   // Initialize memory map.
   cpu_unmap_block(0x0000, CPU_MAP_ALL);

   // Map in internal RAM.
   cpu_map_block_address(WRAMBlockAddress, WRAMBlockSize, cpu__work_ram);

   /* All internal memory ($0000-$07FF) was consistently set to $ff except
       * $0008=$F7
       * $0009=$EF
       * $000a=$DF
       * $000f=$BF */
   for(uint16 address = 0x0000; address <= 0x07FF; address++)
      cpu_write(address, 0xFF);

   cpu_write(0x0008, 0xF7);
   cpu_write(0x0009, 0xEF);
   cpu_write(0x000A, 0xDF);
   cpu_write(0x000F, 0xBF);

   // Check if the game uses battery-backed RAM.
   if(ROM_HAS_SRAM || ROM_HAS_TRAINER) {
      cpu_enable_sram();
      load_sram();

      if(ROM_HAS_TRAINER) {
         // Trainers exist at offset $1000 into SRAM.
         memcpy(cpu__save_ram + SRAMTrainerOffset, ROM_TRAINER, ROM_TRAINER_SIZE);
      }
   }

   // Build interrupt mapping table.
   interruptTable[CPU_INTERRUPT_NMI]           = COREInterruptNMI;
   interruptTable[CPU_INTERRUPT_IRQ]           = COREInterruptIRQ;
   interruptTable[CPU_INTERRUPT_IRQ_DMC]       = COREInterruptIRQ1;
   interruptTable[CPU_INTERRUPT_IRQ_FRAME]     = COREInterruptIRQ2;
   interruptTable[CPU_INTERRUPT_IRQ_MMC]       = COREInterruptIRQ3;
   interruptTable[CPU_INTERRUPT_IRQ_MMC_ASYNC] = COREInterruptIRQ4;

   /* Initialize the core. Note that this can only be done safely
      after the memory map has been initialized. */
   if(!CORE::Initialize()) {
      // The core failed to initialize.
      return 1;
   }

   // Set initial state.
   cpu_reset();

   // Return success.
   return 0;
}

void cpu_exit(void)
{
   // Save data.
   save_patches();
   save_sram();

   // Reset initialization state.
   initialized = false;
}

void cpu_reset(void)
{
   CORE::Reset();
}

CPU_EXECUTION_MODEL cpu_get_execution_model(void)
{
   return executionModel;
}

void cpu_set_execution_model(const CPU_EXECUTION_MODEL model)
{
   executionModel = model;
}

void cpu_update(void)
{
   // Updates the core to external timing changes.
   CORE::BuildTimeTable();
}

/* Generic read and write routines. These are subject to all
   memory-mapped I/O and memory patching, but should be avoided in
   situations where performance is crucial. */
UINT8 cpu_read(const UINT16 address)
{
   return cpu__fast_read(address);
}

void cpu_write(const UINT16 address, const UINT8 data)
{
   cpu__fast_write(address, data);
}

cpu_time_t cpu_execute(const cpu_time_t time)
{
   switch(executionModel) {
      case CPU_EXECUTION_MODEL_NORMAL:
         return CORE::Execute(time);
      case CPU_EXECUTION_MODEL_TURBO:
         return CORE::ExecuteTurbo(time);
      case CPU_EXECUTION_MODEL_UNCHAINED:
         return CORE::ExecuteUnchained(time);

      default:
         WARN_GENERIC();
         break;
   }

   return 0;
}

/* Interrupt routines. cpu_set_interrupt() queues an interrupt to
   occur at a specific time (in master clock cycles), and
   cpu_clear_interrupt() both unqueues any pending interrupts and
   acknowledges any existing interrupts. */
void cpu_set_interrupt(const CPU_INTERRUPT type, const cpu_time_t time)
{
   CORE::SetInterrupt(interruptTable[type], time);
}

void cpu_clear_interrupt(const CPU_INTERRUPT type)
{
   CORE::ClearInterrupt(interruptTable[type]);
}

/* Counter management routines. These get, set, or otherwise
   manipulate the internal cycle counter. */
cpu_time_t cpu_get_time(void)
{
   return CORE::GetTime();
}

cpu_time_t cpu_get_time_elapsed(cpu_time_t* time)
{
   Safeguard(time);

   const cpu_time_t elapsed = CORE::GetTimeElapsed(*time);
   *time = cpu_get_time();

   return elapsed;
}

void cpu_burn(const cpu_time_t time)
{
   CORE::Burn(time);
}

// Save state routines.
void cpu_load_state(PACKFILE* file, const int version)
{
   Safeguard(file);

   COREContext context;
   memset(context, 0, sizeof(COREContext));

   // Load clock counter.
   context.time = pack_igetl(file);

   // Load registers.
   context.registers.pc.word = pack_igetw(file);
   context.registers.a = pack_getc(file);
   context.registers.p = pack_getc(file);
   context.registers.s = pack_getc(file);
   context.registers.x = pack_getc(file);
   context.registers.y = pack_getc(file);

   // Load interrupt queue.
   context.interrupts.clear();

   const int count = pack_getc(file);
   for(int i = 0; i < count; i++) {
      COREInterrupt interrupt;
      interrupt.time = pack_igetl(file);
      interrupt.type = pack_getc(file);

      context.interrupts.push_back(interrupt);
   }

   // Miscellaneous.
   context.afterCLI = LOAD_BOOLEAN(pack_getc(file));

   // Load memory contents.
   pack_fread(cpu__save_ram, CPU__SAVE_RAM_SIZE, file);
   pack_fread(cpu__work_ram, CPU__WORK_RAM_SIZE, file);

   // Set context.
   CORE::SetContext(context);
}

void cpu_save_state(PACKFILE* file, const int version)
{
   Safeguard(file);

   COREContext context;
   CORE::GetContext(&context);

   // Save clock counter.
   pack_iputl(context.time, file);

   // Save registers.
   pack_iputw(context.registers.pc.word, file);
   pack_putc(context.registers.a, file);
   pack_putc(context.registers.p, file);
   pack_putc(context.registers.s, file);
   pack_putc(context.registers.x, file);
   pack_putc(context.registers.y, file);

   // Save interrupt queue.
   pack_putc(file, context.interrupts.size());
   for(COREInterruptQueue::iterator i = context.interrupts.begin(); i != context.interrupts.end(); ) {
      COREInterrupt& interrupt = *i;
      pack_putc(interrupt.type, file);
      pack_iputl(interrupt.time, file);

      i++;
   }

   // Miscellaneous.
   pack_putc(SAVE_BOOLEAN(context.afterCLI), file);

   // Save memory contents.
   pack_fwrite(cpu__save_ram, CPU__SAVE_RAM_SIZE, file);
   pack_fwrite(cpu__work_ram, CPU__WORK_RAM_SIZE, file);
}

/* Memory-mapping routines. These allow for the configuration of the
   dynamic memory map. */
void cpu_enable_sram(void)
{
   cpu_map_block_address(SRAMBlockAddress, SRAMBlockSize, cpu__save_ram);
}

void cpu_disable_sram(void)
{
   cpu_unmap_block(SRAMBlockAddress, SRAMBlockSize);
}

void cpu_map_block_address(const UINT16 address, const int pages, UINT8* data)
{
   Safeguard(pages > 0);
   Safeguard(data);

   cpu_map_block_read_address(address, pages, data);
   cpu_map_block_write_address(address, pages, data);
}

void cpu_map_block_read_address(const UINT16 address, const int pages, const UINT8* data)
{
   Safeguard(pages > 0);
   Safeguard(data);

   const int start = address / CPU__MAP_PAGE_SIZE;

   for(int page = 0; page < pages; page++) {
      const int index = start + page;
      cpu__read_address[index] = data + (page * CPU__MAP_PAGE_SIZE);
      cpu__read_handler[index] = NULL;
   }

   const uint16 startAddress = start * CPU__MAP_PAGE_SIZE;
   const uint16 endAddress = startAddress + (pages * CPU__MAP_PAGE_SIZE);
   map_patches(startAddress, endAddress);
}

void cpu_map_block_write_address(const UINT16 address, const int pages, UINT8* data)
{
   Safeguard(pages > 0);
   Safeguard(data);

   const int start = address / CPU__MAP_PAGE_SIZE;

   for(int page = 0; page < pages; page++) {
      const int index = start + page;
      cpu__write_address[index] = data + (page * CPU__MAP_PAGE_SIZE);
      cpu__write_handler[index] = NULL;
   }
}

void cpu_map_block_read_handler(const UINT16 address, const int pages, CPU_READ_HANDLER(handler))
{
   Safeguard(pages > 0);
   Safeguard(handler);

   const int start = address / CPU__MAP_PAGE_SIZE;

   for(int page = 0; page < pages; page++) {
      const int index = start + page;
      cpu__read_address[index] = NULL;
      cpu__read_handler[index] = handler;
   }

   const uint16 startAddress = start * CPU__MAP_PAGE_SIZE;
   const uint16 endAddress = startAddress + (pages * CPU__MAP_PAGE_SIZE);
   map_patches(startAddress, endAddress);
}

void cpu_map_block_write_handler(const UINT16 address, const int pages, CPU_WRITE_HANDLER(handler))
{
   Safeguard(pages > 0);
   Safeguard(handler);

   const int start = address / CPU__MAP_PAGE_SIZE;

   for(int page = 0; page < pages; page++) {
      const int index = start + page;
      cpu__write_address[index] = NULL;
      cpu__write_handler[index] = handler;
   }
}

void cpu_map_block_rom(const UINT16 address, const int pages, const int rom_page)
{
   Safeguard(pages > 0);
   Safeguard(rom_page >= 0);

   switch(pages) {
      case CPU_MAP_BLOCK_8K: {
         const int page = (rom_page & 1) + (ROM_PRG_ROM_PAGE_LOOKUP[(rom_page / 2) &
            ROM_PRG_ROM_PAGE_OVERFLOW_MASK] * 2);

         cpu_map_block_read_address(address, pages, ROM_PAGE_8K(page));

         break;
      }

      case CPU_MAP_BLOCK_16K: {
         const int page = ROM_PRG_ROM_PAGE_LOOKUP[rom_page & ROM_PRG_ROM_PAGE_OVERFLOW_MASK];
         cpu_map_block_read_address(address, pages, ROM_PAGE_16K(page));

         break;
      }

      case CPU_MAP_BLOCK_32K: {
         const int page = rom_page * 2;
         cpu_map_block_rom(address, CPU_MAP_BLOCK_16K, page);
         cpu_map_block_rom(address + ROM_PAGE_SIZE_16K, CPU_MAP_BLOCK_16K, page + 1);

         break;
      }

      default: {
         WARN("Invalid ROM page size specified (must be 8, 16 or 32).");
         return;
      }
   }
}

void cpu_unmap_block(const UINT16 address, const int pages)
{
   Safeguard(pages > 0);

   cpu_unmap_block_read(address, pages);
   cpu_unmap_block_write(address, pages);
}

void cpu_unmap_block_read(const UINT16 address, const int pages)
{
   Safeguard(pages > 0);

   cpu_map_block_read_handler(address, pages, DummyRead);
}

void cpu_unmap_block_write(const UINT16 address, const int pages)
{
   Safeguard(pages > 0);

   cpu_map_block_write_handler(address, pages, DummyWrite);
}

// --------------------------------------------------------------------------------

// Dummy read/write functions, used when nothing is mapped in.
static UINT8 DummyRead(const UINT16 address)
{
   return 0x00;
}

static void DummyWrite(const UINT16 address, const UINT8 data)
{
}
