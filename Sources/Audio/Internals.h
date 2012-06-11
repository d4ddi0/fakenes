/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef AUDIO__INTERNALS_H__INCLUDED
#define AUDIO__INTERNALS_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern int audio_channels;
extern int audio_sample_rate;
extern int audio_sample_bits;
extern BOOL audio_signed_samples;
extern int audio_buffer_length_ms;

extern unsigned audio_buffer_size_frames;
extern unsigned audio_buffer_size_samples;
extern unsigned audio_buffer_size_bytes;

#ifdef __cplusplus
} // extern "C"

#include <vector>
#include "Common/Math.h"

extern std::vector<uint16> audioQueue;

// Keep this inline and using references for speed.
express_function void audio_queue_sample(const real& sample)
{
   // Convert to 16-bit unsigned and clip.
   const uint16 packed = (((int16)Clamp<int>( Round<real>(sample * 32768.0), -32768, 32767 )) ^ 0x8000);
   // Store it in the queue.
   audioQueue.push_back(packed);
}

#endif /* __cplusplus */
#endif /* !AUDIO__INTERNALS_H__INCLUDED */
