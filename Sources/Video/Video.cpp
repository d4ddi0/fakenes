/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "Color.h"
#include "Internals.h"
#include "Local.hpp"
#include "PPU.h"
#include "Video.h"

// TODO: Finish display update routines.
// TODO: Check for non-power-of-2 texture support.
// TODO: Move OpenGL stuff into its own container and initialize it properly.
// TODO: Color correction quantization and tinting.
// TODO: Verify that all settings are in a valid range.
// TODO: Add filters and blitters.
// TODO: Improve HUD.
// TODO: OpenGL error checking.
// TODO: Add support for the monolithic and custom fonts.
// TODO: More verbose logging and warnings in general.

using namespace std;

// Internal functions.
static bool Initialize();
static void Exit();
static bool SetupOpenGL();
static void ExitOpenGL();
static bool IsOpenGL();
static void LoadFonts();
static void UpdateColor();
static void UpdateDisplay();
static void UpdateScreen();
static void UpdateScreenOpenGL();
static void DrawLightgunCursor();
static void DrawHUD();
static void DrawMessages();

/* When hardware acceleration is enabled without OpenGL, page flipping or triple buffering is used.
   These constants define the parameters for video memory page allocation and usage. */
static const int PagesForPageFlipping = 2;
static const int PagesForTripleBuffering = 3;
static const int MaximumPages = 3;

// Buffers used for drawing both to memory and the screen.
typedef struct _VideoBuffers {
   BITMAP* display;		// Buffer for the display. Slow, avoid when possible.
   BITMAP* blit, *filter;	// Hold output from filters and blitters, sized as needed, 16-bit.
   BITMAP* extra;		// Extra buffer used for compatibility situations.
   BITMAP* overlay;		// This usually just points to render.
   BITMAP* pages[MaximumPages];	// Virtual screens for hardware acceleration.
   int pageCount, currentPage;
   BITMAP* render;		// Holds output from the PPU itself, always 256x240, 16-bit.

} VideoBuffers;

// Information for managing colors on the display.
typedef struct _VideoColor {
   ENUM palette;		// Palette for (re)mapping NES colors.
   real hue, saturation;	// Color correction applied to the palette.
   real brightness, contrast;	// ...
   real gamma;			// ...

} VideoColor;

// Information about the display mode.
typedef struct _VideoDisplay {
   int driver;		// The current display driver, out of any that Allegro supports.
   int width, height;	// The width and height of the display (screen or window).
   int colorDepth;	// The display color depth, in bits-per-pixel.
   //
   bool indexed;	// If this is set, a 256-color palette will be managed.
   PALETTE palette;	// If a palette is being managed, it will be stored here.
   RGB_MAP rgbMap;	// For fast truecolor to indexed color conversions.
   bool swapRGB;	// Set when red and blue need to be swapped.
   bool doubleBuffer;	// Use display double-buffering, e.g for the GUI.

} VideoDisplay;

// Bitmap font support, to be replaced with TrueType later, possibly.
typedef struct _VideoFonts {
   FONT* smallLow, *mediumLow, *largeLow;	// Low-resolution fonts in three sizes.
   FONT* smallHigh, *mediumHigh, *largeHigh;	// High-resolutions fonts, also in three sizes.
   FONT* legacy;				// The old 5x6 font, which is still heavily used.
   FONT* monolithic;				// Multilingual Unicode font.
   FONT* shadowedBottom, *shadowedTop;		// A shadowed version of the smallest font.
   FONT* custom;				// A custom font, overrides low and high res fonts.

} VideoFonts;

/* Various options that can be set externally with video_set_option().
   Note that all options apply to all modes, especially OpenGL. These are merely hints. */
typedef struct _VideoOptions {
   bool enableAcceleration;	// Allow page flipping or triple buffering to be used.
   bool enableDither;		// Dither any conversions between different color depths.
   bool enableFullscreen;	// Always try to set a fullscreen mode.
   bool enableHUD;		// Show emulator and virtual machine status in-game.
   bool enableTextureFilter;	// Allow OpenGL texture filtering.
   bool enableVSync;		// Sync to VBlank before final blits.

} VideoOptions;

// Output options, which handle filtering, blitting and scaling.
typedef struct VideoOutput {
   LIST filters;			// List of filters to be applied before blitting.
   ENUM blitter;			// The blitter to be used, if any.
   bool scale;				// Enable scaling of the final image to the display.
   real scaleWidth, scaleHeight;	// If scaling, this is the width and height (%).
   //
   int width, height;			// Final width and height after blitting.

} VideoOutput;

#ifdef ALLEGRO_UNIX
/* The X header files define Display as something else, so we have to use this hack to get around it.
   I *really* get tired of OS APIs polluting the primary namespace. */
#define Display Video_Display
#endif

static VideoBuffers Buffers;
static VideoColor Color;
static VideoDisplay Display;
static VideoFonts Fonts;
static VideoOptions Options;
static VideoOutput Output;

/* This is a list of things that have been changed, used for video_update_settings().
   This isn't the most appealing way to do this, but it works. */
enum {
   DirtyNone    = 0,
   DirtyColor   = 1 << 0,
   DirtyDisplay = 1 << 1
};

static LIST dirty;

#ifdef USE_ALLEGROGL
// Hack for AllegroGL.
static bool alglInstalled = false;

// Our OpenGL display texture object.
static GLuint displayTexture = 0;

/* Our OpenGL display list, just to reduce the amount of code needed to draw
   the textured quad in a number of different configurations. */
static GLuint displayList = 0;
#endif

// Support for showing chat and log messages.
typedef struct _Message {
   USTRING text;	// Unicode text representing the message.
   int lifetime;	// Lifetime (ms) of the message, before it is deleted.
   int duration;	// Display duration (ms) of the message.
   int fadeTime;	// How long the message takes to fade out (ms).

} Message;

typedef vector<Message> History;
static History history;

// These functions handle when the user switches to or from the program.
static void SwitchAway(void)
{
   if(file_is_loaded)
      apu_options.squelch = !apu_options.squelch;
}

static void SwitchBack(void)
{
   if(file_is_loaded)
      apu_options.squelch = !apu_options.squelch;
}

// This macro helps with clearing the memory used by bitmaps.
#define FreeBitmap(_bitmap) { \
   if(_bitmap) { \
      destroy_bitmap(_bitmap); \
      _bitmap = NULL; \
   } \
}

/* In order for color math to know when to swap components, yet remain fast and inlined
   whenever possible, we have to export this variable. */
BOOL video__swap_rgb = FALSE;

// --------------------------------------------------------------------------------

void video_load_config(void)
{
   // Set up sensible defaults. Prefer safety over performance or eye candy.
   Display.driver = GFX_SAFE;
   Display.width = 640;
   Display.height = 480;
   Display.colorDepth = -1;

   Color.palette = VIDEO_PALETTE_DEFAULT;
   Color.hue = 0;
   Color.saturation = 0;
   Color.brightness = 0;
   Color.contrast = 0;
   Color.gamma = 0;

   Options.enableAcceleration = false;
   Options.enableDither = false;
   Options.enableFullscreen = false;
   Options.enableHUD = false;
   Options.enableTextureFilter = true;
   Options.enableVSync = false;

   Output.filters = VIDEO_FILTER_NONE;
   LIST_ADD(Output.filters, VIDEO_FILTER_ASPECT_RATIO);
   LIST_ADD(Output.filters, VIDEO_FILTER_OFFSET);

   Output.blitter = VIDEO_BLITTER_NONE;
   Output.scale = true;
   Output.scaleWidth = 200;
   Output.scaleHeight = 200;

   Color.palette    = get_config_int  ("video", "color.palette",    Color.palette);
   Color.hue        = get_config_float("video", "color.hue",        Color.hue);
   Color.saturation = get_config_float("video", "color.saturation", Color.saturation);
   Color.brightness = get_config_float("video", "color.brightness", Color.brightness);
   Color.contrast   = get_config_float("video", "color.contrast",   Color.contrast);
   Color.gamma      = get_config_float("video", "color.gamma",      Color.gamma);

   Display.driver     = get_config_id ("video", "display.driver",      Display.driver);
   Display.width      = get_config_int("video", "display.width",       Display.width);
   Display.height     = get_config_int("video", "display.height",      Display.height);
   Display.colorDepth = get_config_int("video", "display.color_depth", Display.colorDepth);

   Options.enableAcceleration  = get_config_int("video", "option.acceleration",   Options.enableAcceleration);
   Options.enableDither        = get_config_int("video", "option.dithering",      Options.enableDither);
   Options.enableFullscreen    = get_config_int("video", "option.fullscreen",     Options.enableFullscreen);
   Options.enableHUD           = get_config_int("video", "option.hud",            Options.enableHUD);
   Options.enableTextureFilter = get_config_int("video", "option.texture_filter", Options.enableTextureFilter);
   Options.enableVSync         = get_config_int("video", "option.vsync",          Options.enableVSync);

   Output.blitter     = get_config_int  ("video", "output.blitter",      Output.blitter);
   Output.scale       = get_config_int  ("video", "output.scale",        Output.scale);
   Output.scaleWidth  = get_config_float("video", "output.scale_width",  Output.scaleWidth);
   Output.scaleHeight = get_config_float("video", "output.scale_height", Output.scaleHeight);

   const bool aspectRatio        = get_config_int("video", "filter.aspect_ratio",        LIST_COMPARE(Output.filters, VIDEO_FILTER_ASPECT_RATIO));
   const bool offset             = get_config_int("video", "filter.offset",              LIST_COMPARE(Output.filters, VIDEO_FILTER_OFFSET));
   const bool overscanHorizontal = get_config_int("video", "filter.overscan_horizontal", LIST_COMPARE(Output.filters, VIDEO_FILTER_OVERSCAN_HORIZONTAL));
   const bool overscanVertical   = get_config_int("video", "filter.overscan_vertical",   LIST_COMPARE(Output.filters, VIDEO_FILTER_OVERSCAN_VERTICAL));

   Output.filters = VIDEO_FILTER_NONE;
   LIST_ADD(Output.filters, aspectRatio        ? VIDEO_FILTER_ASPECT_RATIO        : VIDEO_FILTER_NONE);
   LIST_ADD(Output.filters, offset             ? VIDEO_FILTER_OFFSET              : VIDEO_FILTER_NONE);
   LIST_ADD(Output.filters, overscanHorizontal ? VIDEO_FILTER_OVERSCAN_HORIZONTAL : VIDEO_FILTER_NONE);
   LIST_ADD(Output.filters, overscanVertical   ? VIDEO_FILTER_OVERSCAN_VERTICAL   : VIDEO_FILTER_NONE);
}

