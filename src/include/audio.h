

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


int audio_init (void);

void audio_exit (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __AUDIO_H__ */
