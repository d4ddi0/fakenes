/* FakeNES - A free, portable, Open Source NES emulator.

   config.c: Wrapper around configuration handlers.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include "apu.h"
#include "audio.h"
#include "common.h"
#include "config.h"
#include "dsp.h"
#include "gui.h"
#include "input.h"
#include "main.h"
#include "rewind.h"
#include "video.h"

void load_config (void)
{
   apu_load_config ();
   audio_load_config ();
   dsp_load_config ();
   gui_load_config ();
   input_load_config ();
   main_load_config ();
   rewind_load_config ();
   video_load_config ();
}

void save_config (void)
{
   apu_save_config ();
   audio_save_config ();
   dsp_save_config ();
   gui_save_config ();
   input_save_config ();
   main_save_config ();
   rewind_save_config ();
   video_save_config ();
}