

/* Mapper #34 (Nina-1). */


static int nina_prg_mask = 0;


static void nina_write (UINT16 address, UINT8 value)
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

        default:

            /* Convert 32k PRG-ROM page index to 16k. */

            value <<= 1;

            value &= nina_prg_mask;


            /* Set requested 32k ROM page at $8000. */
        
            cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (value));
        
            cpu_set_read_address_16k (0xc000, ROM_PAGE_16K ((value + 1)));


            break;
    }
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

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (0));

    cpu_set_read_address_16k (0xc000, ROM_PAGE_16K (1));
}


static INLINE int nina_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #34 (Nina-1) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    /* Mapper requires at least 32k of PRG ROM */
    if (ROM_PRG_ROM_PAGES < 2)
    {
        return -1;
    }

    /* Calculate PRG-ROM mask. */

    if (ROM_PRG_ROM_PAGES == 1) nina_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) nina_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) nina_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) nina_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) nina_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) nina_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) nina_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) nina_prg_mask = 128;
    else nina_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != nina_prg_mask)
    {
        /* Page count not even power of 2. */

        return (1);
    }


    /* Convert PRG-ROM mask to 16k mask. */

    nina_prg_mask --;


    /* Check if CHR-ROM is present. */

    if (ROM_CHR_ROM_PAGES == 0)
    {
        /* No CHR-ROM hardware. */

        ppu_set_ram_8k_pattern_vram ();
    }


    /* Set initial mappings. */

    nina_reset ();


    /* Install write handlers. */

    cpu_set_write_handler_2k (0x7800, nina_write);

    cpu_set_write_handler_32k (0x8000, nina_write);


    return (0);
}
