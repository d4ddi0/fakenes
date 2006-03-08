#include "blit/shared.h"

extern void hq2x (unsigned char *, unsigned char *, int, int, int);
extern void hq3x (unsigned char *, unsigned char *, int, int, int);
extern void hq4x (unsigned char *, unsigned char *, int, int, int);

static INLINE void blit_hq (int multiple, BITMAP *src, BITMAP *dest, int
   x_base, int y_base)
{
   unsigned short *in;
   int *out;
   int w, h, wm, hm;
   int y;

   RT_ASSERT(src);
   RT_ASSERT(dest);

   if ((multiple != 2) && (multiple != 3) && (multiple != 4))
      return;
   if ((multiple == 2) && (!blitter_size_check (dest, 512, 480)))
      return;
   if ((multiple == 3) && (!blitter_size_check (dest, 768, 720)))
      return;
   if ((multiple == 4) && (!blitter_size_check (dest, 1024, 960)))
      return;

   /* Calculate sizes. */
   w = src->w;
   h = src->h;
   wm = (src->w * multiple);
   hm = (src->h * multiple);

   /* Create copy buffers. */
   in = malloc (((w * h) * sizeof (unsigned short)));
   if (!in)
      return;
   out = malloc (((wm * hm) * sizeof (int)));
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
      {
         UINT8 c;
         UINT8 r, g, b;

         c = FAST_GETPIXEL8(src, x, y);

         r = ((getr8 (c) >> 3) & 0x1f);
         g = ((getg8 (c) >> 2) & 0x3f);
         b = ((getb8 (c) >> 3) & 0x1f);

         in[((y * w) + x)] = ((r << 11) | (g << 5) | b);
      }
   }

   switch (multiple)
   {
      case 2:
      {
         /* Perform an HQ2X filtering operation. */

         hq2x ((unsigned char *)in, (unsigned char *)out, w, h, (wm * sizeof
            (int)));

         break;
      }

      case 3:
      {
         /* Perform an HQ3X filtering operation. */

         hq3x ((unsigned char *)in, (unsigned char *)out, w, h, (wm * sizeof
            (int)));

         break;
      }

      case 4:
      {
         /* Perform an HQ4X filtering operation. */

         hq4x ((unsigned char *)in, (unsigned char *)out, w, h, (wm * sizeof
            (int)));

         break;
      }

      default:
         break;
   }


   /* Export out buffer to destination bitmap. */
   for (y = 0; y < hm; y++)
   {
      int yo = (y_base + y);
      int x;

      for (x = 0; x < wm; x++)
      {
         int xo = (x_base + x);
         int c;
         UINT8 r, g, b;
         int d;

         c = out[((y * wm) + x)];

         r = ((c >> 16) & 0xff);
         g = ((c >> 8) & 0xff);
         b = (c & 0xff);

         d = video_create_color (r, g, b);

         switch (color_depth)
         {
            case 8:
            {
               FAST_PUTPIXEL8(dest, xo, yo, d);

               break;
            }

            case 15:
            case 16:
            {
               FAST_PUTPIXEL16(dest, xo, yo, d);

               break;
            }

            case 24:
            {
               FAST_PUTPIXEL24(dest, xo, yo, d);

               break;
            }

            case 32:
            {
               FAST_PUTPIXEL32(dest, xo, yo, d);

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

#define blit_hq2x(s, d, x, y) blit_hq (2, s, d, x, y)
#define blit_hq3x(s, d, x, y) blit_hq (3, s, d, x, y)
#define blit_hq4x(s, d, x, y) blit_hq (4, s, d, x, y)
