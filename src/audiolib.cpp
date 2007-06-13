/* FakeNES - A free, portable, Open Source NES emulator.

   audiolib.cpp: Implementation of the audio library.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <cstdlib>
#include <cstring>
#include "audio.h"
#include "audio_int.h"
#include "audiolib.h"
#include "common.h"
#include "debug.h"
#include "types.h"

#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alut.h>
#endif

static AudiolibDriver *audiolibDriver = null;

int audiolib_init(void)
{
   DEBUG_PRINTF("audiolib_init()\n");

   if(audiolibDriver) {
      WARN_GENERIC();
      audiolib_exit();
   }

   switch(audio_options.subsystem) {
      case AUDIO_SUBSYSTEM_ALLEGRO: {
         audiolibDriver = new AudiolibAllegroDriver;
         if(!audiolibDriver) {
            audiolib_exit();
            return 1;
         }

         const int result = audiolibDriver->initialize();
         if(result != 0) {
            audiolib_exit();
            return (8 + result);
         }

         break;
      }

#ifdef USE_OPENAL
      case AUDIO_SUBSYSTEM_OPENAL: {
         audiolibDriver = new AudiolibOpenALDriver;
         if(!audiolibDriver) {
            audiolib_exit();
            return 1;
         }

         const int result = audiolibDriver->initialize();
         if(result != 0) {
            audiolib_exit();
            return (8 + result);
         }

         break;
      }
#endif

      default: {
         WARN_GENERIC();
         return 2;
      }
   }

   return 0;
}

void audiolib_exit(void)
{
   DEBUG_PRINTF("audiolib_exit()\n");

   if(audiolibDriver) {
      // Deinitialize driver.
      audiolibDriver->deinitialize();
      // Delete driver object.
      delete audiolibDriver;
      audiolibDriver = null;
   }
}

int audiolib_open_stream(void)
{
   DEBUG_PRINTF("audiolib_open_stream()\n");

   if(!audiolibDriver) {
      WARN_GENERIC();
      return 1;
   }

   return audiolibDriver->openStream();
}

void audiolib_close_stream(void)
{
   DEBUG_PRINTF("audiolib_close_stream()\n");

   if(!audiolibDriver) {
      WARN_GENERIC();
      return;
   }

   audiolibDriver->closeStream();
}

void* audiolib_get_buffer(void)
{
   DEBUG_PRINTF("audiolib_get_buffer()\n");

   if(!audiolibDriver) {
      WARN_GENERIC();
      return null;
   }

   return audiolibDriver->getBuffer();
}

void audiolib_free_buffer(void)
{
   DEBUG_PRINTF("audiolib_free_buffer()\n");

   if(!audiolibDriver) {
      WARN_GENERIC();
      return;
   }

   audiolibDriver->freeBuffer();
}

void audiolib_suspend(void)
{
   DEBUG_PRINTF("audiolib_suspend()\n");

   if(!audiolibDriver) {
      WARN_GENERIC();
      return;
   }

   audiolibDriver->suspend();
}

void audiolib_resume(void)
{
   DEBUG_PRINTF("audiolib_open_resume()\n");

   if(!audiolibDriver) {
      WARN_GENERIC();
      return;
   }

   audiolibDriver->resume();
}

// --- Allegro driver. ---
int AudiolibAllegroDriver::initialize(void)
{
   // Initialize variables.
   stream = null;

   // Force interpolation.
   set_mixer_quality(2);
   // Maximize volume level.
   set_volume(255, -1);
   set_volume_per_voice(0);

   if(install_sound(DIGI_AUTODETECT, MIDI_NONE, null) != 0)
      return 1;
   // Sometimes the above call succeeds but we get a null driver.  How dumb. x.x
   if(digi_driver->id == DIGI_NONE)
      return 2;

   // Determine settings to use.
   if(audio_options.sample_rate_hint == -1)
       audio_sample_rate = get_mixer_frequency();
   else
       audio_sample_rate = audio_options.sample_rate_hint;

   audio_sample_bits = get_mixer_bits();

   // Allegro always uses unsignedsamples.
   audio_signed_samples = FALSE;

   if(audio_options.buffer_length_ms_hint == -1) {
      // Any buffer length should do, but ~67ms has served me well in the past.
      audio_buffer_length_ms = 67;
   }
   else
      audio_buffer_length_ms = audio_options.buffer_length_ms_hint;

   // Return success.
   return 0;
}

void AudiolibAllegroDriver::deinitialize(void)
{
   // stop and destroy the stream if it is playing.
   closeStream();

   // Remove Allegro sound driver so that it doesn't conflict.
   remove_sound();
}

int AudiolibAllegroDriver::openStream(void)
{
   if(stream) {
      WARN_GENERIC();
      closeStream();
   }

   // Create stream.
   stream = play_audio_stream(audio_buffer_size_frames,
      audio_sample_bits, (audio_channels == 2), audio_sample_rate, 255, 128);
   if(!stream)
      return 1;

   // Pause stream.
   suspend();

   // Return success.
   return 0;
}

void AudiolibAllegroDriver::closeStream(void)
{
   if(stream) {
      stop_audio_stream(stream);
      stream = null;
   }
}

void* AudiolibAllegroDriver::getBuffer(void)
{
   if(!stream) {
      WARN_GENERIC();
      return null;
   }

   return get_audio_stream_buffer(stream);
}

void AudiolibAllegroDriver::freeBuffer(void)
{
   if(!stream) {
      WARN_GENERIC();
      return;
   }

   free_audio_stream_buffer(stream);

   // Play the stream if we haven't already.
   resume();
}

void AudiolibAllegroDriver::suspend(void)
{
   if(!stream) {
      WARN_GENERIC();
      return;
   }

   voice_stop(stream->voice);
}

void AudiolibAllegroDriver::resume(void)
{
   if(!stream) {
      WARN_GENERIC();
      return;
   }

   voice_start(stream->voice);
}

#ifdef USE_OPENAL
// --- OpenAL driver. ---
int AudiolibOpenALDriver::initialize(void)
{
   return 0;
}

void AudiolibOpenALDriver::deinitialize(void)
{
   closeStream();
}

int AudiolibOpenALDriver::openStream(void)
{
}

void AudiolibOpenALDriver::closeStream(void)
{
}

void* AudiolibOpenALDriver::getBuffer(void)
{
}

void AudiolibOpenALDriver::freeBuffer(void)
{
}

void AudiolibOpenALDriver::suspend(void)
{
}

void AudiolibOpenALDriver::resume(void)
{
}
#endif //USE_OPENAL