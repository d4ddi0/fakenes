/* FakeNES - A free, portable, Open Source NES emulator.

   ppu_int.h: Internal declarations for the PPU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef PPU_INT_H_INCLUDED
#define PPU_INT_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "cpu.h"
#include "timing.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* This macro gets direct access to an Allegro memory bitmap. */
#define PPU__GET_LINE_ADDRESS(_bitmap, _y) \
   ( (_bitmap)->line[(_y)] )

/* These macros are used to get and set pixels in the background scanline buffer. */
#define PPU__BACKGROUND_PIXELS_SIZE PPU_RENDER_CLOCKS /* 256 pixels. */
#define PPU__GET_BACKGROUND_PIXEL(_pixel) \
   ( ppu__background_pixels[(_pixel)] )
#define PPU__PUT_BACKGROUND_PIXEL(_pixel, _color) \
   ( ppu__background_pixels[(_pixel)] = (_color) )

/* NES color indexes start at position 1 in the global palette. */
#define PPU__PALETTE_ADJUST 1
/* These macros map palette indices to NES color values. */
#define PPU__BACKGROUND_PALETTE(_palette, _index) \
   ( (ppu_background_palettes[(_palette)][(_index)] & ppu__palette_mask) + PPU__PALETTE_ADJUST )
#define PPU__SPRITE_PALETTE(_palette, _index) \
   ( (ppu_sprite_palettes[(_palette)][(_index)] & ppu__palette_mask) + PPU__PALETTE_ADJUST )

extern UINT16 ppu__base_name_table_address;
extern BOOL   ppu__generate_interrupts;
extern UINT8  ppu__vram_address_increment;

extern UINT16 ppu__background_tileset;
extern BOOL   ppu__clip_background;
extern BOOL   ppu__enable_background;

extern BOOL   ppu__clip_sprites;
extern BOOL   ppu__enable_sprites;
extern BOOL   ppu__sprite_collision;
extern UINT8  ppu__sprite_height;
extern BOOL   ppu__sprite_overflow;
extern UINT16 ppu__sprite_tileset;

extern BOOL  ppu__intensify_reds;
extern BOOL  ppu__intensify_greens;
extern BOOL  ppu__intensify_blues;
extern UINT8 ppu__palette_mask;

extern BOOL   ppu__enabled;
extern ENUM   ppu__default_mirroring;
extern ENUM   ppu__mirroring;
extern UINT8  ppu__oam_address;
extern UINT8* ppu__one_screen_base_address;
extern UINT8  ppu__scroll_x_position;
extern UINT8  ppu__scroll_y_position;
extern UINT16 ppu__vram_address;
extern BOOL   ppu__vblank_started;

extern UINT8 ppu__background_pixels[PPU__BACKGROUND_PIXELS_SIZE];
extern BOOL  ppu__enable_background_layer;
extern BOOL  ppu__enable_sprite_back_layer;
extern BOOL  ppu__enable_sprite_front_layer;
extern BOOL  ppu__force_rendering;
extern BOOL  ppu__rendering_enabled;

#ifdef __cplusplus
}
#endif
#endif /* !PPU_INT_H_INCLUDED */
