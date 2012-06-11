/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "Audio.h"
#include "AudioLib.hpp"
#include "AudioLibAtoms.hpp"
#include "Internals.h"
#include "Local.hpp"

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

#ifndef USE_OPENAL
      case AUDIO_SUBSYSTEM_AUTOMATIC:
#endif
      case AUDIO_SUBSYSTEM_SAFE:
      case AUDIO_SUBSYSTEM_ALLEGRO: {
         audiolibDriver = new AudiolibAllegroDriver;
         if(!audiolibDriver) {
            log_printf("AUDIOLIB: audiolib_init(): Creating of audio driver failed for AUDIO_SUBSYSTEM_ALLEGRO.");
            audiolib_exit();
            return 1;
         }

         const int result = audiolibDriver->initialize();
         if(result != 0) {
            log_printf("AUDIOLIB: audiolib_init(): Initialization of audio driver failed for AUDIO_SUBSYSTEM_ALLEGRO.");
            log_printf("AUDIOLIB: audiolib_init(): audiolibDriver->initialize() error code %d.", result);
            audiolib_exit();
            return 8 + result;
         }

         break;
      }

#if defined(USE_OPENAL)
      case AUDIO_SUBSYSTEM_AUTOMATIC:	// Prefer OpenAL when auto
      case AUDIO_SUBSYSTEM_OPENAL: {
         audiolibDriver = new AudiolibOpenALDriver;
         if(!audiolibDriver) {
            log_printf("AUDIOLIB: audiolib_init(): Creating of audio driver failed for AUDIO_SUBSYSTEM_OPENAL.");
            audiolib_exit();
            return 1;
         }

         const int result = audiolibDriver->initialize();
         if(result != 0) {
            log_printf("AUDIOLIB: audiolib_init(): Initialization of audio driver failed for AUDIO_SUBSYSTEM_OPENAL.");
            log_printf("AUDIOLIB: audiolib_init(): audiolibDriver->initialize() error code %d.", result);
            audiolib_exit();

            // Fall back to safe-mode
            log_printf("AUDIOLIB: audiolib_init(): Falling back to safe-mode driver.");
            audio_options.subsystem = AUDIO_SUBSYSTEM_SAFE;
            return audiolib_init();
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

void* audiolib_get_buffer(void* buffer)
{
   DEBUG_PRINTF("audiolib_get_buffer()\n");

   RT_ASSERT(buffer);

   if(!audiolibDriver) {
      WARN_GENERIC();
      return null;
   }

   return audiolibDriver->getBuffer(buffer);
}

void audiolib_free_buffer(void* buffer)
{
   DEBUG_PRINTF("audiolib_free_buffer()\n");

   RT_ASSERT(buffer);

   if(!audiolibDriver) {
      WARN_GENERIC();
      return;
   }

   audiolibDriver->freeBuffer(buffer);
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
   DEBUG_PRINTF("audiolib_resume()\n");

   if(!audiolibDriver) {
      WARN_GENERIC();
      return;
   }

   audiolibDriver->resume();
}

// --- Allegro driver. ---
AudiolibAllegroDriver::AudiolibAllegroDriver(void)
{
   // Initialize variables.
   stream = null;
}

int AudiolibAllegroDriver::initialize(void)
{
   // Force interpolation.
   set_mixer_quality(2);
   // Maximize volume level.
   set_volume(255, -1);
   set_volume_per_voice(0);

   const int result = install_sound(DIGI_AUTODETECT, MIDI_NONE, null);
   if(result != 0) {
      log_printf("AUDIOLIB: AudiolibAllegroDriver::initialize(): Installation of sound driver failed.");
      log_printf("AUDIOLIB: AudiolibAllegroDriver::initialize(): install_sound() error code %d.", result);
      log_printf("AUDIOLIB: AudiolibAllegroDriver::initialize(): Allegro says: %s.", allegro_error);
      return 1;
   }
   // Sometimes the above call succeeds but we get a null driver.  How dumb. x.x
   if(digi_driver->id == DIGI_NONE) {
      log_printf("AUDIOLIB: AudiolibAllegroDriver::initialize(): Installation of sound driver failed.");
      log_printf("AUDIOLIB: AudiolibAllegroDriver::initialize(): DIGI_NONE is not supported.");
      return 2;
   }

   // Determine settings to use.
   if(audio_options.sample_rate_hint == -1)
       audio_sample_rate = get_mixer_frequency();
   else
       audio_sample_rate = audio_options.sample_rate_hint;

   audio_sample_bits = get_mixer_bits();

   // Allegro always uses unsigned samples.
   audio_signed_samples = FALSE;

   if(audio_options.buffer_length_ms_hint == -1) {
      // 75ms should do.
      audio_buffer_length_ms = 75;
   }
   else
      audio_buffer_length_ms = audio_options.buffer_length_ms_hint;

   log_printf("\n"
              "AUDIOLIB: AudiolibAllegroDriver::initialize(): Configuration:\n"
              "AUDIOLIB: AudiolibAllegroDriver::initialize():    Channels: %s\n"
              "AUDIOLIB: AudiolibAllegroDriver::initialize():    Sample rate: %d Hz (%s)\n"
              "AUDIOLIB: AudiolibAllegroDriver::initialize():    Sample format: %s %d-bit\n"
              "AUDIOLIB: AudiolibAllegroDriver::initialize():    Buffer length: %dms (%s)\n"
              "\n",
              (audio_channels == 2) ? "Stereo" : "Mono",
              audio_sample_rate,
              (audio_options.sample_rate_hint == -1) ? "Autodetect" : "Forced",
              audio_signed_samples ? "Signed" : "Unsigned",
              audio_sample_bits,
              audio_buffer_length_ms,
              (audio_options.buffer_length_ms_hint == -1) ? "Autodetect" : "Forced");

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
      audio_sample_bits, audio_channels == 2, audio_sample_rate, 255, 128);
   if(!stream) {
      log_printf("AUDIOLIB: AudiolibAllegroDriver::openStream(): Creation of audio stream failed.");
      log_printf("AUDIOLIB: AudiolibAllegroDriver::openStream(): Allegro says: %s.", allegro_error);
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

void* AudiolibAllegroDriver::getBuffer(void* buffer)
{
   RT_ASSERT(buffer);

   if(!stream) {
      WARN_GENERIC();
      return null;
   }

   return get_audio_stream_buffer(stream);
}

void AudiolibAllegroDriver::freeBuffer(void* buffer)
{
   RT_ASSERT(buffer);

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

#if defined(USE_OPENAL)
// --- OpenAL driver. ---
#define AUDIOLIB_OPENAL_BUFFERS	2

AudiolibOpenALDriver::AudiolibOpenALDriver(void)
{
   // Initialize variables.
   device = null;
   context = null;

   buffers = null;
   source = AL_INVALID;

   floatingBuffer = AL_INVALID;
}

int AudiolibOpenALDriver::initialize(void)
{
   // Open the default device.
   device = alcOpenDevice(null);
   if(!device) {
      log_printf("AUDIOLIB: AudiolibOpenALDriver::initialize(): Fatal: Couldn't open default device.");
      deinitialize();
      return 1;
   }

   // Create a context for the device using the default attributes.
   alcGetError(device);
   context = alcCreateContext(device, null);
   if(!context) {
      const ALCenum error = alcGetError(device);
      log_printf("AUDIOLIB: AudiolibOpenALDriver::initialize(): Creation of device context failed.");
      log_printf("AUDIOLIB: AudiolibOpenALDriver::initialize(): alcCreateContext() error code %d.", error);
      log_printf("AUDIOLIB: AudiolibOpenALDriver::initialize(): OpenAL says: %s.", getErrorStringALC(error));
      deinitialize();
      return 2;
   }

   alcMakeContextCurrent(context);

   // Determine settings to use.
   if(audio_options.sample_rate_hint == -1)
       audio_sample_rate = 44100;
   else
       audio_sample_rate = audio_options.sample_rate_hint;

   audio_sample_bits = 16;
   audio_signed_samples = TRUE;

   if(audio_options.buffer_length_ms_hint == -1) {
      // 75ms should do.
      audio_buffer_length_ms = 75;
   }
   else
      audio_buffer_length_ms = audio_options.buffer_length_ms_hint;

   if(audio_channels == 2) 
      format = AL_FORMAT_STEREO16;
   else
      format = AL_FORMAT_MONO16;

   log_printf("\n"
              "AUDIOLIB: AudiolibOpenALDriver::initialize(): Configuration:\n"
              "AUDIOLIB: AudiolibOpenALDriver::initialize():    Channels: %s\n"
              "AUDIOLIB: AudiolibOpenALDriver::initialize():    Sample rate: %d Hz (%s)\n"
              "AUDIOLIB: AudiolibOpenALDriver::initialize():    Sample format: %s %d-bit\n"
              "AUDIOLIB: AudiolibOpenALDriver::initialize():    Buffer length: %dms (%s)\n"
              "\n",
              (audio_channels == 2) ? "Stereo" : "Mono",
              audio_sample_rate,
              (audio_options.sample_rate_hint == -1) ? "Autodetect" : "Forced",
              audio_signed_samples ? "Signed" : "Unsigned",
              audio_sample_bits,
              audio_buffer_length_ms,
              (audio_options.buffer_length_ms_hint == -1) ? "Autodetect" : "Forced");

   // Return success.
   return 0;
}

void AudiolibOpenALDriver::deinitialize(void)
{
   // Stop and destroy the stream if it is playing.
   closeStream();

   if(context) {
      // Destroy the device context.
      alcDestroyContext(context);
      context = null;
   }

   if(device) {
      // Close the device.
      alcCloseDevice(device);
      device = null;
   }
}

int AudiolibOpenALDriver::openStream(void)
{
   buffers = new ALuint[AUDIOLIB_OPENAL_BUFFERS];
   if(!buffers) {
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): Allocation of buffer space failed.");
      closeStream();
      return 1;
   }

   alGetError();
   alGenBuffers(AUDIOLIB_OPENAL_BUFFERS, buffers);
   ALenum error = alGetError();
   if(error != AL_NO_ERROR) {
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): Generation of buffers failed.");
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): alGenBuffers() error code %d.", error);
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): OpenAL says: %s.", getErrorStringAL(error));
      closeStream();
      return 2;
   }

   alGetError();
   alGenSources(1, &source);
   error = alGetError();
   if(error != AL_NO_ERROR) {
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): Generation of source failed.");
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): alGenSources() error code %d.", error);
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): OpenAL says: %s.", getErrorStringAL(error));
      closeStream();
      return 3;
   }

   void* tempBuffer = malloc(audio_buffer_size_bytes);
   if (!tempBuffer) {
      log_printf("AUDIOLIB: AudiolibOpenALDriver::openStream(): Allocation of temporary buffer failed.");
      free(tempBuffer);
      closeStream();
      return 4;
   }

   memset(tempBuffer, 0, audio_buffer_size_bytes);

   for(int index = 0; index < AUDIOLIB_OPENAL_BUFFERS; index++) {
      ALuint buffer = buffers[index];
      alBufferData(buffer, format, tempBuffer, audio_buffer_size_bytes, audio_sample_rate);
   }

   free(tempBuffer);

   alSourceQueueBuffers(source, AUDIOLIB_OPENAL_BUFFERS, buffers);

   alSourcePlay(source);
   
   return 0;
}

