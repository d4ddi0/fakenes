

/*

FakeNES - A portable, open-source NES emulator.

video.h: Declarations for the video interface.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __VIDEO_H__
#define __VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif


int video_display_status;

int video_enable_vsync;


BITMAP * base_video_buffer;

BITMAP * video_buffer;


int video_init (void);

void video_exit (void);


void video_blit (void);


#ifdef __cplusplus
}
#endif

#endif /* ! __VIDEO_H__ */
