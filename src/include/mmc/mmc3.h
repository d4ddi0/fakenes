

/* Mapper #4 (MMC3). */

/* This mapper is fully supported. */


static int mmc3_command = 0;


static int mmc3_prg_address = 0x8000;

static int mmc3_chr_address = 0;

static int mmc3_cpu_bank[2];


static int mmc3_irq_counter = 0;

static int mmc3_irq_latch = 0;

static int mmc3_disable_irqs = TRUE;


static unsigned int mmc3_prg_mask;


#define MMC3_PRG_ADDRESS_BIT 0x40

#define MMC3_CHR_ADDRESS_BIT 0x80


static int mmc3_irq_tick (int line)
{
    if (((line >= FIRST_DISPLAYED_LINE) &&
        (line <= LAST_DISPLAYED_LINE)) &&
        (background_enabled || sprites_enabled))
    {
        if (mmc3_irq_counter --) return 0;

        /* Load next counter position */
        mmc3_irq_counter = mmc3_irq_latch;

        if (mmc3_disable_irqs) return 0;
        return 1;
    }


    return 0;
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

            mmc3_chr_address = (value & MMC3_CHR_ADDRESS_BIT) ? 4 : 0;

            scrap = (value & MMC3_PRG_ADDRESS_BIT) ? 0xC000 : 0x8000;
            if (mmc3_prg_address == scrap) break;

            mmc3_prg_address = scrap;

            if (!mmc3_prg_address)
            /* 8000,A000 swappable */
            {
                /* set address for non-swappable page */
                cpu_set_read_address_8k (0xC000, LAST_ROM_PAGE);

                /* set addresses for swappable pages */
                cpu_set_read_address_8k (0x8000,
                    ROM_PAGE_8K(mmc3_cpu_bank[0]));
                cpu_set_read_address_8k (0xA000,
                    ROM_PAGE_8K(mmc3_cpu_bank[1]));
                
            }
            else
            /* A000,C000 swappable */
            {
                /* set address for non-swappable page */
                cpu_set_read_address_8k (0x8000, LAST_ROM_PAGE);

                /* set addresses for swappable pages */
                cpu_set_read_address_8k (0xA000,
                    ROM_PAGE_8K(mmc3_cpu_bank[1]));
                cpu_set_read_address_8k (0xC000,
                    ROM_PAGE_8K(mmc3_cpu_bank[0]));
                
            }


            break;


        case 0x8001:

            switch (mmc3_command)
            {
                /* VROM swapping commands. */

                case 0:

                case 1:

                    /* 2 1k pages at $0000 or $0800. */

                    if (ROM_CHR_ROM_PAGES > 0)
                    {
                        scrap = (mmc3_command * 2) ^ mmc3_chr_address;

                        ppu_set_ram_1k_pattern_vrom_block (scrap << 10,
                            (value & ~1));

                        ppu_set_ram_1k_pattern_vrom_block (++scrap << 10,
                            (value | 1));
                    }

                    break;


                case 2:

                case 3:

                case 4:

                case 5:

                    /* 1 1k page at $1000 to $1c00. */

                    if (ROM_CHR_ROM_PAGES > 0)
                    {
                        scrap = (mmc3_command + 2) ^ mmc3_chr_address;

                        ppu_set_ram_1k_pattern_vrom_block (scrap << 10,
                            value);
                    }

                    break;


                /* ROM swapping commands. */

                case 6:

                    /* 1 8k page at $8000 or $C000. */

                    mmc3_cpu_bank[0] = value & mmc3_prg_mask;

                    cpu_set_read_address_8k (mmc3_prg_address,
                        ROM_PAGE_8K(value & mmc3_prg_mask));


                    break;


                case 7:

                    /* 1 8k page at $A000. */

                    mmc3_cpu_bank[1] = value & mmc3_prg_mask;

                    cpu_set_read_address_8k (0xA000,
                        ROM_PAGE_8K(value & mmc3_prg_mask));


                    break;
            }


            break;


        case 0xa000:

            /* Mirroring select. */

            if (! (global_rom.control_byte_1 & ROM_CTRL_4SCREEN))
            {
                ppu_set_mirroring (((value & 1) ?
                    MIRRORING_HORIZONTAL : MIRRORING_VERTICAL));
            }


            break;


        case 0xa001:

            /* SRAM disable & enable. */

            /* Disabled for Star Tropics. */

            //if (value & 0x80) enable_sram();
            //else disable_sram();


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

            mmc3_disable_irqs = TRUE;


            break;


        case 0xe001:

            /* Enable IRQs. */

            mmc3_disable_irqs = FALSE;


            break;


        default:

            break;
    }
}


static INLINE void mmc3_reset (void)
{
    int index;


    /* Reset address latches */
    mmc3_prg_address = 0x8000;
    mmc3_chr_address = 0;

    /* Select first 16k page in lower 16k. */

    mmc3_cpu_bank[0] = 0;
    mmc3_cpu_bank[1] = (1 & mmc3_prg_mask);
    cpu_set_read_address_8k (0x8000, ROM_PAGE_8K(0));
    cpu_set_read_address_8k (0xA000, ROM_PAGE_8K(1 & mmc3_prg_mask));


    /* Select last 16k page in upper 16k. */

    cpu_set_read_address_16k (0xC000, LAST_ROM_PAGE);


    if (ROM_CHR_ROM_PAGES > 0)
    {
        /* Select first 8k page. */

        for (index = 0; index < 8; index ++)
        {
            ppu_set_ram_1k_pattern_vrom_block (index << 10, index);
        }
    }    
}


static INLINE int mmc3_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #4 (MMC3) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    if (ROM_PRG_ROM_PAGES == 1) mmc3_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) mmc3_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) mmc3_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) mmc3_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) mmc3_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) mmc3_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) mmc3_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) mmc3_prg_mask = 128;
    else mmc3_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != mmc3_prg_mask)
    {
        /* Bank count not even power of 2, unhandled. */

        return (1);
    }

    /* Convert 16k mask to 8k mask. */

    mmc3_prg_mask = ((mmc3_prg_mask * 2) - 1);


    if (ROM_CHR_ROM_PAGES == 0)
    {
        /* No VROM is present. */

        ppu_set_ram_8k_pattern_vram ();
    }


    mmc3_reset ();

    mmc_write = mmc3_write;
    cpu_set_write_handler_32k (0x8000, mmc3_write);


    mmc_scanline_start = mmc3_irq_tick;
    mmc3_disable_irqs = TRUE;


    return (0);
}
