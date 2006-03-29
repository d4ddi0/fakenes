#include "blit/shared.h"
#include "nes_ntsc.h"

/* Variables. */

static nes_ntsc_t       _nes_ntsc_ntsc;
static nes_ntsc_setup_t _nes_ntsc_setup;
static int              _nes_ntsc_phase;

/* Blitter. */

static void blit_nes_ntsc (BITMAP *src, BITMAP *dest, int x_base, int
   y_base)
{
   unsigned char *in;
   unsigned short *out;
   int w, h, wm, hm;
   int y;
             
   RT_ASSERT(src);
   RT_ASSERT(dest);

   if (!blitter_size_check (dest, 602, 480))
      return;

   /* Check buffers. */
   if (!blit_buffer_in || !blit_buffer_out)
      return;

   /* Set buffers. */
   in  = (unsigned char  *)blit_buffer_in;
   out = (unsigned short *)blit_buffer_out;

   /* Calculate sizes. */
   w = src->w;
   h = src->h;
   wm = 602;
   hm = 240;

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
   nes_ntsc_blit (&_nes_ntsc_ntsc, in, w, _nes_ntsc_phase, wm, hm, out, (wm
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
         int d1, d2;
         int yi;

         c = out[((y * wm) + x)];

         r = (((c >> 11) & 0x1f) << 3);
         g = (((c >> 5) & 0x3f) << 2);
         b = ((c & 0x1f) << 3);

         d1 = video_create_color (r, g, b);

         if (y < (hm - 1))
         {
            c = out[(((y + 1) * wm) + x)];

            r += (((c >> 11) & 0x1f) << 3);
            g += (((c >> 5) & 0x3f) << 2);
            b += ((c & 0x1f) << 3);

            r /= 2;
            g /= 2;
            b /= 2;

            d2 = video_create_color (r, g, b);
         }
         else
         {
            d2 = d1;
         }

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
               break;
         }
      }
   }
}

/* Initializer. */

static void init_nes_ntsc (BITMAP *src, BITMAP *dest)
{
   int w, h, wm, hm;

   RT_ASSERT(src);
   RT_ASSERT(dest);

   /* Initialize nes_ntsc. */
   memset (&_nes_ntsc_setup, 0, sizeof (_nes_ntsc_setup));
   _nes_ntsc_setup.merge_fields = 1;
   nes_ntsc_init (&_nes_ntsc_ntsc, &_nes_ntsc_setup);

   /* Calculate sizes. */
   w = src->w;
   h = src->h;
   wm = 602;
   hm = 240;

   /* Allocate input buffer. */
   blit_buffer_in = malloc (((w * h) * sizeof (unsigned char)));
   if (!blit_buffer_in)
      return;

   /* Allocate output buffer. */
   blit_buffer_out = malloc (((wm * hm) * sizeof (unsigned short)));
   if (!blit_buffer_out)
   {
      free (blit_buffer_in);
      return;
   }

   blit_x_offset = ((dest->w / 2) - (602 / 2));
   blit_y_offset = ((dest->h / 2) - (480 / 2));
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
