

/*

FakeNES - A portable, Open Source NES emulator.

ppu.c: Declarations for the PPU emulation.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __PPU_H__
#define __PPU_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "rom.h"

#include "misc.h"

/* register $2000 */
#define PPU_VBLANK_NMI_FLAG_BIT     (1 << 7)
#define PPU_PPU_SLAVE_BIT           (1 << 6)
#define PPU_SPRITE_SIZE_BIT         (1 << 5)
#define PPU_BACKGROUND_TILESET_BIT  (1 << 4)
#define PPU_SPRITE_TILESET_BIT      (1 << 3)
#define PPU_ADDRESS_INCREMENT_BIT   (1 << 2)
#define PPU_NAME_TABLE_SELECT       (3 << 0)

#define PPU_WANT_VBLANK_NMI \
 (ppu_register_2000 & PPU_VBLANK_NMI_FLAG_BIT)


/* register $2001 */
#define PPU_COLOR_INTENSITY         (7 << 5)
#define PPU_SPRITES_ENABLE_BIT      (1 << 4)
#define PPU_BACKGROUND_ENABLE_BIT   (1 << 3)
#define PPU_SPRITES_SHOW_LEFT_EDGE_BIT      (1 << 2)
#define PPU_BACKGROUND_SHOW_LEFT_EDGE_BIT   (1 << 1)
#define PPU_MONOCHROME_DISPLAY_BIT  (1 << 0)

#define PPU_BACKGROUND_ENABLED \
 (ppu_register_2001 & PPU_BACKGROUND_ENABLE_BIT)

#define PPU_SPRITES_ENABLED \
 (ppu_register_2001 & PPU_SPRITES_ENABLE_BIT)

#define PPU_BACKGROUND_CLIP_ENABLED \
 (!(ppu_register_2001 & PPU_BACKGROUND_SHOW_LEFT_EDGE_BIT))

#define PPU_SPRITES_CLIP_ENABLED \
 (!(ppu_register_2001 & PPU_SPRITES_SHOW_LEFT_EDGE_BIT))


/* register $2002 */
#define PPU_VBLANK_FLAG_BIT         (1 << 7)
#define PPU_SPRITE_0_COLLISION_BIT  (1 << 6)
#define PPU_SPRITE_OVERFLOW_BIT     (1 << 5)



UINT8 ppu_register_2000;
UINT8 ppu_register_2001;

int ppu_enable_sprite_layer_a;
int ppu_enable_sprite_layer_b;
int ppu_enable_background_layer;

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
void ppu_vblank_nmi (void);


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

void ppu_set_mirroring_one_screen (void);


int ppu_get_mirroring (void);

void ppu_set_mirroring (int);


void ppu_invert_mirroring (void);


void ppu_set_name_table_address (int, UINT8 *);
void ppu_set_name_table_address_rom (int table, UINT8 *address);
void ppu_set_name_table_address_vrom (int table, int vrom_block);


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
