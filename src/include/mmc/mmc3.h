

/* Mapper #4 (MMC3). */

/* This mapper is fully supported. */


static int mmc3_init (void);

static void mmc3_reset (void);


const MMC mmc_mmc3 =
{
    4, "MMC3",

    mmc3_init, mmc3_reset
};


static int mmc3_command = 0;


static int mmc3_prg_address = 0x8000;
static int mmc3_chr_address = 0;

static UINT8 mmc3_prg_bank[2];
static UINT8 mmc3_chr_bank[6];


static int mmc3_irq_counter = 0;

static int mmc3_irq_latch = 0;

static int mmc3_disable_irqs = TRUE;
static int mmc3_counter_latched = FALSE;


#define MMC3_PRG_ADDRESS_BIT 0x40

#define MMC3_CHR_ADDRESS_BIT 0x80


static int mmc3_irq_tick (int line)
{
    if (
        (((line >= FIRST_DISPLAYED_LINE) && (line <= LAST_DISPLAYED_LINE)) ||
            (line == ppu_frame_last_line)) &&
        (PPU_BACKGROUND_ENABLED || PPU_SPRITES_ENABLED))
    {
        mmc3_counter_latched = TRUE;

        if (mmc3_irq_counter --) return 0;

        /* Load next counter position */
        mmc3_irq_counter = mmc3_irq_latch;

        if (mmc3_disable_irqs) return 0;

        mmc3_counter_latched = FALSE;

        return 1;
    }


    return 0;
}


static void mmc3_write (UINT16 address, UINT8 value)
{
    int scrap;


    switch (address & 0xE001)
    {
        case 0x8000:

            /* Bits 0 to 2 (command). */

            mmc3_command = (value & 0x07);


            /* Bits 6 and 7 (latches). */

            scrap = (value & MMC3_CHR_ADDRESS_BIT) ? 4 : 0;
            if (mmc3_chr_address != scrap && ROM_CHR_ROM_PAGES > 0)
            {
                mmc3_chr_address = scrap;

                ppu_set_ram_1k_pattern_vrom_block (
                    (mmc3_chr_address << 10), mmc3_chr_bank [0] & ~1);
                ppu_set_ram_1k_pattern_vrom_block (
                    ((1 + mmc3_chr_address) << 10), mmc3_chr_bank [0] | 1);

                ppu_set_ram_1k_pattern_vrom_block (
                    ((2 + mmc3_chr_address) << 10), mmc3_chr_bank [1] & ~1);
                ppu_set_ram_1k_pattern_vrom_block (
                    ((3 + mmc3_chr_address) << 10), mmc3_chr_bank [1] | 1);

                ppu_set_ram_1k_pattern_vrom_block (
                    ((4 - mmc3_chr_address) << 10), mmc3_chr_bank [2]);

                ppu_set_ram_1k_pattern_vrom_block (
                    ((5 - mmc3_chr_address) << 10), mmc3_chr_bank [3]);

                ppu_set_ram_1k_pattern_vrom_block (
                    ((6 - mmc3_chr_address) << 10), mmc3_chr_bank [4]);

                ppu_set_ram_1k_pattern_vrom_block (
                    ((7 - mmc3_chr_address) << 10), mmc3_chr_bank [5]);
            }


            scrap = (value & MMC3_PRG_ADDRESS_BIT) ? 0xC000 : 0x8000;
            if (mmc3_prg_address != scrap)
            {
                mmc3_prg_address = scrap;

                /* set address for non-swappable page */
                cpu_set_read_address_8k (
                    (mmc3_prg_address == 0x8000) ? 0xC000 : 0x8000,
                    LAST_ROM_PAGE);

                /* set addresses for swappable pages */
                cpu_set_read_address_8k_rom_block (mmc3_prg_address,
                    mmc3_prg_bank[0]);
                cpu_set_read_address_8k_rom_block (0xA000,
                    mmc3_prg_bank[1]);
            }

            break;


        case 0x8001:

            switch (mmc3_command)
            {
                /* VROM swapping commands. */

                case 0:

                case 1:

                    /* 2 1k pages at $0000 or $0800. */

                    mmc3_chr_bank [mmc3_command] = value;

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

                    mmc3_chr_bank [mmc3_command] = value;

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

                    mmc3_prg_bank [0] = value;

                    cpu_set_read_address_8k_rom_block (mmc3_prg_address,
                        value);


                    break;


                case 7:

                    /* 1 8k page at $A000. */

                    mmc3_prg_bank [1] = value;

                    cpu_set_read_address_8k_rom_block (0xA000, value);


                    break;
            }


            break;


        case 0xa000:

            /* Mirroring select. */

            if (! (global_rom.control_byte_1 & ROM_CTRL_FOUR_SCREEN))
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

            mmc3_irq_latch = value;
            if (!mmc3_counter_latched) mmc3_irq_counter = mmc3_irq_latch;


            break;


        case 0xc001:

            /* Set IRQ latch. */

            mmc3_counter_latched = FALSE;
            mmc3_irq_counter = mmc3_irq_latch;


            break;


        case 0xe000:

            /* Disable IRQs. */

            mmc3_disable_irqs = TRUE;
            if (!mmc3_counter_latched) mmc3_irq_counter = mmc3_irq_latch;


            break;


        case 0xe001:

            /* Enable IRQs. */

            mmc3_disable_irqs = FALSE;
            if (!mmc3_counter_latched) mmc3_irq_counter = mmc3_irq_latch;


            break;


        default:

            break;
    }
}


static void mmc3_reset (void)
{
    int index;


    /* Reset address latches */
    mmc3_prg_address = 0x8000;
    mmc3_chr_address = 0;

    /* Select first 16k page in lower 16k. */

    mmc3_prg_bank [0] = 0;
    mmc3_prg_bank [1] = 1;
    cpu_set_read_address_8k_rom_block (0x8000, mmc3_prg_bank [0]);
    cpu_set_read_address_8k_rom_block (0xA000, mmc3_prg_bank [1]);


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


static int mmc3_init (void)
{
    if (ROM_CHR_ROM_PAGES == 0)
    {
        /* No VROM is present. */

        ppu_set_ram_8k_pattern_vram ();
    }


    mmc3_reset ();

    cpu_set_write_handler_32k (0x8000, mmc3_write);


    mmc_scanline_end = mmc3_irq_tick;
    mmc3_disable_irqs = TRUE;


    return (0);
}
