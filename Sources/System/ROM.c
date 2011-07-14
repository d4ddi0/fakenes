/* FakeNES - A free, portable, Open Source NES emulator.

   rom.c: Implementation of the ROM file handling.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "debug.h"
#include "cpu.h"
#include "mmc.h"
#include "ppu.h"
#include "rom.h"
#include "shared/bufferfile.h"
#include "types.h"
#ifdef USE_ZLIB
#include <zlib.h>
#include "etc/unzip.h"
#endif

/* Global ROM container. */
ROM global_rom;

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

/* Size of the transfer buffer when loading ROM files into memory (larger is
   faster, but don't get too carried away). */
#define BUFFER_SIZE  65536

static UINT8* get_prg_rom_pages(ROM* rom);
static UINT8* get_chr_rom_pages(ROM* rom);
static void free_prg_rom(ROM* rom);
static void free_chr_rom(ROM* rom);

int load_ips (const UDATA *filename, PACKFILE *buffer_file)
{
   PACKFILE *file;
   UINT8 signature[5];

   RT_ASSERT(filename);
   RT_ASSERT(buffer_file);

   /* Open IPS file. */
   file = pack_fopen (filename, "r");
   if (!file)
   {
      log_printf ("ROM: IPS loader: Couldn't open file (%s).", filename);
      return (1);
   }

   /* Verify signature. */
   pack_fread (signature, sizeof(signature), file);

   if (strncmp (signature, "PATCH", 5) != 0)
   {
      /* Verification failed. */
      log_printf ("ROM: IPS loader: Vertification failed.");
      pack_fclose (file);
      return (2);
   }

   for (;;)
   {
      UINT8 marker[3];
      UINT32 offset;
      UINT16 length;

      /* Load next marker. */
      pack_fread (marker, sizeof(marker), file);

      /* Check for end of file marker. */
      if (strncmp (marker, "EOF", 3) == 0)
         break;          

      /* Generate packed offset. */
      /* IPS is big-endian, so this works on all systems */
      offset = ((marker[0] << 16) | (marker[1] << 8) | marker[2]);

      /* Seek to offset in output. */
      pack_fseek (buffer_file, offset);

      /* Load block length. */
      length = pack_mgetw (file);

      /* 0 = RLE encoded. */
      if (length == 0)
      {
         UINT16 rle_length;
         UINT8 rle_char;
         int rle_count;

         /* Run length - 0 to 0xFFFF. */
         rle_length = pack_mgetw (file);

         /* Fill value. */
         rle_char = pack_getc (file);

         /* Overwrite existing data sequentially. */
         for (rle_count = 0; rle_count < rle_length; rle_count++)
            pack_putc (rle_char, buffer_file);
      }
      else
      {
         /* Block copy. */

         UINT8 buffer[0xFFFF];
         unsigned bytes;

         bytes = pack_fread (buffer, length, file);
         if (bytes < length)
            log_printf ("ROM: IPS loader: Warning: Length underrun.");

         pack_fwrite (buffer, bytes, buffer_file);
      }
   }

   /* Close IPS file. */
   pack_fclose (file);

   /* Return success. */
   return (0);
}

