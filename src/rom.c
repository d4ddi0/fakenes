

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


#include "misc.h"


int rom_is_loaded = FALSE;


#ifdef USE_ZLIB


#include "zlib.h"


int load_rom (AL_CONST UINT8 * filename, ROM * rom)
{
    gzFile rom_file;

    NES_HEADER nes_header;

    UINT8 test;


    /* Open the file. */

    rom_file = gzopen (filename, "rb");

    if (! rom_file)
    {
        return (1);
    }


    /* Read the header. */
    gzread (rom_file, &nes_header, 16);

    /* Verify the signature. */

    if (strncmp((char *) nes_header.signature, "NES\x1A", 4))
    {
        gzclose (rom_file);
    
        return (1);
    }


    /* Verify that PRG ROM exists */

    if (nes_header.prg_rom_pages == 0)
    {
        gzclose (rom_file);

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


    /* Allocate and load trainer. */

    /* Pointer should be initialized even if no trainer exists */
    rom -> trainer = NULL;

    if (rom -> control_byte_1 & ROM_CTRL_TRAINER)
    {
        rom -> trainer = malloc (512);

        if (! rom -> trainer)
        {
            gzclose (rom_file);

            return (1);
        }


        gzread (rom_file, rom -> trainer, 512);
    }


    /* Allocate and load PRG-ROM. */

    rom -> prg_rom = malloc ((rom -> prg_rom_pages * 0x4000));

    if (! rom -> prg_rom)
    {
        if (rom -> trainer)
        {
            free (rom -> trainer);
        }


        gzclose (rom_file);

        return (1);
    }


    gzread (rom_file, rom ->
        prg_rom, (rom -> prg_rom_pages * 0x4000));


    /* Allocate and load CHR-ROM. */

    if (! ppu_get_chr_rom_pages (rom))
    {
        if (rom -> trainer)
        {
            free (rom -> trainer);
        }


        free (rom -> prg_rom);


        gzclose (rom_file);

        return (1);
    }


    gzread (rom_file, rom -> chr_rom,
        (rom -> chr_rom_pages * 0x2000));


    /* Fill in extra stuff. */

    append_filename (rom ->
        filename, "", filename, sizeof (rom -> filename));

    rom -> sram_flag = (rom -> control_byte_1 & ROM_CTRL_SRAM);


    ppu_set_mirroring (((rom -> control_byte_1 &
        ROM_CTRL_MIRROR) ? MIRRORING_VERTICAL : MIRRORING_HORIZONTAL));


    gzclose (rom_file);

    return (0);
}


#else


int load_rom (AL_CONST UINT8 * filename, ROM * rom)
{
    FILE * rom_file;

    NES_HEADER nes_header;

    UINT8 test;


    /* Open the file. */

    rom_file = fopen (filename, "rb");

    if (! rom_file)
    {
        return (1);
    }


    /* Read the header. */
    fread (&nes_header, 1, 16, rom_file);

    /* Verify the signature. */

    if (strncmp((char *) nes_header.signature, "NES\x1A", 4))
    {
        fclose (rom_file);
    
        return (1);
    }



    /* Verify that PRG ROM exists */

    if (nes_header.prg_rom_pages == 0)
    {
        fclose (rom_file);

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


    /* Allocate and load trainer. */

    if (rom -> control_byte_1 & ROM_CTRL_TRAINER)
    {
        rom -> trainer = malloc (512);

        if (! rom -> trainer)
        {
            fclose (rom_file);

            return (1);
        }


        fread (rom -> trainer, 1, 512, rom_file);
    }


    /* Allocate and load PRG-ROM. */

    rom -> prg_rom = malloc ((rom -> prg_rom_pages * 0x4000));

    if (! rom -> prg_rom)
    {
        if (rom -> trainer)
        {
            free (rom -> trainer);
        }


        fclose (rom_file);

        return (1);
    }


    fread (rom -> prg_rom, 1,
        (rom -> prg_rom_pages * 0x4000), rom_file);


    /* Allocate and load CHR-ROM. */

    if (! ppu_get_chr_rom_pages (rom))
    {
        if (rom -> trainer)
        {
            free (rom -> trainer);
        }


        free (rom -> prg_rom);


        fclose (rom_file);

        return (1);
    }


    fread (rom -> chr_rom, 1,
        (rom -> chr_rom_pages * 0x2000), rom_file);


    /* Fill in extra stuff. */

    append_filename (rom ->
        filename, "", filename, sizeof (rom -> filename));

    rom -> sram_flag = (rom -> control_byte_1 & ROM_CTRL_SRAM);


    ppu_set_mirroring (((rom -> control_byte_1 &
        ROM_CTRL_MIRROR) ? MIRRORING_VERTICAL : MIRRORING_HORIZONTAL));


    fclose (rom_file);

    return (0);
}


#endif /* ! USE_ZLIB */


void free_rom (ROM * rom)
{
    if (rom -> trainer)
    {
        free (rom -> trainer);
    }


    free (rom -> prg_rom);

    ppu_free_chr_rom (rom);
}
