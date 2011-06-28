/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef AUDIO__INTERNALS_H__INCLUDED
#define AUDIO__INTERNALS_H__INCLUDED
#include <vector>
#include "common.h"
#include "types.h"

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
   // Convert to 16-bit unsigned and clip.
   const uint16 packed = (((int16)fix((int)ROUND(sample * 32768.0), -32768, 32767)) ^ 0x8000);
   // Store it in the queue.
   audioQueue.push_back(packed);
}

#endif // !AUDIO__INTERNALS_H__INCLUDED
