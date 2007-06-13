/* FakeNES - A free, portable, Open Source NES emulator.

   audiolib.cpp: Implementation of the audio library.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <cstdlib>
#include <cstring>
#include "audio.h"
#include "audiolib.h"
#include "common.h"
#include "debug.h"
#include "types.h"

#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alut.h>
#endif

// TODO: Re-add OpenAL support.
// TODO: More graceful handling of OpenAL failures, rather than just printing the error and exiting.

class AudiolibDriver {
public:
   AudiolibDriver(void);
   virtual ~AudiolibDriver(void);

   virtual int initialize(void);
   virtual void deinitialize(void);
   virtual int openStream(void);
   virtual void closeStream(void);
   virtual void* getBuffer(void);
   virtual void freeBuffer(void);
   virtual void suspend(void);
   virtual void resume(void);
};

class AudiolibAllegroDriver : public AudiolibDriver {
public:
   int initialize(void);
   void deinitialize(void);
   int openStream(void);
   void closeStream(void);
   void* getBuffer(void);
   void freeBuffer(void);
   void suspend(void);
   void resume(void);

private:
   AUDIOSTREAM* stream;
};

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

#if 0
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

// --- Null driver. ---
AudiolibDriver::AudiolibDriver(void)
{
}

AudiolibDriver::~AudiolibDriver(void)
{
}

int AudiolibDriver::initialize(void)
{
   return 1;
}

void AudiolibDriver::deinitialize(void)
{
}

int AudiolibDriver::openStream(void)
{
   return 1;
}

void AudiolibDriver::closeStream(void)
{
}

void* AudiolibDriver::getBuffer(void)
{
   return null;
}

void AudiolibDriver::freeBuffer(void)
{
}

void AudiolibDriver::suspend(void)
{
}

void AudiolibDriver::resume(void)
{
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

#if 0
#ifdef USE_OPENAL
// --- OpenAL driver. ---
// OpenAL AUDIOSTREAM-like streaming routines.

typedef struct ALSTREAM {
   ALuint source;     // The source we are playing on
   int len;           // The length of the sample buffer (in samples)
   int len_bytes;     // The length of the sample buffer (in bytes)
   int bits;          // The bits per sample
   int stereo;        // Mono or stereo
   int freq;          // The sampling frequency, in Hz
   void* buffer;      // The sample buffer
   ALuint alformat;   // Format of sample buffer according to OpenAL
   ALuint* albuffers; // OpenAL buffers for this stream

} ALSTREAM;

static ALSTREAM *play_al_stream (int, int, int, int);
static void stop_al_stream (ALSTREAM *);
static void *get_al_stream_buffer (ALSTREAM *);
static void free_al_stream_buffer (ALSTREAM *);

#define AL_CHECK() { \
   int error;  \
   if ((error = alGetError ()) != AL_NO_ERROR) {   \
      allegro_message ("OpenAL error #%d at line %d of %s", error,   \
         __LINE__, __FILE__); \
      exit (-1);  \
   }  \
}

#define NUM_BUFFERS  2

static ALSTREAM *play_al_stream (int len, int bits, int stereo, int freq)
{
   ALSTREAM *stream;
   int index;

   stream = malloc (sizeof (ALSTREAM));
   if (!stream)
      return (NULL);

   stream->len = len;
   stream->bits = bits;
   stream->stereo = stereo;
   stream->freq = freq;

   stream->len_bytes = len;
   if (bits == 16)
      stream->len_bytes *= 2;
   if (stereo)
      stream->len_bytes *= 2;

   stream->buffer = malloc (stream->len_bytes);
   if (!stream->buffer)
   {
      free (stream);
      return (NULL);
   }

   memset (stream->buffer, 0, stream->len_bytes);

   stream->albuffers = malloc ((sizeof (ALuint) * NUM_BUFFERS));
   if (!stream->albuffers)
   {
      free (stream->buffer);
      free (stream);
      return (NULL);
   }

   if (stereo)
   {
      if (bits == 16)
         stream->alformat = AL_FORMAT_STEREO16;
      else
         stream->alformat = AL_FORMAT_STEREO8;
   }
   else
   {
      if (bits == 16)
         stream->alformat = AL_FORMAT_MONO16;
      else
         stream->alformat = AL_FORMAT_MONO8;
   }

   alGenBuffers (NUM_BUFFERS, stream->albuffers);
   AL_CHECK();

   alGenSources (1, &stream->source);
   AL_CHECK();

   for (index = 0; index < NUM_BUFFERS; index++)
   {
      int buffer = stream->albuffers[index];

      alBufferData (buffer, stream->alformat, stream->buffer,
         stream->len_bytes, stream->freq);
      AL_CHECK();
   }

   alSourceQueueBuffers (stream->source, NUM_BUFFERS, stream->albuffers);
   AL_CHECK();

   alSourcePlay (stream->source);
   AL_CHECK();

   return (stream);
}

static void stop_al_stream (ALSTREAM *stream)
{
   RT_ASSERT(stream);
   RT_ASSERT(stream->albuffers);
   RT_ASSERT(stream->buffer);

   alSourceStop (stream->source);
   AL_CHECK();

   alDeleteSources (1, &stream->source);
   AL_CHECK();

   alDeleteBuffers (NUM_BUFFERS, stream->albuffers);
   AL_CHECK();

   free (stream->albuffers);
   free (stream->buffer);
   free (stream);
}

static ALuint floating_buffer = 0;

static void *get_al_stream_buffer (ALSTREAM *stream)
{
   int processed;

   RT_ASSERT(stream);
   RT_ASSERT(stream->buffer);

   alGetSourcei (stream->source, AL_BUFFERS_PROCESSED, &processed);
   AL_CHECK();

   if (processed == 0)
      return (NULL);

   alSourceUnqueueBuffers (stream->source, 1, &floating_buffer);
   AL_CHECK();

   return (stream->buffer);
}

static void free_al_stream_buffer (ALSTREAM *stream)
{
   int state;

   RT_ASSERT(stream);
   RT_ASSERT(stream->buffer);

   alBufferData (floating_buffer, stream->alformat, stream->buffer,
      stream->len_bytes, stream->freq);
   AL_CHECK();

   alSourceQueueBuffers (stream->source, 1, &floating_buffer);
   AL_CHECK();

   alGetSourcei (stream->source, AL_SOURCE_STATE, &state);
   AL_CHECK();

   if (state == AL_STOPPED)
   {
      alSourcePlay (stream->source);
      AL_CHECK();
   }
}

static ALSTREAM *audiolib_openal_stream = NULL;

static int audiolib_openal_init (void)
{
   static BOOL initialized = FALSE;

   if (!initialized)
   {
      /* Hack for freealut not liking being initialized/deinitialized more
         than once on Linux and possibly other Unices. */

      alutInit (&saved_argc, saved_argv);
      AL_CHECK();

      /* Install exit handler. */
      atexit (alutExit);

      /* Make sure we don't get initialized again. */
      initialized = TRUE;
   }

   /* Autodetect settings. */

   if (audio_sample_rate == -1)
       audio_sample_rate = 44100;
   if (audio_sample_size == -1)
       audio_sample_size = 16;

   if (audio_sample_size == 8)
      audio_unsigned_samples = TRUE;
   else
      audio_unsigned_samples = FALSE;

   /* Return success. */
   return (0);
}

