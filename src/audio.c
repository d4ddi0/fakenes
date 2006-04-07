/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   audio.c: Implementation of the audio interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "debug.h"
#include "gui.h"
#include "timing.h"
#include "types.h"

/* Once audio_init() has been called, please do not change the values of
   any of these variables without first calling audio_exit(), otherwise a
   series of memory leaks could occur. */

/* Parameters. */
BOOL audio_enable_output    = TRUE;
ENUM audio_subsystem        = AUDIO_SUBSYSTEM_ALLEGRO;
int  audio_sample_rate      = 48000;
int  audio_sample_size      = 16;
BOOL audio_unsigned_samples = TRUE;
int  audio_buffer_length    = 6;

/* Buffer sizes. */
int audio_buffer_size_samples = 0;
int audio_buffer_size_bytes = 0;
int audio_buffer_frame_size_samples = 0;
int audio_buffer_frame_size_bytes = 0;

/* Samples. */
static SAMPLE *audio_sample     = NULL;   /* Primary sample. */
static SAMPLE *audio_sample_alt = NULL;   /* Alternate sample. */
static SAMPLE *open_sample      = NULL;   /* Pointer to read-only sample. */
static SAMPLE *closed_sample    = NULL;   /* Pointer to write-only sample. */

/* Voices. */
static int audio_voice     = 0;  /* Voice for primary sample. */
static int audio_voice_alt = 0;  /* Voice for alternate sample. */
static int active_voice    = 0;  /* Active voice. */

/* Miscellaneous. */
volatile int audio_fps = 0;

int audio_init (void)
{
   /* Load configuration. */

   audio_enable_output = get_config_int ("audio", "enable_output", audio_enable_output);
   audio_subsystem     = get_config_int ("audio", "subsystem",     audio_subsystem);
   audio_sample_rate   = get_config_int ("audio", "sample_rate",   audio_sample_rate);
   audio_sample_size   = get_config_int ("audio", "sample_size",   audio_sample_size);
   audio_buffer_length = get_config_int ("audio", "buffer_length", audio_buffer_length);

   if (!audio_enable_output)
      return (0);

   /* Initialize subsystem. */

   switch (audio_subsystem)
   {
      case AUDIO_SUBSYSTEM_ALLEGRO:
      {
         /* Disable interpolation, since we have no use for it. */
         set_mixer_quality (1);
      
         set_volume_per_voice (0);
              
         if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
            return (1);

         if (digi_driver->id == DIGI_NONE)
            return (2);
      
         audio_unsigned_samples = TRUE;

         break;
      }

      default:
         WARN_GENERIC();
   }


   /* Calculate buffer sizes. */

   /* Individual frames. */

   audio_buffer_frame_size_samples = (audio_sample_rate /
      timing_get_speed ());

   audio_buffer_frame_size_bytes = audio_buffer_frame_size_samples;
   if (audio_sample_size == 16)
      audio_buffer_frame_size_bytes *= 2;
   if (apu_stereo_mode)
      audio_buffer_frame_size_bytes *= 2;

   /* Entire buffer. */

   audio_buffer_size_samples = (audio_buffer_frame_size_samples *
      audio_buffer_length);
                 
   audio_buffer_size_bytes = audio_buffer_size_samples;
   if (audio_sample_size == 16)
      audio_buffer_size_bytes *= 2;
   if (apu_stereo_mode)
      audio_buffer_size_bytes *= 2;

   /* Create samples. */

   audio_sample = create_sample (audio_sample_size, apu_stereo_mode,
      audio_sample_rate, audio_buffer_size_samples);
   if (!audio_sample)
      return (3);

   audio_sample_alt = create_sample (audio_sample_size, apu_stereo_mode,
      audio_sample_rate, audio_buffer_size_samples);
   if (!audio_sample_alt)
   {
      destroy_sample (audio_sample);
      return (4);
   }

   /* Set initial sample mapping. */
   open_sample = audio_sample;
   closed_sample = audio_sample_alt;

   /* Create voices. */

   audio_voice     = allocate_voice (audio_sample);
   audio_voice_alt = allocate_voice (audio_sample_alt);

   active_voice = audio_voice_alt;

   return (0);
}

void audio_exit (void)
{
   if (audio_enable_output)
   {
      /* Destroy samples. */

      if (audio_sample)
         destroy_sample (audio_sample);
      if (audio_sample_alt)
         destroy_sample (audio_sample_alt);

      /* Destroy voices. */

      if (voice_check (audio_voice))
         deallocate_voice (audio_voice);
      if (voice_check (audio_voice_alt))
         deallocate_voice (audio_voice_alt);

      /* Remove sound driver. */

      remove_sound ();
   }

   /* Save configuration. */

   set_config_int ("audio", "enable_output", audio_enable_output);
   set_config_int ("audio", "subsystem",     audio_subsystem);
   set_config_int ("audio", "sample_rate",   audio_sample_rate);
   set_config_int ("audio", "sample_size",   audio_sample_size);
   set_config_int ("audio", "buffer_length", audio_buffer_length);
}

void audio_suspend (void)
{
   if (!audio_enable_output)
      return;

   if (!voice_check (active_voice))
      return;

   voice_stop (active_voice);
}

void audio_resume (void)
{
   if (!audio_enable_output)
      return;

   if (!voice_check (active_voice))
      return;

   voice_start (active_voice);
}

void *audio_get_buffer (void)
{
   if (!audio_enable_output)
      return (NULL);

   if (!open_sample)
      return (NULL);

   return (open_sample->data);
}

void audio_play (void)
{
   SAMPLE *old_open_sample;

   if (!audio_enable_output)
      return;

   /* Swap samples. */

   old_open_sample = open_sample;
   open_sample = closed_sample;
   closed_sample = old_open_sample;

   /* Assign voice. */

   if (closed_sample == audio_sample)
      active_voice = audio_voice;
   else
      active_voice = audio_voice_alt;

   /* Start voice. */
   voice_start (active_voice);

   audio_fps += audio_buffer_length;
}
