#include "blit/shared.h"
#include "nes_ntsc.h"

/* Variables. */

static nes_ntsc_t       _nes_ntsc_ntsc;
static nes_ntsc_setup_t _nes_ntsc_setup;
static int              _nes_ntsc_phase;
static int              _nes_ntsc_scanline_doubling;
static BOOL             _nes_ntsc_interpolation;

/* Blitter. */

static void blit_nes_ntsc (BITMAP *src, BITMAP *dest, int x_base, int
   y_base)
{
   int w, h, wm, hm, hx;
   unsigned char *in;
   unsigned short *out;
   int y;
             
   RT_ASSERT(src);
   RT_ASSERT(dest);

   /* Calculate sizes. */
   w = src->w;
   h = src->h;
   wm = NES_NTSC_OUT_WIDTH(w);
   hm = 240;         /* Output height from nes_ntsc_blit(). */
   hx = (240 * 2);   /* Output height from blit_nes_ntsc(). */

   if (!blitter_size_check (dest, wm, hx))
      return;

   /* Check buffers. */
   if (!blit_buffer_in || !blit_buffer_out)
      WARN_BREAK_GENERIC();

   /* Set buffers. */
   in  = (unsigned char  *)blit_buffer_in;
   out = (unsigned short *)blit_buffer_out;

   /* Import source bitmap to input buffer. */
   for (y = 0; y < h; y++)
   {
      int x;

      for (x = 0; x < w; x++)
         in[((y * w) + x)] = (FAST_GETPIXEL8(src, x, y) - 1);
   }

   /* Force 'phase' to 0 if 'setup.merge_fields' is on. */
   _nes_ntsc_phase ^= 1;
   if (_nes_ntsc_setup.merge_fields)
      _nes_ntsc_phase = 0;

   /* Perform an NTSC filtering operation. */
   nes_ntsc_blit (&_nes_ntsc_ntsc, in, w, _nes_ntsc_phase, w, h, out, (wm
      * 2));

   /* Export out buffer to destination bitmap. */
   for (y = 0; y < hm; y++)
   {
      int yo = (y_base + (y * 2));
      int x;

      for (x = 0; x < wm; x++)
      {
         int xo = (x_base + x);
         int c;
         int r, g, b;
         BOOL modify = FALSE;
         int d1, d2 = d2;  /* Kill warning. */
         int yi;

         c = out[((y * wm) + x)];

         r = (((c >> 11) & 0x1f) << 3);
         g = (((c >> 5) & 0x3f) << 2);
         b = ((c & 0x1f) << 3);

         d1 = video_create_color (r, g, b);

         if (_nes_ntsc_interpolation)
         {
            /* Interpolate missing scanlines. */
            
            if (y < (hm - 1))
            {
               c = out[(((y + 1) * wm) + x)];
   
               r += (((c >> 11) & 0x1f) << 3);
               g += (((c >> 5) & 0x3f) << 2);
               b += ((c & 0x1f) << 3);
   
               r /= 2;
               g /= 2;
               b /= 2;
   
               modify = TRUE;
            }
         }

         switch (_nes_ntsc_scanline_doubling)
         {
            case 0:  /* Normal. */
               break;

            case 1:  /* Brighten. */
            {
               /* Supposed to be 8.33, but we approximate to avoid floating
                  point math here. */
               r = fix ((r + (r / 8)), 0, 255);
               g = fix ((g + (g / 8)), 0, 255);
               b = fix ((b + (b / 8)), 0, 255);

               modify = TRUE;

               break;
            }

            case 2:  /* Darken. */
            {
               r = fix ((r - (r / 8)), 0, 255);
               g = fix ((g - (g / 8)), 0, 255);
               b = fix ((b - (b / 8)), 0, 255);

               modify = TRUE;

               break;
            }

            default:
               WARN_GENERIC();
         }

         if (modify)
            d2 = video_create_color (r, g, b);
         else
            d2 = d1;

         yi = (yo + 1);

         switch (color_depth)
         {
            case 8:
            {                                
               FAST_PUTPIXEL8(dest, xo, yo, d1);
               FAST_PUTPIXEL8(dest, xo, yi, d2);

               break;
            }

            case 15:
            case 16:
            {
               FAST_PUTPIXEL16(dest, xo, yo, d1);
               FAST_PUTPIXEL16(dest, xo, yi, d2);

               break;
            }

            case 24:
            {
               FAST_PUTPIXEL24(dest, xo, yo, d1);
               FAST_PUTPIXEL24(dest, xo, yi, d2);

               break;
            }

            case 32:
            {
               FAST_PUTPIXEL32(dest, xo, yo, d1);
               FAST_PUTPIXEL32(dest, xo, yi, d2);

               break;
            }

            default:
               WARN_GENERIC();
         }
      }
   }
}

/* Initializer. */