int load_ines_rom (PACKFILE *file, ROM *rom)
{
   /* Generic loader function for loading the iNES ROM format from an
      already open PACKFILE. */

   INES_HEADER header;
   unsigned size;

   RT_ASSERT(file);
   RT_ASSERT(rom);

   /* Read the header. */
   pack_fread (&header, sizeof(INES_HEADER), file);

   /* Verify the signature. */
   if (strncmp ((char *)header.signature, "NES\x1a", 4))
   {
      /* Verification failed. */
      log_printf ("ROM: iNES loader: Vertification failed.");
      return (1);
   }

   /* Verify that PRG-ROM exists. */
   if (header.prg_rom_pages == 0)
   {
      /* No code to run. ;) */
      WARN_GENERIC();
      log_printf ("ROM: iNES loader: PRG ROM is missing.");
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
   rom->mapper_number = ((rom->control_byte_2 & 0xf0) | ((rom->control_byte_1 & 0xf0) >> 4));

   /* Set mapper. */
   mmc_request (rom->mapper_number);

   /* Load trainer. */
   if ((rom->control_byte_1 & ROM_CTRL_TRAINER))
   {
      rom->trainer = malloc (ROM_TRAINER_SIZE);
      if (!rom->trainer)
      {
         WARN_GENERIC();
         log_printf ("ROM: iNES loader: Failed to allocate memory for the trainer.");
         return (3);
      }

      pack_fread (rom->trainer, ROM_TRAINER_SIZE, file);
   }

   /* Load PRG-ROM. */
   if (!get_prg_rom_pages (rom))
   {
      WARN_GENERIC();
      log_printf ("ROM: iNES loader: Failed to allocate memory for PRG ROM.");
      free_rom (rom);
      return (4);
   }

   size = rom->prg_rom_pages * ROM_PRG_ROM_PAGE_SIZE;
   pack_fread (rom->prg_rom, size, file);

   /* Compute CRC32 for PRG-ROM. */
   rom->prg_rom_crc32 = calculate_crc32(rom->prg_rom, size);
   rom->prg_rom_md5 = calculate_md5(rom->prg_rom, size);
   log_printf("PRG-ROM CRC: %08X, MD5: %s\n", rom->prg_rom_crc32, rom->prg_rom_md5.hex);

   /* Load CHR-ROM. */
   if (rom->chr_rom_pages > 0)
   {
      if (!get_chr_rom_pages (rom))
      {
         WARN_GENERIC();
         log_printf ("ROM: iNES loader: Failed to allocate memory for CHR ROM.");
         free_rom (rom);
         return (5);
      }

      size = rom->chr_rom_pages * ROM_CHR_ROM_PAGE_SIZE;
      pack_fread (rom->chr_rom, size, file);

      /* Compute CRC for CHR-ROM. */
      rom->chr_rom_crc32 = calculate_crc32(rom->chr_rom, size);
      rom->chr_rom_md5 = calculate_md5(rom->chr_rom, size);
      log_printf("PRG-ROM CRC: %08X, MD5: %s\n", rom->chr_rom_crc32, rom->chr_rom_md5.hex);
   }

   /* Copy SRAM flag. */
   rom->sram_flag = (rom->control_byte_1 & ROM_CTRL_BATTERY);

   /* Set mirroring. */
   if ((rom->control_byte_1 & ROM_CTRL_FOUR_SCREEN))
      ppu_set_default_mirroring (PPU_MIRRORING_FOUR_SCREEN);
   else
      ppu_set_default_mirroring (((rom->control_byte_1 & ROM_CTRL_MIRRORING) ?
         PPU_MIRRORING_VERTICAL : PPU_MIRRORING_HORIZONTAL));

   /* Return success. */
   return (0);
}

int load_rom (const UDATA *filename, ROM *rom)
{
   LR_FILE file;
   PACKFILE *buffer_file;
   long bytes;
   USTRING ips;
   int error;

   RT_ASSERT(filename);
   RT_ASSERT(rom);

   /* Initialize the ROM context. */
   memset (rom, NULL, sizeof(ROM));

   /* Check if ROM is inside a ZIP file. */
   if (ustrnicmp (get_extension (filename), "zip", USTRING_SIZE) == 0)
      return (load_rom_from_zip (filename, rom));

   /* Open the file. */
   file = LR_OPEN(filename, "r");
   if (!file)
   {
      log_printf ("ROM: Couldn't open file (%s).", filename);
      return (1);
   }

   /* Open the buffer file. */
   buffer_file = BufferFile_open ();
   if (!buffer_file)
   {
      WARN_GENERIC();
      log_printf ("ROM: Couldn't open buffer file (out of memory?).");
      LR_CLOSE(file);
      return (2);
   }

   /* Transfer the file contents to the buffer file. */
   for (;;)
   {
      UINT8 buffer[BUFFER_SIZE];

      bytes = LR_READ(file, &buffer, sizeof(buffer));
      if (bytes <= 0)
         break;

      pack_fwrite (&buffer, bytes, buffer_file);
   }

   /* Close the file. */
   LR_CLOSE(file);

   /* Seek back to the beginning. */
   pack_fseek (buffer_file, 0);

   /* See if we have a matching IPS file. */
   USTRING_CLEAR(ips);
   replace_extension (ips, filename, "ips", (sizeof(ips) - 1));

   if (file_size (ips))
   {
      /* Load it and patch our data. */
      error = load_ips (ips, buffer_file);
      if (error != 0)
      {
         log_printf ("ROM: IPS load failed (consult above messages if any).");
         pack_fclose (buffer_file);
         return ((8 + error));
      }

      /* Seek back to the beginning. */
      pack_fseek (buffer_file, 0);
   }

   /* Load the ROM. */
   error = load_ines_rom (buffer_file, rom);
   if (error != 0)
   {
      log_printf ("ROM: iNES load failed (consult above messages if any).");
      pack_fclose (buffer_file);
      return ((8 + error));
   }

   /* Close the buffer file. */
   pack_fclose (buffer_file);

   /* Fill in filename. */
   append_filename (rom->filename, empty_string, filename, sizeof(rom->filename));

   /* Return success. */
   return (0);
}

int load_rom_from_zip (const UDATA *filename, ROM *rom)
{
#ifdef USE_ZLIB
   unzFile file;
   PACKFILE *buffer_file;
   long bytes;
   USTRING ips;
   int error;
   unz_file_info unused;

   RT_ASSERT(filename);
   RT_ASSERT(rom);

   /* Open the ZIP file. */
   file = unzOpen (filename);
   if (!file)
   {
      log_printf ("ROM: ZIP loader: Couldn't open file (%s).", filename);
      return (1);
   }

   /* Open the first file in the ZIP file. */
   /* TODO: Error checking(?) */
   unzGoToFirstFile (file);
   unzOpenCurrentFile (file);

   /* Open the buffer file. */
   buffer_file = BufferFile_open ();
   if (!buffer_file)
   {
      WARN_GENERIC();
      log_printf ("ROM: ZIP loader: Couldn't open buffer file (out of memory?).");
      unzCloseCurrentFile (file);
      unzClose (file);
      return (2);
   }

   /* Transfer the file contents to the buffer file. */
   for (;;)
   {
      UINT8 buffer[BUFFER_SIZE];

      bytes = unzReadCurrentFile (file, &buffer, sizeof(buffer));
      if (bytes <= 0)
         break;

      pack_fwrite (&buffer, bytes, buffer_file);
   }

   /* Fill in filename. */
   unzGetCurrentFileInfo (file, &unused, rom->filename, sizeof(rom->filename), NULL, NULL, NULL, NULL);

   /* Close the file. */
   unzCloseCurrentFile (file);
   unzClose (file);

   /* Seek back to the beginning. */
   pack_fseek (buffer_file, 0);

   /* See if we have a matching IPS file. */
   USTRING_CLEAR(ips);
   replace_extension (ips, filename, "ips", (sizeof(ips) - 1));

   if (!exists (ips))
   {
      /* Try a variation of the ZIP'ed filename instead. */
      USTRING_CLEAR(ips);
      replace_extension (ips, rom->filename, "ips", (sizeof(ips) - 1));
   }

   if (exists (ips))
   {
      /* Load it and patch our data. */
      error = load_ips (ips, buffer_file);
      if (error != 0)
      {
         log_printf ("ROM: IPS load failed (consult above messages if any).");
         pack_fclose (buffer_file);
         return ((8 + error));
      }

      /* Seek back to the beginning. */
      pack_fseek (buffer_file, 0);
   }

   /* Load the ROM. */
   error = load_ines_rom (buffer_file, rom);
   if (error != 0)
   {
      log_printf ("ROM: ZIP loader: iNES load failed (consult above messages if any).");
      pack_fclose (buffer_file);
      return ((8 + error));
   }

   /* Close the buffer file. */
   pack_fclose (buffer_file);

   /* Return success. */
   return (0);

#else /* USE_ZLIB */

   /* Not supported. */
   return (7);

#endif
}

void free_rom (ROM *rom)
{
   RT_ASSERT(rom);

   if (rom->trainer) {
      free (rom->trainer);
      rom->trainer = NULL;
   }

   free_prg_rom (rom);
   free_chr_rom (rom);
}

/* ---------------------------------------------------------------------- */

static UINT8* get_prg_rom_pages(ROM* rom)
{
   int num_pages;
   int copycount, missing, count, next, pages_mirror_size;

   RT_ASSERT(rom);

   num_pages = rom->prg_rom_pages;

    /* Compute a mask used to wrap invalid PRG ROM page numbers.
     *  As PRG ROM uses a 16k page size, this mask is based
     *  on a 16k page size.
     */
    if (((num_pages * 2 - 1) & (num_pages - 1)) == (num_pages - 1))
    /* compute mask for even power of two */
    {
        pages_mirror_size = num_pages;
    }
    else
    /* compute mask */
    {
        int i;

        /* compute the smallest even power of 2 greater than
           PRG ROM page count, and use that to compute the mask */
        for (i = 0; (num_pages >> i) > 0; i++);

        pages_mirror_size = (1 << i);
    }

    rom->prg_rom_page_overflow_mask = (pages_mirror_size - 1);

    /* identify-map all the present pages */
    for (copycount = 0; copycount < num_pages; copycount++)
    {
        rom->prg_rom_page_lookup[copycount] = copycount;
    }

    /* mirror-map all the not-present pages */
    for (next = num_pages, missing = pages_mirror_size - num_pages,
        count = 1; missing; count <<= 1, missing >>= 1)
    {
        if (missing & 1)
        {
            for (copycount = count; copycount; copycount--, next++)
            {
                rom->prg_rom_page_lookup[next] =
                  rom->prg_rom_page_lookup[(next - count)];
            }
        }
    }

    /* 16k PRG ROM page size */
    rom->prg_rom = malloc ((num_pages * 0x4000));
    if (rom->prg_rom)
    {
        /* initialize to a known value for areas not present in image */
        memset (rom->prg_rom, 0xff, (num_pages * 0x4000));
    }

    return (rom->prg_rom);
}

static UINT8* get_chr_rom_pages(ROM* rom)
{
   RT_ASSERT(rom);

   const int num_pages = rom->chr_rom_pages;

   /* Compute a mask used to wrap invalid CHR ROM page numbers.
      As CHR ROM uses a 8k page size, this mask is based
      on a 8k page size. */
   int pages_mirror_size;
   if(((num_pages * 2 - 1) & (num_pages - 1)) == (num_pages - 1)) {
      // Compute mask for even power of two.
      pages_mirror_size = num_pages;
   }
   else {
      /* Compute the smallest even power of 2 greater than
         CHR ROM page count, and use that to compute the mask. */
      int i;
      for(i = 0; (num_pages >> i) > 0; i++);
      pages_mirror_size = 1 << i;
   }

   rom->chr_rom_page_overflow_mask = pages_mirror_size - 1;

   // Identify-map all the present pages.
   int copycount;
   for(copycount = 0; copycount < num_pages; copycount++)
      rom->chr_rom_page_lookup[copycount] = copycount;

   // Mirror-map all the not-present pages.
   int missing, count, next;
   for(next = num_pages, missing = pages_mirror_size - num_pages,
       count = 1; missing; count <<= 1, missing >>= 1)
        if(missing & 1)
            for(copycount = count; copycount; copycount--, next++)
                rom->chr_rom_page_lookup[next] = rom->chr_rom_page_lookup[next - count];

    // 8k CHR ROM page size.
    const unsigned size = num_pages * 0x2000;
    rom->chr_rom = (UINT8*)malloc(size);
    if(rom->chr_rom)
        // Initialize to a known value for areas not present in image.
        memset(rom->chr_rom, 0xFF, size);

    return rom->chr_rom;
}

static void free_prg_rom(ROM* rom)
{
   RT_ASSERT(rom);

   if(rom->prg_rom) {
      free(rom->prg_rom);
      rom->prg_rom = NULL;
   }
}

static void free_chr_rom(ROM* rom)
{
   RT_ASSERT(rom);

   if(rom->chr_rom) {
      free(rom->chr_rom);
      rom->chr_rom = NULL;
   }
}
