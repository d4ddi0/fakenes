

/* Mapper #3 (CNROM). */

/* This mapper is fully supported. */


static void cnrom_write (UINT16 address, UINT8 value)
{
    int index;


    /* Convert 8k page # to 1k. */

    value *= 8;


    /* Select requested 8k page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), (value + index));
    }
}


static INLINE void cnrom_reset (void)
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


static INLINE int cnrom_init (void)
{
    /* Set initial mappings. */

    cnrom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, cnrom_write);


    return (0);
}

AL_CONST MMC mmc_cnrom =
{
 "CNROM",
 cnrom_init,
 cnrom_reset
};
