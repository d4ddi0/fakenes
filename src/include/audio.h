/* FakeNES - A free, portable, Open Source NES emulator.

   audio.h: Declarations for the audio interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

BOOL audio_enable_output;
ENUM audio_subsystem;
int audio_sample_rate;
int audio_sample_size;
BOOL audio_unsigned_samples;
BOOL audio_interpolation;
int audio_buffer_length;

int audio_buffer_size_samples;
int audio_buffer_size_bytes;
int audio_buffer_frame_size_samples;
int audio_buffer_frame_size_bytes;
volatile int audio_fps;

int audio_init (void);
void audio_exit (void);
void *audio_get_buffer (void);
void audio_play (void);
void audio_suspend (void);
void audio_resume (void);

/* Subsystems. */
enum
{
   AUDIO_SUBSYSTEM_NONE,
   AUDIO_SUBSYSTEM_ALLEGRO
};

#ifdef __cplusplus
}
#endif
#endif  /* !AUDIO_H_INCLUDED */
