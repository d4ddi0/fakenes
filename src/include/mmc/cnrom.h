

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
        ppu_set_ram_1k_pattern_vrom_block (index << 10, value + index);
    }
}


static INLINE void cnrom_reset (void)
{
    int index;


    if (ROM_PRG_ROM_PAGES == 1)
    {
        /* Select first 16k page (mirrored). */

        cpu_set_read_address_16k (0x8000, ROM_PAGE_16K(0));
        cpu_set_read_address_16k (0xC000, ROM_PAGE_16K(0));
    }
    else
    {
        /* Select first 32k page. */

        cpu_set_read_address_16k (0x8000, ROM_PAGE_16K(0));
        cpu_set_read_address_16k (0xC000, ROM_PAGE_16K(1));
    }


    /* Select first 8k page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block (index << 10, index);
    }
}


static INLINE int cnrom_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #3 (CNROM) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    mmc_no_vrom = FALSE;


    cnrom_reset ();

    mmc_write = cnrom_write;
    cpu_set_write_handler_32k (0x8000, cnrom_write);


    return (0);
}
