

/*

FakeNES - A portable, Open Source NES emulator.

rom.c: Implementation of the ROM file handling.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <stdio.h>

#include <stdlib.h>

#include <string.h>


#include "ppu.h"

#include "rom.h"

#include "cpu.h"


#include "misc.h"


int rom_is_loaded = FALSE;


#ifdef USE_ZLIB

#   include "zlib.h"


#   define LR_FILE  gzFile


#   define LR_OPEN(name, mode)  (gzopen (name, mode))

#   define LR_CLOSE(file)       (gzclose (file))


#   define LR_READ(file, buffer, size)  (gzread (file, buffer, size))

#else

#   define LR_FILE  FILE *


#   define LR_OPEN(name, mode)  (fopen (name, mode))

#   define LR_CLOSE(file)       (fclose (file))


#   define LR_READ(file, buffer, size)  (fread (buffer, 1, size, file))

#endif


#define ROM_TRAINER_SIZE    512


int load_rom (const UINT8 * filename, ROM * rom)
{
    INES_HEADER ines_header;

    LR_FILE rom_file;


    UINT8 test;


    /* Initialize the ROM context. */

    memset (rom, NULL, sizeof (ROM));


    /* Open the file. */

    rom_file = LR_OPEN (filename, "rb");

    if (! rom_file)
    {
        return (1);
    }


    /* Read the header. */

    LR_READ (rom_file, &ines_header, sizeof (INES_HEADER));


    /* Verify the signature. */

    if (strncmp ((char *) ines_header.signature, "NES\x1a", 4))
    {
        LR_CLOSE (rom_file);
    
        return (1);
    }


    /* Verify that PRG-ROM exists. */

    if (ines_header.prg_rom_pages == 0)
    {
        LR_CLOSE (rom_file);

        return (1);
    }


    /* Check for 'DiskDude!'. */

    if ((ines_header.control_byte_2 == 'D') &&
        (! (strncmp ((char *) ines_header.reserved, "iskDude!", 8))))
    {
        ines_header.control_byte_2 = 0;
    }


    /* Read page/bank count. */

    rom -> prg_rom_pages = ines_header.prg_rom_pages;

    rom -> chr_rom_pages = ines_header.chr_rom_pages;


    /* Read control bytes. */

    rom -> control_byte_1 = ines_header.control_byte_1;

    rom -> control_byte_2 = ines_header.control_byte_2;


    /* Derive mapper number. */

    rom -> mapper_number = ((rom -> control_byte_2 & 0xf0) | ((rom -> control_byte_1 & 0xf0) >> 4));


    mmc_request (rom);


    /* Allocate and load trainer. */

    if ((rom -> control_byte_1 & ROM_CTRL_TRAINER))
    {
        rom -> trainer = malloc (ROM_TRAINER_SIZE);

        if (! rom -> trainer)
        {
            LR_CLOSE (rom_file);

            return (1);
        }


        LR_READ (rom_file, rom -> trainer, ROM_TRAINER_SIZE);
    }


    /* Allocate and load PRG-ROM. */

    if (! cpu_get_prg_rom_pages (rom))
    {
        free_rom (rom);

        LR_CLOSE (rom_file);


        return (1);
    }


    LR_READ (rom_file, rom -> prg_rom, (rom -> prg_rom_pages * 0x4000));


    /* Allocate and load CHR-ROM. */

    if (rom -> chr_rom_pages)
    {
        if (! ppu_get_chr_rom_pages (rom))
        {
            free_rom (rom);

            LR_CLOSE (rom_file);


            return (1);
        }

        LR_READ (rom_file, rom -> chr_rom, (rom -> chr_rom_pages * 0x2000));
    }


    /* Fill in extra stuff. */

    append_filename (rom -> filename, "", filename, sizeof (rom -> filename));

    rom -> sram_flag = (rom -> control_byte_1 & ROM_CTRL_BATTERY);


    if ((rom -> control_byte_1 & ROM_CTRL_FOUR_SCREEN))
    {
        ppu_set_mirroring (MIRRORING_FOUR_SCREEN);
    }
    else
    {
        ppu_set_mirroring (((rom -> control_byte_1 & ROM_CTRL_MIRRORING)
            ? MIRRORING_VERTICAL : MIRRORING_HORIZONTAL));
    }


    /* Close the file. */

    LR_CLOSE (rom_file);


    return (0);
}


void free_rom (const ROM * rom)
{
    if (rom -> trainer)
    {
        free (rom -> trainer);
    }


    cpu_free_prg_rom (rom);

    ppu_free_chr_rom (rom);
}
