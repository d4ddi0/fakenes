/* FakeNES - A free, portable, Open Source NES emulator.

   audio_int.h: Internal declarations for both the audio
      output system and the audio library.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef AUDIO_INT_H_INCLUDED
#define AUDIO_INT_H_INCLUDED
#include <vector>
#include "common.h"
#include "types.h"

// Driver interfaces for the audio library.
class AudiolibDriver {
public:
   AudiolibDriver(void) { }
   virtual ~AudiolibDriver(void) { }

   virtual int initialize(void) { return 1; }
   virtual void deinitialize(void) { }
   virtual int openStream(void) { return 1; }
   virtual void closeStream(void) { }
   virtual void* getBuffer(void) { return null; }
   virtual void freeBuffer(void) { }
   virtual void suspend(void) { }
   virtual void resume(void) { }
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

#ifdef USE_OPENAL
class AudiolibOpenALDriver : public AudiolibDriver {
public:
   int initialize(void);
   void deinitialize(void);
   int openStream(void);
   void closeStream(void);
   void* getBuffer(void);
   void freeBuffer(void);
   void suspend(void);
   void resume(void);

};
#endif //USE_OPENAL

// Stuff from audio.c.
extern int audio_channels;
extern int audio_sample_rate;
extern int audio_sample_bits;
extern BOOL audio_signed_samples;
extern int audio_buffer_length_ms;

extern unsigned audio_buffer_size_frames;
extern unsigned audio_buffer_size_samples;
extern unsigned audio_buffer_size_bytes;

extern std::vector<uint16> audioQueue;

// Keep this inline and using references for speed.
static inline void audio_queue_sample(real& sample)
{
   // Clip to valid range.
   sample = fixf(sample, -1.0, 1.0);
   // Convert to 16-bit unsigned.
   uint16 packed = (((int16)fix((int)ROUND(sample * 32768.0), -32768, 32767)) ^ 0x8000);
   // Store it in the queue.
   audioQueue.push_back(packed);
}

#endif //!AUDIO_INT_H_INCLUDED