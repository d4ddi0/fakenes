

/* Mapper #2 (UNROM). */

/* This mapper is fully supported. */


static void unrom_write (UINT16 address, UINT8 value)
{
    /* Select requested 16k page. */

    cpu_set_read_address_16k_rom_block (0x8000, value);
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


    /* No VROM hardware. */

    ppu_set_ram_8k_pattern_vram ();


    /* Set initial mappings. */

    unrom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, unrom_write);


    return (0);
}