static void init_nes_ntsc (BITMAP *src, BITMAP *dest)
{
   int preset;
   int merge_fields, doubling, interpolation;
   nes_ntsc_setup_t *setup;
   int w, h, wm, hm, hx;
   
   RT_ASSERT(src);
   RT_ASSERT(dest);

   /* Get setup structure. */
   setup = &_nes_ntsc_setup;

   /* Load configuration. */

   preset = get_config_int ("nes_ntsc", "preset", -1);

   if (preset != -1)
   {
      const nes_ntsc_setup_t *presets[5];

      /* Load a preset. */

      preset = fix (preset, 0, 4);

      presets[0] = &nes_ntsc_composite;   /* Default. */
      presets[1] = &nes_ntsc_composite;
      presets[2] = &nes_ntsc_svideo;
      presets[3] = &nes_ntsc_rgb;
      presets[4] = &nes_ntsc_monochrome;

      memcpy (setup, presets[preset], sizeof (nes_ntsc_setup_t));

      set_config_int ("nes_ntsc", "preset",      -1);
      set_config_int ("nes_ntsc", "hue",         ROUND(setup->hue         * 100.0f));
      set_config_int ("nes_ntsc", "saturation",  ROUND(setup->saturation  * 100.0f));
      set_config_int ("nes_ntsc", "contrast",    ROUND(setup->contrast    * 100.0f));
      set_config_int ("nes_ntsc", "brightness",  ROUND(setup->brightness  * 100.0f));
      set_config_int ("nes_ntsc", "sharpness",   ROUND(setup->sharpness   * 100.0f));
      set_config_int ("nes_ntsc", "gamma",       ROUND(setup->gamma       * 100.0f));
      set_config_int ("nes_ntsc", "resolution",  ROUND(setup->resolution  * 100.0f));
      set_config_int ("nes_ntsc", "artifacts",   ROUND(setup->artifacts   * 100.0f));
      set_config_int ("nes_ntsc", "fringing",    ROUND(setup->fringing    * 100.0f));
      set_config_int ("nes_ntsc", "bleed",       ROUND(setup->bleed       * 100.0f));
      set_config_int ("nes_ntsc", "hue_warping", ROUND(setup->hue_warping * 100.0f));
   }
   else
   {
      int hue, saturation, contrast, brightness, sharpness, hue_warping,
         gamma, resolution, artifacts, fringing, bleed;

      /* All parameters range from -100 to +100. */
   
      hue          = get_config_int ("nes_ntsc", "hue",          0);
      saturation   = get_config_int ("nes_ntsc", "saturation",   0);
      contrast     = get_config_int ("nes_ntsc", "contrast",     0);
      brightness   = get_config_int ("nes_ntsc", "brightness",   0);
      sharpness    = get_config_int ("nes_ntsc", "sharpness",    0);
      gamma        = get_config_int ("nes_ntsc", "gamma",        0);
      resolution   = get_config_int ("nes_ntsc", "resolution",   0);
      artifacts    = get_config_int ("nes_ntsc", "artifacts",    0);
      fringing     = get_config_int ("nes_ntsc", "fringing",     0);
      bleed        = get_config_int ("nes_ntsc", "bleed",        0);
      hue_warping  = get_config_int ("nes_ntsc", "hue_warping",  0);

      /* Initialize nes_ntsc. */
   
      memset (setup, 0, sizeof (nes_ntsc_setup_t));
   
      setup->hue         = (hue         / 100.0f);
      setup->saturation  = (saturation  / 100.0f);
      setup->contrast    = (contrast    / 100.0f);
      setup->brightness  = (brightness  / 100.0f);
      setup->sharpness   = (sharpness   / 100.0f);
      setup->hue_warping = (hue_warping / 100.0f);
      setup->gamma       = (gamma       / 100.0f);
      setup->resolution  = (resolution  / 100.0f);
      setup->artifacts   = (artifacts   / 100.0f);
      setup->fringing    = (fringing    / 100.0f);
      setup->bleed       = (bleed       / 100.0f);
   }

   merge_fields  = get_config_int ("nes_ntsc", "merge_fields", 1);
   doubling      = get_config_int ("nes_ntsc", "doubling",     0);
   interpolation = get_config_int ("nes_ntsc", "interpolated", 1);

   setup->merge_fields         = merge_fields;
   _nes_ntsc_scanline_doubling = fix (doubling, 0, 2);
   _nes_ntsc_interpolation     = interpolation;

   nes_ntsc_init (&_nes_ntsc_ntsc, setup);

   /* Calculate sizes. */
   w = src->w;
   h = src->h;
   wm = NES_NTSC_OUT_WIDTH(w);
   hm = 240;         /* Output height from nes_ntsc_blit(). */
   hx = (240 * 2);   /* Output height from blit_nes_ntsc(). */

   /* Allocate input buffer. */
   blit_buffer_in = malloc (((w * h) * sizeof (unsigned char)));
   if (!blit_buffer_in)
      WARN_BREAK_GENERIC();

   /* Allocate output buffer. */
   blit_buffer_out = malloc (((wm * hm) * sizeof (unsigned short)));
   if (!blit_buffer_out)
   {
      WARN_GENERIC();
      free (blit_buffer_in);
      return;
   }

   blit_x_offset = ((dest->w / 2) - (wm / 2));
   blit_y_offset = ((dest->h / 2) - (hx / 2));
}

/* Deinitializer. */

static void deinit_nes_ntsc (void)
{
   /* Destroy buffers. */

   if (blit_buffer_in)
      free (blit_buffer_in);
   if (blit_buffer_out)
      free (blit_buffer_out);

   blit_buffer_in  = NULL;
   blit_buffer_out = NULL;
}

/* Interface. */

static const BLITTER blitter_nes_ntsc =
{
   init_nes_ntsc, deinit_nes_ntsc,
   blit_nes_ntsc
};
