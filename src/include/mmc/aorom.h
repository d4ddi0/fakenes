

/* Mapper #7 (AOROM). */

/* This mapper is fully supported. */


#define AOROM_MIRRORING_BIT (1 << 4)


static int aorom_prg_mask = 0;


static void aorom_write (UINT16 address, UINT8 value)
{
    int index;


    /* Is this reversed? */

    ppu_set_mirroring (((value & AOROM_MIRRORING_BIT) ?
        MIRRORING_ONE_SCREEN_2400 : MIRRORING_ONE_SCREEN_2000));


    /* Mask off upper 4 bits. */

    value &= 0x0f;


    /* Convert 32k page # to 16k. */

    value <<= 1;

    value &= aorom_prg_mask;


    /* Select requested 32k page. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (value));

    cpu_set_read_address_16k (0xC000, ROM_PAGE_16K ((value + 1)));
}


static INLINE void aorom_reset (void)
{
    int index;


    /* Select first 32k page. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (0));

    cpu_set_read_address_16k (0xC000, ROM_PAGE_16K (1));
}


static INLINE int aorom_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #7 (AOROM) "
            "(%d PRG, no CHR).\n\n", ROM_PRG_ROM_PAGES);
    }


    /* Mapper requires at least 32k of PRG ROM */
    if (ROM_PRG_ROM_PAGES < 2)
    {
        return -1;
    }

    /* Calculate PRG-ROM mask. */

    if (ROM_PRG_ROM_PAGES == 1) aorom_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) aorom_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) aorom_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) aorom_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) aorom_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) aorom_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) aorom_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) aorom_prg_mask = 128;
    else aorom_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != aorom_prg_mask)
    {
        /* Page count not even power of 2. */

        return (1);
    }


    /* Convert PRG-ROM mask to 16k mask. */

    aorom_prg_mask --;


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
