

/* Mapper #66 (GNROM). */

/* This mapper is fully supported. */


static void gnrom_write (UINT16 address, UINT8 value)
{
    int index;

    int chr_bank, prg_bank;


    chr_bank = (value & 0x0f);


    prg_bank = (value & 0xf0) >> 4;


    /* Select requested 32k page. */

    cpu_set_read_address_32k_rom_block (0x8000, prg_bank);


    /* Convert 8k page # to 1k. */

    chr_bank *= 8;


    /* Select requested 8k page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), (chr_bank + index));
    }
}


static INLINE void gnrom_reset (void)
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


static INLINE int gnrom_init (void)
{
    /* Set initial mappings. */

    gnrom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, gnrom_write);


    return (0);
}

AL_CONST MMC mmc_gnrom =
{
 "GNROM",
 gnrom_init,
 gnrom_reset
};
