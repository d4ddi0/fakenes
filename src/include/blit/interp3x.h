

#include "blit/shared.h"


static INLINE void blit_interpolated_3x (BITMAP * source, BITMAP * target, int x, int y)
{
    int x_base;

    int y_base;


    int x_offset;

    int y_offset;


    int pixels [9];


    BLITTER_SIZE_CHECK (768, 720);


    for (y_offset = 0; y_offset < source -> h; y_offset ++)
    {
        y_base = (y + (y_offset * 3));


        for (x_offset = 0; x_offset < source -> w; x_offset ++)
        {
            x_base = (x + (x_offset * 3));


            pixels [0] = FAST_GETPIXEL (source, x_offset, y_offset);


            pixels [1] = SAFE_GETPIXEL (source, (x_offset - 1), (y_offset - 1), pixels [0]);

            pixels [2] = SAFE_GETPIXEL (source, x_offset, (y_offset - 1), pixels [0]);

            pixels [3] = SAFE_GETPIXEL (source, (x_offset + 1), (y_offset - 1), pixels [0]);

            pixels [4] = SAFE_GETPIXEL (source, (x_offset + 1), y_offset, pixels [0]);

            pixels [5] = SAFE_GETPIXEL (source, (x_offset + 1), (y_offset + 1), pixels [0]);

            pixels [6] = SAFE_GETPIXEL (source, x_offset, (y_offset + 1), pixels [0]);

            pixels [7] = SAFE_GETPIXEL (source, (x_offset - 1), (y_offset + 1), pixels [0]);

            pixels [8] = SAFE_GETPIXEL (source, (x_offset - 1), y_offset, pixels [0]);


            MAGIC_PUTPIXEL (target, (x_base + 1), (y_base + 1), palette_color [pixels [0]]);


            MAGIC_PUTPIXEL (target, x_base, y_base, mix (pixels [0], pixels [1]));

            MAGIC_PUTPIXEL (target, (x_base + 1), y_base, mix (pixels [0], pixels [2]));
                                                              
            MAGIC_PUTPIXEL (target, (x_base + 2), y_base, mix (pixels [0], pixels [3]));

            MAGIC_PUTPIXEL (target, (x_base + 2), (y_base + 1), mix (pixels [0], pixels [4]));

            MAGIC_PUTPIXEL (target, (x_base + 2), (y_base + 2), mix (pixels [0], pixels [5]));
                                                                    
            MAGIC_PUTPIXEL (target, (x_base + 1), (y_base + 2), mix (pixels [0], pixels [6]));
                 
            MAGIC_PUTPIXEL (target, x_base, (y_base + 2), mix (pixels [0], pixels [7]));
                                                             
            MAGIC_PUTPIXEL (target, x_base, (y_base + 1), mix (pixels [0], pixels [8]));
        }
    }
}
