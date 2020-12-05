

/* Mapper #1 (MMC1). */

/* This mapper is only partially supported. */


static int mmc1_init (void);

static void mmc1_reset (void);


static void mmc1_save_state (PACKFILE *, int);

static void mmc1_load_state (PACKFILE *, int);


const MMC mmc_mmc1 =
{
    1, "MMC1",

    mmc1_init, mmc1_reset,


    "MMC1\0\0\0\0",

    mmc1_save_state, mmc1_load_state
};


static int mmc1_bit_stream;

static int mmc1_bit_counter;


static int mmc1_register[4];

static int mmc1_previous_register = 0x0000;

static int mmc1_256k_bank_num;

static int mmc1_cpu_bank[2];


#define MMC1_MIRRORING_ADDRESS_BIT  1
#define MMC1_MIRRORING_MODE_BIT     2
#define MMC1_PRG_BANK_SELECT_BIT    4
#define MMC1_PRG_BANK_SIZE_BIT      8
#define MMC1_VROM_BANK_SIZE_BIT     0x10
#define MMC1_USE_256K_SELECT_1_BIT  0x10

#define MMC1_256K_SELECT_0_BIT      0x10

#define MMC1_256K_SELECT_1_BIT      0x10


static INLINE void mmc1_cpu_bank_sort (void)
{
  cpu_set_read_address_16k_rom_block (0x8000, (mmc1_256k_bank_num * (256 / 16)) + mmc1_cpu_bank[0]);
  cpu_set_read_address_16k_rom_block (0xC000, (mmc1_256k_bank_num * (256 / 16)) + mmc1_cpu_bank[1]);
}