void AudiolibOpenALDriver::closeStream(void)
{
   if(source != AL_INVALID) {
      alSourceStop(source);
      alDeleteSources(1, &source);
      source = AL_INVALID;
   }

   if(buffers) {
      if(buffers[0] != AL_INVALID)
         alDeleteBuffers(AUDIOLIB_OPENAL_BUFFERS, buffers);

      delete[] buffers;
      buffers = null;
   }
}

void* AudiolibOpenALDriver::getBuffer(void* buffer)
{
   RT_ASSERT(buffer);

   if(source == AL_INVALID) {
      WARN_GENERIC();
      return null;
   }

   ALint processed;
   alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
   if(processed == 0)
      return null;

   alSourceUnqueueBuffers(source, 1, &floatingBuffer);
   if(floatingBuffer == AL_INVALID) {
      WARN_GENERIC();
      return null;
   }

   return buffer;
}

void AudiolibOpenALDriver::freeBuffer(void* buffer)
{
   RT_ASSERT(buffer);

   if(source == AL_INVALID) {
      WARN_GENERIC();
      return;
   }

   if(floatingBuffer == AL_INVALID) {
      WARN_GENERIC();
      return;
   }

   alBufferData(floatingBuffer, format, buffer, audio_buffer_size_bytes, audio_sample_rate);
   alSourceQueueBuffers(source, 1, &floatingBuffer);

   floatingBuffer = AL_INVALID;

   ALint state;
   alGetSourcei(source, AL_SOURCE_STATE, &state);
   if(state == AL_STOPPED)
      alSourcePlay(source);
}

