

#include "blit/shared.h"


static INLINE void blit_2xsoe (BITMAP * source, BITMAP * target, int x, int y)
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


        /* source grid: A B C  output grid: E1 E2
         *              D E F               E3 E4
         *              G H I
         */

        /* Handle first pixel on line. */

        x_offset = 0;

        x_base = x;


        center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


        /* A,D,G = invalid, E1,E3 = E */
        FAST_PUTPIXEL (target, x_base, y_base, center_pixel);

        FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);


        /* if C,F,I == invalid, E2,E4 = E */
        if (source -> w < 2)
        {
            FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);

            FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);


            continue;
        }


        east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


        /* if B = invalid, E2 = E */
        /* else E2 = B == F ? F : E; */
        if (y_offset > 0)
        {
            north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


            if (north_pixel != east_pixel)
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, east_pixel);
            }    
        }
        else
        {
            FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
        }


        /* if H = invalid, E4 = E */
        /* else E4 = F == H ? F : E; */
        if ((y_offset + 1) < source -> h)
        {
            south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


            if (south_pixel != east_pixel)
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), east_pixel);
            }    
        }
        else
        {
            FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
        }


        for (x_offset = 1; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            west_pixel = center_pixel;

            center_pixel = east_pixel;


            /* D = valid */
            /* if B = invalid, E1 = E */
            /* else E1 = B == D ? D : E; */
            if (y_offset > 0)
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


                if (north_pixel != west_pixel)
                {
                    FAST_PUTPIXEL (target, x_base, y_base, center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, x_base, y_base, west_pixel);
                }    
            }
            else
            {
                north_pixel = -1;


                FAST_PUTPIXEL (target, x_base, y_base, center_pixel);
            }


            /* if H = invalid, E1 = E */
            /* else E1 = D == H ? D : E; */
            if ((y_offset + 1) < source -> h)
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


                if (south_pixel != west_pixel)
                {
                    FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, x_base, (y_base + 1), west_pixel);
                }    
            }
            else
            {
                south_pixel = -1;


                FAST_PUTPIXEL (target, x_base, (y_base + 1), center_pixel);
            }


            /* if F = invalid, E2,E4 = E */
            /* else */
            /* if B = invalid, E2 = E */
            /* else E2 = B == F ? F : E; */
            /* if H = invalid, E4 = E */
            /* else E4 = F == H ? F : E; */
            if ((x_offset + 1) < source -> w)
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


                if ((north_pixel < 0) || (north_pixel != east_pixel))
                {
                    FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, (x_base + 1), y_base, east_pixel);
                }    


                if ((south_pixel < 0) || (south_pixel != east_pixel))
                {
                    FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
                }
                else
                {
                    FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), east_pixel);
                }    
            }
            else
            {
                FAST_PUTPIXEL (target, (x_base + 1), y_base, center_pixel);

                FAST_PUTPIXEL (target, (x_base + 1), (y_base + 1), center_pixel);
            }
        }
    }
}
