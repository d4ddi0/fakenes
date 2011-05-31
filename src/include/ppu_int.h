/* FakeNES - A free, portable, Open Source NES emulator.

   ppu_int.h: Internal declarations for the PPU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef PPU_INT_H_INCLUDED
#define PPU_INT_H_INCLUDED
#include "common.h"
#include "timing.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* This macro gets direct access to an Allegro memory bitmap. */
#define PPU__GET_LINE_ADDRESS(_BITMAP, _LINE) \
   ( (_BITMAP)->line[(_LINE)] )

/* These macros are used to get and set pixels in the background scanline buffer. */
#define PPU__BACKGROUND_PIXELS_SIZE PPU_RENDER_CLOCKS /* 256 pixels. */
#define PPU__GET_BACKGROUND_PIXEL(_PIXEL) \
   ( ppu__background_pixels[(_PIXEL)] )
#define PPU__PUT_BACKGROUND_PIXEL(_PIXEL, _COLOR) \
   ( ppu__background_pixels[(_PIXEL)] = (_COLOR) )

/* NES color indexes start at position 1 in the global palette. */
#define PPU__PALETTE_ADJUST 1
/* These macros map palette indices to NES color values. */
#define PPU__BACKGROUND_PALETTE(_PALETTE, _INDEX) \
   ( (ppu__background_palettes[(_PALETTE)][(_INDEX)] & ppu__palette_mask) + PPU__PALETTE_ADJUST )
#define PPU__SPRITE_PALETTE(_PALETTE, _INDEX) \
   ( (ppu__sprite_palettes[(_PALETTE)][(_INDEX)] & ppu__palette_mask) + PPU__PALETTE_ADJUST )

/* This macro helps us create a large number of arrays in a clean manner. */
#define PPU__ARRAY(_TYPE, _NAME, _SIZE)	_TYPE _NAME[(_SIZE)]

/* ****************************************
   ********** INTERNAL VARIABLES **********
   ****************************************/
/* General. */
extern UINT16 ppu__base_name_table_address;
extern BOOL   ppu__generate_interrupts;
extern UINT8  ppu__vram_address_increment;
/* Background. */
extern UINT16 ppu__background_tileset;
extern BOOL   ppu__clip_background;
extern BOOL   ppu__enable_background;
/* Sprites. */
extern BOOL   ppu__clip_sprites;
extern BOOL   ppu__enable_sprites;
extern UINT8  ppu__sprite_height;
extern UINT16 ppu__sprite_tileset;
/* Palettes and colors. */
extern BOOL   ppu__intensify_reds;
extern BOOL   ppu__intensify_greens;
extern BOOL   ppu__intensify_blues;
extern UINT8  ppu__palette_mask;
/* Register-derived. */
extern BOOL   ppu__enabled;
extern UINT8  ppu__oam_address;
extern UINT8  ppu__scroll_x_position;
extern UINT8  ppu__scroll_y_position;
extern UINT16 ppu__vram_address;
/* Emulation only. */
extern ENUM   ppu__default_mirroring;
extern ENUM   ppu__mirroring;
extern UINT8* ppu__one_screen_base_address;
extern BOOL   ppu__sprite_collision;
extern BOOL   ppu__sprite_overflow;
extern BOOL   ppu__vblank_started;
/* Rendering only. */
extern UINT8  ppu__background_pixels[PPU__BACKGROUND_PIXELS_SIZE];
extern BOOL   ppu__enable_background_layer;
extern BOOL   ppu__enable_rendering;
extern BOOL   ppu__enable_sprite_back_layer;
extern BOOL   ppu__enable_sprite_front_layer;
extern BOOL   ppu__force_rendering;

/* ****************************************************
   ********** NAME TABLES AND PATTERN TABLES **********
   ****************************************************/