void video_save_config (void)
{
   set_config_float("video", "color.hue",        Color.hue);
   set_config_float("video", "color.saturation", Color.saturation);
   set_config_float("video", "color.brightness", Color.brightness);
   set_config_float("video", "color.contrast",   Color.contrast);
   set_config_float("video", "color.gamma",      Color.gamma);

   set_config_id ("video", "display.driver",      Display.driver);
   set_config_int("video", "display.width",       Display.width);
   set_config_int("video", "display.height",      Display.height);
   set_config_int("video", "display.color_depth", Display.colorDepth);

   set_config_int("video", "option.acceleration",   Options.enableAcceleration);
   set_config_int("video", "option.dither",         Options.enableDither);
   set_config_int("video", "option.fullscreen",     Options.enableFullscreen);
   set_config_int("video", "option.hud",            Options.enableHUD);
   set_config_int("video", "option.texture_filter", Options.enableTextureFilter);
   set_config_int("video", "option.vsync",          Options.enableVSync);

   set_config_int  ("video", "output.blitter",      Output.blitter);
   set_config_int  ("video", "output.scale",        Output.scale);
   set_config_float("video", "output.scale_width",  Output.scaleWidth);
   set_config_float("video", "output.scale_height", Output.scaleHeight);

   set_config_int("video", "filter.aspect_ratio",        LIST_COMPARE(Output.filters, VIDEO_FILTER_ASPECT_RATIO));
   set_config_int("video", "filter.offset",              LIST_COMPARE(Output.filters, VIDEO_FILTER_OFFSET));
   set_config_int("video", "filter.overscan_horizontal", LIST_COMPARE(Output.filters, VIDEO_FILTER_OVERSCAN_HORIZONTAL));
   set_config_int("video", "filter.overscan_vertical",   LIST_COMPARE(Output.filters, VIDEO_FILTER_OVERSCAN_VERTICAL));
}

int video_init(void)
{
   if(!Initialize()) {
      log_printf("The display failed to initialize, trying safe mode.\n");

      Display.driver = GFX_SAFE;
      Display.width = 320;
      Display.height = 200;
      Display.colorDepth = -1;

      Options.enableAcceleration = false;
      Options.enableFullscreen = false;

      if(!Initialize()) {
         log_printf("Unable to initialize the display even in safe mode.\n"
                    "This is an unrecoverable error. We apologise for the inconvenience.\n");

         return 1;
      }

      WARN("The program is now running in safe mode. Check the log for details.");
   }

   return 0;
}

void video_exit(void)
{
   Exit();
}

int video_get_profile_integer(const ENUM key)
{
   switch(key) {
      case VIDEO_PROFILE_DISPLAY_DRIVER:
         return Display.driver;
      case VIDEO_PROFILE_DISPLAY_WIDTH:
         return Display.width;
      case VIDEO_PROFILE_DISPLAY_HEIGHT:
         return Display.height;
      case VIDEO_PROFILE_DISPLAY_COLOR_DEPTH:
         return Display.colorDepth;

      default:
         WARN_GENERIC();
         break;
   }

   return 0;
}

REAL video_get_profile_real(const ENUM key)
{
   switch(key) {
      case VIDEO_PROFILE_COLOR_HUE:
         return Color.hue;
      case VIDEO_PROFILE_COLOR_SATURATION:
         return Color.saturation;
      case VIDEO_PROFILE_COLOR_BRIGHTNESS:
         return Color.brightness;
      case VIDEO_PROFILE_COLOR_CONTRAST:
         return Color.contrast;
      case VIDEO_PROFILE_COLOR_GAMMA:
         return Color.gamma;

      case VIDEO_PROFILE_OUTPUT_SCALE_WIDTH:
         return Output.scaleWidth;
      case VIDEO_PROFILE_OUTPUT_SCALE_HEIGHT:
         return Output.scaleHeight;
      
      default:
         WARN_GENERIC();
         break;
   }

   return 0;
}

ENUM video_get_profile_enum(const ENUM key)
{
   switch(key) {
      case VIDEO_PROFILE_COLOR_PALETTE:
         return Color.palette;

      case VIDEO_PROFILE_OUTPUT_BLITTER:
         return Output.blitter;

      default:
         WARN_GENERIC();
         break;
   }

   return 0;
}

BOOL video_get_profile_boolean(const ENUM key)
{
   switch(key) {
      case VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER:
         return Display.doubleBuffer;

      case VIDEO_PROFILE_FILTER_ASPECT_RATIO:
         return LIST_COMPARE(Output.filters, VIDEO_FILTER_ASPECT_RATIO);
      case VIDEO_PROFILE_FILTER_OFFSET:
         return LIST_COMPARE(Output.filters, VIDEO_FILTER_OFFSET);
      case VIDEO_PROFILE_FILTER_OVERSCAN_HORIZONTAL:
         return LIST_COMPARE(Output.filters, VIDEO_FILTER_OVERSCAN_HORIZONTAL);
      case VIDEO_PROFILE_FILTER_OVERSCAN_VERTICAL:
         return LIST_COMPARE(Output.filters, VIDEO_FILTER_OVERSCAN_VERTICAL);

      case VIDEO_PROFILE_OPTION_ACCELERATION:
         return Options.enableAcceleration;
      case VIDEO_PROFILE_OPTION_DITHER:
         return Options.enableDither;
      case VIDEO_PROFILE_OPTION_FULLSCREEN:
         return Options.enableFullscreen;
      case VIDEO_PROFILE_OPTION_HUD:
         return Options.enableHUD;
      case VIDEO_PROFILE_OPTION_TEXTURE_FILTER:
         return Options.enableTextureFilter;
      case VIDEO_PROFILE_OPTION_VSYNC:
         return Options.enableVSync;

      case VIDEO_PROFILE_OUTPUT_SCALE:
         return Output.scale;

      default:
         WARN_GENERIC();
         break;
   }

   return FALSE;
}

void video_set_profile_integer(const ENUM key, const int value)
{
   switch(key) {
      case VIDEO_PROFILE_DISPLAY_DRIVER:
      case VIDEO_PROFILE_DISPLAY_WIDTH:
      case VIDEO_PROFILE_DISPLAY_HEIGHT:
      case VIDEO_PROFILE_DISPLAY_COLOR_DEPTH:
         LIST_ADD(dirty, DirtyDisplay);
   }

   switch(key) {
      case VIDEO_PROFILE_DISPLAY_DRIVER:
         Display.driver = value;
         break;
      case VIDEO_PROFILE_DISPLAY_WIDTH:
         Display.width = value;
         break;
      case VIDEO_PROFILE_DISPLAY_HEIGHT:
         Display.height = value;
         break;
      case VIDEO_PROFILE_DISPLAY_COLOR_DEPTH:
         Display.colorDepth = value;
         break;

      default:
         WARN_GENERIC();
         break;
   }
}

