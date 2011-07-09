/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

/* iNES Mapper #7 (AxROM).
   This mapper is fully supported. */

#include "AxROM.h"
#include "Internals.h"
#include "Local.hpp"

// Mapper profile.
const MAPPER_PROFILE mapper_axrom = {
   7,				// iNES mapper number
   "AxROM",			// Symbolic name
   "AxROM\0\0\0",		// Symbolic ID
   mapper_axrom_init,		// Initialization handler
   mapper_axrom_reset,		// Reset handler
   mapper_axrom_load_state,	// Load state handler
   mapper_aorom_save_state,	// Save state handler
};

namespace {

/* The first four bits are ROM banking, the fifth bit is the mirroring,
   and the remaining 3 bits seem to be unused. */
const unsigned BankingMask = _00001111b;
const unsigned MirroringMask = _00010000b;

// The last value written to any register, used for state saving.
uint8 lastWrite = 0x00;

} // namespace anonymous

// Function prototypes (defined at bottom).
static void RegisterWrite(UINT16 address, UINT8 data);

// --------------------------------------------------------------------------------
// PUBLIC INTERFACE
// --------------------------------------------------------------------------------

int mapper_aorom_init(void)
{
   // AxROM contains no VROM hardware, so disable it.
   mmc_pattern_vram_in_use = TRUE;

   // Set up default mirroring.
   mmc_name_table_count = 2;
   ppu_set_default_mirroring(PPU_MIRRORING_ONE_SCREEN_2000);

   // Install write handler.
   cpu_map_block_write_handler(0x8000, CPU_MAP_BLOCK_32K, RegisterWrite);

   // Set initial state.
   mapper_axrom_reset();

   // Return success.
   return 0;
}

void mapper_axrom_reset(void)
{
   // Set up VRAM.
   ppu_set_8k_pattern_table_vram();

   // Set up mirroring.
   ppu_set_mirroring (PPU_MIRRORING_ONE_SCREEN_2000);

   // Select the first 32K ROM page.
   cpu_map_block_rom(0x8000, CPU_MAP_BLOCK_32K, 0);
}

void mapper_axrom_load_state(PACKFILE* file, const int version)
{
   Safeguard(file);

   // Restore mirroring and banking.
   RegisterWrite(0x8000, pack_getc(file));
}

void mapper_axrom_save_state(PACKFILE* file, const int version)
{
   Safeguard(file);

   // Save mirroring and banking.
   pack_putc(lastWrite, file);
}

// --------------------------------------------------------------------------------
// PRIVATE FUNCTIONS
// --------------------------------------------------------------------------------

static void RegisterWrite(UINT16 address, UINT8 data)
{
   // Preserve the written value for state saving.
   lastWrite = value;

   // Set single-screen mirroring.
   ppu_set_mirroring((data & MirroringMask) ?
      PPU_MIRRORING_ONE_SCREEN_2400 : PPU_MIRRORING_ONE_SCREEN_2000);

   // Set ROM banking using a 32K page size.
   cpu_map_block_rom(0x8000, CPU_MAP_BLOCK_32K, data & BankingMask);
}
