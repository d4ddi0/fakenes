#include "blit/shared.h"

static INLINE void blit_interpolated_3x (BITMAP *src, BITMAP *dest, int
   x_base, int y_base)
{
   int y;

   RT_ASSERT(src);
   RT_ASSERT(dest);

   if (!blitter_size_check (dest, 768, 720))
      return;

   for (y = 0; y < src->h; y++)
   {
      int yo = (y_base + (y * 3));
      int x;

      for (x = 0; x < src->w; x++)
      {
         int xo = (x_base + (x * 3));
         UINT8 c;
         int xn, yn, xp, yp;
         UINT8 p[8];
         int cp[8];
         int xi, xi2, yi, yi2;

         c = FAST_GETPIXEL8(src, x, y);

         memset (p, 0, sizeof (p));

         xn = (x - 1);
         yn = (y - 1);
         xp = (x + 1);
         yp = (y + 1);

         if (xn >= 0)
         {
            p[7] = FAST_GETPIXEL8(src, xn, y);
            if (yn >= 0)
               p[0] = FAST_GETPIXEL8(src, xn, yn);
            if (yp < src->h)
               p[6] = FAST_GETPIXEL8(src, xn, yp);
         }

         if (xp < src->w)
         {
            p[3] = FAST_GETPIXEL8(src, xp, y);
            if (yn >= 0)
               p[2] = FAST_GETPIXEL8(src, xp, yn);
            if (yp < src->h)
               p[4] = FAST_GETPIXEL8(src, xp, yp);
         }

         if (yn >= 0)
            p[1] = FAST_GETPIXEL8(src, x, yn);

         if (yp < src->h)
            p[5] = FAST_GETPIXEL8(src, x, yp);

         cp[0] = mix (c, p[0]);
         cp[1] = mix (c, p[1]);
         cp[2] = mix (c, p[2]);
         cp[3] = mix (c, p[3]);
         cp[4] = mix (c, p[4]);
         cp[5] = mix (c, p[5]);
         cp[6] = mix (c, p[6]);
         cp[7] = mix (c, p[7]);

         xi  = (xo + 1);
         xi2 = (xo + 2);
         yi  = (yo + 1);
         yi2 = (yo + 2);

         if (color_depth == 8)
         {
            FAST_PUTPIXEL8(dest, xi,  yi,  c);
            FAST_PUTPIXEL8(dest, xo,  yo,  cp[0]);
            FAST_PUTPIXEL8(dest, xi,  yo,  cp[1]);
            FAST_PUTPIXEL8(dest, xi2, yo,  cp[2]);
            FAST_PUTPIXEL8(dest, xi2, yi,  cp[3]);
            FAST_PUTPIXEL8(dest, xi2, yi2, cp[4]);
            FAST_PUTPIXEL8(dest, xi,  yi2, cp[5]);
            FAST_PUTPIXEL8(dest, xo,  yi2, cp[6]);
            FAST_PUTPIXEL8(dest, xo,  yi,  cp[7]);
         }
         else
         {
            int cx = palette_color[c];

            switch (color_depth)
            {
               case 15:
               case 16:
               {
                  FAST_PUTPIXEL16(dest, xi,  yi,  cx);
                  FAST_PUTPIXEL16(dest, xo,  yo,  cp[0]);
                  FAST_PUTPIXEL16(dest, xi,  yo,  cp[1]);
                  FAST_PUTPIXEL16(dest, xi2, yo,  cp[2]);
                  FAST_PUTPIXEL16(dest, xi2, yi,  cp[3]);
                  FAST_PUTPIXEL16(dest, xi2, yi2, cp[4]);
                  FAST_PUTPIXEL16(dest, xi,  yi2, cp[5]);
                  FAST_PUTPIXEL16(dest, xo,  yi2, cp[6]);
                  FAST_PUTPIXEL16(dest, xo,  yi,  cp[7]);

                  break;
               }

               case 24:
               {
                  FAST_PUTPIXEL24(dest, xi,  yi,  cx);
                  FAST_PUTPIXEL24(dest, xo,  yo,  cp[0]);
                  FAST_PUTPIXEL24(dest, xi,  yo,  cp[1]);
                  FAST_PUTPIXEL24(dest, xi2, yo,  cp[2]);
                  FAST_PUTPIXEL24(dest, xi2, yi,  cp[3]);
                  FAST_PUTPIXEL24(dest, xi2, yi2, cp[4]);
                  FAST_PUTPIXEL24(dest, xi,  yi2, cp[5]);
                  FAST_PUTPIXEL24(dest, xo,  yi2, cp[6]);
                  FAST_PUTPIXEL24(dest, xo,  yi,  cp[7]);

                  break;
               }

               case 32:
               {
                  FAST_PUTPIXEL32(dest, xi,  yi,  cx);
                  FAST_PUTPIXEL32(dest, xo,  yo,  cp[0]);
                  FAST_PUTPIXEL32(dest, xi,  yo,  cp[1]);
                  FAST_PUTPIXEL32(dest, xi2, yo,  cp[2]);
                  FAST_PUTPIXEL32(dest, xi2, yi,  cp[3]);
                  FAST_PUTPIXEL32(dest, xi2, yi2, cp[4]);
                  FAST_PUTPIXEL32(dest, xi,  yi2, cp[5]);
                  FAST_PUTPIXEL32(dest, xo,  yi2, cp[6]);
                  FAST_PUTPIXEL32(dest, xo,  yi,  cp[7]);

                  break;
               }
            }
         }
      }
   }
}
