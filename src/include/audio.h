

/*

FakeNES - A portable, open-source NES emulator.

audio.h: Declarations for the audio interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


int audio_enable_output;


int audio_sample_rate;

int audio_sample_size;


int audio_buffer_length;


int audio_pseudo_stereo;


volatile int audio_fps;


int audio_init (void);

void audio_exit (void);


void audio_start (void);

void audio_stop (void);


void audio_suspend (void);

void audio_resume (void);


void * audio_buffer;


#ifdef __cplusplus
}
#endif

#endif /* ! __AUDIO_H__ */
