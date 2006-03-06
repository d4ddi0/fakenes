#include "blit/shared.h"

static INLINE void blit_interpolated_2x (BITMAP *src, BITMAP *dest, int
   x_base, int y_base)
{
   int y;

   RT_ASSERT(src);
   RT_ASSERT(dest);

   if (!blitter_size_check (dest, 512, 480))
      return;

   for (y = 0; y < src->h; y++)
   {
      int yo = (y_base + (y * 2));
      int x;

      for (x = 0; x < src->w; x++)
      {
         int xo = (x_base + (x * 2));
         UINT8 c;
         UINT8 e = 0, s = 0, se = 0;
         int ce, cs, cse;
         int xi, yi;

         c = FAST_GETPIXEL8(src, x, y);

         if ((x + 1) < src->w) e = FAST_GETPIXEL8(src, (x + 1), y);
         if ((y + 1) < src->h) s = FAST_GETPIXEL8(src, x, (y + 1));

         if (((x + 1) < src->w) && ((y + 1) < src->h))
            se = FAST_GETPIXEL8(src, (x + 1), (y + 1));

         ce  = mix (c, e);
         cs  = mix (c, s);
         cse = mix (c, se);

         xi = (xo + 1);
         yi = (yo + 1);

         if (color_depth == 8)
         {
            FAST_PUTPIXEL8(dest, xo, yo, c);
            FAST_PUTPIXEL8(dest, xi, yo, ce);
            FAST_PUTPIXEL8(dest, xo, yi, cs);
            FAST_PUTPIXEL8(dest, xi, yi, cse);
         }
         else
         {
            int cx = palette_color[c];

            switch (color_depth)
            {
               case 15:
               case 16:
               {
                  FAST_PUTPIXEL16(dest, xo, yo, cx);
                  FAST_PUTPIXEL16(dest, xi, yo, ce);
                  FAST_PUTPIXEL16(dest, xo, yi, cs);
                  FAST_PUTPIXEL16(dest, xi, yi, cse);

                  break;
               }

               case 24:
               {
                  FAST_PUTPIXEL24(dest, xo, yo, cx);
                  FAST_PUTPIXEL24(dest, xi, yo, ce);
                  FAST_PUTPIXEL24(dest, xo, yi, cs);
                  FAST_PUTPIXEL24(dest, xi, yi, cse);

                  break;
               }

               case 32:
               {
                  FAST_PUTPIXEL32(dest, xo, yo, cx);
                  FAST_PUTPIXEL32(dest, xi, yo, ce);
                  FAST_PUTPIXEL32(dest, xo, yi, cs);
                  FAST_PUTPIXEL32(dest, xi, yi, cse);

                  break;
               }

               default:
                  break;
            }
         }
      }
   }
}
