

/* Mapper #7 (AOROM). */

/* This mapper is fully supported. */


#define AOROM_MIRRORING_BIT   16


static void aorom_write (UINT16 address, UINT8 value)
{
    int index;


    /* Is this reversed? */

    set_ppu_mirroring (((value & AOROM_MIRRORING_BIT) ?
        MIRRORING_ONE_SCREEN_2400 : MIRRORING_ONE_SCREEN_2000));


    /* Mask off upper 4 bits. */

    value &= 0x0f;


    /* Convert 32k page # to 8k. */

    value *= 4;


    /* Select requested 32k page. */

    for (index = 0; index < 4; index ++)
    {
        mmc_rom_banks [index] = ROM_PAGE_8K ((value + index));
    }
}


static INLINE void aorom_reset (void)
{
    int index;


    /* Select first 32k page. */

    for (index = 0; index < 4; index ++)
    {
        mmc_rom_banks [index] = ROM_PAGE_8K (index);
    }
}


static INLINE int aorom_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #7 (AOROM) "
            "(%d PRG, no CHR).\n\n", ROM_PRG_ROM_PAGES);
    }


    /* No VROM hardware. */

    mmc_no_vrom = TRUE;


    /* Set the default mirroring. */

    set_ppu_mirroring (MIRRORING_ONE_SCREEN_2000);


    aorom_reset ();

    mmc_write = aorom_write;


    return (0);
}
