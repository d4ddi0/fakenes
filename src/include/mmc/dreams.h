

/* Mapper #11 (Color Dreams). */

/* This mapper is fully supported. */


static void dreams_write (UINT16 address, UINT8 value)
{
    int index;

    int rom_page, vrom_page;


    /* Extract page # from bits. */

    rom_page = (value & 0x0f);

    vrom_page = ((value >> 4) & 0x07);


    /* Convert 8k page # to 1k. */

    vrom_page *= 8;


    /* Select requested 32k page. */

    cpu_set_read_address_32k_rom_block (0x8000, rom_page);


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

    cpu_set_read_address_32k_rom_block (0x8000, 0);


    /* Select first 8k page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), index);
    }
}


static INLINE int dreams_init (void)
{
    /* Set initial mappings. */

    dreams_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, dreams_write);


    return (0);
}

AL_CONST MMC mmc_dreams =
{
 "Color Dreams",
 dreams_init,
 dreams_reset
};
