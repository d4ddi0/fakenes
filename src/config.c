/* FakeNES - A free, portable, Open Source NES emulator.

   config.c: Wrapper around configuration handlers.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include "apu.h"
#include "audio.h"
#include "common.h"
#include "config.h"
#include "gui.h"
#include "input.h"
#include "main.h"
#include "rewind.h"
#include "version.h"
#include "video.h"

static void actually_load_config(void)
{
   apu_load_config();
   audio_load_config();
   gui_load_config();
   input_load_config();
   main_load_config();
   rewind_load_config();
   video_load_config();
}

void load_config(void)
{
   const char* version = get_config_string("meta", "version", "default");
   if(strcmp(version, VERSION_STRING) != 0) {
      /* Version mismatch, or version string missing - start with a fresh config file. */
      push_config_state();

      /* Just set to a nonexistant file so that defaults are always loaded. */
      /* An empty string is good for this purpose because unlike null, it's a valid string, but it is also an invalid
        filename and thus cannot exist on any system. */
      set_config_file("");
      actually_load_config();

      pop_config_state ();

      /* Write the newly loaded defaults out to disk. */
      save_config();

      /* Skip the rest. */
      return;
   }

   actually_load_config();
}

void save_config(void)
{
   set_config_string("meta", "version", VERSION_STRING);

   apu_save_config();
   audio_save_config();
   gui_save_config();
   input_save_config();
   main_save_config();
   rewind_save_config();
   video_save_config();

   flush_config_file();
}

// Helpers for saving and loading boolean variables in a standard way
BOOL get_config_bool(const char* section, const char* name, BOOL default_value)
{
   RT_ASSERT(section);
   RT_ASSERT(name);

   return BOOLEAN(get_config_int(section, name, BINARY(default_value)));
}

void set_config_bool(const char* section, const char* name, BOOL value)
{
   RT_ASSERT(section);
   RT_ASSERT(name);

   set_config_int(section, name, BINARY(value));
}
