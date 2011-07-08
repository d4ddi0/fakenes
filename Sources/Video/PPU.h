/* FakeNES - A free, portable, Open Source NES emulator.

   ppu.h: Declarations for the PPU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef PPU_H_INCLUDED
#define PPU_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "cpu.h"
#include "rom.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

enum {
   PPU_STATUS_INITIALIZING = 0,
   PPU_STATUS_EVALUATING,
   PPU_STATUS_RASTERIZING,
   PPU_STATUS_HBLANK,
   PPU_STATUS_FORCED_BLANK,
   PPU_STATUS_IDLING,
   PPU_STATUS_VBLANK,
   PPU_STATUS_UNKNOWN,
};

enum {
   PPU_OPTION_ENABLE_RENDERING = 0,
   PPU_OPTION_ENABLE_BACKGROUND_LAYER,
   PPU_OPTION_ENABLE_SPRITE_BACK_LAYER,
   PPU_OPTION_ENABLE_SPRITE_FRONT_LAYER
};

enum {
   PPU_MIRRORING_HORIZONTAL = 0,
   PPU_MIRRORING_VERTICAL,
   PPU_MIRRORING_ONE_SCREEN,
   PPU_MIRRORING_ONE_SCREEN_2000, PPU_MIRRORING_ONE_SCREEN_2400,
   PPU_MIRRORING_ONE_SCREEN_2800, PPU_MIRRORING_ONE_SCREEN_2C00,
   PPU_MIRRORING_FOUR_SCREEN
};

// Valid flags for ppu_set_1k_pattern_table_vrom_page_expanded().
enum {
   PPU_EXPAND_INTERNAL   = 1 << 0,
   PPU_EXPAND_BACKGROUND = 1 << 1,
   PPU_EXPAND_SPRITES    = 1 << 2
};

// Interrupt prediction.
enum {
   PPU_PREDICT_NONE    = 0,
   PPU_PREDICT_NMI     = 1 << 0,
   PPU_PREDICT_MMC_IRQ = 1 << 1,
   PPU_PREDICT_ALL = PPU_PREDICT_NMI | PPU_PREDICT_MMC_IRQ
};

extern int ppu_init(void);
extern void ppu_exit(void);
extern void ppu_reset(void);
extern UINT8 ppu_read(const UINT16 address);
extern void ppu_write(const UINT16 address, const UINT8 data);
extern cpu_time_t ppu_execute(const cpu_time_t time);
extern void ppu_predict_interrupts(const cpu_time_t cycles, const unsigned flags);
extern void ppu_repredict_interrupts(const unsigned flags);
extern void ppu_sync_update(void);
extern ENUM ppu_get_status(void);
extern void ppu_set_option(const ENUM option, const BOOL value);
extern BOOL ppu_get_option(const ENUM option);
extern void ppu_load_state(PACKFILE* file, const int version);
extern void ppu_save_state(PACKFILE* file, const int version);
extern UINT8* ppu_get_chr_rom_pages(ROM *rom);
extern void ppu_free_chr_rom(ROM *rom);
extern ENUM ppu_get_mirroring(void);
extern void ppu_set_mirroring(const ENUM mirroring);
extern void ppu_set_default_mirroring(const ENUM mirroring);
extern void ppu_set_name_table_address(const int table, UINT8* address);
extern void ppu_set_name_table_address_read_only(const int table, const UINT8* address);
extern void ppu_set_1k_name_table_vram_page(const int table, const int page);
extern void ppu_set_1k_name_table_vrom_page(const int table, int page);
extern void ppu_set_1k_pattern_table_vram_page(const UINT16 address, int page);
extern void ppu_set_1k_pattern_table_vrom_page(const UINT16 address, int page);
extern void ppu_set_1k_pattern_table_vrom_page_expanded(const UINT16 address, int page, const unsigned flags);
extern void ppu_set_8k_pattern_table_vram(void);
extern void ppu_set_expansion_table_address(const UINT8* address);
extern void ppu_begin_state_restore(void);
extern void ppu_end_state_restore(void);
extern void ppu_map_color(const UINT8 index, const UINT16 value);
extern UINT16 ppu_get_background_color(void);

#ifdef __cplusplus
}
#endif
#endif /* !PPU_H_INCLUDED */
