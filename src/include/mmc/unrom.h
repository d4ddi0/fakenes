

/* Mapper #2 (UNROM). */

/* This mapper is fully supported. */


static int unrom_prg_mask = 0;


static void unrom_write (UINT16 address, UINT8 value)
{
    /* Convert 16k page # to 16k. */

    value = (value & unrom_prg_mask);


    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (value));
}


static INLINE void unrom_reset (void)
{
    /* Select first 16k page in lower 16k. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (0));


    /* Select last 16k page in upper 16k. */

    cpu_set_read_address_16k (0xC000, LAST_ROM_PAGE);
}


static INLINE int unrom_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #2 (UNROM) "
            "(%d PRG, no CHR).\n\n", ROM_PRG_ROM_PAGES);
    }


    if (ROM_PRG_ROM_PAGES == 1) unrom_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) unrom_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) unrom_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) unrom_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) unrom_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) unrom_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) unrom_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) unrom_prg_mask = 128;
    else unrom_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != unrom_prg_mask)
    {
        /* Page count not even power of 2. */

        return (1);
    }

    /* Convert mask to 16k mask. */

    unrom_prg_mask = (unrom_prg_mask - 1);


    /* No VROM hardware. */

    ppu_set_ram_8k_pattern_vram ();


    /* Set initial mappings. */

    unrom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, unrom_write);


    return (0);
}
