

#ifndef BLIT_SHARED_H_INCLUDED

#define BLIT_SHARED_H_INCLUDED


#define FAST_GETPIXEL8(bitmap, x, y)             (((UINT8 *) bitmap -> line [y]) [x])


#define SAFE_GETPIXEL8(bitmap, x, y, default)                                 \
    ((((x >= 0) && (y >= 0)) && ((x < bitmap -> w) && (y < bitmap -> h))) ?   \
    FAST_GETPIXEL8 (bitmap, x, y) : default)


#define FAST_PUTPIXEL8(bitmap, x, y, color)     (((UINT8 *)  bitmap -> line [y]) [x] = color)

#define FAST_PUTPIXEL16(bitmap, x, y, color)    (((UINT16 *) bitmap -> line [y]) [x] = color)

#define FAST_PUTPIXEL32(bitmap, x, y, color)    (((UINT32 *) bitmap -> line [y]) [x] = color)


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
    int r;

    int g;

    int b;


    /* 0 - 63 --> 0 - 127. */

    r = ((internal_palette [color_a].r * 3) + internal_palette [color_b].r);

    g = ((internal_palette [color_a].g * 3) + internal_palette [color_b].g);

    b = ((internal_palette [color_a].b * 3) + internal_palette [color_b].b);


    return (video_create_color (r, g, b));
}


#endif /* ! BLIT_SHARED_H_INCLUDED */
