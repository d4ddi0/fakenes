

/*

FakeNES - A portable, open-source NES emulator.

rom.c: Implementation of ROM file handling.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include <stdio.h>

#include <stdlib.h>

#include <string.h>


#include "ppu.h"

#include "rom.h"

#include "cpu.h"


#include "misc.h"


int rom_is_loaded = FALSE;


#ifdef USE_ZLIB

#include "zlib.h"

#define LR_FILE gzFile
#define LR_OPEN(name,mode) (gzopen (name,mode))
#define LR_READ(file,buffer,size) (gzread (file, buffer, size))
#define LR_CLOSE(file) (gzclose (file))


#else

#define LR_FILE FILE *
#define LR_OPEN(name,mode) (fopen (name,mode))
#define LR_READ(file,buffer,size) (fread (buffer, 1, size, file))
#define LR_CLOSE(file) (fclose (file))


#endif


int load_rom (AL_CONST UINT8 * filename, ROM * rom)
{
    LR_FILE rom_file;

    NES_HEADER nes_header;

    UINT8 test;


    /* Pointers should be initialized even if not used */
    rom -> trainer = NULL;
    rom -> prg_rom = NULL;
    rom -> chr_rom = NULL;
    rom -> chr_rom_cache = NULL;
    rom -> chr_rom_cache_tag = NULL;


    /* Open the file. */

    rom_file = LR_OPEN (filename, "rb");

    if (! rom_file)
    {
        return (1);
    }


    /* Read the header. */
    LR_READ (rom_file, &nes_header, 16);

    /* Verify the signature. */

    if (strncmp((char *) nes_header.signature, "NES\x1A", 4))
    {
        LR_CLOSE (rom_file);
    
        return (1);
    }


    /* Verify that PRG ROM exists */

    if (nes_header.prg_rom_pages == 0)
    {
        LR_CLOSE (rom_file);

        return (1);
    }


    /* Check for 'DiskDude!'. */

    if ((nes_header.control_byte_2 == 'D') &&
        !(strncmp ((char *) nes_header.reserved, "iskDude!", 8)))
    {
        nes_header.control_byte_2 = 0;
    }


    /* Read page/bank count. */

    rom -> prg_rom_pages = nes_header.prg_rom_pages;

    rom -> chr_rom_pages = nes_header.chr_rom_pages;


    /* Read control bytes. */

    rom -> control_byte_1 = nes_header.control_byte_1;

    rom -> control_byte_2 = nes_header.control_byte_2;


    /* Derive mapper number. */

    rom -> mapper_number =
        ((rom -> control_byte_2 & 0xf0) |
        ((rom -> control_byte_1 & 0xf0) >> 4));

    mmc_request (rom);


    /* Allocate and load trainer. */

    if (rom -> control_byte_1 & ROM_CTRL_TRAINER)
    {
        rom -> trainer = malloc (512);

        if (! rom -> trainer)
        {
            LR_CLOSE (rom_file);

            return (1);
        }


        LR_READ (rom_file, rom -> trainer, 512);
    }


    /* Allocate and load PRG-ROM. */

    if (! cpu_get_prg_rom_pages (rom))
    {
        free_rom (rom);

        LR_CLOSE (rom_file);

        return (1);
    }


    LR_READ (rom_file, rom ->
        prg_rom, (rom -> prg_rom_pages * 0x4000));


    /* Allocate and load CHR-ROM. */

    if (rom -> chr_rom_pages)
    {
        if (! ppu_get_chr_rom_pages (rom))
        {
            free_rom (rom);

            LR_CLOSE (rom_file);

            return (1);
        }

        LR_READ (rom_file, rom -> chr_rom,
            (rom -> chr_rom_pages * 0x2000));
    }


    /* Fill in extra stuff. */

    append_filename (rom ->
        filename, "", filename, sizeof (rom -> filename));

    rom -> sram_flag = (rom -> control_byte_1 & ROM_CTRL_SRAM);


    if (rom -> control_byte_1 & ROM_CTRL_4SCREEN)
    {
        ppu_set_mirroring (MIRRORING_FOUR_SCREEN);
    }
    else
    {
        ppu_set_mirroring (((rom -> control_byte_1 &
            ROM_CTRL_MIRROR) ? MIRRORING_VERTICAL : MIRRORING_HORIZONTAL));
    }


    LR_CLOSE (rom_file);

    return (0);
}


void free_rom (ROM * rom)
{
    if (rom -> trainer)
    {
        free (rom -> trainer);
    }


    cpu_free_prg_rom (rom);
    ppu_free_chr_rom (rom);
}
