

/* Mapper #9 (MMC2). */

/* This mapper is fully supported. */


/* Mapper #10 (MMC4). */

/* This mapper is fully supported. */


static int mmc2_init (void);

static void mmc2_reset (void);


const MMC mmc_mmc2 =
{
    9, "MMC2",

    mmc2_init, mmc2_reset,


    "MMC2\0\0\0\0",

    null_save_state, null_load_state
};


static int mmc4_init (void);

static void mmc4_reset (void);


const MMC mmc_mmc4 =
{
    10, "MMC4",

    mmc4_init, mmc4_reset,


    "MMC4\0\0\0\0",

    null_save_state, null_load_state
};


static int mmc2and4_vrom_bank[2][2] = { { 0, 0 }, { 0, 0 } };

static int mmc2and4_latch[2] = { 1, 1 };

/* 0 if MMC2 (8k ROM banks), 1 if MMC4 (16k ROM banks) */
static char mmc2and4_rom_bank_size;


#define MMC2AND4_MIRRORING_BIT   1


static void mmc2and4_check_latches (UINT16 address)
{
    int bank, index, latch;

    /* only accesses < 0x2000 affect latches */
    if (address >= 0x2000) return;

    /* is this latch for $0000 or $1000? */
    bank = address >> 12;

    /* we don't need bit 12 or bits 0-3, get rid of them */
    address &= 0xFF0;

    /* does address lie within valid range? FD0-FEF */
    if (address < 0xFD0 || address > 0xFEF) return;

    /* convert address to latch # */
    latch = (address - 0xFD0) >> 4;

    /* return if there's no change */
    if (mmc2and4_latch[bank] == latch) return;

    /* save new latch setting */
    mmc2and4_latch[bank] = latch;

     /* set new VROM banking */
    for (index = 0; index < 4; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((bank * 4 + index) << 10,
            mmc2and4_vrom_bank[bank][latch] + index);
    }
}


static void mmc2and4_write (UINT16 address, UINT8 value)
{
    int index;


    /* A0-A11 ignored */
    address >>= 12;


    if (address == (0xa000 >> 12))
    {
        if (!mmc2and4_rom_bank_size)
        /* 8k ROM page select (unlatched). */
        {
            cpu_set_read_address_8k_rom_block (0x8000, value);
        }
        else
        /* 16k ROM page select (unlatched). */
        {
            cpu_set_read_address_16k_rom_block (0x8000, value);
        }
    }
    else if (address == (0xb000 >> 12))
    {
        /* Lower 4k VROM page select (latch #1). */

        /* Convert 4k page # to 1k. */

        mmc2and4_vrom_bank[0][0] = value * 4;

        if (mmc2and4_latch[0] == 0)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block (index << 10,
                    mmc2and4_vrom_bank[0][0] + index);
            }
        }
    }
    else if (address == (0xc000 >> 12))
    {
        /* Lower 4k VROM page select (latch #2). */

        /* Convert 4k page # to 1k. */

        mmc2and4_vrom_bank[0][1] = value * 4;

        if (mmc2and4_latch[0] == 1)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block (index << 10,
                    mmc2and4_vrom_bank[0][1] + index);
            }
        }
    }
    else if (address == (0xd000 >> 12))
    {
        /* Upper 4k VROM page select (latch #1). */

        /* Convert 4k page # to 1k. */

        mmc2and4_vrom_bank[1][0] = value * 4;

        if (mmc2and4_latch[1] == 0)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block ((index + 4) << 10,
                    mmc2and4_vrom_bank[1][0] + index);
            }
        }
    }
    else if (address == (0xe000 >> 12))
    {
        /* Upper 4k VROM page select (latch #2). */

        /* Convert 4k page # to 1k. */

        mmc2and4_vrom_bank[1][1] = value * 4;

        if (mmc2and4_latch[1] == 1)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block ((index + 4) << 10,
                    mmc2and4_vrom_bank[1][1] + index);
            }
        }
    }
    else if (address == (0xf000 >> 12))
    {
        /* Mirroring select. */

        ppu_set_mirroring ((value & MMC2AND4_MIRRORING_BIT) ?
             MIRRORING_HORIZONTAL : MIRRORING_VERTICAL);
    }
}


static void mmc2and4_reset_vrom (void)
{
    int index;

    /* Setup VROM banking and latches */
    mmc2and4_latch[0] = 1;
    mmc2and4_latch[1] = 1;

    mmc2and4_vrom_bank[0][0] = 0;
    mmc2and4_vrom_bank[0][1] = 0;
    mmc2and4_vrom_bank[1][0] = 0;
    mmc2and4_vrom_bank[1][1] = 0;

    for (index = 0; index < 4; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block (index << 10,
            mmc2and4_vrom_bank[0][1] + index);
    }

    for (index = 0; index < 4; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index + 4) << 10,
            mmc2and4_vrom_bank[1][1] + index);
    }
}


/* MMC2-specific reset and init */
static void mmc2_reset (void)
{
    /* Select first 8k page in first 8k. */

    cpu_set_read_address_8k (0x8000, FIRST_ROM_PAGE);


    /* Select 3rd to last 8k page in second 8k. */

    cpu_set_read_address_8k_rom_block (0xA000, (ROM_PRG_ROM_PAGES * 2) - 3);


    /* Select last 16k page in remaining 16k. */

    cpu_set_read_address_16k (0xC000, LAST_ROM_PAGE);

    /* Setup VROM banking and latches */
    mmc2and4_reset_vrom();
}


static int mmc2_init (void)
{
    /* Mapper requires some CHR ROM */
    if (ROM_CHR_ROM_PAGES < 1)
    {
        return -1;
    }

    mmc2and4_rom_bank_size = FALSE;
    mmc2_reset ();

    cpu_set_write_handler_8k (0xA000, mmc2and4_write);
    cpu_set_write_handler_16k (0xC000, mmc2and4_write);


    mmc_check_latches = mmc2and4_check_latches;


    return (0);
}


/* MMC4-specific reset and init */
static void mmc4_reset (void)
{
    int index;

    /* Select first 16k page in first 16k. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K(0));


    /* Select last 16k page in remaining 16k. */

    cpu_set_read_address_16k (0xC000, LAST_ROM_PAGE);

    /* Setup VROM banking and latches */
    mmc2and4_reset_vrom();
}


static int mmc4_init (void)
{
    /* Mapper requires some CHR ROM */
    if (ROM_CHR_ROM_PAGES < 1)
    {
        return -1;
    }

    mmc2and4_rom_bank_size = TRUE;
    mmc4_reset ();

    cpu_set_write_handler_8k (0xA000, mmc2and4_write);
    cpu_set_write_handler_16k (0xC000, mmc2and4_write);


    mmc_check_latches = mmc2and4_check_latches;


    return (0);
}

