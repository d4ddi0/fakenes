/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   alstream.h: Declarationsf or OpenAL AUDIOSTREAMs.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifdef USE_OPENAL

#ifndef ALSTREAM_H_INCLUDED
#define ALSTREAM_H_INCLUDED
#include <al.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALSTREAM
{
   ALuint source;       // The source we are playing on
   int len;             // The length of the sample buffer (in samples)
   int len_bytes;       // The length of the sample buffer (in bytes)
   int bits;            // The bits per sample
   int stereo;          // Mono or stereo
   int freq;            // The sampling frequency, in Hz
   void *buffer;        // The sample buffer
   ALuint alformat;     // Format of sample buffer according to OpenAL
   ALuint *albuffers;   // OpenAL buffers for this stream

} ALSTREAM;

ALSTREAM *play_al_stream (int, int, int, int);
void stop_al_stream (ALSTREAM *);
void *get_al_stream_buffer (ALSTREAM *);
void free_al_stream_buffer (ALSTREAM *);

#define AL_CHECK() { \
   int error;  \
   if ((error = alGetError ()) != AL_NO_ERROR) {   \
      allegro_message ("OpenAL error #%d at line %d of %s", error,   \
         __LINE__, __FILE__); \
      exit (-1);  \
   }  \
}

#ifdef __cplusplus
}
#endif
#endif

#endif   /* USE_OPENAL */
