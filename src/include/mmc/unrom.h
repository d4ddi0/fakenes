

/* Mapper #2 (UNROM). */

/* This mapper is fully supported. */


static int unrom_prg_mask;

static void unrom_write (UINT16 address, UINT8 value)
{
    /* Convert 16k page # to 8k. */

    value = (value & unrom_prg_mask) * 2;


    mmc_rom_banks [0] = ROM_PAGE_8K (value);

    mmc_rom_banks [1] = ROM_PAGE_8K (value + 1);
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
        /* Bank count not even power of 2, unhandled. */

        return (1);
    }

    /* Convert mask to 16k mask. */

    unrom_prg_mask = (unrom_prg_mask - 1);


    /* No VROM hardware. */

    mmc_no_vrom = TRUE;


    unrom_reset ();

    mmc_write = unrom_write;


    return (0);
}
