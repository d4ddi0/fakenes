

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

audio.h: Declarations for the audio interface.

Copyright (c) 2005, Randy McDowell.
Copyright (c) 2005, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef AUDIO_H_INCLUDED

#define AUDIO_H_INCLUDED


enum
{
    AUDIO_PSEUDO_STEREO_MODE_1 = 1,

    AUDIO_PSEUDO_STEREO_MODE_2,

    AUDIO_PSEUDO_STEREO_MODE_3
};


int audio_enable_output;


int audio_sample_rate;

int audio_sample_size;


int audio_buffer_length;


int audio_pseudo_stereo;


volatile int audio_fps;


int audio_init (void);

void audio_exit (void);


void audio_poll (void);

void audio_play (void);


void audio_suspend (void);

void audio_resume (void);


void * audio_buffer;


#endif /* ! AUDIO_H_INCLUDED */
