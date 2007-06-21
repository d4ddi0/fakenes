/* FakeNES - A free, portable, Open Source NES emulator.

   video.h: Declarations for the video interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED
#include "common.h"
#include "debug.h"
#include "gui.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern volatile int video_message_duration;

enum
{
   VIDEO_BLITTER_AUTOMATIC = -1,
   VIDEO_BLITTER_NORMAL,
   VIDEO_BLITTER_DES,
   VIDEO_BLITTER_INTERPOLATED_2X,
   VIDEO_BLITTER_INTERPOLATED_2X_HQ,
   VIDEO_BLITTER_2XSCL,
   VIDEO_BLITTER_DESII,
   VIDEO_BLITTER_SUPER_2XSCL,
   VIDEO_BLITTER_ULTRA_2XSCL,
   VIDEO_BLITTER_HQ2X,
   VIDEO_BLITTER_NTSC,
   VIDEO_BLITTER_INTERPOLATED_3X,
   VIDEO_BLITTER_HQ3X,
   VIDEO_BLITTER_HQ4X,
   VIDEO_BLITTER_STRETCHED
};

#define VIDEO_FILTER_SCANLINES_LOW      1
#define VIDEO_FILTER_SCANLINES_MEDIUM   2
#define VIDEO_FILTER_SCANLINES_HIGH     4

extern int video_buffer_width;
extern int video_buffer_height;

extern BOOL video_display_status;
extern BOOL video_enable_page_buffer;
extern BOOL video_enable_vsync;
extern BOOL video_force_fullscreen;
extern int video_cached_color_depth; /* Read only. */

extern int video_driver;
   
extern BITMAP *base_video_buffer;
extern BITMAP *video_buffer;

extern FONT *small_font;

extern LIST video_edge_clipping;

#define VIDEO_EDGE_CLIPPING_HORIZONTAL (1 << 0)
#define VIDEO_EDGE_CLIPPING_VERTICAL   (1 << 1)

extern RGB *video_palette;

extern void video_load_config (void);
extern void video_save_config (void);
extern int video_init (void);
extern int video_reinit (void);
extern int video_init_buffer (void);
extern void video_exit (void);
extern void video_blit (BITMAP *);
extern void video_filter (void);
extern void video_handle_keypress (int, int);
extern void video_set_palette (RGB *);
extern void video_set_palette_id (int);
extern int video_get_palette_id (void);
extern int video_create_color_dither (int, int, int, int, int);
extern int video_create_gradient (int, int, int, int, int);
extern void video_create_gui_gradient (GUI_COLOR *, GUI_COLOR *, int);
extern void video_set_blitter (ENUM);
extern ENUM video_get_blitter (void);
extern void video_blitter_reinit (void);
extern void video_set_filter_list (LIST);
extern LIST video_get_filter_list (void);
extern void video_set_resolution (int, int);
extern int video_get_color_depth (void);
extern void video_set_color_depth (int);
extern void video_set_driver (int);
extern BOOL video_is_opengl_mode (void);
extern void video_show_bitmap (BITMAP *, ENUM, BOOL);

extern void video_message (const UCHAR *, ...);

extern UINT8 video_color_map[32][32][32];

static INLINE int video_create_color (int r, int g, int b)
{
   /* Note: Don't use the makecol() or makecol8() functions here, as they
      don't appear to be inlined by Allegro. */

   switch (video_cached_color_depth)
   {
      case 8:
         return (video_color_map[(r >> 3)][(g >> 3)][(b >> 3)]);

      case 15:
         return (makecol15 (r, g, b));

      case 16:
         return (makecol16 (r, g, b));

      case 24:
         return (makecol24 (r, g, b));

      case 32:
         return (makecol32 (r, g, b));

      default:
         WARN_GENERIC();
   }

   return (0);
}

#ifdef __cplusplus
}
#endif
#endif   /* !VIDEO_H_INCLUDED */
