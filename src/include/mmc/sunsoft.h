

/* Mapper #68 (Sunsoft). */

/* This mapper is fully supported. */


static unsigned sunsoft_prg_mask;

static UINT8 sunsoft_name_table_banks [2];
static UINT8 sunsoft_name_table_control;


static void sunsoft_fixup_name_tables (void)
{
    if (!(sunsoft_name_table_control & 0x10))
    /* VRAM name tables */
    {
        if (!(sunsoft_name_table_control & 2))
        /* horizontal/vertical mirroring */
        {
            ppu_set_mirroring ((sunsoft_name_table_control & 1) ?
                MIRRORING_HORIZONTAL : MIRRORING_VERTICAL);
        }
        else
        /* one-screen mirroring */
        {
            ppu_set_mirroring ((sunsoft_name_table_control & 1) ?
                MIRRORING_ONE_SCREEN_2400 :
                MIRRORING_ONE_SCREEN_2000);
        }
    }
    else
    /* CHR ROM name tables */
    {
        if (!(sunsoft_name_table_control & 2))
        /* horizontal/vertical mirroring */
        {
            if (!(sunsoft_name_table_control & 1))
            /* vertical mirroring */
            {
                ppu_set_name_table_address_vrom (0,
                    sunsoft_name_table_banks [0] | 0x80);
                ppu_set_name_table_address_vrom (1,
                    sunsoft_name_table_banks [1] | 0x80);
                ppu_set_name_table_address_vrom (2,
                    sunsoft_name_table_banks [0] | 0x80);
                ppu_set_name_table_address_vrom (3,
                    sunsoft_name_table_banks [1] | 0x80);
            }
            else
            /* horizontal mirroring */
            {
                ppu_set_name_table_address_vrom (0,
                    sunsoft_name_table_banks [0] | 0x80);
                ppu_set_name_table_address_vrom (1,
                    sunsoft_name_table_banks [0] | 0x80);
                ppu_set_name_table_address_vrom (2,
                    sunsoft_name_table_banks [1] | 0x80);
                ppu_set_name_table_address_vrom (3,
                    sunsoft_name_table_banks [1] | 0x80);
            }
        }
        else
        /* one-screen mirroring */
        {
            ppu_set_name_table_address_vrom (0,
                sunsoft_name_table_banks [sunsoft_name_table_control & 1]
                | 0x80);
            ppu_set_name_table_address_vrom (1,
                sunsoft_name_table_banks [sunsoft_name_table_control & 1]
                | 0x80);
            ppu_set_name_table_address_vrom (2,
                sunsoft_name_table_banks [sunsoft_name_table_control & 1]
                | 0x80);
            ppu_set_name_table_address_vrom (3,
                sunsoft_name_table_banks [sunsoft_name_table_control & 1]
                | 0x80);
        }
    }
}

static void sunsoft_write (UINT16 address, UINT8 value)
{
    address = address >> 12;
    switch (address)
    {
        case 0x8000 >> 12:
        case 0x9000 >> 12:
        case 0xa000 >> 12:
        case 0xb000 >> 12:

            /* Calculate PPU address. */

            address = (address - 8) * 0x800;


            /* Convert 2k page # to 1k. */

            value <<= 1;


            /* Select 2k page at PPU address. */

            ppu_set_ram_1k_pattern_vrom_block (address, value);

            ppu_set_ram_1k_pattern_vrom_block ((address + 0x400), value + 1);


            break;


        case 0xc000 >> 12:
        case 0xd000 >> 12:

            sunsoft_name_table_banks [address - (0xc000 >> 12)] = value;

            sunsoft_fixup_name_tables();

            break;

        case 0xe000 >> 12:

            if ((sunsoft_name_table_control & 0x13) != (value & 0x13))
            /* name tables changed? */
            {
                sunsoft_name_table_control = value;

                sunsoft_fixup_name_tables();
            }

            break;


        case 0xf000 >> 12:

            /* Select 16k page in lower 16k. */

            cpu_set_read_address_16k (0x8000, ROM_PAGE_16K (value & sunsoft_prg_mask));


            break;
    }
}


static INLINE void sunsoft_reset (void)
{
    /* Select first 16k page in lower 16k. */

    cpu_set_read_address_16k (0x8000, FIRST_ROM_PAGE);


    /* Select last 16k page in upper 16k. */

    cpu_set_read_address_16k (0xc000, LAST_ROM_PAGE);
}


static INLINE int sunsoft_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #68 (Sunsoft) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);
    }


    if (ROM_PRG_ROM_PAGES == 1) sunsoft_prg_mask = 1;
    else if (ROM_PRG_ROM_PAGES == 2) sunsoft_prg_mask = 2;
    else if (ROM_PRG_ROM_PAGES <= 4) sunsoft_prg_mask = 4;
    else if (ROM_PRG_ROM_PAGES <= 8) sunsoft_prg_mask = 8;
    else if (ROM_PRG_ROM_PAGES <= 16) sunsoft_prg_mask = 16;
    else if (ROM_PRG_ROM_PAGES <= 32) sunsoft_prg_mask = 32;
    else if (ROM_PRG_ROM_PAGES <= 64) sunsoft_prg_mask = 64;
    else if (ROM_PRG_ROM_PAGES <= 128) sunsoft_prg_mask = 128;
    else sunsoft_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != sunsoft_prg_mask)
    {
        /* Bank count not even power of 2, unhandled. */

        return (1);
    }

    /* Generate 16k mask. */

    sunsoft_prg_mask --;


    sunsoft_name_table_banks [0] = sunsoft_name_table_banks [1] = 0;
    sunsoft_name_table_control = (ppu_get_mirroring() == MIRRORING_VERTICAL)
     ? 0 : 1;

    /* Restore initial state. */

    sunsoft_reset ();


    /* Install the handler. */

    cpu_set_write_handler_32k (0x8000, sunsoft_write);


    return (0);
}
