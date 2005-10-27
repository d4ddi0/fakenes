#include <allegro.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"

#ifdef USE_OPENAL

#include <al.h>
#include <alut.h>
#include "alstream.h"

/* OpenAL AUDIOSTREAM-like streaming routines. */

#define NUM_BUFFERS  2

ALSTREAM *play_al_stream (int len, int bits, int stereo, int freq)
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

void stop_al_stream (ALSTREAM *stream)
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

void *get_al_stream_buffer (ALSTREAM *stream)
{
   int processed;

   RT_ASSERT(stream);
   RT_ASSERT(stream->buffer);

   alGetSourcei (stream->source, AL_BUFFERS_PROCESSED, &processed);
   AL_CHECK();

   if (processed == 0)
      return (NULL);

   return (stream->buffer);
}

void free_al_stream_buffer (ALSTREAM *stream)
{
   int processed;
   ALuint buffer;

   RT_ASSERT(stream);
   RT_ASSERT(stream->buffer);

   alGetSourcei (stream->source, AL_BUFFERS_PROCESSED, &processed);
   AL_CHECK();

   if (processed == 0)
      return;

   while (processed-- > 1)
   {
      // Hrmm... what should the proper behavior here be?

      alSourceUnqueueBuffers (stream->source, 1, &buffer);
      AL_CHECK();

      alSourceQueueBuffers (stream->source, 1, &buffer);
      AL_CHECK();
   }

   alSourceUnqueueBuffers (stream->source, 1, &buffer);
   AL_CHECK();

   alBufferData (buffer, stream->alformat, stream->buffer,
      stream->len_bytes, stream->freq);
   AL_CHECK();

   alSourceQueueBuffers (stream->source, 1, &buffer);
   AL_CHECK();
}

#endif   /* USE_OPENAL */
