

/* Mapper #68 (Sunsoft). */

/* This mapper is fully supported. */


static int sunsoft_init (void);

static void sunsoft_reset (void);


const MMC mmc_sunsoft =
{
    68, "Sunsoft mapper #4",

    sunsoft_init, sunsoft_reset
};


static UINT8 sunsoft_name_table_banks [2];

static UINT8 sunsoft_name_table_control = 0;


static void sunsoft_fixup_name_tables (void)
{
    if (! (sunsoft_name_table_control & 0x10))
    {
        /* VRAM name tables */

        if (! (sunsoft_name_table_control & 2))
        {
            /* Horizontal/vertical mirroring. */

            ppu_set_mirroring (((sunsoft_name_table_control & 1) ?
                MIRRORING_HORIZONTAL : MIRRORING_VERTICAL));
        }
        else
        {
            /* One-screen mirroring. */

            ppu_set_mirroring (((sunsoft_name_table_control & 1) ?
                MIRRORING_ONE_SCREEN_2400 : MIRRORING_ONE_SCREEN_2000));
        }
    }
    else
    {
        /* CHR-ROM name tables. */

        if (! (sunsoft_name_table_control & 2))
        {
            /* Horizontal/vertical mirroring. */

            if (! (sunsoft_name_table_control & 1))
            {
                /* Vertical mirroring. */

                ppu_set_name_table_address_vrom (0, (sunsoft_name_table_banks [0] | 0x80));

                ppu_set_name_table_address_vrom (1, (sunsoft_name_table_banks [1] | 0x80));

                ppu_set_name_table_address_vrom (2, (sunsoft_name_table_banks [0] | 0x80));

                ppu_set_name_table_address_vrom (3, (sunsoft_name_table_banks [1] | 0x80));
            }
            else
            {
                /* Horizontal mirroring. */

                ppu_set_name_table_address_vrom (0, (sunsoft_name_table_banks [0] | 0x80));

                ppu_set_name_table_address_vrom (1, (sunsoft_name_table_banks [0] | 0x80));

                ppu_set_name_table_address_vrom (2, (sunsoft_name_table_banks [1] | 0x80));

                ppu_set_name_table_address_vrom (3, (sunsoft_name_table_banks [1] | 0x80));
            }
        }
        else
        {
            /* One-screen mirroring. */

            ppu_set_name_table_address_vrom (0, (sunsoft_name_table_banks
                [sunsoft_name_table_control & 1] | 0x80));

            ppu_set_name_table_address_vrom (1, (sunsoft_name_table_banks
                [sunsoft_name_table_control & 1] | 0x80));

            ppu_set_name_table_address_vrom (2, (sunsoft_name_table_banks
                [sunsoft_name_table_control & 1] | 0x80));

            ppu_set_name_table_address_vrom (3, (sunsoft_name_table_banks
               [sunsoft_name_table_control & 1] | 0x80));
        }
    }
}


static void sunsoft_write (UINT16 address, UINT8 value)
{
    address >>= 12;


    switch (address)
    {
        case (0x8000 >> 12):

        case (0x9000 >> 12):

        case (0xa000 >> 12):

        case (0xb000 >> 12):

            /* Calculate PPU address. */

            address = ((address - 8) * 0x800);


            /* Convert 2k page # to 1k. */

            value <<= 1;


            /* Select 2k page at PPU address. */

            ppu_set_ram_1k_pattern_vrom_block (address, value);

            ppu_set_ram_1k_pattern_vrom_block ((address + 0x400), (value + 1));


            break;


        case (0xc000 >> 12):

        case (0xd000 >> 12):

            sunsoft_name_table_banks [address - (0xc000 >> 12)] = value;

            sunsoft_fixup_name_tables ();


            break;


        case (0xe000 >> 12):

            if ((sunsoft_name_table_control & 0x13) != (value & 0x13))
            {
                /* Name tables changed? */

                sunsoft_name_table_control = value;

                sunsoft_fixup_name_tables ();
            }


            break;


        case (0xf000 >> 12):

            /* Select 16k page in lower 16k. */

            cpu_set_read_address_16k_rom_block (0x8000, value);


            break;
    }
}


static void sunsoft_reset (void)
{
    /* Select first 16k page in lower 16k. */

    cpu_set_read_address_16k (0x8000, FIRST_ROM_PAGE);


    /* Select last 16k page in upper 16k. */

    cpu_set_read_address_16k (0xc000, LAST_ROM_PAGE);
}


static int sunsoft_init (void)
{
    sunsoft_name_table_banks [0] = sunsoft_name_table_banks [1] = 0;

    sunsoft_name_table_control = ((ppu_get_mirroring () == MIRRORING_VERTICAL) ? 0 : 1);


    /* Set initial mappings. */

    sunsoft_reset ();


    /* Install write handler. */

    cpu_set_write_handler_32k (0x8000, sunsoft_write);


    return (0);
}
