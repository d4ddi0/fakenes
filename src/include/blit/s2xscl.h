

#include "blit/shared.h"


static INLINE void blit_super_2xscl_8 (BITMAP * source, BITMAP * target, int x, int y)
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


            center_pixel = FAST_GETPIXEL8 (source, x_offset, y_offset);


            if (x_offset == 0)
            {
                west_pixel = center_pixel;
            }
            else
            {
                west_pixel = FAST_GETPIXEL8 (source, (x_offset - 1), y_offset);
            }
        

            if ((x_offset + 1) >= source -> w)
            {
                east_pixel = center_pixel;
            }
            else
            {
                east_pixel = FAST_GETPIXEL8 (source, (x_offset + 1), y_offset);
            }
	

            if ((y_offset + 1) >= source -> h)
            {
                south_pixel = center_pixel;
            }
            else
            {
                south_pixel = FAST_GETPIXEL8 (source, x_offset, (y_offset + 1));
            }
	

            if (y_offset == 0)
            {
                north_pixel = center_pixel;
            }
            else
            {
                north_pixel = FAST_GETPIXEL8 (source, x_offset, (y_offset - 1));
            }


            if ((west_pixel == north_pixel) && (north_pixel != east_pixel) && (west_pixel != south_pixel))
            {
                FAST_PUTPIXEL8 (target, x_base, y_base, west_pixel);
            }
            else
            {
                FAST_PUTPIXEL8 (target, x_base, y_base, mix (center_pixel, west_pixel));
            }


            if ((north_pixel == east_pixel) && (north_pixel != west_pixel) && (east_pixel != south_pixel))
            {
                FAST_PUTPIXEL8 (target, (x_base + 1), y_base, east_pixel);
            }
            else
            {
                FAST_PUTPIXEL8 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }


            if ((west_pixel == south_pixel) && (west_pixel != north_pixel) && (south_pixel != east_pixel))
            {
                FAST_PUTPIXEL8 (target, x_base, (y_base + 1), west_pixel);
            }
            else
            {
                FAST_PUTPIXEL8 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
            }


            if ((south_pixel == east_pixel) && (west_pixel != south_pixel) && (north_pixel != east_pixel))
            {
                FAST_PUTPIXEL8 (target, (x_base + 1), (y_base + 1), east_pixel);
            }
            else
            {
                FAST_PUTPIXEL8 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }                
        }
    }
}


static INLINE void blit_super_2xscl_16 (BITMAP * source, BITMAP * target, int x, int y)
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


            center_pixel = FAST_GETPIXEL8 (source, x_offset, y_offset);


            if (x_offset == 0)
            {
                west_pixel = center_pixel;
            }
            else
            {
                west_pixel = FAST_GETPIXEL8 (source, (x_offset - 1), y_offset);
            }
        

            if ((x_offset + 1) >= source -> w)
            {
                east_pixel = center_pixel;
            }
            else
            {
                east_pixel = FAST_GETPIXEL8 (source, (x_offset + 1), y_offset);
            }
	

            if ((y_offset + 1) >= source -> h)
            {
                south_pixel = center_pixel;
            }
            else
            {
                south_pixel = FAST_GETPIXEL8 (source, x_offset, (y_offset + 1));
            }
	

            if (y_offset == 0)
            {
                north_pixel = center_pixel;
            }
            else
            {
                north_pixel = FAST_GETPIXEL8 (source, x_offset, (y_offset - 1));
            }


            if ((west_pixel == north_pixel) && (north_pixel != east_pixel) && (west_pixel != south_pixel))
            {
                FAST_PUTPIXEL16 (target, x_base, y_base, palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, x_base, y_base, mix (center_pixel, west_pixel));
            }


            if ((north_pixel == east_pixel) && (north_pixel != west_pixel) && (east_pixel != south_pixel))
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }


            if ((west_pixel == south_pixel) && (west_pixel != north_pixel) && (south_pixel != east_pixel))
            {
                FAST_PUTPIXEL16 (target, x_base, (y_base + 1), palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
            }


            if ((south_pixel == east_pixel) && (west_pixel != south_pixel) && (north_pixel != east_pixel))
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL16 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }
        }
    }
}


static INLINE void blit_super_2xscl_32 (BITMAP * source, BITMAP * target, int x, int y)
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


            center_pixel = FAST_GETPIXEL8 (source, x_offset, y_offset);


            if (x_offset == 0)
            {
                west_pixel = center_pixel;
            }
            else
            {
                west_pixel = FAST_GETPIXEL8 (source, (x_offset - 1), y_offset);
            }
        

            if ((x_offset + 1) >= source -> w)
            {
                east_pixel = center_pixel;
            }
            else
            {
                east_pixel = FAST_GETPIXEL8 (source, (x_offset + 1), y_offset);
            }
	

            if ((y_offset + 1) >= source -> h)
            {
                south_pixel = center_pixel;
            }
            else
            {
                south_pixel = FAST_GETPIXEL8 (source, x_offset, (y_offset + 1));
            }
	

            if (y_offset == 0)
            {
                north_pixel = center_pixel;
            }
            else
            {
                north_pixel = FAST_GETPIXEL8 (source, x_offset, (y_offset - 1));
            }


            if ((west_pixel == north_pixel) && (north_pixel != east_pixel) && (west_pixel != south_pixel))
            {
                FAST_PUTPIXEL32 (target, x_base, y_base, palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, x_base, y_base, mix (center_pixel, west_pixel));
            }


            if ((north_pixel == east_pixel) && (north_pixel != west_pixel) && (east_pixel != south_pixel))
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), y_base, mix (center_pixel, east_pixel));
            }


            if ((west_pixel == south_pixel) && (west_pixel != north_pixel) && (south_pixel != east_pixel))
            {
                FAST_PUTPIXEL32 (target, x_base, (y_base + 1), palette_color [west_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, x_base, (y_base + 1), mix (center_pixel, west_pixel));
            }


            if ((south_pixel == east_pixel) && (west_pixel != south_pixel) && (north_pixel != east_pixel))
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), palette_color [east_pixel]);
            }
            else
            {
                FAST_PUTPIXEL32 (target, (x_base + 1), (y_base + 1), mix (center_pixel, east_pixel));
            }                
        }
    }
}


static INLINE void blit_super_2xscl (BITMAP * source, BITMAP * target, int x, int y)
{
    switch (color_depth)
    {
        case 8:

            blit_super_2xscl_8 (source, target, x, y);


            break;


        case 15:

        case 16:

            blit_super_2xscl_16 (source, target, x, y);


            break;


        case 32:

            blit_super_2xscl_32 (source, target, x, y);


            break;
    }
}
