/* FakeNES - A free, portable, Open Source NES emulator.

   ppu.cpp: Implementation of the NES Picture
   Processing Unit (PPU) emulation.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */
 
#include <allegro.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "binary.h"
#include "common.h"
#include "cpu.h"
#include "input.h"
#include "mmc.h"
#include "ppu.h"
#include "ppu_int.h"
#include "rom.h"
#include "renderer/renderer.hpp"
#include "shared/crc32.h"
#include "timing.h"
#include "types.h"
#include "video.h"

#define INLINE_MA6R
#include "renderer/renderer.cpp"

/* Coding conventions:
     - Functions and variables exposed publically must follow legacy conventions.
     - Functions and variables used only privately may follow newer, cleaner conventions.
     - A fixed-width data type must be used on anything that gets saved to a save state file.
     - When a variable won't be saved to a save state, try to stick with int or unsigned. */
     - The 'linear' keyword should be used on any function called only once.
     - Similarly, the 'inline' keyword should be used on functions that get called excessively.
     - When a variable won't be modified, make its value const, to improve optimization.
     - When calling functions, pass parameters as const whenever possible. */

// TODO: Proper initialization and reset.
// TODO: State saving support.
// TODO: Add MMC2 & MMC4 latches support.

// --------------------------------------------------------------------------------

/* Internal functions:
      These are used exclusively by this file only. */
// VRAM reading & writing.
static linear uint8 VRAMRead();
static inline uint8 VRAMReadUnbuffered();
static linear void VRAMWrite(const uint8 data)
static inline void IncrementVRAMAddress();
// Sprite VRAM (OAM) reading & writing.
static linear uint8 OAMRead();
static inline void OAMWrite(const uint8 data);
static linear void StartOAMDMA(const uint8 page);
// Mirroring.
static void SetupMirroring();
static void SetupSingleScreenMirroring();
// NMI prediction.
static void PredictNMI(const cpu_time_t cycles);
static void RepredictNMI();
static inline bool ClockScanlineTimer(int16 &nextScanline);
// Frame timing.
static void Synchronize();
static inline cpu_time_t GetTimeElasped();
static linear void StartFrame();
static linear void EndFrame();
static linear void StartScanline();
static linear void StartScanlineCycle(const cpu_time_t cycle);
static linear void EndScanline();
static linear void OAMDMA();

/* Internal state:
      These variables store internal state information, particularly timing information.
      Unless otherwise specified, all cycle counters store *master* clock cycles.  */
namespace PPUState {

cpu_time_t clockBuffer = 0;			// Remaining unexecuted cycles
cpu_time_t clockCounter = 0;			// Last time synchronization was performed
int        initializing = 0;                    // Set while the PPU is initializing
bool       isOddFrame = false;			// Toggled every frame, for emulating quirks
cpu_time_t nmiPredictionCycles = 0;		// Amount of cycles in the prediction buffer
cpu_time_t nmiPredictionTimestamp = 0;		// Last time NMI prediction was performed
uint8      oamDMAByte = 0x00;			// Current sprite DMA transfer byte
bool       oamDMAFlipFlop = false;		// Read a byte when false, write a byte when true
uint8      oamDMAReadAddress = 0x00;		// Sprite DMA source address in system RAM
uint8      oamDMATimer = 0;		        // Number of PPU cycles until the next read or write
uint16     oamDMAWriteAddress = 0x100;		// Sprite DMA target address in OAM (disabled when >$FF)
uint8      readBuffer = 0x00;                 	// Last buffered VRAM read
int16      scanline = 0;			// The current scanline being processed
int16      scanlineTimer = 0;			// Number of PPU cycles left on the current scanline
bool       scrollFlipFlop = false;		// Toggled after every write to PPUSCROLL
bool       synchronizing = false;		// Set during synchronization; see SyncHelper()
bool       timeWarp = false;			// Inserts extra cycles during synchronization
uint8      vblankQuirkTime = 0;			// For emulating the VBL flag read quirk
bool       vramAddressFlipFlop = false;		// Toggled after every write to PPUADDR
uint8      writeBuffer = 0x00;			// Last data byte written to a register

} // namespace PPUState

/* Internal variables:
      These should not be read or written outside of the PPU, Renderer and in some special cases
      Memory-Mapping Controller (MMC).  Always use public functions (when available) instead.
      If no associated function is available for a given option, do not ever access it.
      Lastly, publicly accessible but internal variables should include *two* underscores. */
#define INITIAL_BACKGROUND_TILESET	0x0000
#define INITIAL_BASE_NAME_TABLE_ADDRESS	0x2000
#define INITIAL_MIRRORING		PPU_MIRRORING_HORIZONTAL
#define INITIAL_PALETTE_MASK		0x3F
#define INITIAL_SPRITE_HEIGHT		8
#define INITIAL_SPRITE_TILESET		0x0000

// General registers.
UINT16 ppu__base_name_table_address = INITIAL_BASE_NAME_TABLE_ADDRESS; // Used for scrolling
BOOL   ppu__generate_interrupts = FALSE;	// Generate NMIs at the start of VBlank
UINT8  ppu__vram_address_increment = 0;		// Applied after each VRAM access
// Background registers.
UINT16 ppu__background_tileset = INITIAL_BACKGROUND_TILESET; // Base address for background tiles
BOOL   ppu__clip_background = FALSE;		// Hide background in the left 8 pixels of the screen
BOOL   ppu__enable_background = FALSE;		// Enable emulation of the background-rendering pipeline
// Sprite registers.
BOOL   ppu__clip_sprites = FALSE;		// Hide sprites in the left 8 pixels of the screen
BOOL   ppu__enable_sprites = FALSE;		// Enable emulation of the sprite-rendering pipeline
BOOL   ppu__sprite_collision = FALSE;		// Sprite #0 collision flag
UINT8  ppu__sprite_height = INITIAL_SPRITE_HEIGHT; // Sprite height, either 8 or 16 pixels
BOOL   ppu__sprite_overflow = FALSE;		// More than eight sprites on a scanline
UINT16 ppu__sprite_tileset = INITIAL_SPRITE_TILESET; // Base address for sprite tiles, only used in 8x8 mode
// Color registers.
BOOL  ppu__intensify_reds = FALSE;		// Darkens greens and blues
BOOL  ppu__intensify_greens = FALSE;		// Darkens reds and blues
BOOL  ppu__intensify_blues = FALSE;		// Darkens reds and greens
UINT8 ppu__palette_mask = INITIAL_PALETTE_MASK;	// Controlled by monochrome mode flag in PPUSTATUS

/* These variables are usually derived from registers, but are not directly associated with a
   software-modifiable setting. Rather, they control logic indirectly. */
