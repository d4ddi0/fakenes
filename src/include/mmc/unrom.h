

/* Mapper #2 (UNROM). */

/* This mapper is fully supported. */


static void unrom_write (UINT16 address, UINT8 value)
{
    /* Convert 16k page # to 8k. */

    value *= 2;


    mmc_rom_banks [0] = ROM_PAGE_8K (value);

    mmc_rom_banks [1] = ROM_PAGE_8K (++ value);
}


static INLINE void unrom_reset (void)
{
    /* Select first 16k page in lower 16k. */

    mmc_rom_banks [0] = ROM_PAGE_8K (0);

    mmc_rom_banks [1] = ROM_PAGE_8K (1);


    /* Select last 16k page in upper 16k. */

    mmc_rom_banks [2] = LAST_ROM_PAGE;

    mmc_rom_banks [3] = (LAST_ROM_PAGE + 0x2000);
}


static INLINE int unrom_init (void)
{
    printf ("Using memory mapper #2 (UNROM) "
        "(%d PRG, no CHR).\n\n", ROM_PRG_ROM_PAGES);


    /* No VROM hardware. */

    mmc_no_vrom = TRUE;


    unrom_reset ();

    mmc_write = unrom_write;


    return (0);
}