void video_set_profile_real(const ENUM key, const REAL value)
{
   switch(key) {
      case VIDEO_PROFILE_COLOR_HUE:
      case VIDEO_PROFILE_COLOR_SATURATION:
      case VIDEO_PROFILE_COLOR_BRIGHTNESS:
      case VIDEO_PROFILE_COLOR_CONTRAST:
      case VIDEO_PROFILE_COLOR_GAMMA:
         LIST_ADD(dirty, DirtyColor);
   }

   switch(key) {
      case VIDEO_PROFILE_COLOR_HUE:
         Color.hue = value;
         break;
      case VIDEO_PROFILE_COLOR_SATURATION:
         Color.saturation = value;
         break;
      case VIDEO_PROFILE_COLOR_BRIGHTNESS:
         Color.brightness = value;
         break;
      case VIDEO_PROFILE_COLOR_CONTRAST:
         Color.contrast = value;
         break;
      case VIDEO_PROFILE_COLOR_GAMMA:
         Color.gamma = value;
         break;

      case VIDEO_PROFILE_OUTPUT_SCALE_WIDTH:
         Output.scaleWidth = value;
         break;
      case VIDEO_PROFILE_OUTPUT_SCALE_HEIGHT:
         Output.scaleHeight = value;
         break;
      
      default:
         WARN_GENERIC();
         break;
   }
}

void video_set_profile_enum(const ENUM key, const ENUM value)
{
   switch(key) {
      case VIDEO_PROFILE_COLOR_PALETTE: {
         Color.palette = value;
         LIST_ADD(dirty, DirtyColor);
         break;
      }

      case VIDEO_PROFILE_OUTPUT_BLITTER:
         Output.blitter = value;
         break;

      default:
         WARN_GENERIC();
         break;
   }
}

void video_set_profile_boolean(const ENUM key, const BOOL value)
{
   switch(key) {
      case VIDEO_PROFILE_OPTION_ACCELERATION:
      case VIDEO_PROFILE_OPTION_DITHER:
      case VIDEO_PROFILE_OPTION_FULLSCREEN:
         LIST_ADD(dirty, DirtyDisplay);
   }

   switch(key) {
      case VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER:
         Display.doubleBuffer = value;
         break;

      case VIDEO_PROFILE_FILTER_ASPECT_RATIO:
         LIST_SET(Output.filters, VIDEO_FILTER_ASPECT_RATIO, value);
         break;
      case VIDEO_PROFILE_FILTER_OFFSET:
         LIST_SET(Output.filters, VIDEO_FILTER_OFFSET, value);
         break;
      case VIDEO_PROFILE_FILTER_OVERSCAN_HORIZONTAL:
         LIST_SET(Output.filters, VIDEO_FILTER_OVERSCAN_HORIZONTAL, value);
         break;
      case VIDEO_PROFILE_FILTER_OVERSCAN_VERTICAL:
         LIST_SET(Output.filters, VIDEO_FILTER_OVERSCAN_VERTICAL, value);
         break;

      case VIDEO_PROFILE_OPTION_ACCELERATION:
         Options.enableAcceleration = value;
         break;
      case VIDEO_PROFILE_OPTION_DITHER:
         Options.enableDither = value;
         break;
      case VIDEO_PROFILE_OPTION_FULLSCREEN:
         Options.enableFullscreen = value;
         break;
      case VIDEO_PROFILE_OPTION_HUD:
         Options.enableHUD = value;
         break;
      case VIDEO_PROFILE_OPTION_TEXTURE_FILTER:
         Options.enableTextureFilter = value;
         break;
      case VIDEO_PROFILE_OPTION_VSYNC:
         Options.enableVSync = value;
         break;

      case VIDEO_PROFILE_OUTPUT_SCALE:
         Output.scale = value;
         break;

      default:
         WARN_GENERIC();
         break;
   }
}

void video_update_display(void)
{
   /* When the GUI is active, we follow a simplified pipeline, as it has already
      taken care of most of what we need to do. */
   if(gui_is_active) {
      UpdateScreen();
      return;
   }

   // Update the game display. This includes things like the HUD.
   video_update_game_display();

   // Finally we can display it all.
   UpdateDisplay();
}

void video_update_game_display(void)
{
   if(input_enable_zapper)
      DrawLightgunCursor();

   if(Options.enableHUD)
      DrawHUD();

   DrawMessages();
}

/* Calling this will allow the display driver, width, height, and color depth to be changed,
   as well as certain options like hardware acceleration. However, this destroys the
   drawing buffers, so all graphics will need to be refreshed. */
void video_update_settings(void)
{
   if(LIST_COMPARE(dirty, DirtyDisplay)) {
      log_printf("A settings change was requested, tearing the display down.\n");

      video_exit();
      video_init();
   }
   else if(LIST_COMPARE(dirty, DirtyColor)) {
      /* We only need to handle this if the display wasn't dirty, as reinitializing the display
        automatically sets up the color system again with the latest settings. */
      UpdateColor();
   }

   dirty = DirtyNone;
}

void video_handle_keypress(const int c, const int scancode)
{
   if(input_mode & INPUT_MODE_CHAT)
      return;

   switch(scancode) {
      case KEY_F10: {
         Color.gamma -= 5;
         if(Color.gamma < -100)
            Color.gamma = -100;

         UpdateColor();

         break;
      }

      case KEY_F11: {
         Color.gamma += 5;
         if(Color.gamma > 100)
            Color.gamma = 100;

         UpdateColor();

         break;
      }

      default:
         break;
   }
}

void video_message(const int duration, const UDATA* message, ...)
{
   va_list format;
   USTRING buffer;

   va_start(format, message);
   uvszprintf(buffer, USTRING_SIZE, message, format);
   va_end(format);

   Message msg;
   USTRING_CLEAR(msg.text);
   ustrncat(msg.text, buffer, USTRING_SIZE - 1);

   if(duration == -1)
      msg.duration = 3000;
   else
      msg.duration = duration;

   msg.lifetime = msg.duration * 10;
   msg.fadeTime = msg.duration / 5;

   history.push_back(msg);
}

BOOL video_is_opengl_mode(void)
{
   return IsOpenGL();
}

BITMAP* video_get_display_buffer(void)
{
   return Buffers.display;
}

BITMAP* video_get_blit_buffer(const int width, const int height)
{
   if(Buffers.blit) {
      if((Buffers.blit->w == width) && (Buffers.blit->h == height))
         return Buffers.blit;

      FreeBitmap(Buffers.blit);
   }

   Buffers.blit = create_bitmap_ex(16, width, height);
   return Buffers.blit;
}

BITMAP* video_get_extra_buffer(const int width, const int height)
{
   if(Buffers.extra) {
      if((Buffers.extra->w == width) && (Buffers.extra->h == height))
         return Buffers.extra;

      FreeBitmap(Buffers.extra);
   }

   Buffers.extra = create_bitmap(width, height);
   return Buffers.extra;
}

BITMAP* video_get_filter_buffer(const int width, const int height)
{
   if(Buffers.filter) {
      if((Buffers.filter->w == width) && (Buffers.filter->h == height))
         return Buffers.filter;

      FreeBitmap(Buffers.filter);
   }

   Buffers.filter = create_bitmap_ex(16, width, height);
   return Buffers.filter;
}

BITMAP* video_get_render_buffer(void)
{
   return Buffers.render;
}

FONT* video_get_font(const ENUM type) {
   bool lowRes = false;
   if((SCREEN_W < 512) || (SCREEN_H < 448))
      lowRes = TRUE;

   switch(type) {
      case VIDEO_FONT_SMALLEST:
         return Fonts.smallLow;
      case VIDEO_FONT_SMALL:
         return lowRes ? Fonts.smallLow : Fonts.smallHigh;
      case VIDEO_FONT_MEDIUM:
         return lowRes ? Fonts.mediumLow : Fonts.mediumHigh;
      case VIDEO_FONT_LARGE:
         return lowRes ? Fonts.largeLow : Fonts.largeHigh;

      case VIDEO_FONT_MONOLITHIC:
          return Fonts.monolithic;
      case VIDEO_FONT_SHADOWED_BOTTOM:
         return Fonts.shadowedBottom;
      case VIDEO_FONT_SHADOWED_TOP:
         return Fonts.shadowedTop;
      case VIDEO_FONT_LEGACY:
      case VIDEO_FONT_DEFAULT:
          return Fonts.legacy;

      default:
         WARN_GENERIC();
   }

   return font;
}

// This function finds the best color from the NES palette to represent an RGB triplet.
int video_search_palette(const int color)
{
   const int r = getr(color);
   const int g = getg(color);
   const int b = getb(color);

   return video_search_palette_rgb(r, g, b);
}

int video_search_palette_rgb(const int r, const int g, const int b)
{
   return (bestfit_color(DATA_TO_RGB(PALETTE_RGB), r, g, b) - 1) & 0x3F;
}

/*******************
 * Legacy routines *
 *******************/
// These routines are kept for backwards compatibility.

static const int dither_table[4][4] =
{
   {  0,  2,  0, -2 },
   {  2,  0, -2,  0 },
   {  0, -2,  0,  2 },
   { -2,  0,  2,  0 }
};

