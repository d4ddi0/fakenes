

/* Mapper #3 (CNROM). */

/* This mapper is fully supported. */


static unsigned int cnrom_chr_mask;


static void cnrom_write (UINT16 address, UINT8 value)
{
    int index;


    /* Convert 8k page # to 1k. */

    value &= cnrom_chr_mask;

    value *= 8;


    /* Select requested 8k page. */

    for (index = 0; index < 8; index ++)
    {
        mmc_vrom_banks [index] = VROM_PAGE_1K ((value + index));
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
        mmc_vrom_banks [index] = VROM_PAGE_1K (index);
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


    /* Ugh. :) */

    if (ROM_CHR_ROM_PAGES == 1) cnrom_chr_mask = 1;
    else if (ROM_CHR_ROM_PAGES == 2) cnrom_chr_mask = 2;
    else if (ROM_CHR_ROM_PAGES <= 4) cnrom_chr_mask = 4;
    else if (ROM_CHR_ROM_PAGES <= 8) cnrom_chr_mask = 8;
    else if (ROM_CHR_ROM_PAGES <= 16) cnrom_chr_mask = 16;
    else if (ROM_CHR_ROM_PAGES <= 32) cnrom_chr_mask = 32;
    else if (ROM_CHR_ROM_PAGES <= 64) cnrom_chr_mask = 64;
    else if (ROM_CHR_ROM_PAGES <= 128) cnrom_chr_mask = 128;
    else cnrom_chr_mask = 256;


    if (ROM_CHR_ROM_PAGES != cnrom_chr_mask)
    {
        /* Bank count not even power of 2, unhandled. */

        return (1);
    }


    cnrom_chr_mask --;


    cnrom_reset ();

    mmc_write = cnrom_write;
    cpu_set_write_handler_32k (0x8000, cnrom_write);


    return (0);
}
