#include "blit/shared.h"
#include "nes_ntsc.h"

static INLINE void blit_nes_ntsc (BITMAP *src, BITMAP *dest, int x_base, int
   y_base)
{
   static nes_ntsc_t ntsc;
   static nes_ntsc_setup_t setup;
   static BOOL initialized = FALSE;
   static int phase;
   unsigned char *in;
   unsigned short *out;
   int w, h, wm, hm;
   int y;
             
   RT_ASSERT(src);
   RT_ASSERT(dest);

   if (!blitter_size_check (dest, 602, 480))
      return;

   if (!initialized)
   {
      memset (&setup, 0, sizeof (setup));
      setup.merge_fields = 1;
      nes_ntsc_init (&ntsc, &setup);

      initialized = TRUE;
   }

   /* Calculate sizes. */
   w = src->w;
   h = src->h;
   wm = 602;
   hm = 240;

   /* Create copy buffers. */
   in = malloc (((w * h) * sizeof (unsigned char)));
   if (!in)
      return;
   out = malloc (((wm * hm) * sizeof (unsigned short)));
   if (!out)
   {
      free (in);
      return;
   }

   /* Import source bitmap to input buffer. */
   for (y = 0; y < h; y++)
   {
      int x;

      for (x = 0; x < w; x++)
         in[((y * w) + x)] = (FAST_GETPIXEL8(src, x, y) - 1);
   }

   /* Force 'phase' to 0 if 'setup.merge_fields' is on. */
	phase ^= 1;
   if (setup.merge_fields)
		phase = 0;

   /* Perform an NTSC filtering operation. */
   nes_ntsc_blit (&ntsc, in, w, phase, wm, hm, out, (wm * 2));

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

   /* Free copy buffers. */
   free (in);
   free (out);
}
