/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef VIDEO__VIDEO_H__INCLUDED
#define VIDEO__VIDEO_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#include "GUI/GUI.h"
#ifdef __cplusplus
extern "C" {
#endif

enum {
   VIDEO_BLITTER_NONE = 0,
   VIDEO_BLITTER_HQ2X,
   VIDEO_BLITTER_HQ3X,
   VIDEO_BLITTER_HQ4X,
   VIDEO_BLITTER_INTERPOLATION,
   VIDEO_BLITTER_INTERPOLATION_SCANLINES,
   VIDEO_BLITTER_INTERPOLATION_TV_MODE,
   VIDEO_BLITTER_NTSC
};

enum {
   VIDEO_FILTER_NONE                = 0,
   VIDEO_FILTER_ASPECT_RATIO        = 1 << 0,
   VIDEO_FILTER_OFFSET              = 1 << 1,
   VIDEO_FILTER_OVERSCAN_HORIZONTAL = 1 << 2,
   VIDEO_FILTER_OVERSCAN_VERTICAL   = 1 << 3
};

enum {
   VIDEO_FONT_SMALLEST = 0,
   VIDEO_FONT_SMALL,
   VIDEO_FONT_MEDIUM,
   VIDEO_FONT_LARGE,

   VIDEO_FONT_LEGACY,
   VIDEO_FONT_MONOLITHIC,
   VIDEO_FONT_SHADOWED_BOTTOM,
   VIDEO_FONT_SHADOWED_TOP,
   VIDEO_FONT_SHADOWED = VIDEO_FONT_SHADOWED_TOP,

   VIDEO_FONT_DEFAULT
};

enum {
   VIDEO_PALETTE_NESTER = 0,
   VIDEO_PALETTE_NESTICLE,
   VIDEO_PALETTE_NTSC,
   VIDEO_PALETTE_PAL,
   VIDEO_PALETTE_RGB,

   VIDEO_PALETTE_DEFAULT = VIDEO_PALETTE_NTSC
};

enum {
   VIDEO_PROFILE_COLOR_PALETTE = 0,
   VIDEO_PROFILE_COLOR_HUE,
   VIDEO_PROFILE_COLOR_SATURATION,
   VIDEO_PROFILE_COLOR_BRIGHTNESS,
   VIDEO_PROFILE_COLOR_CONTRAST,
   VIDEO_PROFILE_COLOR_GAMMA,

   VIDEO_PROFILE_DISPLAY_DRIVER,
   VIDEO_PROFILE_DISPLAY_WIDTH,
   VIDEO_PROFILE_DISPLAY_HEIGHT,
   VIDEO_PROFILE_DISPLAY_COLOR_DEPTH,
   VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER,	/* Internal use only. */

   VIDEO_PROFILE_FILTER_ASPECT_RATIO,
   VIDEO_PROFILE_FILTER_OFFSET,
   VIDEO_PROFILE_FILTER_OVERSCAN_HORIZONTAL,
   VIDEO_PROFILE_FILTER_OVERSCAN_VERTICAL,

   VIDEO_PROFILE_OPTION_ACCELERATION,
   VIDEO_PROFILE_OPTION_DITHER,
   VIDEO_PROFILE_OPTION_FULLSCREEN,
   VIDEO_PROFILE_OPTION_HUD,
   VIDEO_PROFILE_OPTION_TEXTURE_FILTER,
   VIDEO_PROFILE_OPTION_VSYNC,

   VIDEO_PROFILE_OUTPUT_BLITTER,
   VIDEO_PROFILE_OUTPUT_SCALE,
   VIDEO_PROFILE_OUTPUT_SCALE_WIDTH,
   VIDEO_PROFILE_OUTPUT_SCALE_HEIGHT
};

extern void video_load_config(void);
extern void video_save_config(void);
extern int video_init(void);
extern void video_exit(void);
extern int video_get_profile_integer(const ENUM key);
extern REAL video_get_profile_real(const ENUM key);
extern ENUM video_get_profile_enum(const ENUM key);
extern BOOL video_get_profile_boolean(const ENUM key);
extern void video_set_profile_integer(const ENUM key, const int value);
extern void video_set_profile_real(const ENUM key, const REAL value);
extern void video_set_profile_enum(const ENUM key, const ENUM value);
extern void video_set_profile_boolean(const ENUM key, const BOOL value);
extern void video_update_display(void);
extern void video_update_game_display(void);
extern void video_update_settings(void);
extern void video_handle_keypress(const int c, const int scancode);
extern void video_message(const int duration, const UDATA* message, ...);
extern BOOL video_is_opengl_mode(void);
extern FONT* video_get_font(const ENUM type);
extern int video_search_palette(const int color);
extern int video_search_palette_rgb(const int r, const int g, const int b);
extern BITMAP* video_get_display_buffer(void);
extern BITMAP* video_get_blit_buffer(const int width, const int height);
extern BITMAP* video_get_extra_buffer(const int width, const int height);
extern BITMAP* video_get_filter_buffer(const int width, const int height);
extern BITMAP* video_get_render_buffer(void);
extern int video_legacy_create_color_dither(int r, int g, int b, int x, int y);
extern int video_legacy_create_gradient(const int start, const int end, const int slices, const int x, const int y);
extern void video_legacy_create_gui_gradient(GUI_COLOR* start, const GUI_COLOR* end, const int slices);
extern void video_legacy_translucent_textout(BITMAP* bitmap, FONT* font, const UDATA* text, const int x, const int y, const int color);
extern void video_legacy_shadow_textout(BITMAP* bitmap, FONT* font, const UDATA* text, const int x, const int y, const int color, const int opacity);
extern void video_legacy_shadow_textprintf(BITMAP* bitmap, FONT* font, const int x, const int y, const int color, const int opacity, const UDATA* text, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !VIDEO__VIDEO_H__INCLUDED */