void AudiolibOpenALDriver::suspend(void)
{
   if(source == AL_INVALID) {
      WARN_GENERIC();
      return;
   }

   alSourceStop(source);
}

void AudiolibOpenALDriver::resume(void)
{
   if(source == AL_INVALID) {
      WARN_GENERIC();
      return;
   }

   alSourcePlay(source);
}

const UDATA* AudiolibOpenALDriver::getErrorStringAL(ALenum error)
{
   switch(error) {
      case AL_INVALID_ENUM:
         return "Invalid enum";
      case AL_INVALID_NAME:
         return "Invalid name";
      case AL_INVALID_OPERATION:
         return "Invalid operation";
      case AL_INVALID_VALUE:
         return "Invalid value";
      case AL_OUT_OF_MEMORY:
         return "Out of memory";
      default:
         return "Unknown error";
   }
}

const UDATA* AudiolibOpenALDriver::getErrorStringALC(ALCenum error)
{
   switch(error) {
      case ALC_INVALID_CONTEXT:
         return "Invalid context";
      case ALC_INVALID_DEVICE:
         return "Invalid device";
      case ALC_INVALID_ENUM:
         return "Invalid enum";
      case ALC_INVALID_VALUE:
         return "Invalid value";

      default:
         return "Unknown error";
   }
}
#endif //USE_OPENAL