int video_legacy_create_color_dither(int r, int g, int b, int x, int y)
{
   if(!Options.enableDither)
      return makecol(r, g, b);

   // We use a more aggressive dithering in indexed color modes.
   if(Display.colorDepth == 8) {
      const int color = makecol15_dither(r, g, b, x, y);
      const int r = getr15(color);
      const int g = getg15(color);
      const int b = getb15(color);

      return makecol(r, g, b);
   }

   if(Display.colorDepth < 24) {
      x &= 3;
      y &= 3;
    
      r = fix(r + dither_table[y][x], 0, 255);
      g = fix(g + dither_table[y][x], 0, 255);
      b = fix(b + dither_table[y][x], 0, 255);
   }

   return makecol(r, g, b);
}


#define GRADIENT_SHIFTS         16
#define GRADIENT_MULTIPLIER     (255 << GRADIENT_SHIFTS)

static int gradient_start[3];
static int gradient_end[3];
static float gradient_delta[3];
static int gradient_slice;
static int gradient_last_x;

int video_legacy_create_gradient(const int start, const int end, const int slices, const int x, const int y)
{
   if(slices) {
      gradient_start[0] = getr(start) << GRADIENT_SHIFTS;
      gradient_start[1] = getg(start) << GRADIENT_SHIFTS;
      gradient_start[2] = getb(start) << GRADIENT_SHIFTS;

      gradient_end[0] = getr(end) << GRADIENT_SHIFTS;
      gradient_end[1] = getg(end) << GRADIENT_SHIFTS;
      gradient_end[2] = getb(end) << GRADIENT_SHIFTS;

      gradient_delta[0] = (gradient_end[0] - gradient_start[0]) / slices;
      gradient_delta[1] = (gradient_end[1] - gradient_start[1]) / slices;
      gradient_delta[2] = (gradient_end[2] - gradient_start[2]) / slices;

      gradient_slice = 0;
      gradient_last_x = -1;
      return 0;
   }
   else
   {
      int red   = gradient_start[0] + (gradient_delta[0] * gradient_slice);
      int green = gradient_start[1] + (gradient_delta[1] * gradient_slice);
      int blue  = gradient_start[2] + (gradient_delta[2] * gradient_slice);

      red >>= GRADIENT_SHIFTS;
      green >>= GRADIENT_SHIFTS;
      blue >>= GRADIENT_SHIFTS;

      if(gradient_last_x != x) {
         gradient_last_x = x;
         gradient_slice++;
      }

      return video_legacy_create_color_dither(red, green, blue, x, y);
   }
}
 
void video_legacy_create_gui_gradient(GUI_COLOR* start, const GUI_COLOR* end, const int slices)
{
   if(slices) {
      gradient_start[0] = start->r * GRADIENT_MULTIPLIER;
      gradient_start[1] = start->g * GRADIENT_MULTIPLIER;
      gradient_start[2] = start->b * GRADIENT_MULTIPLIER;

      gradient_end[0] = end->r * GRADIENT_MULTIPLIER;
      gradient_end[1] = end->g * GRADIENT_MULTIPLIER;
      gradient_end[2] = end->b * GRADIENT_MULTIPLIER;

      gradient_delta[0] = (gradient_end[0] - gradient_start[0]) / slices;
      gradient_delta[1] = (gradient_end[1] - gradient_start[1]) / slices;
      gradient_delta[2] = (gradient_end[2] - gradient_start[2]) / slices;

      gradient_slice = 0;
   }
   else {
      start->r = (gradient_start[0] + (gradient_delta[0] * gradient_slice)) / GRADIENT_MULTIPLIER;
      start->g = (gradient_start[1] + (gradient_delta[1] * gradient_slice)) / GRADIENT_MULTIPLIER;
      start->b = (gradient_start[2] + (gradient_delta[2] * gradient_slice)) / GRADIENT_MULTIPLIER;

      gradient_slice++;
   }
}

/* This is like Allegro's textout(), but it uses slow putpixels, so it supports translucent drawing.
   This is used by video_legacy_shadow_textout(). */
void video_legacy_translucent_textout(BITMAP* bitmap, FONT* font, const UDATA* text, const int x, const int y, const int color)
{
   RT_ASSERT(bitmap);
   RT_ASSERT(font);
   RT_ASSERT(text);

   const int width = text_length(font, text);
   const int height = text_height(font);

   BITMAP* buffer = create_bitmap(width, height);
   if(!buffer) {
      WARN_GENERIC();
      return;
   }

   // For our mask color, we'll use the exact inverse of color.
   clear_to_color(buffer, ~color);

   textout_ex(buffer, font, text, 0, 0, color, -1);

   for(int subY = 0; subY < height; subY++) {
      for(int subX = 0; subX < width; subX++) {
         const int pixel = getpixel(buffer, subX, subY);
         if(pixel == ~color)
            continue;

         putpixel(bitmap, x + subX, y + subY, pixel);
      }
   }

   destroy_bitmap(buffer);
}

// Draws text with a shadow. This is designed to be used with DRAW_MODE_TRANS.
void video_legacy_shadow_textout(BITMAP* bitmap, FONT* font, const UDATA* text, const int x, const int y, const int color, const int opacity)
{
   RT_ASSERT(bitmap);
   RT_ASSERT(font);
   RT_ASSERT(text);

   // If we're drawing with the smallest font, we can literally use a shadowed version.
   const int black = makecol_depth(bitmap_color_depth(bitmap), 0, 0, 0);

   if(font == video_get_font(VIDEO_FONT_SHADOWED)) {
      FONT *shadowFont = video_get_font(VIDEO_FONT_SHADOWED_BOTTOM);
      if(!shadowFont) {
          WARN_GENERIC();
          return;
      }

      set_trans_blender(255, 255, 255, opacity / 2);
      video_legacy_translucent_textout(bitmap, shadowFont, text, x + 1, y + 1, black);

      set_trans_blender(255, 255, 255, opacity);
      video_legacy_translucent_textout(bitmap, shadowFont, text, x, y, black);
      video_legacy_translucent_textout(bitmap, font, text, x, y, color);
   }
   else {
      // Just draw a plain drop shadow instead.
      video_legacy_translucent_textout(bitmap, font, text, x + 1, y + 1, black);
      video_legacy_translucent_textout(bitmap, font, text, x, y, color);
   }
}

void video_legacy_shadow_textprintf(BITMAP* bitmap, FONT* font, const int x, const int y, const int color, const int opacity, const UDATA* text, ...)
{
   RT_ASSERT(bitmap);
   RT_ASSERT(font);
   RT_ASSERT(text);

   va_list format;
   USTRING buffer;

   va_start(format, text);
   uvszprintf(buffer, sizeof(buffer), text, format);
   va_end(format);

   video_legacy_shadow_textout(bitmap, font, buffer, x, y, color, opacity);
}

// --------------------------------------------------------------------------------

