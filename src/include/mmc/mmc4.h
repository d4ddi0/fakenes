

/* Mapper #10 (MMC4). */

/* This mapper is fully supported. */


static int mmc4_vrom_bank[2][2] = { { 0, 0 }, { 0, 0 } };

static int mmc4_latch[2] = { 1, 1 };


#define MMC4_MIRRORING_BIT   1


static unsigned int mmc4_prg_mask;


static void mmc4_check_latches (UINT16 address)
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
    if (mmc4_latch[bank] == latch) return;

    /* save new latch setting */
    mmc4_latch[bank] = latch;

    /* set new VROM banking */
    for (index = 0; index < 4; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((bank * 4 + index) << 10,
            mmc4_vrom_bank[bank][latch] + index);
    }

}


static void mmc4_write (UINT16 address, UINT8 value)
{
    int index;


    /* A0-A11 ignored */
    address >>= 12;


    if (address == (0xa000 >> 12))
    {
        /* 16k ROM page select (unlatched). */

        cpu_set_read_address_16k (0x8000, ROM_PAGE_16K(value & mmc4_prg_mask));
    }
    else if (address == (0xb000 >> 12))
    {
        /* Lower 4k VROM page select (latch #1). */

        /* Convert 4k page # to 1k. */

        mmc4_vrom_bank[0][0] = value * 4;

        if (mmc4_latch[0] == 0)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block (index << 10,
                    mmc4_vrom_bank[0][0] + index);
            }
        }
    }
    else if (address == (0xc000 >> 12))
    {
        /* Lower 4k VROM page select (latch #2). */

        /* Convert 4k page # to 1k. */

        mmc4_vrom_bank[0][1] = value * 4;

        if (mmc4_latch[0] == 1)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block (index << 10,
                    mmc4_vrom_bank[0][1] + index);
            }
        }
    }
    else if (address == (0xd000 >> 12))
    {
        /* Upper 4k VROM page select (latch #1). */

        /* Convert 4k page # to 1k. */

        mmc4_vrom_bank[1][0] = value * 4;

        if (mmc4_latch[1] == 0)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block ((index + 4) << 10,
                    mmc4_vrom_bank[1][0] + index);
            }
        }
    }
    else if (address == (0xe000 >> 12))
    {
        /* Upper 4k VROM page select (latch #2). */

        /* Convert 4k page # to 1k. */

        mmc4_vrom_bank[1][1] = value * 4;

        if (mmc4_latch[1] == 1)
        {
            for (index = 0; index < 4; index ++)
            {
                ppu_set_ram_1k_pattern_vrom_block ((index + 4) << 10,
                    mmc4_vrom_bank[1][1] + index);
            }
        }
    }
    else if (address == (0xf000 >> 12))
    {
        /* Mirroring select. */

        ppu_set_mirroring ((value & MMC4_MIRRORING_BIT) ?
             MIRRORING_HORIZONTAL : MIRRORING_VERTICAL);
    }
}


static INLINE void mmc4_reset (void)
{
    int index;

    /* Select first 16k page in first 16k. */

    cpu_set_read_address_16k (0x8000, ROM_PAGE_16K(0));


    /* Select last 16k page in remaining 16k. */

    cpu_set_read_address_16k (0xC000, LAST_ROM_PAGE);

    /* Setup VROM banking and latches */
    mmc4_latch[0] = 1;
    mmc4_latch[1] = 1;

    mmc4_vrom_bank[0][0] = 0;
    mmc4_vrom_bank[0][1] = 0;
    mmc4_vrom_bank[1][0] = 0;
    mmc4_vrom_bank[1][1] = 0;

    for (index = 0; index < 4; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block (index << 10,
            mmc4_vrom_bank[0][1] + index);
    }

    for (index = 0; index < 4; index ++)
    {
        ppu_set_ram_1k_pattern_vrom_block ((index + 4) << 10,
            mmc4_vrom_bank[1][1] + index);
    }

}


static INLINE int mmc4_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #10 (MMC4) (%d PRG, "
            "%d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    /* Mapper requires at least 16k of PRG ROM, and some CHR ROM */
    if (ROM_PRG_ROM_PAGES < 1 || ROM_CHR_ROM_PAGES < 1)
    {
        return -1;
    }

    if (ROM_PRG_ROM_PAGES == 1) mmc4_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) mmc4_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) mmc4_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) mmc4_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) mmc4_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) mmc4_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) mmc4_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) mmc4_prg_mask = 128;
    else mmc4_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != mmc4_prg_mask)
    {
        /* Bank count not even power of 2, unhandled. */

        return (1);
    }

    /* Convert to 16k mask. */

    mmc4_prg_mask = (mmc4_prg_mask - 1);


    mmc4_reset ();

    mmc_write = mmc4_write;
    cpu_set_write_handler_32k (0x8000, mmc4_write);


    mmc_check_latches = mmc4_check_latches;


    return (0);
}
