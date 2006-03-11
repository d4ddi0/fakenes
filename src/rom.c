/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   rom.c: Implementation of the ROM file handling.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "cpu.h"
#include "ppu.h"
#include "rom.h"
#include "types.h"
#ifdef USE_ZLIB
#include "zlib.h"
#include "unzip.h"
#endif

BOOL rom_is_loaded = FALSE;

#ifdef USE_ZLIB
#  define LR_FILE                     gzFile
#  define LR_OPEN(name, mode)         (gzopen (name, mode "b"))
#  define LR_CLOSE(file)              (gzclose (file))
#  define LR_READ(file, buffer, size) (gzread (file, buffer, size))
#else /* USE_ZLIB */
#  define LR_FILE                     PACKFILE *
#  define LR_OPEN(name, mode)         (pack_fopen (name, mode))
#  define LR_CLOSE(file)              (pack_fclose (file))
#  define LR_READ(file, buffer, size) (pack_fread (buffer, size, file))
#endif   /* !USE_ZLIB */

int load_rom (const UCHAR *filename, ROM *rom)
{
   INES_HEADER header;
   LR_FILE file;
   UINT8 test;

   /* Initialize the ROM context. */
   memset (rom, NULL, sizeof (ROM));

   /* Check if ROM is inside a ZIP file. */
   if (ustrnicmp (get_extension (filename), "zip", USTRING_SIZE) == 0)
      return (load_rom_from_zip (filename, rom));

   /* Open the file. */
   file = LR_OPEN (filename, "r");
   if (!file)
      return (1);

   /* Read the header. */
   LR_READ(file, &header, sizeof (INES_HEADER));

   /* Verify the signature. */
   if (strncmp ((char *)header.signature, "NES\x1a", 4))
   {
      /* Verification failed. */
      LR_CLOSE(file);
      return (1);
   }

   /* Verify that PRG-ROM exists. */
   if (header.prg_rom_pages == 0)
   {
      /* No code to run. ;) */
      LR_CLOSE(file);
      return (2);
   }

   /* Check for 'DiskDude!' contamination. */
   if ((header.control_byte_2 == 'D') &&
       (!(strncmp ((char *)header.reserved, "iskDude!", 8))))
   {
      /* Clean header. */
      header.control_byte_2 = 0;
   }

   /* Read page/bank count. */
   rom->prg_rom_pages = header.prg_rom_pages;
   rom->chr_rom_pages = header.chr_rom_pages;

   /* Read control bytes. */
   rom->control_byte_1 = header.control_byte_1;
   rom->control_byte_2 = header.control_byte_2;

   /* Derive mapper number. */
   rom->mapper_number = ((rom->control_byte_2 & 0xf0) |
      ((rom->control_byte_1 & 0xf0) >> 4));

   /* Set mapper. */
   mmc_request (rom);

   /* Load trainer. */
   if ((rom->control_byte_1 & ROM_CTRL_TRAINER))
   {
      rom->trainer = malloc (ROM_TRAINER_SIZE);
      if (!rom->trainer)
      {
         LR_CLOSE(file);
         return (3);
      }

      LR_READ(file, rom->trainer, ROM_TRAINER_SIZE);
   }

   /* Load PRG-ROM. */
   if (!cpu_get_prg_rom_pages (rom))
   {
      free_rom (rom);
      LR_CLOSE(file);
      return (4);
   }

   LR_READ(file, rom->prg_rom, (rom->prg_rom_pages * 0x4000));

   /* Load CHR-ROM. */
   if (rom->chr_rom_pages > 0)
   {
      if (!ppu_get_chr_rom_pages (rom))
      {
         free_rom (rom);
         LR_CLOSE(file);
         return (5);
      }

      LR_READ(file, rom->chr_rom, (rom->chr_rom_pages * 0x2000));
   }

   /* Close the file. */
   LR_CLOSE (file);

   /* Fill in filename. */
   append_filename (rom->filename, "", filename, sizeof (rom->filename));

   /* Copy SRAM flag. */
   rom->sram_flag = (rom->control_byte_1 & ROM_CTRL_BATTERY);

   /* Set mirroring. */
   if ((rom->control_byte_1 & ROM_CTRL_FOUR_SCREEN))
   {
        ppu_set_mirroring (MIRRORING_FOUR_SCREEN);
   }
   else
   {
      ppu_set_mirroring (((rom->control_byte_1 & ROM_CTRL_MIRRORING) ?
         MIRRORING_VERTICAL : MIRRORING_HORIZONTAL));
   }

   return (0);
}