static bool Initialize()
{
   /* Initialize everything.
      Note: Do not use memset() here, as that would clear our configuration too. */
   Buffers.display = NULL;
   Buffers.blit = NULL;
   Buffers.extra = NULL;
   Buffers.filter = NULL;
   Buffers.overlay = NULL;
   Buffers.render = NULL;

   for(int i = 0; i < MaximumPages; i++)
      Buffers.pages[i] = NULL;

   Buffers.pageCount = 0;
   Buffers.currentPage = 0;

   Display.indexed = false;
   Display.swapRGB = false;
   Display.doubleBuffer = false;

   Fonts.smallLow = NULL;
   Fonts.mediumLow = NULL;
   Fonts.largeLow = NULL;
   Fonts.smallHigh = NULL;
   Fonts.mediumHigh = NULL;
   Fonts.largeHigh = NULL;
   Fonts.legacy = NULL;
   Fonts.monolithic = NULL;
   Fonts.shadowedBottom = NULL;
   Fonts.shadowedTop = NULL;
   Fonts.custom = NULL;

   Output.width = 0;
   Output.height = 0;

   memset(&Display.palette, 0, sizeof(PALETTE));
   memset(&Display.rgbMap, 0, sizeof(RGB_MAP));

   // Display miscellaneous information.
   const string nothing = "";

   string capabilities = nothing;
   capabilities += (cpu_capabilities & CPU_ID)       ? "CPUID "    : nothing;
   capabilities += (cpu_capabilities & CPU_FPU)      ? "FPU "      : nothing;
   capabilities += (cpu_capabilities & CPU_IA64)     ? "IA64 "     : nothing;
   capabilities += (cpu_capabilities & CPU_AMD64)    ? "AMD64 "    : nothing;
   capabilities += (cpu_capabilities & CPU_MMX)      ? "MMX "      : nothing;
   capabilities += (cpu_capabilities & CPU_MMXPLUS)  ? "MMX+ "     : nothing;
   capabilities += (cpu_capabilities & CPU_SSE)      ? "SSE "      : nothing;
   capabilities += (cpu_capabilities & CPU_SSE2)     ? "SSE2 "     : nothing;
   capabilities += (cpu_capabilities & CPU_SSE3)     ? "SSE3 "     : nothing;
   capabilities += (cpu_capabilities & CPU_3DNOW)    ? "3DNOW "    : nothing;
   capabilities += (cpu_capabilities & CPU_ENH3DNOW) ? "ENH3DNOW " : nothing;
   capabilities += (cpu_capabilities & CPU_CMOV)     ? "CMOV "     : nothing;
      
   log_printf("CPU vendor: %s, family %d, model %d\n"
              "Capabilities: %s\n",
              cpu_vendor, cpu_family, cpu_model, capabilities.c_str()); 

   if(os_multitasking)
      log_printf("The operating system is capable of multitasking.\n");

   // Attempt to detect a windowing environment.

   bool haveDesktop = false;
   int desktopWidth, desktopHeight;
   if(get_desktop_resolution(&desktopWidth, &desktopHeight) == 0) {
      if((desktopWidth > 0) && (desktopHeight > 0)) {
         log_printf("A windowing environment was found:\n"
                    "The desktop area appears to be %dx%d pixels.\n",
                    desktopWidth, desktopHeight);

         haveDesktop = true;
      }
   }

   // Determine which driver to use, based upon various parameters.
   if((Display.driver == GFX_AUTODETECT) ||
      (Display.driver == GFX_SAFE)) {
      /* Normally we use GFX_SAFE, but if we're trying to use hardware acceleration, or enter
         a fullscreen mode, we need to switch to GFX_AUTODETECT. */
      if((Display.driver == GFX_SAFE) &&
            (Options.enableAcceleration || Options.enableFullscreen)) {
         log_printf("Either fullscreen or hardware acceleration is requested, disabling safe mode.\n");
         Display.driver = GFX_AUTODETECT;
      }

      // For GFX_AUTODETECT, determine just which automatic driver to use.
      if(Display.driver == GFX_AUTODETECT) {
         /* Full screen is the default choice for AUTODETECT, so we have to force windowed.
            This only works in windowed environments. */
         if(haveDesktop && !Options.enableFullscreen) {
            log_printf("Defaulting to a windowed display, due to the presence of a desktop.\n"
                       "(You can override this by using the fullscreen option.)\n");

            Display.driver = GFX_AUTODETECT_WINDOWED;
         }

#ifdef USE_ALLEGROGL
         // If hardware acceleration is enabled, we want to try to set an OpenGL mode.
         if(Options.enableAcceleration) {
            log_printf("Hardware acceleration has been requested. OpenGL will be used if possible.\n");

            // If no windowing environment is available, just go with the default choice.
            if(!haveDesktop)
               Display.driver = GFX_OPENGL;
            else if(Options.enableFullscreen)
               Display.driver = GFX_OPENGL_FULLSCREEN;
            else
               Display.driver = GFX_OPENGL_WINDOWED;
         }
#endif
      }
   }

#ifdef USE_ALLEGROGL
   /* If we're trying to set an OpenGL mode, we first have to install AllegroGL. */
   if((Display.driver == GFX_OPENGL) ||
      (Display.driver == GFX_OPENGL_FULLSCREEN) ||
      (Display.driver == GFX_OPENGL_WINDOWED)) {
      log_printf("OpenGL support was included in this build. Enabling it.\n");

      // Install AllegroGL.
      install_allegro_gl();

      /* Due to a bug in AllegroGL, we must make sure to remove it before accessing any other
         video modes.  However, we must set this flag to know that we've installed it,
         because there doesn't appear to be any other way to tell. */
      alglInstalled = true;

      // Hint at which modes we want for OpenGL.
      allegro_gl_set(AGL_FULLSCREEN, Options.enableFullscreen);
      allegro_gl_set(AGL_WINDOWED, Options.enableFullscreen);

      // If we'll be setting a windowed mode, we can try to center it on the screen.
      if(haveDesktop) {
         const int x = (desktopWidth / 2) - (Display.width / 2);
         const int y = (desktopHeight / 2) - (Display.height / 2);

         log_printf("The OpenGL window will be placed at %d,%d on the desktop.\n", x, y);

         allegro_gl_set(AGL_WINDOW_X, x);
         allegro_gl_set(AGL_WINDOW_Y, y);
      }
      else {
         // Probably don't need to do this, but better safe than sorry.
         allegro_gl_set(AGL_WINDOW_X, 0);
         allegro_gl_set(AGL_WINDOW_Y, 0);
      }

      // Tell AllegroGL what hints we want to use.
      allegro_gl_set(AGL_SUGGEST, AGL_FULLSCREEN | AGL_WINDOWED | AGL_WINDOW_X | AGL_WINDOW_Y | AGL_COLOR_DEPTH);
   }
#endif

   if(Display.colorDepth == -1) {
      // See if we can get the color depth from the desktop.
      if(haveDesktop) {
         const int depth = desktop_color_depth();
         if(depth > 0) {
            log_printf("Auto-detected a desktop color depth of %d bits-per-pixel. Using it.\n", depth);
            Display.colorDepth = depth;
         }
      }

      /* If there is no windowing environment is available to autodetect the color depth from,
      we have to set a sensible default. This generally only happens in DOS. */
      if(Display.colorDepth == -1) {
         log_printf("Using a default display color depth of 8 bits-per-pixel.\n");
         Display.colorDepth = 8;
      }
   }

   // Make sure we aren't trying to set an invalid color depth.
   if((Display.colorDepth != 8) &&
      (Display.colorDepth != 15) &&
      (Display.colorDepth != 16) &&
      (Display.colorDepth != 24) &&
      (Display.colorDepth != 32)) {
      log_printf("Display initialization error:\n"
                 "\tAn invalid display color depth was specified (%d).\n"
                 "\tValid color depths are 8, 15, 16, 24, and 32, depending on the environment.\n",
                 Display.colorDepth);

      WARN("Invalid display color depth. Check the log for details.");
      Exit();
      return false;
   }

   // Now we can request the desired color depth.
   set_color_depth(Display.colorDepth);

   log_printf("Will now attempt to set the display mode!\n");

   // Attempt to enter a graphics mode with the specified parameters.
   if(set_gfx_mode(Display.driver, Display.width, Display.height, 0, 0) != 0) {
      log_printf("Display initialization error:\n"
                 "\tIt seems that the call to set_gfx_mode() failed for some reason.\n"
                 "\tNote that this could be due to invalid display settings on the user's part.\n"
                 "\tAllegro reports the following error: %s\n",
                 allegro_error);

      WARN("Couldn't set the display mode. Check the log for details.");
      Exit();
      return false;
   }

   log_printf("Successfully set display mode (%dx%d, %d bits-per-pixel, %s).\n",
              SCREEN_W, SCREEN_H, bitmap_color_depth(screen), gfx_driver->name);

   // Synchronize our parameteres with the one that were actually used.
   Display.driver = gfx_driver->id;
   Display.width = SCREEN_W;
   Display.height = SCREEN_H;
   Display.colorDepth = bitmap_color_depth(screen);

   // If we set an OpenGL mode, then we need to configure it.
   if(IsOpenGL()) {
      log_printf("This is an OpenGL mode. Configuring graphics pipeline.\n");

      if(!SetupOpenGL()) {
         log_printf("OpenGL was unable to initialize. This is an unrecoverable error.\n");

         WARN("OpenGL was unable to initialize. Check the log for details.");
         Exit();
         return false;
      }
   }

   /* Check if we're running in an indexed color mode, in which case we need to manage a palette
      and a 15-bit colormap for quick color conversions. */
   Display.indexed = (Display.colorDepth == 8);

   // Some special setup is required for indexed color modes.
   if(Display.indexed) {
       /* Calculate and set an RGB format color palette. This essentially makes an 8-bit
          truecolor mode, allowing all later calculations to work under that assumption. Note
          that Allegro palettes expect components to be in the 0-63 range. */
       int index = 1;
       for(int s = 0; s < 9; s++) {
          Display.palette[index].r = Round(s * (63 / 8.f));
          Display.palette[index].g = Round(s * (63 / 8.f));
          Display.palette[index].b = Round(s * (63 / 8.f));
          index++;
       }

       for(int r = 0; r < 6; r++) {
          for(int g = 0; g < 7; g++) {
             for(int b = 0; b < 6; b++) {
                if((r == b) && (b == g))
                   continue;

                Display.palette[index].r = Round(r * (63 / 5.f));
                Display.palette[index].g = Round(g * (63 / 6.f));
                Display.palette[index].b = Round(b * (63 / 5.f));
                index++;
             }
          }
       }

       log_printf("Display is running in an indexed color mode, setting palette.\n");
       set_palette(Display.palette);

       // Calculate and set a 15-bit color map for fast color conversions.
       create_rgb_table(&Display.rgbMap, Display.palette, NULL);
       rgb_map = &Display.rgbMap;
   }

   /* Enable color conversion so the 16-bit buffers used by the emulator will automatically be
      converted to the screen depth. This affects many other things, too. */
   if(Options.enableDither) {
      log_printf("Dithering will be used during conversions between color depths.\n");
      set_color_conversion(COLORCONV_TOTAL | COLORCONV_KEEP_TRANS | COLORCONV_DITHER);
   }
   else
      set_color_conversion(COLORCONV_TOTAL | COLORCONV_KEEP_TRANS);

   // Attempt to detect whether color component swapping is needed.
   if(IsOpenGL()) {
      // OpenGL always requires RGB<>BGR color swapping.
      Display.swapRGB = true;
   }
   else {
      /* Attempt to detect if color component swapping (R<>B) is required. There are two
         notable cases where we have to swap red and blue:
            1) The format of color_pack_16() and makecol16() do not match.
            2) The display itself is BGR, when we need RGB. */
      video__swap_rgb = FALSE;
      const int c1 = color_pack_16(0, 0, 255);
      const int c2 = makecol16(0, 0, 255);
      const int c3 = makecol(0, 0, 255);

      if((c1 & 1) != (c2 & 1)) {
         log_printf("The 16-bit pixel format is BGR. Converting from RGB.\n");
         Display.swapRGB = true;
      }

      // 0 = BGR, 1 = RGB
      if((c3 & 1) == 0) {
         log_printf("The display has been detected as BGR. Converting from RGB.\n");
         Display.swapRGB = true;
      }
   }

   if(Display.swapRGB)
      log_printf("The display has been detected as BGR. Converting from RGB.\n");

   video__swap_rgb = Display.swapRGB;

   // We have all the information needed now to set up the color system.
   UpdateColor();

   Buffers.display = create_bitmap(SCREEN_W, SCREEN_H);
   if(!Buffers.display) {
      log_printf("Display initialization error:\n",
                 "\tThe display buffer couldn't be created. This is an unrecoverable error.\n",
                 "\tCheck and make sure that your system is not out of memory.\n");

      WARN("Couldn't create the display buffer. Check the log for details.");
      Exit();
      return false;
   }

   // Set up the render, overlay and display buffers.
   Buffers.render = create_bitmap_ex(16, 256, 240);
   if(!Buffers.render) {
      log_printf("Display initialization error:\n",
                 "\tThe rendering buffer couldn't be created. This is an unrecoverable error.\n",
                 "\tCheck and make sure that your system is not out of memory.\n");

      WARN("Couldn't create the rendering buffer. Check the log for details.");
      Exit();
      return false;
   }

   clear_bitmap(Buffers.display);
   clear_bitmap(Buffers.render);
   
   // By default, the overlay buffer just points to the render buffer.
   Buffers.overlay = Buffers.render;

   // Attempt to enable non-OpenGL hardware acceleration (e.g DDraw) if requested.
   if(Options.enableAcceleration && !IsOpenGL()) {
      log_printf("Hardware acceleration was requested, but this is not an OpenGL mode.\n");

      // Attempt to enable triple buffering.
      if(!(gfx_capabilities & GFX_CAN_TRIPLE_BUFFER)) {
         log_printf("Attempting to forcefully enable triple buffering.\n");
         enable_triple_buffer();
      }

      // Determine how many pages to allocate, based upon hardware capabilities.
      int count;
      if(gfx_capabilities & GFX_CAN_TRIPLE_BUFFER) {
         log_printf("This system supports triple buffering. Using it.\n");
         count = PagesForTripleBuffering;
      }
      else {
         log_printf("Triple buffering isn't available. Will use page flipping instead.\n");
         count = PagesForPageFlipping;
      }

      for(int i = 0; i < count; i++) {
         BITMAP* page = create_video_bitmap(SCREEN_W, SCREEN_H);
         if(page) {
            Buffers.pages[i] = page;
            Buffers.pageCount++;
         }
         else {
            // Break out of the loop.
            i = MaximumPages;
         }
      }

      if(Buffers.pageCount < PagesForPageFlipping) {
         log_printf("Not enough video memory could be allocated. Disabling hardware acceleration.\n"
                    "(This can happen if you are running in an unaccelerated environment like X Windows.)\n");

         for(int i = 0; i < MaximumPages; i++) {
            if(Buffers.pages[i]) {
               destroy_bitmap(Buffers.pages[i]);
               Buffers.pages[i] = NULL;
            }
         }

         Buffers.pageCount = 0;
      }
      else {
         log_printf("Allocated %d video memory pages for hardware acceleration.\n",
                    Buffers.pageCount);
      }

      if(Buffers.pageCount == PagesForPageFlipping)
         log_printf("Page flipping is now enabled.\n");
      else if(Buffers.pageCount == PagesForTripleBuffering)
         log_printf("Triple buffering is now enabled.\n");
   }

   // Now is probably a good time to load the fonts.
   LoadFonts();

   // One last thing to handle is the callbacks for switching to and from the program.
   if(is_windowed_mode()) {
      set_display_switch_mode(SWITCH_BACKGROUND);
   }
   else {
      set_display_switch_mode(SWITCH_AMNESIA);
      set_display_switch_callback(SWITCH_IN, SwitchBack);
      set_display_switch_callback(SWITCH_OUT, SwitchAway);
   }

   // All done. Return with sweet success.
   log_printf("Display initialization was successful.\n");
   return true;
}

