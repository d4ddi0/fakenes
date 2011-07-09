/* FakeNES - A free, portable, Open Source NES emulator.

   main.c: Implementation of the main loop.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include "audio.h"
#include "common.h"
#include "config.h"
#include "debug.h"
#include "gui.h"
#include "input.h"
#include "load.h"
#include "log.h"
#include "machine.h"
#include "main.h"
#include "net.h"
#include "netplay.h"
#include "platform.h"
#include "types.h"
#include "version.h"
#include "video.h"

/* TODO: Check error codes on everything. */

/* Flow-control. This allows other modules (such as the GUI code) to cause the
   main loop to exit and similar. */
BOOL want_exit = FALSE;
BOOL want_gui = TRUE;

/* Whether or not this is the first run of the emulator. */
static BOOL first_run = TRUE;

/* Function prototypes. */
static void cleanup(void);

int main(int argc, char *argv[])
{
   int result;

   /* Clear the console. */
   console_clear();

   if(VERSION == 0x030)
      console_printf("This release is dedicated to those who fell in the 9/11 attacks.\n\n");

   console_printf("FakeNES - A free, portable, Open Source NES emulator.\n");
   console_printf("\n");
   console_printf("Copyright (c) 2001-2007, FakeNES Team.\n");
   console_printf("\n");
   console_printf("This software is provided 'as-is', without any express or implied\n");
   console_printf("warranty.  In no event will the authors be held liable for any damages\n");
   console_printf("arising from the use of this software.\n");
   console_printf("\n");
   console_printf("Permission is granted to anyone to use this software for any purpose,\n");
   console_printf("including commercial applications, and to alter it and redistribute it\n");
   console_printf("freely, subject to the following restrictions:\n");
   console_printf("\n");
   console_printf("1. The origin of this software must not be misrepresented; you must not\n");
   console_printf("   claim that you wrote the original software. If you use this software\n");
   console_printf("   in a product, an acknowledgment in the product documentation would be\n");
   console_printf("   appreciated but is not required.\n");
   console_printf("2. Altered source versions must be plainly marked as such, and must not be\n");
   console_printf("   misrepresented as being the original software.\n");
   console_printf("3. This notice may not be removed or altered from any source distribution.\n");

   /* Initialize Allegro. */
   allegro_init();

   /* Initialize timer system. */
   install_timer();

   /* Set initial window title (this is changed by load_file()). */
   set_window_title(WINDOW_TITLE_NORMAL);

   /* ************************************** */
   /* ********** EMULATOR STARTUP ********** */
   /* ************************************** */

   /* Initialize platform-specific functionality. */
   result = platform_init();
   if(result != 0) {
      cleanup();
      return result;
   }

   /* Load the configuration. */
   load_config();

   /* Initialize the GUI. */
   gui_preinit();

   /* Initialize input. */
   if(input_init() != 0) {
      WARN("PANIC: Failed to initialize input interface");
      cleanup();
      return 1;
    }

   /* Initialize network and NetPlay. */
   net_init();
   netplay_init();

   /* Initialize audio. */
   if(audio_init () != 0) {
      WARN("Oops!  It looks like audio failed to initialize.\n"
           "Make sure another application isn't using it (a common problem).\n"
           "\n"
           "I'm disabling it for now.\n"
           "You can try to re-enable it from the Audio menu once inside the program.");

      audio_options.enable_output = FALSE;
      audio_init();
   }

#ifdef ALLEGRO_DOS
   /* Fade out the palette (DOS only). */
   fade_out(4);
#endif

   if(video_init() != 0) {
      WARN("Failed to initialize video interface");
      cleanup();
      return 1;
   }

   /* Finish GUI setup. This should always be the last step. */
   gui_init();

   /* Check if a filename was passed on the command-line. */
   if(argc >= 2) {
      const UDATA* error = NULL;

      /* Attempt to load file. */
      error = load_file(argv[1]);
      if(error) {
         WARN("Couldn't load the file");
         log_printf("Error:\n"
                    "\tUnable to load file '%s'\n",
                    "\tThe error message is: %s\n",
                    argv[1], error);

         cleanup();
         return 1;
      }

      /* Skip the GUI and head straight into emulation mode. */
      want_gui = FALSE;
   }

   /* *********************************** */
   /* ********** EMULATOR LOOP ********** */
   /* *********************************** */

   /* This is the main loop, it runs into want_exit becomes true. */
   while(!want_exit) {
      /* Check if we should enter the GUI (ESC was pressed). */
      if(want_gui) {
         BOOL result;

         /* Clear the request. */
         want_gui = FALSE;

         /* Check if the GUI is already running. */
         if(!gui_is_active) {
            /* Enter the GUI. This returns TRUE when the program should exit. */
            show_gui(first_run);
            if(first_run)
	       first_run = FALSE;

            /* Make sure the exit request is handled immediately. */
            if(want_exit)
               continue;
         }
      }

      /* Once we've reached this point, the GUI is not running. In this case, if a file is
         loaded, we want to run the emulation normally. However, if a file is not loaded, then
         it is time for the program to exit, as there is nothing else to do. */
      if(!file_is_loaded) {
         want_exit = TRUE;
         continue;
      }

      /* machine_main() handles all normal emulation, timing and input processing. */
      machine_main();
   }

   /* *************************************** */
   /* ********** EMULATOR SHUTDOWN ********** */
   /* *************************************** */

   /* Before exiting, all we have to do is call cleanup(), which handles everything for us. */
   cleanup();

   /* Now we can safely return ot the OS. */
   return 0;
}
END_OF_MAIN()

void main_load_config(void)
{
   first_run = get_config_int("main", "first_run", first_run);
}

void main_save_config(void)
{
   set_config_int("main", "first_run", first_run);
}

/* ---------------------------------------------------------------------- */

static void cleanup(void)
{
   /* Deinitialize in reverse order of initialization. */
#ifdef ALLEGRO_DOS
  fade_out(4);
#endif

   gui_exit();

   video_exit();
   audio_exit();
   netplay_exit();
   net_exit();
   input_exit();

   save_config();

   platform_exit();
}
