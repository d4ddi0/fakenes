

/* Mapper #7 (AOROM). */

/* This mapper is fully supported. */


static int aorom_init (void);

static void aorom_reset (void);


const MMC mmc_aorom =
{
    7, "AOROM",

    aorom_init, aorom_reset
};


#define AOROM_MIRRORING_BIT (1 << 4)


static void aorom_write (UINT16 address, UINT8 value)
{
    int index;


    /* Is this reversed? */

    ppu_set_mirroring (((value & AOROM_MIRRORING_BIT) ?
        MIRRORING_ONE_SCREEN_2400 : MIRRORING_ONE_SCREEN_2000));


    /* Mask off upper 4 bits. */

    value &= 0x0f;


    /* Select requested 32k page. */

    cpu_set_read_address_32k_rom_block (0x8000, value);

}


static void aorom_reset (void)
{
    int index;


    /* Select first 32k page. */

    cpu_set_read_address_32k_rom_block (0x8000, 0);

}


static int aorom_init (void)
{
    /* No VROM hardware. */

    ppu_set_ram_8k_pattern_vram ();


    /* Set the default mirroring. */

    ppu_set_mirroring (MIRRORING_ONE_SCREEN_2000);


    /* Set initial mappings. */

    aorom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, aorom_write);


    return (0);
}