BOOL   ppu__enabled = FALSE;			// Don't access VRAM when this is cleared
ENUM   ppu__default_mirroring = INITIAL_MIRRORING; // Set by cartridge and loaded at reset
ENUM   ppu__mirroring = ppu__default_mirroring;	// Name table layout and mirroring
UINT8  ppu__oam_address = 0;                    // Current read/write location in OAM
UINT8* ppu__one_screen_base_address = NULL;	// Base address for one screen mirroring
UINT8  ppu__scroll_x_position = 0;		// Background scrolling horizontal offset
UINT8  ppu__scroll_y_position = 0;		// Background scrolling vertical offset
UINT16 ppu__vram_address = 0;			// Current read/write location in VRAM
BOOL   ppu__vblank_started = FALSE;		// Set when first vblank scanline reached

// These variables are used exclusively by the renderer.
UINT8 ppu__background_pixels[PPU__BACKGROUND_PIXELS_SIZE]; // Used for sprite #0 hitscan and sprite priority
BOOL  ppu__enable_background_layer = TRUE;	// Enable drawing of the background to the framebuffer
BOOL  ppu__enable_sprite_back_layer = TRUE;	// Enable drawing of sprites to the framebuffer
BOOL  ppu__enable_sprite_front_layer = TRUE;	// Same as above, but for front-priority sprites
BOOL  ppu__force_rendering = FALSE;		// Overrides ppu__rendering_enabled
BOOL  ppu__rendering_enabled = TRUE;		// Disable rendering, used for frame skip

/* Cached settings:
      These are options that can be modified publically. However, since we don't ever want to
      corrupt the PPUs state, they are only reloaded at the end of each frame. All of these
      are set via ppu_set_option() and are mostly cosmetic. Cached options are never cleared
      across a reset or by a power cycle. */
static bool cache_enable_background_layer = ppu__enable_background_layer;
static bool cache_enable_sprite_back_layer = ppu__enable_sprite_back_layer;
static bool cache_enable_sprite_front_layer = ppu__enable_sprite_front_layer;
static bool cache_rendering_enabled = ppu__rendering_enabled;

// --------------------------------------------------------------------------------

// Video memory - name tables and pattern tables.
#define BYTES_PER_NAME_TABLE		1024
#define BYTES_PER_PATTERN_TABLE		4096
#define NAME_TABLE_COUNT		2
#define NAME_TABLE_MAXIMUM		4
#define NAME_TABLE_PAGE_SIZE		BYTES_PER_NAME_TABLE
#define NAME_TABLE_VRAM_SIZE 		(BYTES_PER_NAME_TABLE * NAME_TABLE_COUNT)
#define PATTERN_TABLE_COUNT		2
#define PATTERN_TABLE_PAGE_SIZE		1024
#define PATTERN_TABLE_PAGE_COUNT	(8192 / PATTERN_TABLE_PAGE_SIZE)
#define PATTERN_TABLE_VRAM_SIZE		(BYTES_PER_PATTERN_TABLE * PATTERN_TABLE_COUNT)

UINT8  ppu__name_table_dummy[BYTES_PER_NAME_TABLE];
UINT8  ppu__name_table_vram[NAME_TABLE_VRAM_SIZE];
const UINT8* ppu__name_tables_read[NAME_TABLE_MAXIMUM];
UINT8* ppu__name_tables_write[NAME_TABLE_MAXIMUM];
UINT8  ppu__pattern_table_dummy[BYTES_PER_PATTERN_TABLE];
UINT8  ppu__pattern_table_vram[PATTERN_TABLE_VRAM_SIZE]
const UINT8* ppu__pattern_tables_read[PATTERN_TABLE_PAGE_COUNT];
UINT8* ppu__pattern_tables_write[PATTERN_TABLE_PAGE_COUNT];

// Video memory - palettes.
#define BACKGROUND_PALETTE_COUNT	4
#define BYTES_PER_PALETTE		4
#define BYTES_PER_SPRITE		4
#define SPRITE_COUNT			64
#define SPRITE_PALETTE_COUNT		4
#define SPRITE_VRAM_SIZE		(BYTES_PER_SPRITE * SPRITE_COUNT)
#define PALETTE_VRAM_SIZE		(BYTES_PER_PALETTE * (BACKGROUND_PALETTE_COUNT + SPRITE_PALETTE_COUNT))

UINT8* ppu__background_palettes[BACKGROUND_PALETTE_COUNT];
UINT8  ppu__palette_vram[PALETTE_VRAM_SIZE];
UINT8* ppu__sprite_palettes[SPRITE_PALETTE_COUNT];
UINT8  ppu__sprite_vram[SPRITE_VRAM_SIZE];

/* Table containing expanded name/attribute data. Use ppu_set_expansion_table_address(block)
   to set this, or ppu_set_expansion_table_address(NULL) to disable EXRAM. The format
   should be identical to that used by MMC5. */
const UINT8* ppu__expansion_table = NULL;

// Masks, shifts and tables for data referenced by reads or writes to the registers.
#define BACKGROUND_PATTERN_TABLE_ADDRESS_MASK	_00010000b
#define BACKGROUND_PATTERN_TABLE_ADDRESS_OFF	0x0000
#define BACKGROUND_PATTERN_TABLE_ADDRESS_ON	0x1000
#define BASE_NAME_TABLE_ADDRESS_MASK		_00000011b
#define BASE_NAME_TABLE_ADDRESS_SHIFT		0
#define BASE_NAME_TABLE_ADDRESS_MAX		3
#define GENERATE_NMI_MASK                       _10000000b
#define GRAYSCALE_MASK				_00000001b
#define GRAYSCALE_OFF				0x3F
#define GRAYSCALE_ON				0x30
#define INTENSIFY_BLUES_MASK			_10000000b
#define INTENSIFY_GREENS_MASK			_01000000b
#define INTENSIFY_REDS_MASK			_00100000b
#define SHOW_BACKGROUND_MASK			_00001000b
#define SHOW_BACKGROUND_LEFTMOST_COLUMN_MASK	_00000010b
#define SHOW_SPRITES_MASK			_00010000b
#define SHOW_SPRITES_LEFTMOST_COLUMN_MASK	_00000100b
#define SPRITE_OVERFLOW_BIT			_00100000b
#define SPRITE_PATTERN_TABLE_ADDRESS_MASK	_00001000b
#define SPRITE_PATTERN_TABLE_ADDRESS_ON		0x1000
#define SPRITE_PATTERN_TABLE_ADDRESS_OFF	0x0000
#define SPRITE_SIZE_MASK			_00100000b
#define SPRITE_SIZE_OFF				8
#define SPRITE_SIZE_ON				16
#define SPRITE_ZERO_HIT_BIT			_01000000b
#define VERTICAL_BLANK_BIT			_10000000b
#define VRAM_ADDRESS_INCREMENT_MASK		_00000010b
#define VRAM_ADDRESS_INCREMENT_OFF		1
#define VRAM_ADDRESS_INCREMENT_ON		32

static const UINT16 base_name_table_address_lut[BASE_NAME_TABLE_ADDRESS_MAX + 1] = {
   0x2000, 0x2400, 0x2800, 0x2C00
};

