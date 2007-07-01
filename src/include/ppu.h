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

#define PPU_DISPLAY_LINES   240

/* Register $2000. */
#define PPU_VBLANK_NMI_FLAG_BIT     (1 << 7)
#define PPU_PPU_SLAVE_BIT           (1 << 6)
#define PPU_SPRITE_SIZE_BIT         (1 << 5)
#define PPU_BACKGROUND_TILESET_BIT  (1 << 4)
#define PPU_SPRITE_TILESET_BIT      (1 << 3)
#define PPU_ADDRESS_INCREMENT_BIT   (1 << 2)

#define PPU_NAME_TABLE_SELECT       (3 << 0)

#define PPU_WANT_VBLANK_NMI     \
    (ppu_register_2000 & PPU_VBLANK_NMI_FLAG_BIT)

/* Register $2001. */
#define PPU_COLOR_INTENSITY                 (7 << 5)

#define PPU_SPRITES_ENABLE_BIT              (1 << 4)
#define PPU_BACKGROUND_ENABLE_BIT           (1 << 3)
#define PPU_SPRITES_SHOW_LEFT_EDGE_BIT      (1 << 2)
#define PPU_BACKGROUND_SHOW_LEFT_EDGE_BIT   (1 << 1)
#define PPU_MONOCHROME_DISPLAY_BIT          (1 << 0)

#define PPU_BACKGROUND_ENABLED  \
    (ppu_register_2001 & PPU_BACKGROUND_ENABLE_BIT)

#define PPU_SPRITES_ENABLED     \
    (ppu_register_2001 & PPU_SPRITES_ENABLE_BIT)

#define PPU_BACKGROUND_CLIP_ENABLED     \
    (! (ppu_register_2001 & PPU_BACKGROUND_SHOW_LEFT_EDGE_BIT))

#define PPU_SPRITES_CLIP_ENABLED        \
    (! (ppu_register_2001 & PPU_SPRITES_SHOW_LEFT_EDGE_BIT))

/* Register $2002. */
#define PPU_VBLANK_FLAG_BIT         (1 << 7)
#define PPU_SPRITE_0_COLLISION_BIT  (1 << 6)
#define PPU_SPRITE_OVERFLOW_BIT     (1 << 5)

#define PPU_MAP_RAM         1
#define PPU_MAP_BACKGROUND  2
#define PPU_MAP_SPRITES     4

extern UINT8 ppu_register_2000;
extern UINT8 ppu_register_2001;
extern int ppu_enable_sprite_layer_a;
extern int ppu_enable_sprite_layer_b;
extern int ppu_enable_background_layer;

extern int ppu_frame_last_line;
extern BOOL ppu_is_rendering;

extern int background_enabled;
extern int sprites_enabled;

extern UINT8 * one_screen_base_address;

extern void ppu_free_chr_rom (ROM *);
extern UINT8 * ppu_get_chr_rom_pages (ROM *);
extern void ppu_cache_chr_rom_pages (void);

extern void ppu_set_ram_1k_pattern_vram_block (UINT16, int);
extern void ppu_set_ram_1k_pattern_vrom_block (UINT16, int);
extern void ppu_set_ram_1k_pattern_vrom_block_ex (UINT16, int, int);
extern void ppu_set_ram_8k_pattern_vram (void);

extern int ppu_init (void);
extern void ppu_exit (void);
extern void ppu_reset (void);

extern UINT8 ppu_read (UINT16);
extern void ppu_write (UINT16, UINT8);

extern void ppu_clear_palette (void);

extern void ppu_sync_update(void);
extern void ppu_disable_rendering(void);
extern void ppu_enable_rendering(void);
extern void ppu_predict_nmi(cpu_time_t cycles);

extern void ppu_set_mirroring_one_screen (void);
extern int ppu_get_mirroring (void);
extern void ppu_set_mirroring (int);
extern void ppu_invert_mirroring (void);

extern void ppu_set_name_table_internal (int, int);
extern void ppu_set_name_table_address (int, UINT8 *);
extern void ppu_set_name_table_address_rom (int, UINT8 *);
extern void ppu_set_name_table_address_vrom (int, int);
extern void ppu_set_expansion_table_address (UINT8 *);

enum
{
    MIRRORING_HORIZONTAL, MIRRORING_VERTICAL,
    MIRRORING_FOUR_SCREEN,
    MIRRORING_ONE_SCREEN,
    MIRRORING_ONE_SCREEN_2000, MIRRORING_ONE_SCREEN_2400,
    MIRRORING_ONE_SCREEN_2800, MIRRORING_ONE_SCREEN_2C00
};

extern void ppu_save_state (PACKFILE *, int);
extern void ppu_load_state (PACKFILE *, int);

extern UINT8 ppu_get_background_color (void);

#ifdef __cplusplus
}
#endif
#endif /* !PPU_H_INCLUDED */
