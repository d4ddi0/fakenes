

/*

FakeNES - A portable, Open Source NES emulator.

video.h: Declarations for the video interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __VIDEO_H__
#define __VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif


#define VIDEO_BLITTER_NORMAL        0

#define VIDEO_BLITTER_STRETCHED     1


int video_display_status;

int video_enable_vsync;


BITMAP * base_video_buffer;

BITMAP * video_buffer;


int video_init (void);

void video_exit (void);


void video_blit (BITMAP *);


void video_zoom_in (void);

void video_zoom_out (void);


void video_set_palette (RGB *);


void video_set_blitter (int);

int video_get_blitter (void);


void video_set_resolution (int, int);


#ifdef __cplusplus
}
#endif

#endif /* ! __VIDEO_H__ */
