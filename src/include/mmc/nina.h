

/* Mapper #34 (Nina-1). */


static void nina_write_low (UINT16 address, UINT8 value)
{
    int index;


    /* Check if address is out-of-bounds. */

    if (address < 0x7ffd)
    {
        return;
    }


    switch (address)
    {
        case 0x7ffe:

            /* Verify that CHR-ROM is present. */

            if (ROM_CHR_ROM_PAGES == 0)
            {
                break;
            }


            /* Convert 4k CHR-ROM page index to 1k. */

            value <<= 2;


            /* Set requested 4k CHR-ROM page at PPU $0000. */

            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block ((index << 10), (value + index));
            }


            break;


        case 0x7fff:

            /* Verify that CHR-ROM is present. */

            if (ROM_CHR_ROM_PAGES == 0)
            {
                break;
            }


            /* Convert 4k CHR-ROM page index to 1k. */

            value <<= 2;


            /* Set requested 4k CHR-ROM page at PPU $1000. */

            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block (((index << 10) + 0x1000), (value + index));
            }


            break;


        case 0x7ffd:

            /* Set requested 32k ROM page at $8000. */
        
            cpu_set_read_address_32k_rom_block (0x8000, value);


            break;
    }
}


static void nina_write_high (UINT16 address, UINT8 value)
{
    /* Set requested 32k ROM page at $8000. */
        
    cpu_set_read_address_32k_rom_block (0x8000, value);
}


static INLINE void nina_reset (void)
{
    int index;


    /* Check if CHR-ROM is present. */

    if (ROM_CHR_ROM_PAGES != 0)
    {
        /* Select first 8k CHR-ROM page at PPU $0000. */

        for (index = 0; index < 8; index ++)
        {
            ppu_set_ram_1k_pattern_vrom_block ((index << 10), index);
        }
    }


    /* Select first 32k ROM page at $8000. */

    cpu_set_read_address_32k_rom_block (0x8000, 0);
}


static INLINE int nina_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #34 (Nina-1) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    /* Check if CHR-ROM is present. */

    if (ROM_CHR_ROM_PAGES == 0)
    {
        /* No CHR-ROM hardware. */

        ppu_set_ram_8k_pattern_vram ();
    }


    /* Set initial mappings. */

    nina_reset ();


    /* Install write handlers. */

    cpu_set_write_handler_2k (0x7800, nina_write_low);

    cpu_set_write_handler_32k (0x8000, nina_write_high);


    return (0);
}
