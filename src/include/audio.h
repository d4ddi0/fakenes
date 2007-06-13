/* FakeNES - A free, portable, Open Source NES emulator.

   audio.h: Declarations for the audio interface.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
#include "apu.h"
#include "common.h"
#include "types.h"
#ifdef __cplusplus
// -- Begin C++ only section
#include <vector>

extern std::vector<uint16> audioQueue;

// Keep this inline and using references for speed.
static inline void audio_queue_sample(real& sample)
{
   // Clip to valid range.
   sample = fixf(sample, -1.0, 1.0);
   // Convert to 16-bit.
   uint16 packed = (uint16)fix((int)ROUND(sample * 32768.0), -32768, 32767);
   // Store it in the queue.
   audioQueue.push_back(packed);
}
// -- End C++ only section

extern "C" {
#endif

typedef struct audio_options_s {
   BOOL enable_output;
   ENUM subsystem;
   int sample_rate_hint;
   int buffer_length_ms_hint;

} audio_options_t;

extern audio_options_t audio_options;

// Read only variables (to be written by the audio system only).
extern int audio_channels;
extern int audio_sample_rate;
extern int audio_sample_bits;
extern BOOL audio_signed_samples;
extern int audio_buffer_length_ms;

extern unsigned audio_buffer_size_frames;
extern unsigned audio_buffer_size_samples;
extern unsigned audio_buffer_size_bytes;

extern volatile int audio_fps;

extern void audio_load_config(void);
extern void audio_save_config(void);
extern int audio_init(void);
extern void audio_exit(void);
extern void audio_update(void);
extern void audio_suspend(void);
extern void audio_resume(void);
extern int audio_open_wav(const UCHAR* filename);
extern void audio_close_wav(void);

/* Subsystems. */
enum {
   AUDIO_SUBSYSTEM_NONE,
   AUDIO_SUBSYSTEM_ALLEGRO,
   AUDIO_SUBSYSTEM_OPENAL,
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !AUDIO_H_INCLUDED */