/* These macros help in extracting values from data passed to ppu_write(). They are designed to be
   used in combination with the information defined above. */
#define DATA_FLAG(_name) \
   ( TRUE_OR_FALSE(data & _name##_MASK) )
#define DATA_SWITCH(_name) \
   ( (data & _name##_MASK) ? _name##_ON : _name##_OFF )
#define DATA_TABLE(_name,_table) \
    ( (_table)[(data & _name##_MASK) >> name##_SHIFT] )

/* For sprite DMA transfers, this is the number of PPU cycles between each read or write.
   The first read will occur after 3 cycles (as DMA takes at least 1 CPU cycle to initialize), and
   the first write will occur after 3+3=6 cycles. See StartOAMDMA() for further details. */
#define OAM_DMA_READ_CYCLES	3
#define OAM_DMA_WRITE_CYCLES	3

// --------------------------------------------------------------------------------

/* Since Synchronize() calls other functions such as MMC handlers, and those handlers in turn can
   access the PPU, a re-entry condition develops that could be problematic. In order to solve this,
   we want to avoid a synchronization attempt when one is already in progress. In these cases,
   the PPU state is reflected as it is at the time of the external call, which is usually correct.
   Otherwise, a full synchronization is performed and the PPU state is up-to-date.

   As an example, imagine that a call to ppu_read() is performed by the CPU. The PPU performs a
   synchronization in order to "catch up", but this in turn triggers a call to mmc_scanline_start().
   The MMC handler then tries to map PPU memory, resulting in calls back to the PPU, which would
   then try to synchronize again, possibly causing state corruption. By avoiding the second
   synchronization attempt, the MMC sees the state of the PPU up until that point, makes its
   changes, then the first synchronization attempt continues from there.

   Another problematic situation we want to avoid is synchronization while the PPU is initializing.
   We want to avoid doing any processing until *all* of the components of the virtual machine have
   been fully initialized, especially the CPU as we depend on its counters. */
#define SyncHelper() { \
   if(!(PPUState::initializing || PPUState::synchronizing)) \
      Synchronize(); \
}

// --------------------------------------------------------------------------------
// PUBLIC INTERFACE
// --------------------------------------------------------------------------------

int ppu_init(void)
{
   // Begin initialization sequence.
   PPUState::initializing = 1;

   // End initialization sequence.
   PPUState::initializing--;
}

void ppu_exit(void)
{
}

void ppu_reset(void)
{
   // Begin initialization sequence.
   PPUState::initializing++;

   // End initialization sequence.
   PPUState::initializing--;
}

UINT8 ppu_read(const UINT16 address)
{
   SyncHelper();

   /* The PPU exposes eight memory-mapped registers to the CPU.
      These nominally sit at $2000 through $2007 in the CPU's address space,
      but because they're incompletely decoded,
      they're mirrored in every 8 bytes from $2008 through $3FFF,
      so a write to $3456 is the same as a write to $2006. */
   if((address < 0x2000) || (address > 0x3FFF))
      // Out of range
      return;

   /* Map mirrored address to registers:
         0 - Write-only
         1 - Write-only
         2 - PPUSTATUS
         3 - Write-only
         4 - OAMDATA
         5 - Write-only
         6 - Write-only
         7 - PPUDATA */
   switch(address & 7) {
      // PPUSTATUS
      case 2: {
         /* 76543210
            ||||||||
            |||+++++- Least significant bits previously written into a PPU register
            |||       (due to register not being updated for this address)
            ||+------ Sprite overflow. The PPU can handle only eight sprites on one
            ||        scanline and sets this bit if it starts dropping sprites.
            ||        Normally, this triggers when there are 9 sprites on a scanline,
            ||        but the actual behavior is significantly more complicated.
            |+------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 'hits'
            |            a nonzero background pixel.  Used for raster timing.
            +-------- Vertical blank has started (0: not in VBLANK; 1: in VBLANK) */
  
         uint8 data = PPUState::writeBuffer & _00011111b;

         if(ppu__sprite_overflow)
            data |= SPRITE_OVERFLOW_BIT;
         if(ppu__sprite_collision)
            data |= SPRITE_ZERO_HIT_BIT;

         if(ppu__vblank_started) {
            /* Reading PPUSTATUS at the exact start of vertical blank will return a 0 in D7 but clear the latch anyway,
               causing the program to miss frames. */
            if(PPUState::vblankQuirkTime == 0)
               data |= VERTICAL_BLANK_BIT;

            ppu__vblank_started = FALSE;
         }

         break;
      }

      // OAMDATA
      case 4:
         return OAMRead();
         break;

      // PPUDATA
      case 7:
         return VRAMRead();
         break;
   }

   return 0x00;
}

void ppu_write(const UINT16 address, const UINT8 data)
{
   SyncHelper();

   // Register $4014 is used for performing sprite DMA. 
   if(address == 0x4014) {
      StartOAMDMA(data);
      return;
   }
   else if((address < 0x2000) || (address > 0x3FFF))
      // Out of range
      return;

   // Save our data so it can be accessed by later reads/writes.
   PPUState::writeBuffer = data;

   /* Map mirrored address to registers:
         0 - PPUCTRL
         1 - PPUMASK
         2 - Read-only
         3 - OAMADDR
         4 - OAMDATA
         5 - PPUSCROLL
         6 - PPUADDR
         7 - PPUDATA */
   switch(address & 7) {
      // PPUCTRL
      case 0: {
         /* 76543210
            ||||||||
            ||||||++- Base nametable address
            ||||||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
            |||||+--- VRAM address increment per CPU read/write of PPUDATA
            |||||     (0: increment by 1, going across; 1: increment by 32, going down)
            ||||+---- Sprite pattern table address for 8x8 sprites
            ||||      (0: $0000; 1: $1000; ignored in 8x16 mode)
            |||+----- Background pattern table address (0: $0000; 1: $1000)
            ||+------ Sprite size (0: 8x8; 1: 8x16)
            |+------- PPU master/slave select (has no effect on the NES)
            +-------- Generate an NMI at the start of the
                      vertical blanking interval (0: off; 1: on) */

         // Save the current value of the NMI flag, to determine if reprediction is neccessary.
         const BOOL old_bit_7 = ppu__generate_interrupts;

         ppu__base_name_table_address = DATA_TABLE( BASE_NAME_TABLE_ADDRESS, base_name_table_address_lut );
         ppu__vram_address_increment = DATA_SWITCH( VRAM_ADDRESS_INCREMENT );
         ppu__sprite_tileset = DATA_SWITCH( SPRITE_PATTERN_TABLE_ADDRESS );
         ppu__background_tileset = DATA_SWITCH( BACKGROUND_PATTERN_TABLE_ADDRESS );
         ppu__generate_interrupts = DATA_FLAG( GENERATE_NMI );

         // If the state of bit 7 has changed, then we need to repredict the NMI.
         if(ppu__generate_interrupts != old_bit_7)
            RepredictNMI();

         break;
      }

      // PPUMASK
      case 1: {
         /* 76543210
            ||||||||
            |||||||+- Grayscale (0: normal color; 1: produce a monochrome display)
            ||||||+-- 1: Show background in leftmost 8 pixels of screen; 0: Hide
            |||||+--- 1: Show sprites in leftmost 8 pixels of screen; 0: Hide
            ||||+---- 1: Show background
            |||+----- 1: Show sprites
            ||+------ Intensify reds (and darken other colors)
            |+------- Intensify greens (and darken other colors)
            +-------- Intensify blues (and darken other colors) */

         ppu__palette_mask = DATA_SWITCH( GRAYSCALE );
         ppu__clip_background = !DATA_SWITCH( SHOW_BACKGROUND_LEFTMOST_COLUMN );
         ppu__clip_sprites = !DATA_SWITCH( SHOW_SPRITES_LEFTMOST_COLUMN );
         ppu__enable_background = DATA_SWITCH( SHOW_BACKGROUND );
         ppu__enable_sprites = DATA_SWITCH( SHOW_SPRITES );
         ppu__intensify_reds = DATA_FLAG( INTENSIFY_REDS );
         ppu__intensify_greens = DATA_FLAG( INTENSIFY_GREENS );
         ppu__intensify_blues = DATA_FLAG( INTENSIFY_BLUES );

         // When both background and sprites are disabled, the PPU enters forced blanking.
         ppu__enabled = ppu__enable_background || ppu__enable_sprites;

         break;
      }

      // OAMADDR
      case 3:
         ppu__oam_address = data;
         break;

      // OAMDATA
      case 4:
         OAMWrite(data);
         break;

      // PPUSCROLL
      case 5: {
         if(PPUState::scrollFlipFlop)
            ppu_scroll_y_offset = data;
         else
            ppu_scroll_x_offset = data;
 
         /* A simple flip-flop is used to determine which byte to write to in the latch (low or high).
            By inverting it every write, it switches to the other byte. */
         PPUState::scrollFlipFlop = !PPUState::scrollFlipFlop;

         break;
      }

      // PPUADDR
      case 6: {
         ppu__vram_address |= data << (PPUState::vramAddressFlipFlop ? 8 : 0);

         // Check if the new address is complete.
         if(PPUState::vramAddressFlipFlop)
            /* Valid addresses are $0000-$3FFF; higher addresses will be mirrored down. */
            ppu__vram_address &= 0x3FFF;

         // Toggle flip-flop for the address latch.
         PPUState::vramAddressFlipFlop = !PPUState::vramAddressFlipFlop;

         break;
      }

      // PPUDATA
      case 7:
         VRAMWrite(data);
         break;
   }
}

void ppu_predict_nmi(const cpu_time_t cycles)
{
   SyncHelper();

   // Save parameters for re-prediction if a mid-scanline change occurs.
   PPUState::predictionTimestamp = cpu_get_cycles();
   // We'll actually emulate a little bit longer than requested, since it doesn't hurt to do so.
   PPUState::predictionCycles = cycles + PREDICTION_BUFFER_CYCLES + (1 * PPU_CLOCK_MULTIPLIER);

   // Convert from master clock to PPU clock.
   const cpu_time_t ppuCycles = PPUState::predictionCycles / PPU_CLOCK_DIVIDER;
   if(ppuCycles == 0)
      return;

   PredictNMI(ppuCycles);
}

void ppu_sync_update(void)
{
   // This just makes sure that the PPU state is up-to-date.
   SyncHelper();
}

void ppu_set_option(const ENUM option, const BOOL value)
{
   switch(option) {
      case PPU_OPTION_ENABLE_RENDERING:
         cache_enable_rendering = value;
         break;
      case PPU_OPTION_ENABLE_BACKGROUND_LAYER:
         cache_enable_background_layer = value;
         break;
      case PPU_OPTION_ENABLE_SPRITE_BACK_LAYER:
         cache_enable_sprite_back_layer = value;
         break;
      case PPU_OPTION_ENABLE_SPRITE_FRONT_LAYER;
         cache_enable_sprite_front_layer = value;
         break;

      default:
         WARN_GENERIC();
         break;
   }
}

BOOL ppu_get_option(const ENUM option)
{
   /* Note that this function always returns the cached values, for the GUI to work properly.
      The actual PPU state may be delayed by as much as one frame. */
   switch(option) {
      case PPU_OPTION_ENABLE_RENDERING:
         return cache_enable_rendering;
      case PPU_OPTION_ENABLE_BACKGROUND_LAYER:
         return cache_enable_background_layer;
      case PPU_OPTION_ENABLE_SPRITE_BACK_LAYER:
         return cache_enable_sprite_back_layer;
      case PPU_OPTION_ENABLE_SPRITE_FRONT_LAYER;
         return cache_enable_sprite_front_layer;

      default:
         WARN_GENERIC();
         break;
   }

   // Just go into dumb mode.
   return FALSE;
}

void ppu_save_state(PACKFILE* file, const int version)
{
}

void ppu_load_state(PACKFILE* file, const int version)
{
}

// --------------------------------------------------------------------------------

UINT8* ppu_get_chr_rom_pages(ROM *rom)
{
   RT_ASSERT(rom);

   const int num_pages = rom->chr_rom_pages;

   /* Compute a mask used to wrap invalid CHR ROM page numbers.
      As CHR ROM uses a 8k page size, this mask is based
      on a 8k page size. */
   if(((num_pages * 2 - 1) & (num_pages - 1)) == (num_pages - 1)) {
      // Compute mask for even power of two.
      pages_mirror_size = num_pages;
   }
   else {
      /* Compute the smallest even power of 2 greater than
         CHR ROM page count, and use that to compute the mask. */
      for(int i = 0; (num_pages >> i) > 0; i++);
      pages_mirror_size = 1 << i;
   }

   rom->chr_rom_page_overflow_mask = pages_mirror_size - 1;

   // Identify-map all the present pages.
   int copycount;
   for(copycount = 0; copycount < num_pages; copycount++)
      rom->chr_rom_page_lookup[copycount] = copycount;

   // Mirror-map all the not-present pages.
   int missing, count, next, pages_mirror_size;
   for(next = num_pages, missing = pages_mirror_size - num_pages,
       count = 1; missing; count <<= 1, missing >>= 1)
        if(missing & 1)
            for(copycount = count; copycount; copycount--, next++)
                rom->chr_rom_page_lookup[next] = rom->chr_rom_page_lookup[next - count];

    // 8k CHR ROM page size.
    const unsigned size = num_pages * 0x2000;
    rom->chr_rom = (UINT8*)malloc(size);
    if(rom->chr_rom)
        // Initialize to a known value for areas not present in image.
        memset(rom->chr_rom, 0xFF, size);

    return rom->chr_rom;
}

void ppu_free_chr_rom(ROM *rom)
{
   RT_ASSERT(rom);

   if(rom->chr_rom) {
      free(rom->chr_rom);
      rom->chr_rom = NULL;
   }
}

ENUM ppu_get_mirroring(void)
{
   SyncHelper();
   return ppu__mirroring;
}

void ppu_set_mirroring(const ENUM mirroring)
{
   SyncHelper();

   ppu__mirroring = mirroring;
   SetupMirroring();
}

void ppu_set_default_mirroring(const ENUM mirroring)
{
   ppu__default_mirroring = mirroring;
}

void ppu_set_name_table_address(const int table, UINT8* address)
{
   RT_ASSERT( (table >= 0) && (table < NAME_TABLE_MAXIMUM) );
   RT_ASSERT(address);

   SyncHelper();

   ppu__name_tables_read[table] = address;
   ppu__name_tables_write[table] = address;
}

void ppu_set_name_table_address_read_only(const int table, UINT8* address)
{
   RT_ASSERT( (table >= 0) && (table < NAME_TABLE_MAXIMUM) );
   RT_ASSERT(address);

   SyncHelper();

   ppu__name_tables_read[table] = address;
   ppu__name_tables_write[table] = ppu__name_table_dummy;
}

void ppu_set_1k_name_table_vram_page(const int table, const int page)
{
   RT_ASSERT( (table >= 0) && (table < NAME_TABLE_MAXIMUM) );
   RT_ASSERT(page >= 0);

   SyncHelper();
   ppu_set_name_table_address(table, ppu__name_table_vram + (page * NAME_TABLE_PAGE_SIZE));
}

void ppu_set_1k_name_table_vrom_page(const int table, int page)
{
   RT_ASSERT( (table >= 0) && (table < NAME_TABLE_MAXIMUM) );
   RT_ASSERT(page >= 0);

   SyncHelper();

   // CHR-ROM address fixup.
   page = (page & 7) + ROM_CHR_ROM_PAGE_LOOKUP[(page / 8) &
      ROM_CHR_ROM_PAGE_OVERFLOW_MASK] * 8;

   ppu__name_tables_read[table] = ROM_CHR_ROM + (page * NAME_TABLE_PAGE_SIZE);
   ppu__name_tables_write[table] = ppu__name_table_dummy;
}

void ppu_set_1k_pattern_table_vram_page(const UINT16 address, int page)
{
   RT_ASSERT(page >= 0);

   SyncHelper();

   const unsigned index = address / PATTERN_TABLE_PAGE_SIZE;
   page *= PATTERN_TABLE_PAGE_SIZE;
   ppu__pattern_tables_read[index] = ppu__pattern_table_vram + page;
   ppu__pattern_tables_write[index] = ppu__pattern_table_vram + page;
}

void ppu_set_1k_pattern_table_vrom_page(const UINT16 address, int page)
{
   RT_ASSERT(page >= 0);

   SyncHelper();

   // CHR-ROM address fixup.
   page = (page & 7) + ROM_CHR_ROM_PAGE_LOOKUP
      [(page / 8) & ROM_CHR_ROM_PAGE_OVERFLOW_MASK] * 8;

   const unsigned index = address / PATTERN_TABLE_PAGE_SIZE;
   ppu__pattern_tables_read[index] = ROM_CHR_ROM + (page * PATTERN_TABLE_PAGE_SIZE);
   ppu__pattern_tables_write[index] = ppu__pattern_table_dummy;
}

void ppu_set_8k_pattern_table_vram(void)
{
   SyncHelper();

   ppu_set_1k_pattern_table_vram_page(0x0000, 0);
   ppu_set_1k_pattern_table_vram_page(0x0400, 1);
   ppu_set_1k_pattern_table_vram_page(0x0800, 2);
   ppu_set_1k_pattern_table_vram_page(0x0C00, 3);
   ppu_set_1k_pattern_table_vram_page(0x1000, 4);
   ppu_set_1k_pattern_table_vram_page(0x1400, 5);
   ppu_set_1k_pattern_table_vram_page(0x1800, 6);
   ppu_set_1k_pattern_table_vram_page(0x1C00, 7);
}

void ppu_set_expansion_table_address(const UINT8* address)
{
   RT_ASSERT(address);

   SyncHelper();
   ppu__expansion_table = address;
}

UINT8 ppu_get_background_color(void)
{
   SyncHelper();

   /* Returns the current PPU background color - for drawing overscan e.g for NTSC.
      In the future, this should be rendered by the PPU itself into a special kind of buffer. 
      Returned as an index into the 256 color palette */
   return PPU__BACKGROUND_PALETTE(0);
}

void ppu_clear_palette(void)
{
   /* Clears the palette - easier than writing to VRAM when the palette needs
      to be cleared from an external source. */
   memset(&ppu__palette_vram, 0, PALETTE_VRAM_SIZE);
}

// --------------------------------------------------------------------------------
// PRIVATE FUNCTIONS
// --------------------------------------------------------------------------------

/* When reading while the VRAM address is in the range 0-$3EFF,
   the read will return the contents of an internal buffer.
   After the CPU reads,
   the PPU will then immediately read the byte at the current VRAM address into this internal buffer.
   Thus, after setting the VRAM address,
   one should first read this register and discard the result.
   This behavior doesn't occur when the VRAM address is in the $3F00-$3FFF palette range;
   reads come directly from palette RAM and don't affect the internal buffer. */
static linear uint8 VRAMRead()
{
   uint8 data = 0x00;
   if(address <= 0x3EFF) {
      // Retrieve the current byte in the read buffer (initially garbage).
      data = PPUState::readBuffer;
      // Fill the read buffer with the next byte.
      PPUState::readBuffer = VRAMReadUnbuffered();
   }
   else
      // Direct access to palette VRAM.
      data = VRAMReadUnbuffered();

   // Increment the VRAM address and return the byte read.
   IncrementVRAMAddress();
   return data;
}

static inline uint8 VRAMReadUnbuffered()
{
   if(ppu__vram_address <= 0x1FFF) {
      /* Read from pattern tables. The pattern tables occupy 8,192 bytes starting at $0000 and
         ending at $1FFF. Unlike name tables and palettes, they are not mirrored. */
      const int page = ppu__vram_address / PATTERN_TABLE_PAGE_SIZE;
      const unsigned offset = ppu__vram_address - (page * PATTERN_TABLE_PAGE_SIZE);
      const uint8* data = pattern_tables_read[page];
      return data[offset];
   }
   else if(ppu__vram_address <= 0x3EFF) {
      /* Read from name tables. The name tables occupy 4,096 bytes starting at $2000 and
         ending at $2FFF, and are then mirrored from $3000 to $3EFF. */
      const unsigned address = (ppu__vram_address - 0x2000) & 0xFFF;
      const int table = address / BYTES_PER_NAME_TABLE;
      const unsigned offset = address - (table * BYTES_PER_NAME_TABLE);
      const uint8* data = name_tables_read[table];
      return data[offset];
   }
   else {
      /* Read from palettes. The palettes occupy 32 bytes starting at $3F00 and ending at $3F1F,
         and are then mirrored every 32 bytes from $3F20 to $3FFF. */
      const unsigned address = (ppu__vram_address - 0x3F00) & 0x1F;
      return ppu__palette_vram[address];
   }
}

static linear void VRAMWrite(const uint8 data)
{
   if(ppu__vram_address <= 0x1FFF) {
      // Write to pattern tables.
      const int page = ppu__vram_address / PATTERN_TABLE_PAGE_SIZE;
      const unsigned offset = ppu__vram_address - (page * PATTERN_TABLE_PAGE_SIZE);
      uint8* patternData = pattern_tables_write[page];
      patternData[offset] = data;
   }
   else if(ppu__vram_address <= 0x3EFF) {
      // Write to name tables.
      const unsigned address = (ppu__vram_address - 0x2000) & 0xFFF;
      const int table = address / BYTES_PER_NAME_TABLE;
      const unsigned offset = address - (table * BYTES_PER_NAME_TABLE);
      uint8* nameTableData = name_tables_write[table];
      nameTableData[offset] = data;
   }
   else {
      // Write to palettes.
      const unsigned address = (ppu__vram_address - 0x3F00) & 0x1F;
      ppu__palette_vram[address] = data;
   }
}

static inline void IncrementVRAMAddress()
{
   ppu__vram_address += ppu__vram_address_increment;
}

static linear uint8 OAMRead()
{
   /* Each OAM entry is 29 bits wide.
      The unimplemented bits of each sprite's byte 2 do not exist in the PPU.
      On PPU revisions that allow reading PPU OAM through $2004,
      the unimplemented bits of each sprite's byte 2 always read back as 0.
      This can be emulated by ANDing byte 2 with $E3,
      either when writing to OAM or when reading back.
      It has not been determined whether the PPU actually drives these bits low
      or whether this is the effect of data bus capacitance from reading the last
      byte of the instruction (LDA $2004, which assembles to AD 04 20). */
   if((ppu__oam_address % 4) == 2)
      return ppu__sprite_vram[ppu__oam_address] & 0xE3;
   else
      return ppu__sprite_vram[ppu__oam_address];
}

static inline void OAMWrite(const uint8 data)
{
   /* Writes will increment OAMADDR after the write;
      reads during vertical or forced blanking return the value from OAM at that address but do not increment. */
   ppu__sprite_vram[ppu__oam_address] = data;
   ppu__oam_address++;
}

static linear void StartOAMDMA(const uint8 page)
{
   using namespace PPUState;

   /* When $4014 is written to, the data passed to the register indicates which 256-byte "page" of
      system RAM to access for copying sprite data into OAM. */
   oamDMAReadAddress = page * 0x100;
   /* Sprite DMA seems to use the internal OAM address latch for writing. This is evidenced by the
      fact that you typically have to write $00 to OAMADDR before doing sprite DMA. */
   oamDMAWriteAddress = ppu__oam_address;
   /* Make sure that we start with a read operation. We also delay the first read by one CPU cycle,
      as DMA always takes at least that long to initialize (see below). */
   oamDMATimer = 3; // 3 PPU cycles = 1 CPU cycle
   oamDMAFlipFlop = false;
   oamDMAByte = 0x00;

   /* Sprite DMA takes 1,539-1,542 PPU clock cycles (513-514 CPU clock cycles). The variation in the
      time taken is due to sync issues between the CPU and PPU.

      <tepples> The DMA unit can only stop the CPU on a read cycle.
      <tepples> The DMA unit and most of the sound hardware do different things on the odd and even CPU cycles.
                Quietust recently found a Phi2/2 divider in the 2A03.
      <tepples> If the CPU is writing, it ignores the pause line.
      <tepples> So when the CPU writes to $4014 (OAM DMA source register), the DMA unit stops the CPU for one
                cycle just in case the program did a read-modify-write on $4014, and then if it's on the odd
                cycle, it waits for the even cycle
      <tepples> It's predictable if you can manage to sync your main loop to the odd-even cycle of the CPU.
                I think one of Blargg's last demos does this.
      <tepples> But yes, in "ordinary" code, plan on STA $4014 taking 517 to 518 cycles to finish executing. */

   /* We'll just go with 514 CPU cycles for now, since we can't stop the CPU mid-instruction to
      get this implemented properly anyway. Calling cpu_burn() causes time to be taken away
      from the CPU and given to the APU and PPU, effectively "overclocking" the next
      synchronization attempt by the DMA transfer length, which simulates freezing the CPU for
      the same amount of time. */
   cpu_burn(514 * CPU_CLOCK_MULTIPLIER);

   /* One important coveat exists. Because OAMDMA() takes place in ppu_write(), after a
      synchronization has already taken place, and then returns control the CPU, the processor
      would continue to execute during the time when the DMA transfer was supposed to occur.
      Even though the PPU will perform the transfer at the next synchronization, the CPU
      could've changed the contents of RAM before then, so we have to force a synchronization
      to process the cycles *before* control returns to the CPU. */
   if(synchronizing) 
      /* If we are already synchronizing, signal that we need to re-evaluate the time elapsed.
         This causes our new cycles to get loaded mid-synchronization. */
      timeWarp = true;
   else
      // Force a synchronization attempt to process the new cycles.
      Synchronize();
}

static void SetupMirroring()
{
   switch(ppu__mirroring) {
      case PPU_MIRRORING_HORIZONTAL: {
         /* 0 0
            1 1 */
         ppu_set_name_table_internal(0, 0);
         ppu_set_name_table_internal(1, 0);
         ppu_set_name_table_internal(2, 1);
         ppu_set_name_table_internal(3, 1);

         break;
      }

      case PPU_MIRRORING_VERTICAL: {
         /* 0 1
            0 1 */
         ppu_set_name_table_internal(0, 0);
         ppu_set_name_table_internal(1, 1);
         ppu_set_name_table_internal(2, 0);
         ppu_set_name_table_internal(3, 1);

         break;
      }

      case PPU_MIRRORING_ONE_SCREEN:
         SetupSingleScreenMirroring();
         break;

      case PPU_MIRRORING_ONE_SCREEN_2000: {
         ppu__one_screen_base_address = ppu__name_table_vram;
         SetupSingleScreenMirroring();
         break;
      }

      case PPU_MIRRORING_ONE_SCREEN_2400: {
         ppu__one_screen_base_address = ppu__name_table_vram + 0x400;
         SetupSingleScreenMirroring();
         break;
      }

      case PPU_MIRRORING_ONE_SCREEN_2800: {
         ppu__one_screen_base_address = ppu__name_table_vram + 0x800;
         SetupSingleScreenMirroring();
         break;
      }

      case PPU_MIRRORING_ONE_SCREEN_2C00: {
         ppu__one_screen_base_address = ppu__name_table_vram + 0xC00;
         SetupSingleScreenMirroring();
         break;
      }

      case PPU_MIRRORING_FOUR_SCREEN: {
         /* 0 1
            2 3 */
         ppu_set_name_table_internal(0, 0);
         ppu_set_name_table_internal(1, 1);
         ppu_set_name_table_internal(2, 2);
         ppu_set_name_table_internal(3, 3);

         break;
      }

      default:
         WARN_GENERIC();
         break;
   }
}

static void SetupSingleScreenMirroring()
{
    ppu_set_name_table_address(0, ppu__one_screen_base_address);
    ppu_set_name_table_address(1, ppu__one_screen_base_address);
    ppu_set_name_table_address(2, ppu__one_screen_base_address);
    ppu_set_name_table_address(3, ppu__one_screen_base_address);
}

static void PredictNMI(const cpu_time_t cycles)
{
   // Clear pending interrupt just in case.
   cpu_unqueue_interrupt(CPU_INTERRUPT_NMI);

   // NMIs only occur if the flag is set.
   if(!ppu__generate_interrupts)
      return;

   // Save variables since we just want to simulate.
   const int16 savedScanline = PPUState::scanline;
   const int16 savedScanlineTimer = PPUState::scanlineTimer;

   // Note that 'cycles' represents the number of *PPU* cycles to simulate.
   for(cpu_time_t current = 0; current < cycles; current++) {
      // Get current scanline clock cycle (starting at 1).
      const cpu_time_t cycle = (PPU_SCANLINE_CLOCKS - PPUState::scanlineTimer) + 1;

      // VBlank NMI occurs on the 1st cycle of the line after the VBlank flag is set.
      if((cycle == 1) &&
         (PPUState::scanline == PPU_FIRST_VBLANK_LINE))
         cpu_queue_interrupt(CPU_INTERRUPT_NMI, PPUState::predictionTimestamp + (current * PPU_CLOCK_MULTIPLIER));

      ClockScanlineTimer(PPUState::scanline);
   }

   // Restore variables.
   PPUState::scanline = savedScanline;
   PPUState::scanlineTimer = savedScanlineTimer;
}

static void RepredictNMI()
{
   // Determine how much time has elapsed since our initial prediction.
   const cpu_time_t timestamp = cpu_get_cycles();
   const cpu_time_t cyclesElapsed = (cpu_rtime_t)timestamp - (cpu_rtime_t)PPUState::predictionTimestamp;
   // Calculate how many cycles are left in the prediction buffer.
   cosnt cpu_rtime_t cyclesRemaining = PPUState::predictionCycles - cyclesElapsed;
   if(cyclesRemaining <= 0)
      return;

   // Convert from master clock to PPU clock.
   const cpu_time_t ppuCyclesRemaining = cyclesRemaining / PPU_CLOCK_DIVIDER;
   if(ppuCyclesRemaining == 0)
      return;

   PredictNMI(ppuCyclesRemaining);
}

static inline bool ClockScanlineTimer(int16 &nextScanline)
{
   using namespace PPUState;

   if(scanlineTimer > 0)
      scanlineTimer--;
   if(scanlineTimer <= 0) {
      // The current scanline ended. Reload the scanline timer.
      scanlineTimer += PPU_SCANLINE_CLOCKS;
      /* Determine the next scanline to be processed, wrapping back around to the beginning
         of the frame if neccessary. */
      nextScanline = scanline + 1;
      if(nextScanline > PPU_LAST_LINE)
         nextScanline = PPU_FIRST_LINE;

      // Signal to the caller that the scanline has ended and a new one has begun.
      return TRUE;
   }

   // Signal to the caller that the current scanline is still ongoing.
   return FALSE;
}

// --------------------------------------------------------------------------------

static void Synchronize()
{
   if(PPUState::synchronizing)
      // No point to continue if we're already synchronizing.
      return;

   // Find out how many PPU cycles have elapsed since the last synchronization.
   cpu_time_t time = GetTimeElapsed();
   if(time == 0)
      // Nothing to do.
      return;

   // Set a simple flag to avoid re-entry, which is bad.
   PPUState::synchronizing = true;

   for(cpu_time_t current = 0; current < time; current++) {
      // Get current scanline clock cycle (starting at 1).
      const cpu_time_t cycle = (PPU_SCANLINE_CLOCKS - PPUState::scanlineTimer) + 1;
      if(cycle == 1) {
         // This is the start of a new scanline, so let's check if it's the first one.
         if(PPUState::scanline == PPU_FIRST_LINE)
            // This is the first scanline, so start a new frame.
            StartFrame();

         // Start a new scanline.
         StartScanline();
      }

      // Renderered lines (-1 to 239). These are when the PPU is "active".
      if((PPUState::scanline >= PPU_FIRST_LINE) &&
         (PPUState::scanline <= PPU_LAST_DISPLAYED_LINE))
         StartScanlineCycle(cycle);

      if((PPUState::scanline == PPU_CLOCK_SKIP_LINE) && (cycle == PPU_CLOCK_SKIP_CYCLE) &&
          PPUState::isOddFrame && ppu__enable_background) {
         /* The PPU has an even/odd flag that is toggled every frame,
            regardless of whether the BG is enabled or disabled.

            With BG disabled, each PPU frame is 341*262=89342 PPU clocks long.
            There is no skipped clock every other frame.

            With BG enabled, each odd PPU frame is one PPU clock shorter than
            normal. I've timed this to occur around PPU clock 328 on
            scanline 20, but haven't written a test ROM for it yet. */

         // Steal a clock from the scanline timer.
         PPUState::scanlineTimer--;
      }

      /* Clock the scanline timer. This returns TRUE if the scanline ended on this cycle, along with
         the next scanline to be procesed. */
      int16 nextScanline;
      if(ClockScanlineTimer(nextScanline)) {
         // End the current scanline and do some cleanup.
         EndScanline();
         // Check if this is the last line of the frame.
         if(PPUState::scanline == PPU_FLAST_LINE)
            // End the current frame and copy the fully rendered framebuffer to the screen.
            EndFrame();

         // Update the scanline counter.
         PPUState::scanline = nextScanline;
      }

      /* Handle sprite DMA timing. I put this at the end as it seems to run in a separate process
         from the rest of the PPU logic. */
      OAMDMA();

      /* If timeWarp is set, we need to re-evaluate the time elapsed. This can happen if cycles have
         been stolen from the CPU while the PPU was synchronizing, e.g by sprite DMA. */
      if(PPUState::timeWarp) {
         time += GetTimeElapsed();
         PPUState::timeWarp = false;
      }
   }

   // End unbreakable code section.
   PPUState::synchronizing = false;
}

static inline cpu_time_t GetTimeElasped()
{
   // Calculate the delta period.
   const cpu_time_t elapsedCycles = cpu_get_elapsed_cycles(&PPUState::clockCounter) + PPUState::clockBuffer;
   if(elapsedCycles == 0) // Always > 0 when ppu_clock_buffer > 0
      // Nothing to do. 
      return 0;

   // Scale from master clock to PPU and buffer the remainder to avoid possibly losing cycles.
   const cpu_time_t ppuElapsedCycles = elapsedCycles / PPU_CLOCK_DIVIDER;
   PPUState::clockBuffer = elapsedCycles - (ppuElapsedCycles * PPU_CLOCK_DIVIDER);
   return ppuElapsedCycles;
}

static linear void StartFrame()
{
   // Clear stale VBlank flag.
   ppu__vblank_started = FALSE;
   // Clear stale sprite #0 collision flag.
   ppu__sprite_collision = FALSE;

   // Perform renderer setup for this frame.
   Renderer::Frame();

   // Grabbing the light gun position once per frame should be enough.
   if(input_enable_zapper)
      input_update_zapper_offsets();
}

static linear void EndFrame()
{
   // Toggle the even/odd frame flag, which slightly changes the numbers of cycles per scanline.
   PPUState::isOddFrame = !PPUState::isOddFrame;

   /* Current frame has ended, but we only have to draw the buffer to the screen
      if rendering had been enabled (i.e this was not a skipped frame). */
   if(ppu__rendering_enabled)
      video_blit(screen);

   // Now we can reload our cached options into the actual variables.
   ppu__enable_background_layer = cache_enable_background_layer;
   ppu__enable_sprite_back_layer = cache_enable_sprite_back_layer;
   ppu__enable_sprite_front_layer = cache_enable_sprite_front_layer;
   ppu__rendering_enabled = cache_rendering_enabled;
}

static linear void StartScanline() 
{
   // Visible lines, #0-239
   if((PPUState::scanline >= PPU_FIRST_DISPLAYED_LINE) &&
      (PPUState::scanline <= PPU_LAST_DISPLAYED_LINE)) {
      // Clear stale sprite overflow flag.
      ppu__sprite_overflow = FALSE;

      /* This needs to be called before the sprite #0 and light gun code below it,
         otherwise the neccessary information might not be available yet. */
      Renderer::Line(PPUState::scanline);

      /* We need to force rendering when sprite #0 is present on the line.
         This is kind of ugly, but it works. */
      if(Renderer::render.sprites[0].index == 0)
         ppu__force_rendering = TRUE;

      /* Light gun hitscan detection works similarly. We could perform this check down to 
         the exact pixel position of the light gun hitscan, but that just adds more
         unneccessary overhead, so we'll just check it per-scanline. */
       if(input_enable_zapper && input_zapper_on_screen &&
          (input_zapper_y_offset == PPUState::scanline))
          ppu__force_rendering = TRUE;
   }
   // The PPU only idles on scanline #240.
   else if(PPUState::scanline == PPU_IDLE_LINE) {
      // Clear stale sprite overflow flag from the last line.
      ppu__sprite_overflow = FALSE;
   }
   // VBlank starts on scanline #241.
   else if(PPUState::scanline == PPU_FIRST_VBLANK_LINE) {
      // Enter the vertical blanking period.
      ppu__vblank_started = TRUE;
      PPUState::vblankQuirkTime = 1;
   }

   // If the MMC has a hook installed, we need to call it.
   if(mmc_scanline_start)
      cpu_interrupt(mmc_scanline_start(PPUState::scanline));
}

/* This is only called for scanlines -1 to 239, as the PPU is idle during other lines
   (excepting what is handled by StartScanline(), of course). */
static linear void StartScanlineCycle(const cpu_time_t cycle)
{
   /* Generate a clock for the renderer. This handles things like background and sprite
      timing, pretty much everything that doesn't involve drawing a pixel. */
   Renderer::Clock();

   // The PPU renders one pixel per clock for the first 256 clock cycles.
   if((cycle <= PPU_RENDER_CLOCKS) &&
      (PPUState::scanline >= PPU_FIRST_DISPLAYED_LINE))
      Renderer::Pixel();

   // After this is the HBlank period. If the MMC has a hook installed, we need to call it.
   if((cycle == PPU_HBLANK_START) && mmc_hblank_start)
      cpu_interrupt(mmc_hblank_start(PPUState::scanline));

   /* After entering VBlank, the VBL flag from PPUSTATUS is "off-limits" at the same time it is
      being set by the PPU. Reading from PPUSTATUS during that time clears the flag but still
      reports it as unset in the returned value. This is a quirky behavior that we probably
      don't have to emulate, but it takes very little effort to do so. */
   if(PPUState::vblankQuirkTime > 0)
      PPUState::vblankQuirkTime--;
}

static linear void EndScanline()
{
   // Here we just handle some cleanup for events that occured in StartScanline().
   if((PPUState::scanline >= PPU_FIRST_DISPLAYED_LINE) &&
      (PPUState::scanline <= PPU_LAST_DISPLAYED_LINE)) {
      if(ppu__force_rendering)
         ppu__force_rendering = FALSE;

      if(input_enable_zapper && input_zapper_on_screen &&
         (input_zapper_y_offset == PPUState::scanline))
         input_update_zapper();
   }

   // If the MMC has a hook installed, we need to call it.
   if(mmc_scanline_end)
      cpu_interrupt(mmc_scanline_end(PPUState::scanline));
}

static linear void OAMDMA()
{
   using namespace PPUState;

   /* When the sprite DMA countdown timer (oamDMATimer) is non-zero, and oamDMAWriteAddress is
      in a valid range, it means we need to process OAM DMA. */
   if(oamDMATimer == 0)
      return;
   if(oamDMAWriteAddress >= SPRITE_VRAM_SIZE) {
      // Sprite DMA disabled, or finished.
      oamDMATimer = 0;
      return;
   }

   // Clock the OAM countdown timer, and perform a read/write when it is zero.
   oamDMATimer--;
   if(oamDMATimer > 0)
      return;

  if(oamDMAFlipFlop) {
      // This is a write cycle.
      OAMWrite(oamDMAByte);
      /* Increment oamDMAWriteAddress. The actual location in OAM that we are writing to is
         derived from ppu__oam_address. This is just used to make sure we don't copy more
         than SPRITE_VRAM_SIZE bytes per DMA transfer. */
      oamDMAWriteAddress++;
      // Our next operation will be a read, so reload the timer accordingly.
      oamDMATimer = OAM_DMA_READ_CYCLES;
   }
   else {
      // This is a read cycle. Fetch a byte from system RAM into the DMA buffer.
      oamDMAByte = cpu_read(oamDMAReadAddress);
      oamDMAReadAddress++;
      // Our next operation will be a write, so reload the timer accordingly.
      oamDMATimer = OAM_DMA_WRITE_CYCLES;
   }

   // A simple flip-flop is used to separate read and write cycles.
   oamDMAFlipFlop = !oamDMAFlipFlop;
}
