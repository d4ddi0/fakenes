

/*

FakeNES - A portable, open-source NES emulator.

audio.h: Declarations for the audio interface.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


int audio_enable_output;


int audio_sample_rate;

int audio_sample_size;


int audio_buffer_length;


int audio_pseudo_stereo;


#define AUDIO_BUFFER_SIZE   \
    ((audio_sample_rate / 60) * audio_buffer_length)


int audio_init (void);

void audio_exit (void);


void audio_start (void);

void audio_stop (void);


UINT8 * audio_buffer;


#ifdef __cplusplus
}
#endif

#endif /* ! __AUDIO_H__ */