static void Exit()
{
   // Destroy all bitmaps.
   FreeBitmap(Buffers.display);
   FreeBitmap(Buffers.blit);
   FreeBitmap(Buffers.extra);
   FreeBitmap(Buffers.filter);
   // FreeBitmap(Buffers.overlay);
   FreeBitmap(Buffers.render);

   for(int i = 0; i < MaximumPages; i++) 
      FreeBitmap(Buffers.pages[i]);

   Buffers.pageCount = 0;

   // Remove callbacks.
   remove_display_switch_callback(SwitchAway);
   remove_display_switch_callback(SwitchBack);

   if(IsOpenGL())
      ExitOpenGL();

#ifdef USE_ALLEGROGL
   // Remove AllegroGL and restore Allegro GFX drivers.
   if(alglInstalled) {
      remove_allegro_gl();
      alglInstalled = false;
   }
#endif

#ifdef ALLEGRO_DOS
   // Return to text mode.
   set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
#endif
}

static bool SetupOpenGL()
{
#ifdef USE_ALLEGROGL
   // Clear front and back buffers.
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);
   allegro_gl_flip();
   glClear(GL_COLOR_BUFFER_BIT);
   allegro_gl_flip();

   // Set up a standard 2D projection matrix.
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0.0, (SCREEN_W - 1), (SCREEN_H - 1), 0.0);

   // Create OpenGL display texture.
   glGenTextures(1, &displayTexture);
   if(!displayTexture) {
      log_printf("OpenGL initialization error:\n",
                  "\tIt seems the OpenGL display texture couldn't be allocated\n"
                  "\tEither OpenGL is not working properly, or you are out of graphics memory.\n"
                  "\tAllegro says: %s\n",
                  allegro_error);

      WARN("Couldn't create OpenGL display texture. Check log for details");
      return false;
   }

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, displayTexture);

   if(allegro_gl_opengl_version() >= 1.2) {
      log_printf("Found OpenGL version 1.2 or higher.\n"
                 "Using GL_CLAMP_TO_EDGE for texture edge clamping.\n");

      /* OpenGL 1.2 and higher supports GL_CLAMP_TO_EDGE as part of the core API,
         so we can just use it directly. */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }
   else if(allegro_gl_is_extension_supported("GL_EXT_texture_edge_clamp")) {
      log_printf("Found GL_EXT_texture_edge_clamp"
                 "Using GL_CLAMP_TO_EDGE_EXT for texture edge clamping.\n");

      /* We manually define GL_CLAMP_TO_EDGE_EXT and use that, since the implementation
         reports that texture edge clamping is supported. */
#define GL_CLAMP_TO_EDGE_EXT2 (GLenum)0x812F
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT2);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT2);
   }
   else {
      log_printf("No texture edge clamping available. Using GL_CLAMP instead.\n");

      /* OpenGL 1.1 without texture edge clamping.  In this case, linear filtering may
         result in noticeable border artifacts. */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   }

   // Create OpenGL display list.
   displayList = glGenLists(1);
   if(!displayList) {
      log_printf("OpenGL initialization error:\n",
                  "\tIt seems the OpenGL display list couldn't be created\n"
                  "\tEither OpenGL is not working properly, or you are out of graphics memory.\n"
                  "\tAllegro says: %s\n",
                  allegro_error);

      WARN("Couldn't create OpenGL display list. Check log for details");
      return false;
   }

   if(Options.enableDither) {
      log_printf("Dithering was requested, so enabling it.\n");
      glEnable(GL_DITHER);
   }

   // This probably isn't needed, but it doesn't hurt.
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
 
   return true;
#endif
}

static void ExitOpenGL()
{
#ifdef USE_ALLEGROGL
   if(displayTexture) {
      glDeleteTextures(1, &displayTexture);
      displayTexture = 0;
   }

   if(displayList) {
      glDeleteLists(displayList, 1);
      displayList = 0;
   }
#endif
}

