

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


    /* Convert 32k page # to 16k. */

    value *= 2;


    /* Select requested 32k page. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K(value));
    cpu_set_read_address_16k (0xC000, ROM_PAGE_16K(value + 1));
}


static INLINE void aorom_reset (void)
{
    int index;


    /* Select first 32k page. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K(0));
    cpu_set_read_address_16k (0xC000, ROM_PAGE_16K(1));
}


static INLINE int aorom_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #7 (AOROM) "
            "(%d PRG, no CHR).\n\n", ROM_PRG_ROM_PAGES);
    }


    /* No VROM hardware. */

    ppu_set_ram_8k_pattern_vram ();

    mmc_no_vrom = TRUE;


    /* Set the default mirroring. */

    set_ppu_mirroring (MIRRORING_ONE_SCREEN_2000);


    aorom_reset ();

    mmc_write = aorom_write;
    cpu_set_write_handler_32k (0x8000, aorom_write);


    return (0);
}