static void audiolib_openal_deinit (void)
{
   stop_al_stream (audiolib_openal_stream);

   /* Due to a bug in freealut, we don't call it's exit routine until we're
      actually exiting the program (via atexit()). */
   /* alutExit (); */
}

static int audiolib_openal_play (void)
{
   /* Create stream. */

   if (!(audiolib_openal_stream = play_al_stream (audio_buffer_size_samples,
      audio_sample_size, AUDIO_STEREO, audio_sample_rate)))
   {
      WARN_GENERIC();
      return (1);
   }

   /* Return success. */
   return (0);
}

static void audiolib_openal_stop (void)
{
   stop_al_stream (audiolib_openal_stream);
   audiolib_openal_stream = NULL;
}

static void *audiolib_openal_get_buffer (void)
{
   return (get_al_stream_buffer (audiolib_openal_stream));
}

static void audiolib_openal_free_buffer (void)
{
   free_al_stream_buffer (audiolib_openal_stream);
}

static void audiolib_openal_suspend (void)
{
   alSourceStop (audiolib_openal_stream->source);
   AL_CHECK ();
}

static void audiolib_openal_resume (void)
{
   alSourcePlay (audiolib_openal_stream->source);
   AL_CHECK ();
}

static AUDIOLIB_DRIVER audiolib_openal_driver =
{
   audiolib_openal_init,
   audiolib_openal_deinit,
   audiolib_openal_play,
   audiolib_openal_stop,
   audiolib_openal_get_buffer,
   audiolib_openal_free_buffer,
   audiolib_openal_suspend,
   audiolib_openal_resume
};

#endif   /* USE_OPENAL */
#endif
