

/* Mapper #11 (Color Dreams). */

/* This mapper is fully supported. */


static int dreams_prg_mask = 0;


static void dreams_write (UINT16 address, UINT8 value)
{
    int index;

    int rom_page, vrom_page;


    /* Extract page # from bits. */

    rom_page = (value & 0x0f);

    vrom_page = ((value >> 4) & 0x07);


    /* Convert 32k page # to 16k. */

    rom_page *= 2;


    /* Convert 8k page # to 1k. */

    vrom_page *= 8;


    /* Select requested 32k page. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K ((rom_page & dreams_prg_mask)));

    cpu_set_read_address_16k (0xC000, ROM_PAGE_16K (((rom_page + 1) & dreams_prg_mask)));


    /* Select requested 8k page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), (vrom_page + index));
    }
}


static INLINE void dreams_reset (void)
{
    int index;


    /* Select first 32k page. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (0));

    cpu_set_read_address_16k (0xC000, ROM_PAGE_16K (1));


    /* Select first 8k page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), index);
    }
}


static INLINE int dreams_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #11 (Color Dreams) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    if (ROM_PRG_ROM_PAGES == 1) dreams_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) dreams_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) dreams_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) dreams_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) dreams_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) dreams_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) dreams_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) dreams_prg_mask = 128;
    else dreams_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != dreams_prg_mask)
    {
        /* Page count not an even power of 2. */

        return (1);
    }


    /* Convert mask to 16k mask. */

    dreams_prg_mask = (dreams_prg_mask - 1);


    /* Set initial mappings. */

    dreams_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, dreams_write);


    return (0);
}
