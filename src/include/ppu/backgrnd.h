/* VRAM address bit layout          */
/* -YYY VHyy yyyx xxxx              */
/* x = x tile offset in name table  */
/* y = y tile offset in name table  */
/* H = horizontal name table        */
/* V = vertical name table          */
/* Y = y line offset in tile        */

static void ppu_render_background (int line)
{
    int attribute_address;
    int name_table;
    UINT8 *name_table_address;
    UINT8 attribute_byte = 0;

    int x, sub_x;
    int y, sub_y;

    if (PPU_BACKGROUND_CLIP_ENABLED)
    {
        hline (video_buffer, 0, line, 7, 0);
        hline (video_buffer, 8, line, 255, ppu_background_palette [0]);
    }
    else
    {
        hline (video_buffer, 0, line, 255, ppu_background_palette [0]);
    }

    /* dummy reads for write-back cache line loading */
    ppu_vram_dummy_write [0] =
        PPU_GETPIXEL(video_buffer, 0, line) |
        PPU_GETPIXEL(video_buffer, 16, line) |
        PPU_GETPIXEL(video_buffer, 16*2, line) |
        PPU_GETPIXEL(video_buffer, 16*3, line) |
        PPU_GETPIXEL(video_buffer, 16*4, line) |
        PPU_GETPIXEL(video_buffer, 16*5, line) |
        PPU_GETPIXEL(video_buffer, 16*6, line) |
        PPU_GETPIXEL(video_buffer, 16*7, line) |
        PPU_GETPIXEL(video_buffer, 16*8, line) |
        PPU_GETPIXEL(video_buffer, 16*9, line) |
        PPU_GETPIXEL(video_buffer, 16*10, line) |
        PPU_GETPIXEL(video_buffer, 16*11, line) |
        PPU_GETPIXEL(video_buffer, 16*12, line) |
        PPU_GETPIXEL(video_buffer, 16*13, line) |
        PPU_GETPIXEL(video_buffer, 16*14, line) |
        PPU_GETPIXEL(video_buffer, 16*15, line);

    name_table = (vram_address >> 10) & 3;
    name_table_address = name_tables_read[name_table];

    y = (vram_address >> 5) & 0x1F;
    sub_y = (vram_address >> 12) & 7;

    attribute_address =
     /* Y position */
     ((y >> 2) * 8) +
     /* X position */
     ((vram_address & 0x1F) >> 2);

    if (vram_address & 3)
    /* fetch and shift first attribute byte */
    {
        attribute_byte =
            name_table_address [0x3C0 + attribute_address] >>
            ( ( (y & 2) * 2 + (vram_address & 2)));
    }


    /* If background clip left edge is enabled, then skip the entirety
     * of the first tile
     */
    /* Draw the background. */
    for (x = 0; x < (256 / 8) + 1; x ++)
    {
        UINT8 attribute, cache_tag;
        int tile_name, tile_address;
        UINT8 *cache_address;
        int cache_bank, cache_index;

        if (!(vram_address & 3))
        /* fetch and shift attribute byte */
        {
            attribute_byte =
            name_table_address [0x3C0 + attribute_address] >>
                ( (y & 2) * 2);
        }

        tile_name = name_table_address [vram_address & 0x3FF];
        tile_address = ((tile_name * 16) + background_tileset);

        if (mmc_check_latches)
        {
            if ((tile_name >= 0xFD) && (tile_name <= 0xFE))
            {
                mmc_check_latches(tile_address);
            }
        }
   
    
        cache_bank = tile_address >> 10;
        cache_index = ((tile_address & 0x3FF) / 2) + sub_y;

        cache_tag = ppu_vram_block_cache_tag_address [cache_bank]
            [cache_index];

        if (cache_tag)
        /* some non-transparent pixels */
        {
            cache_address = ppu_vram_block_cache_address [cache_bank] +
                cache_index * 8;

            attribute = attribute_table [attribute_byte & 3];

            if (PPU_BACKGROUND_CLIP_ENABLED && (x <= 1))
            {
                if (x == 0) sub_x = 8;
                else /* (x == 1) */ sub_x = x_offset;
            }
            else
            {
                sub_x = 0;
            }

            if (cache_tag != 0xFF)
            /* some transparent pixels */
            {
                for (; sub_x < 8; sub_x ++)
                {
                    UINT8 color = cache_address[sub_x] & attribute;

                    if (color == 0) continue;

                    background_pixels [8 + ((x * 8) + sub_x - x_offset)] = color;

                    color = ppu_background_palette [color];


                    if (ppu_enable_background_layer)
                    {
                        PPU_PUTPIXEL (video_buffer,
                            ((x * 8) + sub_x - x_offset), line, color);
                    }
                }
            }
            else
            /* no transparent pixels */
            {
                for (; sub_x < 8; sub_x ++)
                {
                    UINT8 color = cache_address[sub_x] & attribute;

                    background_pixels [8 + ((x * 8) + sub_x - x_offset)] = color;

                    color = ppu_background_palette [color];


                    if (ppu_enable_background_layer)
                    {
                        PPU_PUTPIXEL (video_buffer,
                            ((x * 8) + sub_x - x_offset), line, color);
                    }
                }
            }
        }

        ++vram_address;

        /* next name byte */
        if (!(vram_address & 1))
        /* new attribute */
        {
            if (!(vram_address & 2))
            /* new attribute byte */
            {
                ++attribute_address;

                if ((vram_address & 0x1F) == 0)
                /* horizontal name table toggle */
                {
                    name_table ^= 1;
                    name_table_address = name_tables_read[name_table];

                    /* handle address wrap */
                    vram_address = (vram_address - (1 << 5)) ^ (1 << 10);
                    attribute_address -= (1 << 3);
                }
            }
            else
            /* same attribute byte */
            {
                attribute_byte >>= 2;
            }
        }
    }
}
