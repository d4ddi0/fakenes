

/*

FakeNES - A portable, open-source NES emulator.

rom.h: Declarations for ROM file handling.

Copyright (c) 2002, Randy McDowell and Ian Smith.
All rights reserved, see 'LICENSE' for details.

*/


#ifndef __ROM_H__
#define __ROM_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <allegro.h>


#include "misc.h"


#define ROM_CTRL_MIRROR    1
#define ROM_CTRL_SRAM      2
#define ROM_CTRL_TRAINER   4
#define ROM_CTRL_4SCREEN   8


typedef struct _ROM
{
    UINT8 prg_rom_pages;   /* # of 16k PRG-ROM pages               */
    UINT8 chr_rom_pages;   /* # of 8k CHR-ROM pages                */

    UINT8 control_byte_1;  /* ROM Control byte #1                  */
    UINT8 control_byte_2;  /* ROM Control byte #2                  */

    UINT8 mapper_number;   /* Mapper #, derived from control bytes */

    UINT8 * trainer;       /* Trainer buffer                       */

    UINT8 * prg_rom;       /* PRG-ROM buffer                       */
    UINT8 * chr_rom;       /* CHR-ROM buffer                       */
    UINT8 * chr_rom_cache; /* CHR-ROM tile cache buffer            */
                           /* CHR-ROM tile cache tag buffer        */
    UINT8 * chr_rom_cache_tag;
                           /* CHR-ROM bank # wrapping mask         */
    UINT8 chr_rom_page_overflow_mask;

    UINT8 sram_flag;       /* SRAM supported (extension)           */
    UINT8 filename [256];  /* Filename (extension)                 */

} ROM;


typedef struct _NES_HEADER
{
    UINT8 signature[4];     /* NES^Z signature                      */

    UINT8 prg_rom_pages;    /* # of 16k PRG-ROM pages               */
    UINT8 chr_rom_pages;    /* # of 8k CHR-ROM pages                */

    UINT8 control_byte_1;   /* ROM Control byte #1                  */
    UINT8 control_byte_2;   /* ROM Control byte #2                  */

    UINT8 reserved[8];      /* Mapper #, derived from control bytes */

} NES_HEADER;


int load_rom (AL_CONST UINT8 *, ROM *);

void free_rom (ROM *);


ROM global_rom;


#define ROM_PRG_ROM_PAGES   (global_rom.prg_rom_pages)
#define ROM_CHR_ROM_PAGES   (global_rom.chr_rom_pages)

#define ROM_MAPPER_NUMBER   (global_rom.mapper_number)

#define ROM_PRG_ROM         (global_rom.prg_rom)
#define ROM_CHR_ROM         (global_rom.chr_rom)
#define ROM_CHR_ROM_CACHE   (global_rom.chr_rom_cache)
#define ROM_CHR_ROM_CACHE_TAG   (global_rom.chr_rom_cache_tag)
#define ROM_CHR_ROM_PAGE_OVERFLOW_MASK (global_rom.chr_rom_page_overflow_mask)


int rom_is_loaded;


#ifdef __cplusplus
}
#endif

#endif /* ! __ROM_H__ */
