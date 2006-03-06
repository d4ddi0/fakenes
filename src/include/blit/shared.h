#ifndef BLIT_SHARED_H_INCLUDED
#define BLIT_SHARED_H_INCLUDED

/* Pixel access macros. */

#define FAST_GETPIXEL8(bmp, x, y)      (bmp->line[y][x])

#define FAST_PUTPIXEL8(bmp, x, y, c)   (bmp->line[y][x] = c)
#define FAST_PUTPIXEL16(bmp, x, y, c)  (((UINT16 *)bmp->line[y])[x] = c)
#define FAST_PUTPIXEL24(bmp, x, y, c)  (_putpixel24 (bmp, x, y, c))
#define FAST_PUTPIXEL32(bmp, x, y, c)  (((UINT32 *)bmp->line[y])[x] = c)

/* Utility functions. */

static INLINE BOOL blitter_size_check (BITMAP *bmp, int width, int height)
{
   int y;

   if ((bmp->w >= width) || (bmp->h >= height))
      return (TRUE);

   y = ((bmp->h / 2) - (text_height (font) / 2));

   textout_centre_ex (bmp, font, "Your resolution too low.", (bmp->w / 2),
      y, VIDEO_COLOR_WHITE, -1);

   y += (text_height (font) + 1);

   textprintf_centre_ex (bmp, font, (bmp->w / 2), y, VIDEO_COLOR_WHITE, -1,
      "At least %dx%d is required.", width, height);
                                                                                                                \
   return (FALSE);
}

#define BLITTER_SIZE_CHECK(width, height)                                                                         \
    if ((target -> w < width) || (target -> h < height))                                                          \
    {                                                                                                             \
        /* Center error message on target. */                                                                     \
                                                                                                                  \
        y += ((source -> h / 2) - (((text_height (font) * 2) + (text_height (font) / 2)) / 2));                   \
                                                                                                                  \
                                                                                                                  \
        textout_ex (target, font, "Target dimensions are not large enough.", x, y, VIDEO_COLOR_WHITE, -1);        \
                                                                                                                  \
                                                                                                                  \
        textprintf_ex (target, font, x, ((y + text_height (font)) + (text_height (font) /  2)),                   \
            VIDEO_COLOR_WHITE, -1, "At least %dx%d pixels are required.", width, height);                         \
                                                                                                                  \
                                                                                                                  \
        return;                                                                                                   \
    }

static INLINE int mix (int color_a, int color_b)
{
   const RGB *ca = &internal_palette[color_a];
   const RGB *cb = &internal_palette[color_b];
   int r, g, b;

   /* 0 - 63 --> 0 - 127. */
   r = ((ca->r * 3) + cb->r);
   g = ((ca->g * 3) + cb->g);
   b = ((ca->b * 3) + cb->b);

   return (video_create_color (r, g, b));
}

static INLINE int unfilter_filter (int color_a, int color_b)
{
   int ra, ga, ba;
   int rb, gb, bb;
   int r, g, b;

   ra = getr (color_a);
   ga = getg (color_a);
   ba = getb (color_a);

   rb = getr (color_b);
   gb = getg (color_b);
   bb = getb (color_b);

   r = ((fix ((ra + (ra - rb)), 0, 255) + rb) >> 1);
   g = ((fix ((ga + (ga - gb)), 0, 255) + gb) >> 1);
   b = ((fix ((ba + (ba - bb)), 0, 255) + bb) >> 1);

   return (video_create_color (r, g, b));
}

#endif /* !BLIT_SHARED_H_INCLUDED */