/* passed a register selection address, 0-3 */
static void mmc1_set_register_value (unsigned address, UINT8 value)
{
    mmc1_register[address] = value;

    switch (address)
    {
	case 0:
        {
            if (mmc1_register[0] & MMC1_MIRRORING_MODE_BIT)  
            /* standard h/v mirroring? */
            {
		/* which way then? */
                ppu_set_mirroring (
                    (mmc1_register[0] & MMC1_MIRRORING_ADDRESS_BIT) ?
                    MIRRORING_HORIZONTAL : MIRRORING_VERTICAL);
            }
	    else
            /* one-screen mirroring */
            {
	        /* which page then? */
                ppu_set_mirroring (
                    (mmc1_register[0] & MMC1_MIRRORING_ADDRESS_BIT) ?
                    MIRRORING_ONE_SCREEN_2400 : MIRRORING_ONE_SCREEN_2000);
            }

            if (mmc1_register[0] & MMC1_PRG_BANK_SIZE_BIT)
            /* 16k PRG banking? */
            {
                int mmc1_bank_number = mmc1_register[3] & 0x0F;

                if (mmc1_register[0] & MMC1_PRG_BANK_SELECT_BIT)
                /* 8000-BFFF PRG banking? */
                {
                    mmc1_cpu_bank [0] = mmc1_bank_number;
                    mmc1_cpu_bank [1] = (ROM_PRG_ROM_PAGES - 1) & 0x0F;
                }
                else
                /* No, C000-FFFF PRG banking */
                {
                    mmc1_cpu_bank [0] = 0;
                    mmc1_cpu_bank [1] = mmc1_bank_number;
                }
            }
            else
            /* No, 32k PRG banking */
            {
                int mmc1_bank_number = (mmc1_register[3] & ~1);

                mmc1_cpu_bank [0] = mmc1_bank_number & 0x0F;
                if (ROM_CHR_ROM_PAGES <= 16)
                {
                    mmc1_cpu_bank [1] = (mmc1_bank_number + 1) & 0x0F;
                }
            }

            /* Handle >256K addressing */
            if (mmc1_register[0] & MMC1_USE_256K_SELECT_1_BIT)
            {
                mmc1_256k_bank_num =
                    (mmc1_register[1] & MMC1_256K_SELECT_0_BIT) >> 4;
                if (mmc1_register[0] & MMC1_PRG_BANK_SIZE_BIT)    /* Er...? */
                {
                    mmc1_256k_bank_num |=
                        ((mmc1_register[2] & MMC1_256K_SELECT_1_BIT) >> 3);
                }
            }
            else
            {
                mmc1_256k_bank_num =
                    (mmc1_register[1] & MMC1_256K_SELECT_0_BIT) ? 3 : 0;
            }

            mmc1_cpu_bank_sort();

            if (ROM_CHR_ROM_PAGES > 0)
            {
                if (mmc1_register[0] & MMC1_VROM_BANK_SIZE_BIT)
                /* 4k VROM mapping? */
                {
                    int mmc1_bank_number = mmc1_register[1] * 4;

                    /* swap 4k of CHR-ROM */
                    ppu_set_ram_1k_pattern_vrom_block (0 << 10,
                        mmc1_bank_number);
                    ppu_set_ram_1k_pattern_vrom_block (1 << 10,
                        mmc1_bank_number + 1);
                    ppu_set_ram_1k_pattern_vrom_block (2 << 10,
                        mmc1_bank_number + 2);
                    ppu_set_ram_1k_pattern_vrom_block (3 << 10,
                        mmc1_bank_number + 3);


                    mmc1_bank_number = mmc1_register[2] * 4;

                    /* swap other 4k of CHR-ROM */
                    ppu_set_ram_1k_pattern_vrom_block (4 << 10,
                        mmc1_bank_number);
                    ppu_set_ram_1k_pattern_vrom_block (5 << 10,
                        mmc1_bank_number + 1);
                    ppu_set_ram_1k_pattern_vrom_block (6 << 10,
                        mmc1_bank_number + 2);
                    ppu_set_ram_1k_pattern_vrom_block (7 << 10,
                        mmc1_bank_number + 3);
                }
                else
                /* No, 8k VROM mapping */
                {
                    int mmc1_bank_number = (mmc1_register[1] & ~1) * 4;

                    /* swap 8k of CHR-ROM */ //never called??
                    ppu_set_ram_1k_pattern_vrom_block (0 << 10,
                        mmc1_bank_number);
                    ppu_set_ram_1k_pattern_vrom_block (1 << 10,
                        mmc1_bank_number + 1);
                    ppu_set_ram_1k_pattern_vrom_block (2 << 10,
                        mmc1_bank_number + 2);
                    ppu_set_ram_1k_pattern_vrom_block (3 << 10,
                        mmc1_bank_number + 3);
                    ppu_set_ram_1k_pattern_vrom_block (4 << 10,
                        mmc1_bank_number + 4);
                    ppu_set_ram_1k_pattern_vrom_block (5 << 10,
                        mmc1_bank_number + 5);
                    ppu_set_ram_1k_pattern_vrom_block (6 << 10,
                        mmc1_bank_number + 6);
                    ppu_set_ram_1k_pattern_vrom_block (7 << 10,
                        mmc1_bank_number + 7);
                }
            }
            return;
        }
	
	case 1:
        {
            /* Handle >256K addressing */
            if (mmc1_register[0] & MMC1_USE_256K_SELECT_1_BIT)
            {
                mmc1_256k_bank_num =
                    (mmc1_register[1] & MMC1_256K_SELECT_0_BIT) >> 4;
                if (mmc1_register[0] & MMC1_PRG_BANK_SIZE_BIT)    /* Er...? */
                {
                    mmc1_256k_bank_num |=
                        ((mmc1_register[2] & MMC1_256K_SELECT_1_BIT) >> 3);
                }
            }
            else
            {
                mmc1_256k_bank_num =
                    (mmc1_register[1] & MMC1_256K_SELECT_0_BIT) ? 3 : 0;
            }

            mmc1_cpu_bank_sort();

            if (ROM_CHR_ROM_PAGES > 0)
            {
                if (mmc1_register[0] & MMC1_VROM_BANK_SIZE_BIT)
                /* 4k VROM mapping? */
                {
                    int mmc1_bank_number = mmc1_register[1] * 4;

                    /* swap 4k of CHR-ROM */ //works!!!
                    ppu_set_ram_1k_pattern_vrom_block (0 << 10,
                        mmc1_bank_number);
                    ppu_set_ram_1k_pattern_vrom_block (1 << 10,
                        mmc1_bank_number + 1);
                    ppu_set_ram_1k_pattern_vrom_block (2 << 10,
                        mmc1_bank_number + 2);
                    ppu_set_ram_1k_pattern_vrom_block (3 << 10,
                        mmc1_bank_number + 3);
                }
                else
                /* No, 8k VROM mapping */
                {
                    int mmc1_bank_number = (mmc1_register[1] & ~1) * 4;

                    /* swap 8k of CHR-ROM */ //never called??
                    ppu_set_ram_1k_pattern_vrom_block (0 << 10,
                        mmc1_bank_number);
                    ppu_set_ram_1k_pattern_vrom_block (1 << 10,
                        mmc1_bank_number + 1);
                    ppu_set_ram_1k_pattern_vrom_block (2 << 10,
                        mmc1_bank_number + 2);
                    ppu_set_ram_1k_pattern_vrom_block (3 << 10,
                        mmc1_bank_number + 3);
                    ppu_set_ram_1k_pattern_vrom_block (4 << 10,
                        mmc1_bank_number + 4);
                    ppu_set_ram_1k_pattern_vrom_block (5 << 10,
                        mmc1_bank_number + 5);
                    ppu_set_ram_1k_pattern_vrom_block (6 << 10,
                        mmc1_bank_number + 6);
                    ppu_set_ram_1k_pattern_vrom_block (7 << 10,
                        mmc1_bank_number + 7);
                }
            }
            else
            {
                return;
            }
        }

        case 2:
        {
            int mmc1_bank_number = mmc1_register[2];
            /* Handle >256K addressing */
            if (mmc1_register[0] & MMC1_USE_256K_SELECT_1_BIT)
            {
                mmc1_256k_bank_num =
                    (mmc1_register[1] & MMC1_256K_SELECT_0_BIT) >> 4;
                if (mmc1_register[0] & MMC1_PRG_BANK_SIZE_BIT)    /* Er...? */
                {
                    mmc1_256k_bank_num |=
                        ((mmc1_register[2] & MMC1_256K_SELECT_1_BIT) >> 3);
                }
            }
            else
            {
                mmc1_256k_bank_num =
                    (mmc1_register[1] & MMC1_256K_SELECT_0_BIT) ? 3 : 0;
            }

            mmc1_cpu_bank_sort();

            if (ROM_CHR_ROM_PAGES == 0)
            {
                return;
            }

            if (mmc1_register[0] & MMC1_VROM_BANK_SIZE_BIT)
            /* 4k VROM mapping? */
            {
                int mmc1_bank_number = mmc1_register[2] * 4;

                /* swap other 4k of CHR-ROM */ //works!!!
                ppu_set_ram_1k_pattern_vrom_block (4 << 10,
                    mmc1_bank_number);
                ppu_set_ram_1k_pattern_vrom_block (5 << 10,
                    mmc1_bank_number + 1);
                ppu_set_ram_1k_pattern_vrom_block (6 << 10,
                    mmc1_bank_number + 2);
                ppu_set_ram_1k_pattern_vrom_block (7 << 10,
                    mmc1_bank_number + 3);
            }

            return;
        }

        case 3:
        {
            if (mmc1_register[0] & MMC1_PRG_BANK_SIZE_BIT)
            /* 16k PRG banking? */
            {
                int mmc1_bank_number = mmc1_register[3] & 0x0F;

                if (mmc1_register[0] & MMC1_PRG_BANK_SELECT_BIT)
                /* 8000-BFFF PRG banking? */
                {
                    mmc1_cpu_bank [0] = mmc1_bank_number;
                    mmc1_cpu_bank [1] = (ROM_PRG_ROM_PAGES - 1) & 0x0F;
                }
                else
                /* No, C000-FFFF PRG banking */
                {
                    mmc1_cpu_bank [0] = 0;
                    mmc1_cpu_bank [1] = mmc1_bank_number;
                }
            }
            else
            /* No, 32k PRG banking */
            {
                int mmc1_bank_number = (mmc1_register[3] & ~1);

                mmc1_cpu_bank [0] = mmc1_bank_number & 0x0F;
                if (ROM_CHR_ROM_PAGES <= 16)
                {
                    mmc1_cpu_bank [1] = (mmc1_bank_number + 1) & 0x0F;
                }
            }

            mmc1_cpu_bank_sort();
        }
    return;
    }
}


