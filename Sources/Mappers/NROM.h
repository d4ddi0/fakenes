/* Mapper #0 (No mapper). */
/* This mapper is fully supported. */

#include "mmc/shared.h"

static int none_init(void);
static void none_reset(void);
static void none_save_state(PACKFILE*, const int);
static void none_load_state(PACKFILE*, const int);

const MMC mmc_none = {
   0, "No mapper",
   none_init, none_reset,
   "NONE\0\0\0\0",
   none_save_state, none_load_state,
   NULL, NULL, NULL, NULL
};

static void none_reset(void)
{
   /* Select first 32k page. */
   cpu_set_read_address_32k_rom_block(0x8000, 0);

   if(ROM_CHR_ROM_PAGES > 0) {
      int page;
      /* Select first 8k page. */
      for(page = 0; page < 8; page++)
         ppu_set_1k_pattern_table_vrom_page(page << 10, page);
    }
    else
        /* No VROM is present. */
        ppu_set_8k_pattern_table_vram();
}

static int none_init(void)
{
    none_reset();
    return 0;
}

static void none_save_state(PACKFILE* file, const int version)
{
    /* Do nothing. */
}

static void none_load_state(PACKFILE* file, const int version)
{
    /* Do nothing. */
}
