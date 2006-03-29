/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   video.h: Declarations for the video interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED
#include "common.h"
#include "gui.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

volatile int video_message_duration;

enum
{
   VIDEO_BLITTER_AUTOMATIC = -1,
   VIDEO_BLITTER_NORMAL,
   VIDEO_BLITTER_DES,
   VIDEO_BLITTER_INTERPOLATED_2X,
   VIDEO_BLITTER_2XSCL,
   VIDEO_BLITTER_DESII,
   VIDEO_BLITTER_SUPER_2XSCL,
   VIDEO_BLITTER_ULTRA_2XSCL,
   VIDEO_BLITTER_HQ2X,
   VIDEO_BLITTER_NES_NTSC,
   VIDEO_BLITTER_INTERPOLATED_3X,
   VIDEO_BLITTER_HQ3X,
   VIDEO_BLITTER_HQ4X,
   VIDEO_BLITTER_STRETCHED
};

#define VIDEO_FILTER_SCANLINES_LOW      1
#define VIDEO_FILTER_SCANLINES_MEDIUM   2
#define VIDEO_FILTER_SCANLINES_HIGH     4

BOOL video_display_status;
BOOL video_enable_vsync;
BOOL video_force_fullscreen;

int video_driver;

BITMAP *base_video_buffer;
BITMAP *video_buffer;

FONT *small_font;

LIST video_edge_clipping;

#define VIDEO_EDGE_CLIPPING_HORIZONTAL (1 << 0)
#define VIDEO_EDGE_CLIPPING_VERTICAL   (1 << 1)

RGB *video_palette;

int video_init (void);
int video_reinit (void);
void video_exit (void);
void video_blit (BITMAP *);
void video_filter (void);
void video_handle_keypress (int);
void video_set_palette (RGB *);
void video_set_palette_id (int);
int video_get_palette_id (void);
int video_create_color (int, int, int);
int video_create_color_dither (int, int, int, int, int);
int video_create_gradient (int, int, int, int, int);
void video_create_gui_gradient (GUI_COLOR *, GUI_COLOR *, int);
void video_set_blitter (ENUM);
ENUM video_get_blitter (void);
void video_set_filter_list (LIST);
LIST video_get_filter_list (void);
void video_set_resolution (int, int);
int video_get_color_depth (void);
void video_set_color_depth (int);
void video_set_driver (int);

void video_message (const UCHAR *, ...);

static INLINE int fix (int value, int base, int limit)
{
   if (value < base)
      value = base;
   if (value > limit)
      value = limit;

   return (value);
}

#ifdef __cplusplus
}
#endif
#endif   /* !VIDEO_H_INCLUDED */
