

/* Mapper #2 (UNROM). */

/* This mapper is fully supported. */


#include "mmc/shared.h"


static int unrom_init (void);

static void unrom_reset (void);


static void unrom_save_state (PACKFILE *, int);

static void unrom_load_state (PACKFILE *, int);


static const MMC mmc_unrom =
{
    2, "UNROM",

    unrom_init, unrom_reset,


    "UNROM\0\0\0",

    unrom_save_state, unrom_load_state
};


static UINT8 unrom_last_write = 0;


static void unrom_write (UINT16 address, UINT8 value)
{
    /* Store page # for state saving. */

    unrom_last_write = value;


    /* Select requested 16k ROM page. */

    cpu_set_read_address_16k_rom_block (0x8000, value);
}


static void unrom_reset (void)
{
    /* Select first 16k ROM page in lower 16k. */

    cpu_set_read_address_16k_rom_block (0x8000, MMC_FIRST_ROM_BLOCK);


    /* Select last 16k ROM page in upper 16k. */

    cpu_set_read_address_16k_rom_block (0xC000, MMC_LAST_ROM_BLOCK);
}


static int unrom_init (void)
{
    /* No VROM hardware. */

    ppu_set_ram_8k_pattern_vram ();

    mmc_pattern_vram_in_use = TRUE;


    /* Set initial mappings. */

    unrom_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, unrom_write);


    return (0);
}


static void unrom_save_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    pack_putc (unrom_last_write, file_chunk);


    pack_fclose_chunk (file_chunk);
}


static void unrom_load_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    unrom_write (0x8000, pack_getc (file_chunk));


    pack_fclose_chunk (file_chunk);

}
