

#include "blit/shared.h"


static INLINE void blit_2xscl (BITMAP * source, BITMAP * target, int x, int y)
{
    int x_base;

    int y_base;


    int x_offset;

    int y_offset;


    int center_pixel;


    int north_pixel;

    int south_pixel;


    int east_pixel;

    int west_pixel;


    BLITTER_SIZE_CHECK (512, 480);


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 2));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);
        

            if (x_offset == 0)
            {
                west_pixel = center_pixel;
            }
            else
            {
                west_pixel = FAST_GETPIXEL (source, (x_offset - 1), y_offset);
            }
        

            if ((x_offset + 1) >= source -> w)
            {
                east_pixel = center_pixel;
            }
            else
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);
            }
	

            if ((y_offset + 1) >= source -> h)
            {
                south_pixel = center_pixel;
            }
            else
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));
            }
	

            if (y_offset == 0)
            {
                north_pixel = center_pixel;
            }
            else
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));
            }


            if ((west_pixel == north_pixel) && (north_pixel != east_pixel) && (west_pixel != south_pixel))
            {
                FAST_PUTPIXEL (target, x_base, y_base, west_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, x_base, y_base, center_pixel);
            }


            if ((north_pixel == east_pixel) && (north_pixel != west_pixel) && (east_pixel != south_pixel))
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, east_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
            }


            if ((west_pixel == south_pixel) && (west_pixel != north_pixel) && (south_pixel != east_pixel))
            {
                FAST_PUTPIXEL (target, x_base, (y_base + 1), west_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);
            }


            if ((south_pixel == east_pixel) && (west_pixel != south_pixel) && (north_pixel != east_pixel))
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), east_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
            }
        }
    }
}
