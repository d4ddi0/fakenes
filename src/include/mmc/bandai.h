

/* Mapper #16 (Bandai). */

/* This mapper is only partially supported. */


/* Todo: Add EEPROM support. */


#define PSEUDO_CLOCKS_PER_SCANLINE  114


static int bandai_init (void);

static void bandai_reset (void);


const MMC mmc_bandai =
{
    "Bandai",

    bandai_init, bandai_reset
};


static const int bandai_mirroring_table [] =
{
    MIRRORING_VERTICAL, MIRRORING_HORIZONTAL,

    MIRRORING_ONE_SCREEN_2000, MIRRORING_ONE_SCREEN_2400
};


static int bandai_enable_irqs = FALSE;


/* Defined in core.h. */

#undef byte

#undef word


struct
{
    struct
    {
#ifdef LSB_FIRST

        UINT8 low, high;
#else

        UINT8 high, low;
#endif
    }
    bytes;


    UINT16 word;
}
bandai_irq_latch;


static int bandai_irq_counter = 0;


static int bandai_irq_tick (int line)
{
    if (bandai_enable_irqs)
    {
        bandai_irq_counter -= PSEUDO_CLOCKS_PER_SCANLINE;


        if (bandai_irq_counter <= 0)
        {
            bandai_irq_counter = 0;


            bandai_enable_irqs = FALSE;

            return (TRUE);
        }
        else
        {
            return (FALSE);
        }
    }
    else
    {
        return (FALSE);
    }
}


static void bandai_write (UINT16 address, UINT8 value)
{
    /* Extract write port index. */

    address &= 0x000d;


    if (address <= 7)
    {
        /* Calculate PPU address. */

        address *= 0x0400;


        /* Set requested 1k CHR-ROM page. */

        ppu_set_ram_1k_pattern_vrom_block (address, value);
    }
    else
    {
        /* Convert $600X and $7ffX to $800X. */

        address += 0x8000;


        switch (address)
        {
            case 0x8008:

                /* Set requested 16k ROM page at $8000. */
            
                cpu_set_read_address_16k_rom_block (0x8000, value);


                break;


            case 0x8009:

                /* Mirroring select. */

                /* Mask off upper 6 bits. */

                value &= 0x03;


                /* Use value from LUT. */

                ppu_set_mirroring (bandai_mirroring_table [value]);


                break;


            case 0x800a:

                /* Enable/disable IRQs. */

                bandai_enable_irqs = (value & 0x01);

                bandai_irq_counter = bandai_irq_latch.word;


                break;


            case 0x800b:

                /* Low byte of IRQ counter. */

                bandai_irq_latch.bytes.low = value;


                break;


            case 0x800c:

                /* High byte of IRQ counter. */

                bandai_irq_latch.bytes.high = value;


                break;
        }
    }
}


static void bandai_reset (void)
{
    /* Select first 16k page in lower 16k. */

    cpu_set_read_address_16k (0x8000, FIRST_ROM_PAGE);


    /* Select last 16k page in upper 16k. */

    cpu_set_read_address_16k (0xc000, LAST_ROM_PAGE);
}


static int bandai_init (void)
{
    /* Start out with VRAM. */

    ppu_set_ram_8k_pattern_vram ();


    /* Set initial mappings. */

    bandai_reset ();


    /* Install write handlers. */

    cpu_set_write_handler_4k (0x6000, bandai_write);

    cpu_set_write_handler_4k (0x7000, bandai_write);

    cpu_set_write_handler_4k (0x8000, bandai_write);


    /* Install IRQ tick handler. */

    mmc_scanline_end = bandai_irq_tick;


    return (0);
}