static void mmc1_write (UINT16 address, UINT8 value)
{
    int mmc1_current_register;
        
    mmc1_current_register = (address & 0x6000) >> 13;

    /* just in case the write is to a different register */
    if (mmc1_current_register != mmc1_previous_register)
    {
        mmc1_bit_counter = 0;
	mmc1_bit_stream = 0x00;
        mmc1_previous_register = mmc1_current_register;
    }

    /* check the reset flag */
    if (value & 0x80)
    {
        mmc1_bit_counter = 0;
	mmc1_bit_stream = 0x00;
        mmc1_set_register_value (0, mmc1_register[0] | 0x0C);
	return;
    }

    /* buffer the 5 bit writes into mmc1_bit_stream */
    if (value & 0x01) mmc1_bit_stream |= (1 << mmc1_bit_counter); //neat idea, nester dude!

    mmc1_bit_counter++;
    if (mmc1_bit_counter < 5) return;

    /* pack the buffered bits into the register */
    mmc1_set_register_value (mmc1_current_register, mmc1_bit_stream);

    /* clean up buffer & counter */
    mmc1_bit_stream = 0x00;
    mmc1_bit_counter = 0;
}

static void mmc1_reset (void)
{
    int index;

    mmc1_register[0]=0x0c;
    mmc1_register[1]=0x00;
    mmc1_register[2]=0x00;
    mmc1_register[3]=0x00;

    mmc1_256k_bank_num = 0;

    /* Select first 16k page in lower 16k. */

    mmc1_cpu_bank [0] = 0;


    /* Select last 16k page in upper 16k. */

    mmc1_cpu_bank [1] = (ROM_PRG_ROM_PAGES - 1) & 0x0F;

    
    mmc1_cpu_bank_sort();


    if (ROM_CHR_ROM_PAGES)
    {
        /* Select first 8k page. */

        for (index = 0; index < 8; index ++)
        {
            ppu_set_ram_1k_pattern_vrom_block (index << 10, index);
        }
    }    
}


