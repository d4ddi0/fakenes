

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


int load_rom (CONST UINT8 * filename, ROM * rom)
{
    gzFile rom_file;


    UINT32 signature;

    UINT8 test;


    /* Open the file. */

    rom_file = gzopen (filename, "rb");

    if (! rom_file)
    {
        return (1);
    }


    /* Read the signature. */

    gzread (rom_file, &signature, 4);

    if (signature != 0x1a53454e)
    {
        gzclose (rom_file);
    
        return (1);
    }


    /* Read page/bank count. */

    rom -> prg_rom_pages = gzgetc (rom_file);

    rom -> chr_rom_pages = gzgetc (rom_file);


    /* Read control bytes. */

    rom -> control_byte_1 = gzgetc (rom_file);

    rom -> control_byte_2 = gzgetc (rom_file);


    if (rom -> prg_rom_pages == 0)
    {
        gzclose (rom_file);

        return (1);
    }


    /* Check for 'DiskDude!'. */

    test = gzgetc (rom_file);

    if ((rom -> control_byte_2 == 'D') && (test == 'i'))
    {
        rom -> control_byte_2 = 0;
    }


    /* Derive mapper number. */

    rom -> mapper_number = ((rom ->
        control_byte_2 >> 4) | ((rom -> control_byte_1 & 0xf0) >> 4));


    /* Skip reserved bytes. */

    gzseek (rom_file, 7, SEEK_CUR);
  

    /* Allocate and load trainer. */

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

    rom -> chr_rom = malloc ((rom -> chr_rom_pages * 0x2000));

    if (! rom -> chr_rom)
    {
        if (rom -> trainer)
        {
            free (rom -> trainer);
        }


        free (rom -> prg_rom);


        gzclose (rom_file);

        return (1);
    }


    gzread (rom_file, rom ->
        chr_rom, (rom -> chr_rom_pages * 0x2000));


    /* Fill in extra stuff. */

    append_filename (rom ->
        filename, "", filename, sizeof (rom -> filename));

    rom -> sram_flag = (rom -> control_byte_1 & ROM_CTRL_SRAM);


    set_ppu_mirroring (((rom -> control_byte_1 &
        ROM_CTRL_MIRROR) ? MIRRORING_VERTICAL : MIRRORING_HORIZONTAL));


    gzclose (rom_file);

    return (0);
}


#else


int load_rom (CONST UINT8 * filename, ROM * rom)
{
    FILE * rom_file;


    UINT32 signature;

    UINT8 test;


    /* Open the file. */

    rom_file = fopen (filename, "rb");

    if (! rom_file)
    {
        return (1);
    }


    /* Read the signature. */

    fread (&signature, 1, 4, rom_file);

    if (signature != 0x1a53454e)
    {
        fclose (rom_file);
    
        return (1);
    }


    /* Read page/bank count. */

    rom -> prg_rom_pages = fgetc (rom_file);

    rom -> chr_rom_pages = fgetc (rom_file);


    /* Read control bytes. */

    rom -> control_byte_1 = fgetc (rom_file);

    rom -> control_byte_2 = fgetc (rom_file);


    if (rom -> prg_rom_pages == 0)
    {
        fclose (rom_file);

        return (1);
    }


    /* Check for 'DiskDude!'. */

    test = fgetc (rom_file);

    if ((rom -> control_byte_2 == 'D') && (test == 'i'))
    {
        rom -> control_byte_2 = 0;
    }


    /* Derive mapper number. */

    rom -> mapper_number = ((rom ->
        control_byte_2 >> 4) | ((rom -> control_byte_1 & 0xf0) >> 4));


    /* Skip reserved bytes. */

    fseek (rom_file, 7, SEEK_CUR);
  

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

    rom -> chr_rom = malloc ((rom -> chr_rom_pages * 0x2000));

    if (! rom -> chr_rom)
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


    set_ppu_mirroring (((rom -> control_byte_1 &
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

    free (rom -> chr_rom);
}
