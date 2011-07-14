/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef SYSTEM__ROM_H__INCLUDED
#define SYSTEM__ROM_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#include "Toolkit/MD5.h"
#ifdef __cplusplus
extern "C" {
#endif
   
typedef struct _ROM
{
   UINT8 *trainer;                     /* Pointer to trainer data. */
   UINT8 *prg_rom;                     /* Pointer to PRG-ROM data. */
   UINT8 *chr_rom;                     /* Pointer to CHR-ROM data. */
   UINT32 trainer_crc32;               /* Checksum for trainer. */
   UINT32 prg_rom_crc32;               /* Checksum for PRG-ROM. */
   UINT32 chr_rom_crc32;               /* Checksum for CHR-ROM. */
   MD5_HASH trainer_md5;               /* Checksum for trainer(MD5). */
   MD5_HASH prg_rom_md5;               /* Checksum for PRG-ROM(MD5). */
   MD5_HASH chr_rom_md5;               /* Checksum for CHR-ROM(MD5). */
   int prg_rom_pages;                  /* Number of PRG-ROM pages. */
   int chr_rom_pages;                  /* Nimber of CHR-ROM pages. */
   UINT8 control_byte_1;               /* Header control byte #1. */
   UINT8 control_byte_2;               /* Header control byte #2. */
   int mapper_number;                  /* Number of MMC/mapper to use. */
   UINT8 chr_rom_page_overflow_mask;   /* CHR-ROM bank # wrapping mask. */
   UINT8 chr_rom_page_lookup[256];     /* ?? */
   UINT8 prg_rom_page_overflow_mask;   /* PRG-ROM bank # wrapping mask. */
   UINT8 prg_rom_page_lookup[256];     /* ?? */
   BOOL sram_flag;                     /* If Save RAM/SRAM is present. */
   USTRING filename;                   /* Full Unicode filename. */

} ROM;

typedef struct _INES_HEADER
{
   UINT8 signature[4];
   UINT8 prg_rom_pages;
   UINT8 chr_rom_pages;
   UINT8 control_byte_1;
   UINT8 control_byte_2;
   UINT8 reserved[8];

} INES_HEADER;

extern int load_rom (const UDATA*, ROM *);
extern int load_rom_from_zip (const UDATA*, ROM *);
extern void free_rom (ROM *);

extern ROM global_rom;

#define ROM_TRAINER_SIZE	512
#define ROM_PRG_ROM_PAGE_SIZE	16384
#define ROM_CHR_ROM_PAGE_SIZE	8192

/* Control byte flags. */
enum {
   ROM_CTRL_MIRRORING    = (1 << 0),
   ROM_CTRL_BATTERY      = (1 << 1),
   ROM_CTRL_TRAINER      = (1 << 2),
   ROM_CTRL_FOUR_SCREEN  = (1 << 2)
};

/* Shortcut macros. */
#define ROM_MAPPER_NUMBER              global_rom.mapper_number

#define ROM_PRG_ROM                    global_rom.prg_rom
#define ROM_PRG_ROM_PAGES              global_rom.prg_rom_pages
#define ROM_PRG_ROM_SIZE               (ROM_PRG_ROM_PAGES * ROM_PRG_ROM_PAGE_SIZE)
#define ROM_PRG_ROM_PAGE_LOOKUP        global_rom.prg_rom_page_lookup
#define ROM_PRG_ROM_PAGE_OVERFLOW_MASK global_rom.prg_rom_page_overflow_mask
#define ROM_PRG_ROM_CRC                global_rom.prg_rom_crc32
#define ROM_PRG_ROM_MD5                global_rom.prg_rom_md5.hex

#define ROM_CHR_ROM                    global_rom.chr_rom
#define ROM_CHR_ROM_PAGES              global_rom.chr_rom_pages
#define ROM_CHR_ROM_SIZE               (ROM_CHR_ROM_PAGES * ROM_CHR_ROM_PAGE_SIZE)
#define ROM_CHR_ROM_PAGE_LOOKUP        global_rom.chr_rom_page_lookup
#define ROM_CHR_ROM_PAGE_OVERFLOW_MASK global_rom.chr_rom_page_overflow_mask
#define ROM_CHR_ROM_CRC                global_rom.chr_rom_crc32
#define ROM_CHR_ROM_MD5                global_rom.chr_rom_md5.hex

#define ROM_HAS_TRAINER               (global_rom.control_byte_1 & ROM_CTRL_TRAINER)
#define ROM_TRAINER                   global_rom.trainer

#define ROM_HAS_SRAM                  global_rom.sram_flag

/* Memory map helper macros. */
#define ROM_PAGE_SIZE_8K    8192
#define ROM_PAGE_SIZE_16K   16384
#define ROM_PAGE_8K(_PAGE)  (ROM_PRG_ROM + ((_PAGE) * ROM_PAGE_SIZE_8K))
#define ROM_PAGE_16K(_PAGE) (ROM_PRG_ROM + ((_PAGE) * ROM_PAGE_SIZE_16K))
#define FIRST_ROM_PAGE     ROM_PRG_ROM
#define LAST_ROM_PAGE      (ROM_PAGE_16K((ROM_PRG_ROM_PAGES - 1)))

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !SYSTEM__ROM_H__INCLUDED */
