

/* Mapper #2 (UNROM). */

/* This mapper is fully supported. */


static int unrom_init (void);

static void unrom_reset (void);


const MMC mmc_unrom =
{
    2, "UNROM",

    unrom_init, unrom_reset
};


static void unrom_write (UINT16 address, UINT8 value)
{
    /* Select requested 16k page. */

    cpu_set_read_address_16k_rom_block (0x8000, value);
}


static void unrom_reset (void)
{
    /* Select first 16k page in lower 16k. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (0));


    /* Select last 16k page in upper 16k. */

    cpu_set_read_address_16k (0xC000, LAST_ROM_PAGE);
}


static int unrom_init (void)
{
    /* No VROM hardware. */

    ppu_set_ram_8k_pattern_vram ();


    /* Set initial mappings. */

    unrom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, unrom_write);


    return (0);
}
