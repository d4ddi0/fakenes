

/* Mapper #66 (GNROM). */

/* This mapper is fully supported. */


static int gnrom_prg_mask = 0;


static void gnrom_write (UINT16 address, UINT8 value)
{
    int index;

    int chr_bank, prg_bank;


    chr_bank = (value & 0x0f);


    /* Convert 32k page # to 16k. */

    prg_bank = ((((value & 0xf0) >> 4) * 2));


    /* Select requested 32k page. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K ((prg_bank & gnrom_prg_mask)));

    cpu_set_read_address_16k (0xc000, ROM_PAGE_16K (((prg_bank + 1) & gnrom_prg_mask)));


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


    /* Select first 16k page in lower 16k. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (0));


    /* Select second 16k page in upper 16k. */

    cpu_set_read_address_16k (0xc000, ROM_PAGE_16K ((1 & gnrom_prg_mask)));


    /* Select first 8k page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), index);
    }
}


static INLINE int gnrom_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #66 (GNROM) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    if (ROM_PRG_ROM_PAGES == 1) gnrom_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) gnrom_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) gnrom_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) gnrom_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) gnrom_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) gnrom_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) gnrom_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) gnrom_prg_mask = 128;
    else gnrom_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != gnrom_prg_mask)
    {
        /* Page count not an even power of 2. */

        return (1);
    }


    /* Convert mask to 16k mask. */

    gnrom_prg_mask --;


    /* Set initial mappings. */

    gnrom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, gnrom_write);


    return (0);
}