int load_rom_from_zip (const UCHAR *filename, ROM *rom)
{
    INES_HEADER header;

#ifdef USE_ZLIB
   unzFile file;
   unz_file_info unused;
   UINT8 test;

   /* Open the ZIP file. */
   file = unzOpen (filename);
   if (!file)
      return (1);

   /* Open the first file in the ZIP file. */
   unzGoToFirstFile (file);
   unzOpenCurrentFile (file);

   unzReadCurrentFile (file, &header, sizeof (INES_HEADER));

   if (strncmp ((char *)header.signature, "NES\x1a", 4))
   {
      unzCloseCurrentFile (file);
      unzClose (file);
      return (2);
   }

   if (header.prg_rom_pages == 0)
   {
      unzCloseCurrentFile (file);
      unzClose (file);
      return (3);
   }

   if ((header.control_byte_2 == 'D') &&
       (!(strncmp ((char *)header.reserved, "iskDude!", 8))))
   {
      header.control_byte_2 = 0;
   }

   rom->prg_rom_pages  = header.prg_rom_pages;
   rom->chr_rom_pages  = header.chr_rom_pages;
   rom->control_byte_1 = header.control_byte_1;
   rom->control_byte_2 = header.control_byte_2;

   rom->mapper_number = ((rom->control_byte_2 & 0xf0) |
      ((rom->control_byte_1 & 0xf0) >> 4));
   mmc_request (rom);

   if ((rom->control_byte_1 & ROM_CTRL_TRAINER))
   {
      rom->trainer = malloc (ROM_TRAINER_SIZE);
      if (!rom->trainer)
      {
         unzCloseCurrentFile (file);
         unzClose (file);
         return (4);
      }

      unzReadCurrentFile (file, rom->trainer, ROM_TRAINER_SIZE);
   }

   if (!cpu_get_prg_rom_pages (rom))
   {
      free_rom (rom);
      unzCloseCurrentFile (file);
      unzClose (file);
      return (5);
   }

   unzReadCurrentFile (file, rom->prg_rom, (rom->prg_rom_pages * 0x4000));

   if (rom->chr_rom_pages > 0)
   {
      if (!ppu_get_chr_rom_pages (rom))
      {
         free_rom (rom);
         unzCloseCurrentFile (file);
         unzClose (file);
         return (6);
      }

      unzReadCurrentFile (file, rom->chr_rom, (rom->chr_rom_pages *
         0x2000));
   }

   unzGetCurrentFileInfo (file, &unused, rom->filename, sizeof
      (rom->filename), NULL, NULL, NULL, NULL);

   /* Close the file. */
   unzCloseCurrentFile (file);
   unzClose (file);

   rom->sram_flag = (rom->control_byte_1 & ROM_CTRL_BATTERY);

   if ((rom->control_byte_1 & ROM_CTRL_FOUR_SCREEN))
   {
      ppu_set_mirroring (MIRRORING_FOUR_SCREEN);
   }
   else
   {
      ppu_set_mirroring (((rom->control_byte_1 & ROM_CTRL_MIRRORING) ?
         MIRRORING_VERTICAL : MIRRORING_HORIZONTAL));
   }

   return (0);

#else /* USE_ZLIB */

   return (7);

#endif
}

void free_rom (const ROM *rom)
{
   if (rom->trainer)
      free (rom->trainer);

   cpu_free_prg_rom (rom);
   ppu_free_chr_rom (rom);
}
