

/* Mapper #11 (Color Dreams). */

/* This mapper is fully supported. */


#include "mmc/shared.h"


static int dreams_init (void);

static void dreams_reset (void);


const MMC mmc_dreams =
{
    11, "Color Dreams",

    dreams_init, dreams_reset
};


#define DREAMS_PRG_ROM_MASK     0x0f

#define DREAMS_CHR_ROM_MASK     0x70


static void dreams_write (UINT16 address, UINT8 value)
{
    int index;


    int prg_page;

    int chr_page;


    /* Extract ROM page # (0000xxxx). */

    prg_page = (value & DREAMS_PRG_ROM_MASK);


    /* Select requested 32k ROM page. */

    cpu_set_read_address_32k_rom_block (0x8000, prg_page);


    /* Extract CHR-ROM page # (0xxx0000). */

    chr_page = ((value & DREAMS_CHR_ROM_MASK) >> 4);


    /* Convert 8k page # to 1k. */

    chr_page *= 8;


    /* Select requested 8k CHR-ROM page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), (chr_page + index));
    }
}


static void dreams_reset (void)
{
    int index;


    /* Select first 32k ROM page. */

    cpu_set_read_address_32k_rom_block (0x8000, MMC_FIRST_ROM_BLOCK);


    /* Select first 8k CHR-ROM page. */

    for (index = 0; index < 8; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index << 10), index);
    }
}


static int dreams_init (void)
{
    /* Set initial mappings. */

    dreams_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, dreams_write);


    return (0);
}