/* Object counts and sizes. */
#define PPU__BYTES_PER_NAME_TABLE	1024
#define PPU__BYTES_PER_PATTERN_TABLE	4096
#define PPU__NAME_TABLE_COUNT		2	/* Number of internal name tables. */
#define PPU__NAME_TABLE_MAXIMUM		4	/* Number of name tables that can be used (total). */
#define PPU__NAME_TABLE_PAGE_SIZE	PPU__BYTES_PER_NAME_TABLE
#define PPU__PATTERN_TABLE_COUNT	2
#define PPU__PATTERN_TABLE_PAGE_SIZE	1024
#define PPU__PATTERN_TABLE_PAGE_COUNT	(8192 / PPU__PATTERN_TABLE_PAGE_SIZE)
/* Array sizes. */
#define PPU__NAME_TABLE_DUMMY_SIZE	PPU__BYTES_PER_NAME_TABLE
#define PPU__NAME_TABLE_VRAM_SIZE 	(PPU__BYTES_PER_NAME_TABLE * PPU__NAME_TABLE_COUNT)
#define PPU__NAME_TABLES_READ_SIZE	PPU__NAME_TABLE_MAXIMUM
#define PPU__NAME_TABLES_WRITE_SIZE	PPU__NAME_TABLE_MAXIMUM
#define PPU__PATTERN_TABLE_DUMMY_SIZE	PPU__BYTES_PER_PATTERN_TABLE
#define PPU__PATTERN_TABLE_VRAM_SIZE	(PPU__BYTES_PER_PATTERN_TABLE * PPU__PATTERN_TABLE_COUNT)
#define PPU__PATTERN_TABLES_READ_SIZE	PPU__PATTERN_TABLE_PAGE_COUNT
#define PPU__PATTERN_TABLES_WRITE_SIZE	PPU__PATTERN_TABLE_PAGE_COUNT
/* Arrays. */
extern PPU__ARRAY( UINT8,        ppu__name_table_dummy,                PPU__NAME_TABLE_DUMMY_SIZE     );
extern PPU__ARRAY( UINT8,        ppu__name_table_vram,                 PPU__NAME_TABLE_VRAM_SIZE      );
extern PPU__ARRAY( const UINT8*, ppu__name_tables_read,                PPU__NAME_TABLES_READ_SIZE     );
extern PPU__ARRAY( UINT8*,       ppu__name_tables_write,               PPU__NAME_TABLES_WRITE_SIZE    );
extern PPU__ARRAY( UINT8,        ppu__pattern_table_dummy,             PPU__PATTERN_TABLE_DUMMY_SIZE  );
extern PPU__ARRAY( UINT8,        ppu__pattern_table_vram,              PPU__PATTERN_TABLE_VRAM_SIZE   );
extern PPU__ARRAY( const UINT8*, ppu__pattern_tables_read,             PPU__PATTERN_TABLES_READ_SIZE  );
extern PPU__ARRAY( UINT8*,       ppu__pattern_tables_write,            PPU__PATTERN_TABLES_WRITE_SIZE );
extern PPU__ARRAY( const UINT8*, ppu__background_pattern_tables_read,  PPU__PATTERN_TABLES_READ_SIZE  );
extern PPU__ARRAY( UINT8*,       ppu__background_pattern_tables_write, PPU__PATTERN_TABLES_WRITE_SIZE );
extern PPU__ARRAY( const UINT8*, ppu__sprite_pattern_tables_read,      PPU__PATTERN_TABLES_READ_SIZE  );
extern PPU__ARRAY( UINT8*,       ppu__sprite_pattern_tables_write,     PPU__PATTERN_TABLES_WRITE_SIZE );

/* **************************************
   ********** PALETTES AND OAM **********
   **************************************/
/* Object counts and sizes. */
#define PPU__BACKGROUND_PALETTE_COUNT	4
#define PPU__BACKGROUND_PALETTE_OFFSET	0
#define PPU__BYTES_PER_PALETTE		4
#define PPU__BYTES_PER_SPRITE		4
#define PPU__SPRITE_COUNT		64
#define PPU__SPRITE_PALETTE_COUNT	4
#define PPU__SPRITE_PALETTE_OFFSET	(PPU__BACKGROUND_PALETTE_COUNT * PPU__BYTES_PER_PALETTE)
/* Array sizes. */
#define PPU__BACKGROUND_PALETTES_SIZE	PPU__BACKGROUND_PALETTE_COUNT
#define PPU__PALETTE_VRAM_SIZE		(PPU__BYTES_PER_PALETTE * (PPU__BACKGROUND_PALETTE_COUNT + PPU__SPRITE_PALETTE_COUNT))
#define PPU__SPRITE_VRAM_SIZE		(PPU__BYTES_PER_SPRITE * PPU__SPRITE_COUNT)
#define PPU__SPRITE_PALETTES_SIZE	PPU__SPRITE_PALETTE_COUNT
/* Arrays. */
extern PPU__ARRAY( UINT8*, ppu__background_palettes, PPU__BACKGROUND_PALETTES_SIZE );
extern PPU__ARRAY( UINT8,  ppu__palette_vram,        PPU__PALETTE_VRAM_SIZE        );
extern PPU__ARRAY( UINT8*, ppu__sprite_palettes,     PPU__SPRITE_PALETTES_SIZE     );
extern PPU__ARRAY( UINT8,  ppu__sprite_vram,         PPU__SPRITE_VRAM_SIZE         );

#ifdef __cplusplus
}
#endif
#endif /* !PPU_INT_H_INCLUDED */
