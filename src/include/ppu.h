

/*

FakeNES - A portable, open-source NES emulator.

ppu.h: Declarations for the PPU emulation.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __PPU_H__
#define __PPU_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "rom.h"

#include "misc.h"


void ppu_free_chr_rom (ROM *rom);
UINT8 * ppu_get_chr_rom_pages (ROM *rom);
void ppu_cache_chr_rom_pages (void);

void ppu_set_ram_1k_pattern_vram_block (UINT16 block_address, int vram_block);
void ppu_set_ram_1k_pattern_vrom_block (UINT16 block_address, int vrom_block);
void ppu_set_ram_8k_pattern_vram (void);

int ppu_scanline;
int ppu_frame_last_line;
int ppu_init (void);
void ppu_exit (void);


void ppu_reset (void);


UINT8 ppu_read (UINT16);
void ppu_write (UINT16, UINT8);


void ppu_clear (void);

void ppu_vblank (void);


void ppu_start_line (void);
void ppu_end_line (void);


void ppu_stub_render_line (int);

void ppu_render_line (int);


void ppu_start_frame (void);
void ppu_start_render (void);
void ppu_end_render (void);


int background_enabled;
int sprites_enabled;


UINT8 * one_screen_base_address;

void set_ppu_mirroring_one_screen (void);


int get_ppu_mirroring (void);

void set_ppu_mirroring (int);


void invert_ppu_mirroring (void);


void set_name_table_address (int, UINT8 *);


#define MIRRORING_HORIZONTAL    0

#define MIRRORING_VERTICAL      1


#define MIRRORING_FOUR_SCREEN   2

#define MIRRORING_ONE_SCREEN    3


#define MIRRORING_ONE_SCREEN_2000   4

#define MIRRORING_ONE_SCREEN_2400   5

#define MIRRORING_ONE_SCREEN_2800   6

#define MIRRORING_ONE_SCREEN_2C00   7


#ifdef __cplusplus
}
#endif

#endif /* ! __PPU_H__ */