static bool IsOpenGL() {
#ifdef USE_ALLEGROGL
   if(!gfx_driver)
      return FALSE;

   return (gfx_driver->id == GFX_OPENGL) ||
          (gfx_driver->id == GFX_OPENGL_FULLSCREEN) || 
          (gfx_driver->id == GFX_OPENGL_WINDOWED);
#endif

   return FALSE;
}

static void LoadFonts()
{
   // Low-resolution fonts.
   Fonts.smallLow = DATA_TO_FONT(FONT_SMALL_LOW);
   Fonts.mediumLow = DATA_TO_FONT(FONT_MEDIUM_LOW);
   Fonts.largeLow = DATA_TO_FONT(FONT_LARGE_LOW);

   // High-resolution fonts.
   Fonts.smallHigh = DATA_TO_FONT(FONT_SMALL_HIGH);
   Fonts.mediumHigh = DATA_TO_FONT(FONT_MEDIUM_HIGH);
   Fonts.largeHigh = DATA_TO_FONT(FONT_LARGE_HIGH);

   // Monolithic and legacy fonts.
   // Fonts.monolithic = DATA_TO_FONT(FONT_MONOLITHIC);
   Fonts.legacy = DATA_TO_FONT(FONT_LEGACY);
   Fonts.monolithic = NULL;
   Fonts.shadowedBottom = DATA_TO_FONT(FONT_SHADOWED_BOTTOM);
   Fonts.shadowedTop = DATA_TO_FONT(FONT_SHADOWED_TOP);

   // Custom font (not yet supported).
   Fonts.custom = NULL;
}

static void UpdateColor()
{
   // Determine which palette to use.
   RGB* palette = NULL;
   switch(Color.palette) {
      case VIDEO_PALETTE_NESTER:
         palette = DATA_TO_RGB(PALETTE_NESTER);
         break;
      case VIDEO_PALETTE_NESTICLE:
         palette = DATA_TO_RGB(PALETTE_NESTICLE);
         break;
      case VIDEO_PALETTE_NTSC:
         palette = DATA_TO_RGB(PALETTE_NTSC);
         break;
      case VIDEO_PALETTE_PAL:
         palette = DATA_TO_RGB(PALETTE_PAL);
         break;
      case VIDEO_PALETTE_RGB:
         palette = DATA_TO_RGB(PALETTE_RGB);
         break;

      default:
         WARN_GENERIC();
         break;
   }

   for(int i = 0; i < PPU__COLOR_MAP_SIZE; i++) {
      // Get our color from the palette. For legacy reasons, they are offset by one.
      const RGB& color = palette[i + 1];

      // First we have to map from Allegro's 0-63 to the normal 0-255 range.
      int r = color.r << 2;
      int g = color.g << 2;
      int b = color.b << 2;

      // Convert from RGB to HSL.
      real h, s, l;
      rgb_to_hsl(r, g, b, &h, &s, &l);

      // Convert color controls from [-100,100] range to [-1,1].
      const real hue = Color.hue / 100;
      const real saturation = Color.saturation / 100;
      const real brightness = Color.brightness / 100;
      const real contrast = Color.contrast / 100;
      const real gamma = Color.gamma / 100;

      // Apply hue control.
      if(hue)
         h = fixf(h + (hue / 2), 0, 1);

      // Apply saturation control.
      if(saturation)
         s = fixf(s + saturation, 0, 1);

      // Convert back to RGB.
      hsl_to_rgb(h, s, l, &r, &g, &b);

      // Convert to normalized floating point.
      real R = r / 255.0;
      real G = g / 255.0;
      real B = b / 255.0;

      // Apply oversaturation.
      if(saturation > 0.0) {
         const real average = (R + G + B) / 3;
         R += (R - average) * saturation * average;
         G += (G - average) * saturation * average;
         B += (B - average) * saturation * average;
      }
      
      // Apply contrast control.
      if(contrast) {
         real scale;
         if(contrast > 0.0)
            scale = 1.0 + (contrast * 2);
         else
            scale = 1.0 + (contrast / 2);

         R = 0.5 + ((R - 0.5) * scale);
         G = 0.5 + ((G - 0.5) * scale);
         B = 0.5 + ((B - 0.5) * scale);
      }

      // Apply gamma control.
      if(gamma) {
         const real scale = 1.0 + gamma;
         R *= scale;
         G *= scale;
         B *= scale;
      }

      // Apply brightness control.
      if(brightness) {
         const real gain = brightness / 2;
         R += gain;
         G += gain;
         B += gain;
      }

      // Clip values down to a fixed range.
      R = fixf(R, 0, 1);
      G = fixf(G, 0, 1);
      B = fixf(B, 0, 1);

      // Convert back to integer.
      r = Round(R * 255.0);
      g = Round(G * 255.0);
      b = Round(B * 255.0);

      // Build 16-bit packed color value and send it to the PPU for color mapping.
      ppu_map_color(i, color_pack_16(r, g, b));
   }
}

static void UpdateDisplay()
{
   BITMAP* source = Buffers.render;
   int sourceWidth = source->w;
   int sourceHeight = source->h;

   if(Output.scale) {
      /* Allegro's stretch_blit can't convert between different color formats. The best way around this
         is to blit to a temporary buffer to do so, although that is slow. Oh well. */
      if(Display.colorDepth != bitmap_color_depth(source)) {
         BITMAP* buffer = video_get_extra_buffer(sourceWidth, sourceHeight);
         if(buffer) {
            blit(source, buffer, 0, 0, 0, 0, sourceWidth, sourceHeight);

            source = buffer;
            sourceWidth = source->w;
            sourceHeight = source->h;
         }
      }

      // Calculate the size of the scaled image.
      const int width = Round(sourceWidth * (Output.scaleWidth / 100));
      const int height = Round(sourceHeight * (Output.scaleHeight / 100));

      // Calculate where to place the scaled image on the screen.
      const int x = (Display.width / 2) - (width / 2);
      const int y = (Display.height / 2) - (height / 2);

      // Scale the image and place it on the screen.
      acquire_screen();
      stretch_blit(source, screen, 0, 0, sourceWidth, sourceHeight, x, y, width, height);
      release_screen();
   }
   else {
      // Calculate where to place thed image on the screen.
      const int x = (Display.width / 2) - (sourceWidth / 2);
      const int y = (Display.height / 2) - (sourceHeight / 2);

      // Place the image on the screen.
      acquire_screen();
      blit(source, screen, 0, 0, 0, 0, sourceWidth, sourceHeight);
      release_screen();
   }

   UpdateScreen();
}

static void UpdateScreen()
{
   // If we're in an OpenGL mode, we'll defer to a dedicated pipeline.
   if(IsOpenGL()) {
      UpdateScreenOpenGL();
      return;
   }

   if(Display.doubleBuffer) {
      acquire_screen();
      blit(Buffers.display, screen, 0, 0, 0, 0, Buffers.display->w, Buffers.display->h);
      release_screen();
   }
}

static void UpdateScreenOpenGL()
{
}

static void DrawLightgunCursor()
{
   BITMAP* cursor = DATA_TO_BITMAP(CURSOR_TARGET);
   const int width = cursor->w;
   const int height = cursor->h;
   const int depth = bitmap_color_depth(Buffers.overlay);

   BITMAP* scratch = create_bitmap_ex(depth, width, height);
   if(scratch) {
      blit(cursor, scratch, 0, 0, 0, 0, width, height);
      draw_sprite(Buffers.overlay, scratch,
         input_zapper_x_offset - (width / 2), input_zapper_y_offset - (height / 2));

      destroy_bitmap(scratch);
   }
}

