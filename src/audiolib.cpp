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

// TODO: Beef up Allegro support.
// TODO: Beef up OpenAL support.  Error handling is a must.

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

   // Allegro always uses unsigned samples.
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
   // Stop and destroy the stream if it is playing.
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
   if(!stream) {
      closeStream();
      return 1;
   }

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
#define AUDIOLIB_OPENAL_BUFFERS	2

int AudiolibOpenALDriver::initialize(void)
{
   // Initialize variables.
   copyBuffer = null;
   buffers = null;

   // Initialize ALUT.
   alutInit(&saved_argc, saved_argv);

   // Determine settings to use.
   if(audio_options.sample_rate_hint == -1)
       audio_sample_rate = 44100;
   else
       audio_sample_rate = audio_options.sample_rate_hint;

   audio_sample_bits = 16;
   audio_signed_samples = TRUE;

   if(audio_options.buffer_length_ms_hint == -1) {
      // OpenAL requires a notably higher buffer size than Allegro - but ~83ms should do.
      audio_buffer_length_ms = 83;
   }
   else
      audio_buffer_length_ms = audio_options.buffer_length_ms_hint;

   if(audio_channels == 2) 
      format = AL_FORMAT_STEREO16;
   else
      format = AL_FORMAT_MONO16;

   // Return success.
   return 0;
}

void AudiolibOpenALDriver::deinitialize(void)
{
   // Stop and destroy the stream if it is playing.
   closeStream();

   // Deinitialize ALUT.
   alutExit();
}

int AudiolibOpenALDriver::openStream(void)
{
   copyBuffer = malloc(audio_buffer_size_bytes);
   if (!copyBuffer) {
      closeStream();
      return 1;
   }

   memset(copyBuffer, 0, audio_buffer_size_bytes);

   buffers = (ALuint*)malloc((sizeof(ALuint) * AUDIOLIB_OPENAL_BUFFERS));
   if(!buffers) {
      closeStream();
      return 2;
   }

   alGenBuffers(AUDIOLIB_OPENAL_BUFFERS, buffers);
   alGenSources(1, &source);

   for(int index = 0; index < AUDIOLIB_OPENAL_BUFFERS; index++) {
      ALuint buffer = buffers[index];
      alBufferData(buffer, format, copyBuffer, audio_buffer_size_bytes, audio_sample_rate);
   }

   alSourceQueueBuffers(source, AUDIOLIB_OPENAL_BUFFERS, buffers);

   return 0;
}

void AudiolibOpenALDriver::closeStream(void)
{
   alSourceStop(source);
   alDeleteSources(1, &source);
   alDeleteBuffers(AUDIOLIB_OPENAL_BUFFERS, buffers);

   if(copyBuffer) {
      free(copyBuffer);
      copyBuffer = null;
   }

   if(buffers) {
      free(buffers);
      buffers = null;
   }
}

void* AudiolibOpenALDriver::getBuffer(void)
{
   ALint processed;
   alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
   if(processed == 0)
      return null;

   alSourceUnqueueBuffers(source, 1, &floatingBuffer);

   return copyBuffer;
}

void AudiolibOpenALDriver::freeBuffer(void)
{
   alBufferData(floatingBuffer, format, copyBuffer, audio_buffer_size_bytes, audio_sample_rate);
   alSourceQueueBuffers(source, 1, &floatingBuffer);
   
   ALint state;
   alGetSourcei(source, AL_SOURCE_STATE, &state);
   if(state == AL_STOPPED)
      resume();
}

void AudiolibOpenALDriver::suspend(void)
{
   alSourceStop(source);
}

void AudiolibOpenALDriver::resume(void)
{
   alSourcePlay(source);
}
#endif //USE_OPENAL