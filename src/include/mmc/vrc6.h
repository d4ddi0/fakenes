

/* Mappers #24 (Konami VRC6). */

/* This mapper is fully supported. */


/* Mapper #26 (Konami VRC6V). */

/* This mapper is fully supported. */


#include "mmc/shared.h"


static int vrc6_init (void);

static void vrc6_reset (void);


static const MMC mmc_vrc6 =
{
    24, "Konami VRC6 + ExSound",

    vrc6_init, vrc6_reset
};


static int vrc6v_init (void);


static const MMC mmc_vrc6v =
{
    26, "Konami VRC6V + ExSound",

    vrc6v_init, vrc6_reset
};


static const char vrc6_mirroring_table [] =
{
    MIRRORING_VERTICAL, MIRRORING_HORIZONTAL,

    MIRRORING_ONE_SCREEN_2000, MIRRORING_ONE_SCREEN_2400
};

static const UINT8 vrc6_mirroring_mask = 0x0c;


static char vrc6_enable_irqs = FALSE;


static int vrc6_irq_counter = 0;

static int vrc6_irq_latch = 0;


static int vrc6_irq_tick (int line)
{
    if (vrc6_enable_irqs & 0x02)
    {
        if (vrc6_irq_counter == 0xff)
        {
            vrc6_irq_counter = vrc6_irq_latch;


            return (TRUE);
        }
        else
        {
            vrc6_irq_counter ++;
        }
    }


    return (FALSE);
}


static char vrc6_swap_address_pins = FALSE;


static void vrc6_write (UINT16 address, UINT8 value)
{
    int major;

    int minor;


    /* Swap address pins. */

    if (vrc6_swap_address_pins)
    {
        address = ((address & 0xfffc) | ((address >> 1) & 1) | ((address << 1) & 2));
    }


    /* Extract command indexes. */

    major = (address & 0xf000);

    minor = (address & 0x000f);


    switch (major)
    {
        case 0x8000:

            if (minor == 0x0000)
            {
                /* Set requested 16k ROM page at $8000. */
            
                cpu_set_read_address_16k_rom_block (0x8000, value);
            }


            break;


        case 0xb000:

            if (minor == 0x0003)
            {
                /* Mirroring select. */

                /* Discard unused bits. */

                value &= vrc6_mirroring_mask;

                value >>= 2;


                /* Use value from LUT. */

                ppu_set_mirroring (vrc6_mirroring_table [value]);
            }


            break;


        case 0xc000:

            if (minor == 0x0000)
            {
                /* Set requested 8k ROM page at $C000. */
            
                cpu_set_read_address_8k_rom_block (0xc000, value);
            }


            break;


        case 0xd000:

            if (minor < 0x0004)
            {
                /* Calculate PPU address. */

                minor *= 0x0400;


                /* Set requested 1k CHR-ROM page. */
        
                ppu_set_ram_1k_pattern_vrom_block (minor, value);
            }


            break;


        case 0xe000:

            if (minor < 0x0004)
            {
                /* Calculate PPU address. */

                minor *= 0x0400;

                minor += 0x1000;


                /* Set requested 1k CHR-ROM page. */
        
                ppu_set_ram_1k_pattern_vrom_block (minor, value);
            }


            break;


        case 0xf000:

            if (minor == 0x0000)
            {
                /* Both (?) bytes of IRQ counter. */

                vrc6_irq_latch = value;
            }
            else if (minor == 0x0001)
            {
                /* Enable/disable IRQs. */

                vrc6_enable_irqs = (value & 0x03);


                if (vrc6_enable_irqs & 0x02)
                {
                    vrc6_irq_counter = vrc6_irq_latch;
                }
            }
            else if (minor == 0x0002)
            {
                /* ?? */

                if (vrc6_enable_irqs & 0x01)
                {
                    vrc6_enable_irqs |= 0x02;
                }
                else
                {
                    vrc6_enable_irqs &= 0x01;
                }
            }


            break;


        default:


            break;
    }


    papu_exwrite (address, value);
}


static void vrc6_reset (void)
{
    /* Select first 16k page in lower 16k. */

    cpu_set_read_address_16k (0x8000, FIRST_ROM_PAGE);


    /* Select last 16k page in upper 16k. */

    cpu_set_read_address_16k (0xc000, LAST_ROM_PAGE);
}


static int vrc6_base_init (void)
{
    /* Start out with VRAM. */

    ppu_set_ram_8k_pattern_vram ();


    /* Set initial mappings. */

    vrc6_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, vrc6_write);


    /* Install IRQ tick handler. */

    mmc_scanline_end = vrc6_irq_tick;


    /* Select ExSound chip. */

    papu_set_exsound (PAPU_EXSOUND_VRC6);


    return (0);
}


static int vrc6_init (void)
{
    /* Disable address pin swap. */

    vrc6_swap_address_pins = FALSE;


    return (vrc6_base_init ());
}


static int vrc6v_init (void)
{
    /* Pins A0 and A1 are swapped in VRC6V. */

    vrc6_swap_address_pins = TRUE;


    return (vrc6_base_init ());
}