static void DrawHUD()
{
   // The bitmap that we will draw onto.
   BITMAP* buffer = Buffers.overlay;

   // The font that we will use when drawing the text.
   FONT* font = video_get_font(VIDEO_FONT_SHADOWED);

   // The color to use for text.
   const uint16 color = ppu__color_map[0x29];
   const int opacity = 240;

   // Initial offsets.
   const int left = 160;
   const int top = 16;
   int y = top;

   // Calculate whitespace.
   const int indent = text_length(font, "X");
   const int line = Round(text_height(font) * 1.25);
   const int spacer = Round(line * 1.5);

   drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);

   // Not the prettiest code, that's for sure. ;)
   if(game_clock_days > 0) {
      video_legacy_shadow_textprintf(buffer, font, left, y, color, opacity, "%03d:%02d:%02d:%02d.%d",
         game_clock_days, game_clock_hours, game_clock_minutes, game_clock_seconds, game_clock_milliseconds / 100);
   }
   else {
      video_legacy_shadow_textprintf(buffer, font, left, y, color, opacity, "%02d:%02d:%02d.%d",
         game_clock_hours, game_clock_minutes, game_clock_seconds, game_clock_milliseconds / 100);
   }

   y += spacer;

   video_legacy_shadow_textout(buffer, font, "Video:", left, y, color, opacity);
   y += line;

   video_legacy_shadow_textprintf(buffer, font, left + indent, y, color, opacity,
      "%02d FPS", timing_fps);
   y += line;

   video_legacy_shadow_textprintf(buffer, font, left + indent, y, color, opacity,
      "Dropped %d%%", Round((1.0 - (rendered_frames / (real)executed_frames)) * 100));
   y += spacer;

   video_legacy_shadow_textout(buffer, font, "Audio:", left, y, color, opacity);
   y += line;

   if(audio_options.enable_output) {
      video_legacy_shadow_textprintf(buffer, font, left + indent, y, color, opacity,
         "%2.1f kHz", timing_audio_fps / 1000.0);

      y += spacer;

      set_trans_blender(255, 255, 255, opacity);

      REAL* vis = apu_get_visdata();
      if(vis) {
         int offset = 0;
         for(int i = 0; i < APU_VISDATA_ENTRIES; i++) {
            if((i == APU_VISDATA_MASTER_1) || (i == APU_VISDATA_MASTER_2))
               continue;

            const int x1 = left + indent + offset;
            const int y1 = y + ((1.0 - fabs(vis[i])) * 8);
            const int x2 = x1 + 6;
            const int y2 = y + 8;

            const int black = makecol_depth(bitmap_color_depth(buffer), 0, 0, 0);

            rectfill(buffer, x1 + 2, y1 + 2, x2 + 2, y2 + 2, black);
            rectfill(buffer, x1 + 1, y1 + 1, x2 - 1, y2 - 1, color);
            rect(buffer, x1, y1, x2, y2, black);
            offset += 12;
         }

         delete[] vis;
      }

      y += 2;
   }
   else
      video_legacy_shadow_textout(buffer, font, "Disabled", left + indent, y, color, opacity);

   y += spacer;

   video_legacy_shadow_textout(buffer, font, "Timing:",  left, y, color, opacity);
   y += line;

   video_legacy_shadow_textprintf(buffer, font, left + indent, y, color, opacity,
      "%s", (machine_type == MACHINE_TYPE_NTSC) ? "NTSC Mode" : "PAL Mode");
   y += line;

   video_legacy_shadow_textprintf(buffer, font, left + indent, y, color, opacity,
      "%02d/%g Hz", timing_hertz, (double)timing_get_frame_rate());
   y += line + 2;

   video_legacy_shadow_textprintf(buffer, font, left + indent, y, color, opacity,
      "PC: $%04X", cpu_get_register(CPU_REGISTER_PC));

   solid_mode();
}

static void DrawMessages()
{
   // The bitmap that we will draw onto.
   BITMAP* buffer = Buffers.overlay;

   // The font that we will use when drawing the text.
   FONT* font = video_get_font(VIDEO_FONT_SHADOWED);

   // Determine colors to use for drawing.
   const uint16 backgroundColor = ppu__color_map[video_search_palette_rgb(64, 128, 128)];
   const uint16 borderColor = ppu__color_map[video_search_palette_rgb(255, 255, 192)];
   const uint16 shadowColor = ppu__color_map[video_search_palette_rgb(64, 64, 64)];
   const uint16 textColor = ppu__color_map[video_search_palette_rgb(255, 255, 255)];

   /* Gather our text sources. Generally, this will include only the message history,
      but in chat mode it may also include the current chat line(s). */
   vector<const UDATA*> sources;
   vector<int> intensities;

   const int maxSources = history.size() + 2;
   sources.resize(maxSources);
   intensities.resize(maxSources);

   int nextSource = 0;

   for(int i = 0; i < maxSources; i++) {
      sources[i] = NULL;
      intensities[i] = 255;
   }

   // Determine if we are in full chat mode.
   const bool chatMode = input_mode & INPUT_MODE_CHAT;

   for(extent i = 0; i < history.size(); i++) {
      const Message& message = history[i];
      // Skip empty lines for a more compact layout.
      if(ustrlen(message.text) == 0)
         continue;

      int intensity = chatMode ? 255 : 240;
      if(!chatMode) {
         // If the message duration has expired, hide it.
         if(message.duration == 0)
            continue;

         // Handle fade-out.
         if(message.duration < message.fadeTime)
            intensity = Round((message.duration / (real)message.fadeTime) * 255);
      }

      sources[nextSource] = message.text;
      intensities[nextSource] = intensity;
      nextSource++;
   }

   USTRING chat;
   if(chatMode) {
      // Separate the current chat line from the message history.
      if(nextSource > 0) {
         sources[nextSource] = "-";
         nextSource++;
      }

      // To let the user know that they are typing, we have to add an underscore to it.
      USTRING_CLEAR(chat);
      ustrncat(chat, input_chat_text, USTRING_SIZE - 1);
      ustrncat(chat, "_",  USTRING_SIZE - 1);

      sources[nextSource] = chat;
      nextSource++;
   }

   // Handle word-wrapping.
   const int maxWidth = 33;	// In characters.
   const int maxHeight = 20;	// In lines.
   USTRING lines[maxHeight];
   int opacities[maxHeight];
   int height = 0;

   for(int i = 0; i < maxHeight; i++)
      USTRING_CLEAR(lines[i]);

   for(int i = 0; i < maxSources; i++) {
      const UDATA* text = sources[i];
      if(!text)
         continue;

      // Cache the intensity for use as an opacity value.
      const int intensity = intensities[i];

      USTRING line, word;
      USTRING_CLEAR(line);
      USTRING_CLEAR(word);

      const int length = ustrlen(text);
      for(int j = 0; j < length; j++) {
         const UCCHAR c = ugetat(text, j);

         /* Check if this is a normal character or whitespace.  We want to avoid processing whitespace
            until after we've determined if the line will be wrapped. */
         if(!uisspace(c))
            uinsert(word, ustrlen(word), c);

         bool newLine = false, wrap = false;

         // If we're at the end of the line, we obviously want to start a new one.
         if(j == (length - 1))
            newLine = true;

         // If we've exceeded the maximum line length, start a new line.
         if((ustrlen(line) + ustrlen(word)) > maxWidth) {
            newLine = true;
            wrap = true;
         }

         // If there is no whitespace to split the line on, we have to force it.
         if(ustrlen(word) >= maxWidth) {
            ustrncat(line, word, USTRING_SIZE - 1);
            USTRING_CLEAR(word);

            newLine = true;
            wrap = true;
         }
         
         if(newLine) {
            // Check if we've exceeded the height limit.
            if(height >= maxHeight) {
               // Drop oldest line to make room for the newest one.
               for(int k = 0; k < (maxHeight - 1); k++) {
                  USTRING_CLEAR(lines[k]);
                  ustrncat(lines[k], lines[k + 1], USTRING_SIZE - 1);
                  opacities[k] = opacities[k + 1];
               }

               height--;
               USTRING_CLEAR(lines[height]);
            }

            // If we aren't wrapping, we need to make sure the line is complete.
            if(!wrap && (ustrlen(word) > 0)) {
               ustrncat(line, word, USTRING_SIZE - 1);
               USTRING_CLEAR(word);
            }

            ustrncat(lines[height], line, USTRING_SIZE - 1);
            opacities[height] = intensity;
            height++;

            USTRING_CLEAR(line);
         }

         // Check if we've reached the end of the word.
         if(uisspace(c)) {
            ustrncat(line, word, USTRING_SIZE - 1);
            ustrncat(line, " ", USTRING_SIZE - 1);
            USTRING_CLEAR(word);
         }
      }
   }

   // Calculate bounding box.
   const int nextLine = Round(text_height(font) * 1.25);
   const int x1 = 16;
   const int y1 = 240 - (16 + (nextLine * height) + 8);
   const int x2 = 224;
   const int y2 = 240 - 16;
   int x = x1 + 4;
   int y = y1 + 4;

   set_trans_blender(255, 255, 255, 128);
   drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);

   // Draw shadow, background and borders.
   if(chatMode) {
      rectfill(buffer, x1 + 4, y1 + 4, x2 + 4, y2 + 4, shadowColor);
      rectfill(buffer, x1 + 1, y1 + 1, x2 - 1, y2 - 1, backgroundColor);
      rect(buffer, x1, y1, x2, y2, borderColor);
   }

   for(int i = 0; i < height; i++) {
      const USTRING& line = lines[i];
      if(ustrlen(line) == 0)
         continue;

      // Check for a separator.
      if(chatMode && (line[0] == '-')) {
         set_trans_blender(255, 255, 255, 128);
         hline(buffer, x1 + 1, y + (nextLine / 2), x2 - 1, borderColor);
      }
      else {
         // Note that shadow_textout() handles our transparency for us.
         video_legacy_shadow_textout(buffer, font, line, x, y, textColor, opacities[i]);
      }
      
      y += nextLine;
   }

   solid_mode();

   // Process message history.
   for(History::iterator i = history.begin(); i != history.end(); ) {
      Message& message = *i;

      const int time = 1000 / timing_get_base_frame_rate();
      message.lifetime -= time;
      if(message.lifetime <= 0) {
         i = history.erase(i);
         continue;
      }

      message.duration -= time;
      if(message.duration < 0)
         message.duration = 0;

      i++;
   }
}
