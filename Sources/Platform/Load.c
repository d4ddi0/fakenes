/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include <allegro.h>
#include "common.h"
#include "debug.h"
#include "machine.h"
#include "nsf.h"
#include "rom.h"
#include "save.h"
#include "types.h"
#include "version.h"

/* TODO: Support loading Zip/GZip NSFs. */

/* Whether an NSF is currently loaded. */
BOOL nsf_is_loaded = FALSE;
/* Whether a ROM (NES or other) is currently loaded. */
BOOL rom_is_loaded = FALSE;

/* Whether any file is currently loaded (either ROM or NSF). */
BOOL file_is_loaded = FALSE;

/* Function prototypes (defined at bottom). */
static const UDATA* load_nsf_file(const UDATA* filename);
static void close_nsf_file(void);
static const UDATA* load_rom_file(const UDATA* filename);
static void close_rom_file(void);

const UDATA* load_file(const UDATA *filename)
{
   const UDATA* result = NULL;

   RT_ASSERT(filename);

   /* Attempt to intercept NSF file. */
   if(ustricmp(get_extension(filename), "NSF") == 0) {
      result = load_nsf_file(filename);
   }
   else {
      /* Just filter down to the ROM loader for other formats. */
      result = load_rom_file(filename);
   }

   /* The return value contains an error string - so NULL is good. */
   if(!result) {
      USTRING scratch;

      /* Start emulation. */
      machine_resume();

      /* Update window title. */
      uszprintf(scratch, sizeof(scratch), "%s - %s", WINDOW_TITLE_FILE, get_filename(filename));
      set_window_title(scratch);
   }

   return result;
}

void close_file(void)
{
   /* Sanity check. */
   if(!file_is_loaded)
      WARN_GENERIC();

   /* Stop emulation. */
   machine_pause();

   /* Check if the open file is an NSF. */
   if(nsf_is_loaded) {
      close_nsf_file();
   }
   else if(rom_is_loaded) {
      /* Normal ROM. */
      close_rom_file();
   }
   else {
      /* This shouldn't ever happen, but just in case. */
      WARN_GENERIC();
   }

   /* Clear file lock. */
   file_is_loaded = FALSE;

   /* Reset window title. */
   set_window_title(WINDOW_TITLE_NORMAL);
}

/* -------------------------------------------------------------------------------- */

static const UDATA* load_nsf_file(const UDATA* filename)
{
   RT_ASSERT(filename);

   /* Any existing NSF must be closed before opening a new one. */
   if(nsf_is_loaded)
      close_file();

   /* Try and open the file. */
   if(nsf_open(filename)) {
      /* Close currently open file and save data. */
      if(file_is_loaded)
         close_file();

      /* Switch to NSF mode. */
      nsf_is_loaded = TRUE;
      /* Set file lock. */
      file_is_loaded = TRUE;

      /* Clear ROM space and set up mapper. */
      memset(&global_rom, 0, sizeof(global_rom));
      mmc_force(&nsf_mapper);

      /* Initialize the virtual machine. */
      machine_init();

      /* Set up the NSF player. */
      nsf_setup();

      /* Return success. */
      return NULL;
   }

   /* The NSF failed to load. */
   return "Failed to load NSF file!";
}

static void close_nsf_file(void)
{
   /* Sanity check. */
   if(!nsf_is_loaded)
      WARN_GENERIC();

   /* Close the NSF player. */
   nsf_teardown();

   /* Close the virtual machine. */
   machine_exit();

   /* Close the file. */
   nsf_close();
   nsf_is_loaded = FALSE;
}

static const UDATA* load_rom_file(const UDATA* filename)
{
   ROM rom;

   if(load_rom(filename, &rom) == 0) {
      /* Close currently open file and save data. */
      if(file_is_loaded)
         close_file();

      /* Switch to ROM mode. */
      rom_is_loaded = TRUE;
      /* Set file lock. */
      file_is_loaded = TRUE;
 
     /* Copy the file into ROM space. */
      memcpy(&global_rom, &rom, sizeof(ROM));

      /* Initialize the virtual machine. */
      machine_init();

      /* Return success. */ 
      return NULL;
   }

   /* The ROM failed to load. */
   return "Failed to load ROM! (Bad format?)";
}

static void close_rom_file(void)
{
   /* Sanity check. */
   if(!rom_is_loaded)
      WARN_GENERIC();

   /* Save SRAM and patches. */
   save_sram();      
   save_patches();

   /* Close the virtual machine. */
   machine_exit();

   /* Close the file. */
   free_rom(&global_rom);
   rom_is_loaded = FALSE;
}
