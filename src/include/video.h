

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

video.h: Declarations for the video interface.

Copyright (c) 2003, Randy McDowell.
Copyright (c) 2003, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef VIDEO_H_INCLUDED

#define VIDEO_H_INCLUDED


#include "misc.h"


#define MAX_MESSAGES        10


#define MAX_MESSAGE_LENGTH  100


UINT8 video_messages [MAX_MESSAGES] [(MAX_MESSAGE_LENGTH + 1)];


volatile int video_message_duration;


enum
{
    VIDEO_BLITTER_AUTOMATIC,


    VIDEO_BLITTER_NORMAL,

    VIDEO_BLITTER_STRETCHED,


    VIDEO_BLITTER_INTERPOLATED_2X,

    VIDEO_BLITTER_INTERPOLATED_3X,


    VIDEO_BLITTER_2XSOE,

    VIDEO_BLITTER_2XSCL,


    VIDEO_BLITTER_SUPER_2XSOE,

    VIDEO_BLITTER_SUPER_2XSCL
};


#define VIDEO_FILTER_SCANLINES_LOW      1

#define VIDEO_FILTER_SCANLINES_MEDIUM   2

#define VIDEO_FILTER_SCANLINES_HIGH     4


int video_display_status;

int video_enable_vsync;


int video_force_window;


int video_driver;


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


void video_set_driver (int);


void video_message (const UINT8 *, ...);


#endif /* ! VIDEO_H_INCLUDED */
