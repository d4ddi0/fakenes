

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


enum
{
    VIDEO_BLITTER_AUTOMATIC,


    VIDEO_BLITTER_NORMAL,

    VIDEO_BLITTER_STRETCHED,

    VIDEO_BLITTER_INTERPOLATED,


    VIDEO_BLITTER_2XSOE,

    VIDEO_BLITTER_2XSCL,


    VIDEO_BLITTER_SUPER_2XSOE,

    VIDEO_BLITTER_SUPER_2XSCL
};


#define VIDEO_FILTER_SCANLINES_LOW      1

#define VIDEO_FILTER_SCANLINES_MEDIUM   2

#define VIDEO_FILTER_SCANLINES_HIGH     4


UINT8 * video_overlay_text;

volatile int video_show_overlay;


int video_display_status;

int video_enable_vsync;


int video_force_window;


BITMAP * base_video_buffer;

BITMAP * video_buffer;


int video_init (void);

int video_reinit (void);

void video_exit (void);


void video_blit (BITMAP *);

void video_filter (void);


void video_handle_keypress (int);


void video_set_palette (RGB *);


void video_set_blitter (int);

int video_get_blitter (void);


void video_set_filter_list (int);

int video_get_filter_list (void);


void video_set_resolution (int, int);


int video_get_color_depth (void);

void video_set_color_depth (int);


#ifdef __cplusplus
}
#endif

#endif /* ! __VIDEO_H__ */
