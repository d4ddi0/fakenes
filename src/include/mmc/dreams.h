

/* Mapper #11 (Color Dreams). */

/* This mapper is fully supported. */


static void dreams_write (UINT16 address, UINT8 value)
{
    int index;

    int rom_page, vrom_page;


    /* Extract page # from bits. */

    rom_page = (value & 0x0f);

    vrom_page = ((value >> 4) & 0x07);


    /* Convert 32k page # to 8k. */

    rom_page *= 4;


    /* Convert 8k page # to 1k. */

    vrom_page *= 8;


    /* Select requested 32k page. */

    for (index = 0; index < 4; index ++)
    {
        mmc_rom_banks [index] = ROM_PAGE_8K ((rom_page + index));
    }


    /* Select requested 8k page. */

    for (index = 0; index < 8; index ++)
    {
        mmc_vrom_banks [index] = VROM_PAGE_1K ((vrom_page + index));
    }
}


static INLINE void dreams_reset (void)
{
    int index;


    /* Select first 32k page. */

    for (index = 0; index < 4; index ++)
    {
        mmc_rom_banks [index] = ROM_PAGE_8K (index);
    }


    /* Select first 8k page. */

    for (index = 0; index < 8; index ++)
    {
        mmc_vrom_banks [index] = VROM_PAGE_1K (index);
    }
}


static INLINE int dreams_init (void)
{
    printf ("Using memory mapper #11 (Color Dreams) "
        "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);


    /* Is this right? */

    mmc_no_vrom = FALSE;


    dreams_reset ();

    mmc_write = dreams_write;


    return (0);
}