static int mmc1_init (void)
{
    if (ROM_CHR_ROM_PAGES == 0)
    {
        /* No VROM is present. */

        ppu_set_ram_8k_pattern_vram ();
    }


    mmc1_reset ();

    cpu_set_write_handler_32k (0x8000, mmc1_write);

    return 0;
}


static void mmc1_save_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    pack_putc (mmc1_previous_register, file_chunk);

    pack_putc (mmc1_bit_stream, file_chunk);
    pack_putc (mmc1_bit_counter, file_chunk);

    pack_putc (mmc1_register[0], file_chunk);
    pack_putc (mmc1_register[1], file_chunk);
    pack_putc (mmc1_register[2], file_chunk);
    pack_putc (mmc1_register[3], file_chunk);


    pack_fclose_chunk (file_chunk);
}


static void mmc1_load_state (PACKFILE * file, int version)
{
    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    mmc1_previous_register = pack_getc (file_chunk);

    mmc1_bit_stream = pack_getc (file_chunk);
    mmc1_bit_counter = pack_getc (file_chunk);


    mmc1_set_register_value (0, pack_getc (file_chunk));
    mmc1_set_register_value (1, pack_getc (file_chunk));
    mmc1_set_register_value (2, pack_getc (file_chunk));
    mmc1_set_register_value (3, pack_getc (file_chunk));


    pack_fclose_chunk (file_chunk);
}

