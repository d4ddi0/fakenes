

/* Mapper #4 (MMC3). */

/* This mapper is fully supported. */


static int mmc3_command = 0;


static int mmc3_prg_address = 0;

static int mmc3_chr_address = 0;


static int mmc3_irq_counter = 0;

static int mmc3_irq_latch = 0;


static unsigned int mmc3_prg_mask;

static unsigned int mmc3_chr_mask;


#define MMC3_PRG_ADDRESS_BIT 0x40

#define MMC3_CHR_ADDRESS_BIT 0x80


static int mmc3_irq_tick (int line)
{
    if ((line >= FIRST_DISPLAYED_LINE) &&
        (line <= LAST_DISPLAYED_LINE))
    {
        mmc3_irq_counter --;
    }


    return ((mmc3_irq_counter ? 0 : 1));
}


static void mmc3_write (UINT16 address, UINT8 value)
{
    int scrap;


    switch (address)
    {
        case 0x8000:

            /* Bits 0 to 2 (command). */

            mmc3_command = (value & 0x07);


            /* Bits 6 and 7 (latches). */

            mmc3_prg_address = (value & MMC3_PRG_ADDRESS_BIT);

            mmc3_chr_address = (value & MMC3_CHR_ADDRESS_BIT);


            break;


        case 0x8001:

            switch (mmc3_command)
            {
                /* VROM swapping commands. */

                case 0:

                case 1:

                    /* 2 1k pages at $0000 or $0800. */

                    scrap = (mmc3_command * 2);

                    scrap ^= (mmc3_chr_address ? 4 : 0);


                    mmc_vrom_banks [scrap] =
                        VROM_PAGE_1K (value & mmc3_chr_mask);

                    mmc_vrom_banks [++ scrap] =
                        VROM_PAGE_1K (++value & mmc3_chr_mask);


                    break;


                case 2:

                case 3:

                case 4:

                case 5:

                    /* 1 1k page at $1000 to $1c00. */

                    scrap = (mmc3_command + 2);

                    scrap ^= (mmc3_chr_address ? 4 : 0);


                    mmc_vrom_banks [scrap] =
                        VROM_PAGE_1K (value & mmc3_chr_mask);


                    break;


                /* ROM swapping commands. */

                case 6:

                    /* 1 8k page at $8000 or $a000. */

                    scrap = (mmc3_prg_address ? 1 : 0);


                    mmc_rom_banks [scrap] =
                        ROM_PAGE_8K (value & mmc3_prg_mask);


                    break;


                case 7:

                    /* 1 8k page at $a000 or $c000. */

                    scrap = (mmc3_prg_address ? 2 : 1);


                    mmc_rom_banks [scrap] =
                        ROM_PAGE_8K (value & mmc3_prg_mask);


                    break;
            }


            break;


        case 0xa000:

            /* Mirroring select. */

            set_ppu_mirroring (((value & 1) ?
                MIRRORING_HORIZONTAL : MIRRORING_VERTICAL));


            break;


        case 0xa001:

            /* SRAM disable & enable. */

            mmc_disable_sram = (! (value >> 7));


            break;


        case 0xc000:

            /* Set IRQ counter. */

            mmc3_irq_counter = value;


            break;


        case 0xc001:

            /* Set IRQ latch. */

            mmc3_irq_latch = value;


            break;


        case 0xe000:

            /* Disable IRQs. */

            mmc_disable_irqs = TRUE;


            /* Copy latch to counter. */

            mmc3_irq_counter = mmc3_irq_latch;


            break;


        case 0xe001:

            /* Enable IRQs. */

            mmc_disable_irqs = FALSE;


            break;


        default:

            break;
    }
}


static INLINE void mmc3_reset (void)
{
    int index;


    /* Select first 16k page in lower 16k. */

    mmc_rom_banks [0] = ROM_PAGE_8K (0 & mmc3_prg_mask);

    mmc_rom_banks [1] = ROM_PAGE_8K (1 & mmc3_prg_mask);


    /* Select last 16k page in upper 16k. */

    mmc_rom_banks [2] = LAST_ROM_PAGE;

    mmc_rom_banks [3] = (LAST_ROM_PAGE + 0x2000);


    if (ROM_CHR_ROM_PAGES > 0)
    {
        /* Select first 8k page. */

        for (index = 0; index < 8; index ++)
        {
            mmc_vrom_banks [index] = VROM_PAGE_1K (index);
        }
    }    
}


static INLINE int mmc3_init (void)
{
    printf ("Using memory mapper #4 (MMC3) "
        "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);


    if ((ROM_PRG_ROM_PAGES) == 1) mmc3_prg_mask = 1;
    else if ((ROM_PRG_ROM_PAGES) == 2) mmc3_prg_mask = 2;
    else if ((ROM_PRG_ROM_PAGES) <= 4) mmc3_prg_mask = 4;
    else if ((ROM_PRG_ROM_PAGES) <= 8) mmc3_prg_mask = 8;
    else if ((ROM_PRG_ROM_PAGES) <= 16) mmc3_prg_mask = 16;
    else if ((ROM_PRG_ROM_PAGES) <= 32) mmc3_prg_mask = 32;
    else if ((ROM_PRG_ROM_PAGES) <= 64) mmc3_prg_mask = 64;
    else if ((ROM_PRG_ROM_PAGES) <= 128) mmc3_prg_mask = 128;
    else mmc3_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != mmc3_prg_mask)
    {
        /* Bank count not even power of 2, unhandled. */

        return (1);
    }

    /* Convert 16k mask to 8k mask. */

    mmc3_prg_mask = ((mmc3_prg_mask * 2) - 1);


    if (! ROM_CHR_ROM_PAGES > 0)
    {
        /* No VROM is present. */

        mmc_no_vrom = TRUE;
    }
    else
    {
        mmc_no_vrom = FALSE;
    }


    if ((ROM_CHR_ROM_PAGES) == 0)
    {
        mmc3_chr_mask = 1;
    }
    else
    {
        if ((ROM_CHR_ROM_PAGES) == 1) mmc3_chr_mask = 1;
        else if ((ROM_CHR_ROM_PAGES) == 2) mmc3_chr_mask = 2;
        else if ((ROM_CHR_ROM_PAGES) <= 4) mmc3_chr_mask = 4;
        else if ((ROM_CHR_ROM_PAGES) <= 8) mmc3_chr_mask = 8;
        else if ((ROM_CHR_ROM_PAGES) <= 16) mmc3_chr_mask = 16;
        else if ((ROM_CHR_ROM_PAGES) <= 32) mmc3_chr_mask = 32;
        else mmc3_chr_mask = 256;


        if (ROM_CHR_ROM_PAGES != mmc3_chr_mask)
        {
            /* Bank count not even power of 2, unhandled. */

            return (1);
        }
    }

    /* Convert 8k mask to 1k mask. */

    mmc3_chr_mask = ((mmc3_chr_mask * 8) - 1);


    mmc3_reset ();

    mmc_write = mmc3_write;


    mmc_scanline_start = mmc3_irq_tick;


    return (0);
}
