

#include "blit/shared.h"


static INLINE void blit_super_2xsoe (BITMAP * source, BITMAP * target, int x, int y)
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


        x_offset = 0;

        x_base = x;


        center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


        FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [center_pixel]);

        FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [center_pixel]);


        if (source -> w < 2)
        {
            FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [center_pixel]);

            FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);


            continue;
        }


        east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


        if (y_offset > 0)
        {
            north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


            if (north_pixel != east_pixel)
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [center_pixel]);
        }


        if ((y_offset + 1) < source -> h)
        {
            south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


            if (south_pixel != east_pixel)
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
        }


        for (x_offset = 1; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            west_pixel = center_pixel;

            center_pixel = east_pixel;


            if (y_offset > 0)
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


                if (north_pixel != west_pixel)
                {
                    FAST_PUTPIXEL16 (target, x_base, y_base, mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [west_pixel]);
                }    
            }
            else
            {
                north_pixel = -1;


                FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [center_pixel]);
            }


            if ((y_offset + 1) < source -> h)
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


                if (south_pixel != west_pixel)
                {
                    FAST_PUTPIXEL16 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [west_pixel]);
                }    
            }
            else
            {
                south_pixel = -1;


                FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [center_pixel]);
            }


            if ((x_offset + 1) < source -> w)
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


                if ((north_pixel < 0) || (north_pixel != east_pixel))
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [east_pixel]);
                }    


                if ((south_pixel < 0) || (south_pixel != east_pixel))
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
                }    
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [center_pixel]);

                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
            }
        }
    }
}


static INLINE void blit_super_2xsoe_32 (BITMAP * source, BITMAP * target, int x, int y)
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


        x_offset = 0;

        x_base = x;


        center_pixel = FAST_GETPIXEL (source, x_offset, y_offset);


        FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [center_pixel]);

        FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [center_pixel]);


        if (source -> w < 2)
        {
            FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [center_pixel]);

            FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);


            continue;
        }


        east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


        if (y_offset > 0)
        {
            north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


            if (north_pixel != east_pixel)
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [center_pixel]);
        }


        if ((y_offset + 1) < source -> h)
        {
            south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


            if (south_pixel != east_pixel)
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }    
        }
        else
        {
            FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
        }


        for (x_offset = 1; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 2));


            west_pixel = center_pixel;

            center_pixel = east_pixel;


            if (y_offset > 0)
            {
                north_pixel = FAST_GETPIXEL (source, x_offset, (y_offset - 1));


                if (north_pixel != west_pixel)
                {
                    FAST_PUTPIXEL32 (target, x_base, y_base, mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [west_pixel]);
                }    
            }
            else
            {
                north_pixel = -1;


                FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [center_pixel]);
            }


            if ((y_offset + 1) < source -> h)
            {
                south_pixel = FAST_GETPIXEL (source, x_offset, (y_offset + 1));


                if (south_pixel != west_pixel)
                {
                    FAST_PUTPIXEL32 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [west_pixel]);
                }    
            }
            else
            {
                south_pixel = -1;


                FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [center_pixel]);
            }


            if ((x_offset + 1) < source -> w)
            {
                east_pixel = FAST_GETPIXEL (source, (x_offset + 1), y_offset);


                if ((north_pixel < 0) || (north_pixel != east_pixel))
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [east_pixel]);
                }    


                if ((south_pixel < 0) || (south_pixel != east_pixel))
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
                }
                else
                {
                    FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
                }    
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [center_pixel]);

                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [center_pixel]);
            }
        }
    }
}
