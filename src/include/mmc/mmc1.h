

/* Mapper #1 (MMC1). */

/* This mapper is only partially supported. */


static int mmc1_prg_address = 0;

static int mmc1_chr_address = 0;


static int mmc1_bit_stream; //could be a char

static int mmc1_bit_counter; //could be a char


static int mmc1_register[4]; //could be a char

static int mmc1_cpu_bank[2];

static int mmc1_previous_register = 0x0000;

static int mmc1_256k_bank_num; //could be a char


static unsigned int mmc1_prg_mask;

static unsigned int mmc1_chr_mask;


static INLINE void mmc1_cpu_bank_sort (void)
{
  mmc_rom_banks[0] = ROM_PAGE_16K (((mmc1_256k_bank_num * (256 / 16)) + mmc1_cpu_bank[0]) & mmc1_prg_mask);
  mmc_rom_banks[1] = ROM_PAGE_16K (((mmc1_256k_bank_num * (256 / 16)) + mmc1_cpu_bank[0]) & mmc1_prg_mask) + 0x2000;
  mmc_rom_banks[2] = ROM_PAGE_16K (((mmc1_256k_bank_num * (256 / 16)) + mmc1_cpu_bank[1]) & mmc1_prg_mask);
  mmc_rom_banks[3] = ROM_PAGE_16K (((mmc1_256k_bank_num * (256 / 16)) + mmc1_cpu_bank[1]) & mmc1_prg_mask) + 0x2000;
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
	return;
    }

    /* buffer the 5 bit writes into mmc1_bit_stream */
    if (value & 0x01) mmc1_bit_stream |= (1 << mmc1_bit_counter); //neat idea, nester dude!

    mmc1_bit_counter++;
    if (mmc1_bit_counter < 5) return;

    /* pack the buffered bits into the register */
    mmc1_register[mmc1_current_register] = mmc1_bit_stream;
    
    /* clean up buffer & counter */
    mmc1_bit_stream = 0x00;
    mmc1_bit_counter = 0;

    switch (mmc1_current_register)
    {
	case 0:
        {
	    if (mmc1_register[0] & 0x02)  
            /* standard h/v mirroring? */
            {
		/* which way then? */
		set_ppu_mirroring ((mmc1_register[0] & 0x01) ? MIRRORING_HORIZONTAL : MIRRORING_VERTICAL);
            }
	    else
            /* one-screen mirroring */
            {
	        /* which page then? */
		set_ppu_mirroring ((mmc1_register[0] & 0x01) ? MIRRORING_ONE_SCREEN_2400 : MIRRORING_ONE_SCREEN_2000);
            }

            if (mmc1_register[0] & 0x08)
            /* 16k PRG banking? */
            {
                int mmc1_bank_number = mmc1_register[3] & 0x0F;

                if (mmc1_register[0] & 0x04)
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
                int mmc1_bank_number = (mmc1_register[3] & ~1) << 1;

                mmc1_cpu_bank [0] = mmc1_bank_number & 0x0F;
                mmc1_cpu_bank [1] = (mmc1_bank_number + 1) & 0x0F;
            }

            /* Handle >256K addressing */
            if (mmc1_register[0] & 0x10)
            {
                mmc1_256k_bank_num = (mmc1_register[1] & 0x10) >> 4;
                if (mmc1_register[0] & 0x08)    /* Er...? */
                {
                    mmc1_256k_bank_num |= ((mmc1_register[2] & 0x10) >> 3);
                }
            }
            else
            {
                mmc1_256k_bank_num = (mmc1_register[1] & 0x10) ? 3 : 0;
            }

            mmc1_cpu_bank_sort();

            if (ROM_CHR_ROM_PAGES > 0)
            {
                if (mmc1_register[0] & 0x10)
                /* 4k VROM mapping? */
                {
                    int mmc1_bank_number = mmc1_register[1] & mmc1_chr_mask;

                    if (mmc1_bank_number > ROM_CHR_ROM_PAGES * 2)
                    {
                     /* not sure how to handle non-existant CHR ROM */
                     /* ignoring high address bits */
                     mmc1_bank_number &= mmc1_chr_mask / 2;
                    }

                    mmc1_bank_number <<= 2;

                    /* swap 4k of CHR-ROM */ //works!!!
                    mmc_vrom_banks[0] = VROM_PAGE_1K (mmc1_bank_number);
                    mmc_vrom_banks[1] = VROM_PAGE_1K (mmc1_bank_number + 1);
                    mmc_vrom_banks[2] = VROM_PAGE_1K (mmc1_bank_number + 2);
                    mmc_vrom_banks[3] = VROM_PAGE_1K (mmc1_bank_number + 3);

                    mmc1_bank_number = mmc1_register[2] & mmc1_chr_mask;

                    if (mmc1_bank_number > ROM_CHR_ROM_PAGES * 2)
                    {
                        /* not sure how to handle non-existant CHR ROM */
                        /* ignoring high address bits */
                        mmc1_bank_number &= mmc1_chr_mask / 2;
                    }

                    mmc1_bank_number <<= 2;

                    /* swap other 4k of CHR-ROM */ //works!!!
                    mmc_vrom_banks[4] = VROM_PAGE_1K (mmc1_bank_number);
                    mmc_vrom_banks[5] = VROM_PAGE_1K (mmc1_bank_number + 1);
                    mmc_vrom_banks[6] = VROM_PAGE_1K (mmc1_bank_number + 2);
                    mmc_vrom_banks[7] = VROM_PAGE_1K (mmc1_bank_number + 3);
                }
                else
                /* No, 8k VROM mapping */
                {
                    int mmc1_bank_number = mmc1_register[1] & mmc1_chr_mask & ~1;

                    if (mmc1_bank_number > ROM_CHR_ROM_PAGES * 2)
                    {
                     /* not sure how to handle non-existant CHR ROM */
                     /* ignoring high address bits */
                     mmc1_bank_number &= mmc1_chr_mask / 2;
                    }

                    mmc1_bank_number <<= 2;

                    /* swap 8k of CHR-ROM */ //never called??
                    mmc_vrom_banks[0] = VROM_PAGE_1K (mmc1_bank_number);
                    mmc_vrom_banks[1] = VROM_PAGE_1K (mmc1_bank_number + 1);
                    mmc_vrom_banks[2] = VROM_PAGE_1K (mmc1_bank_number + 2);
                    mmc_vrom_banks[3] = VROM_PAGE_1K (mmc1_bank_number + 3);
                    mmc_vrom_banks[4] = VROM_PAGE_1K (mmc1_bank_number + 4);
                    mmc_vrom_banks[5] = VROM_PAGE_1K (mmc1_bank_number + 5);
                    mmc_vrom_banks[6] = VROM_PAGE_1K (mmc1_bank_number + 6);
                    mmc_vrom_banks[7] = VROM_PAGE_1K (mmc1_bank_number + 7);
                }
            }
            return;
        }
	
	case 1:
        {
            /* Handle >256K addressing */
            if (mmc1_register[0] & 0x10)
            {
                mmc1_256k_bank_num = (mmc1_register[1] & 0x10) >> 4;
                if (mmc1_register[0] & 0x08)    /* Er...? */
                {
                    mmc1_256k_bank_num |= ((mmc1_register[2] & 0x10) >> 3);
                }
            }
            else
            {
                mmc1_256k_bank_num = (mmc1_register[1] & 0x10) ? 3 : 0;
            }
            mmc1_cpu_bank_sort();

            if (ROM_CHR_ROM_PAGES > 0)
            {
                if (mmc1_register[0] & 0x10)
                /* 4k VROM mapping? */
                {
                    int mmc1_bank_number = mmc1_register[1] & mmc1_chr_mask;

                    if (mmc1_bank_number > ROM_CHR_ROM_PAGES * 2)
                    {
                     /* not sure how to handle non-existant CHR ROM */
                     /* ignoring high address bits */
                     mmc1_bank_number &= mmc1_chr_mask / 2;
                    }

                    mmc1_bank_number <<= 2;

                    /* swap 4k of CHR-ROM */ //works!!!
                    mmc_vrom_banks[0] = VROM_PAGE_1K (mmc1_bank_number);
                    mmc_vrom_banks[1] = VROM_PAGE_1K (mmc1_bank_number + 1);
                    mmc_vrom_banks[2] = VROM_PAGE_1K (mmc1_bank_number + 2);
                    mmc_vrom_banks[3] = VROM_PAGE_1K (mmc1_bank_number + 3);
                }
                else
                /* No, 8k VROM mapping */
                {
                    int mmc1_bank_number = mmc1_register[1] & mmc1_chr_mask & ~1;

                    if (mmc1_bank_number > ROM_CHR_ROM_PAGES * 2)
                    {
                     /* not sure how to handle non-existant CHR ROM */
                     /* ignoring high address bits */
                     mmc1_bank_number &= mmc1_chr_mask / 2;
                    }

                    mmc1_bank_number <<= 2;

                    /* swap 8k of CHR-ROM */ //never called??
                    mmc_vrom_banks[0] = VROM_PAGE_1K (mmc1_bank_number);
                    mmc_vrom_banks[1] = VROM_PAGE_1K (mmc1_bank_number + 1);
                    mmc_vrom_banks[2] = VROM_PAGE_1K (mmc1_bank_number + 2);
                    mmc_vrom_banks[3] = VROM_PAGE_1K (mmc1_bank_number + 3);
                    mmc_vrom_banks[4] = VROM_PAGE_1K (mmc1_bank_number + 4);
                    mmc_vrom_banks[5] = VROM_PAGE_1K (mmc1_bank_number + 5);
                    mmc_vrom_banks[6] = VROM_PAGE_1K (mmc1_bank_number + 6);
                    mmc_vrom_banks[7] = VROM_PAGE_1K (mmc1_bank_number + 7);
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
            if (mmc1_register[0] & 0x10)
            {
                mmc1_256k_bank_num = (mmc1_register[1] & 0x10) >> 4;
                if (mmc1_register[0] & 0x08)    /* Er...? */
                {
                    mmc1_256k_bank_num |= ((mmc1_register[2] & 0x10) >> 3);
                }
            }
            else
            {
                mmc1_256k_bank_num = (mmc1_register[1] & 0x10) ? 3 : 0;
            }
            mmc1_cpu_bank_sort();

            if (ROM_CHR_ROM_PAGES == 0)
            {
                return;
            }

            if (mmc1_register[0] & 0x10)
            /* 4k VROM mapping? */
            {
                int mmc1_bank_number = mmc1_register[2] & mmc1_chr_mask;

                if (mmc1_bank_number > ROM_CHR_ROM_PAGES * 2)
                {
                    /* not sure how to handle non-existant CHR ROM */
                    /* ignoring high address bits */
                    mmc1_bank_number &= mmc1_chr_mask / 2;
                }

                mmc1_bank_number <<= 2;

                /* swap other 4k of CHR-ROM */ //works!!!
                mmc_vrom_banks[4] = VROM_PAGE_1K (mmc1_bank_number);
                mmc_vrom_banks[5] = VROM_PAGE_1K (mmc1_bank_number + 1);
                mmc_vrom_banks[6] = VROM_PAGE_1K (mmc1_bank_number + 2);
                mmc_vrom_banks[7] = VROM_PAGE_1K (mmc1_bank_number + 3);
            }

            return;
        }

        case 3:
        {
            if (mmc1_register[0] & 0x08)
            /* 16k PRG banking? */
            {
                int mmc1_bank_number = mmc1_register[3] & 0x0F;

                if (mmc1_register[0] & 0x04)
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
                int mmc1_bank_number = (mmc1_register[3] & ~1) << 1;

                mmc1_cpu_bank [0] = mmc1_bank_number & 0x0F;
                mmc1_cpu_bank [1] = (mmc1_bank_number + 1) & 0x0F;
            }
            mmc1_cpu_bank_sort();
        }
    return;
    }
}

static INLINE void mmc1_reset (void)
{
    int index;

    mmc1_register[0]=0x0c;//0c
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
            mmc_vrom_banks [index] = VROM_PAGE_1K (index);
        }
    }    
}


static INLINE int mmc1_init (void)
{
    if (! gui_is_active)
    {
        printf ("Using memory mapper #1 (MMC1) "
            "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);


        printf ("Warning: This mapper is only partially supported.\n");
        printf ("Press any key to continue...\n");


        while (!keypressed ());
        readkey ();
    }

    if (ROM_CHR_ROM_PAGES == 1) mmc1_prg_mask = 1;
    else if (ROM_CHR_ROM_PAGES == 2) mmc1_prg_mask = 2;
    else if (ROM_CHR_ROM_PAGES <= 4) mmc1_prg_mask = 4;
    else if (ROM_CHR_ROM_PAGES <= 8) mmc1_prg_mask = 8;
    else if (ROM_CHR_ROM_PAGES <= 16) mmc1_prg_mask = 16;
    else if (ROM_CHR_ROM_PAGES <= 32) mmc1_prg_mask = 32;
    else if (ROM_CHR_ROM_PAGES <= 64) mmc1_prg_mask = 64;
    /* max 1024K PRG ROM */
    else mmc1_prg_mask = 256;


    if (ROM_PRG_ROM_PAGES != mmc1_prg_mask)
    {
        /* Bank count not even power of 2, unhandled. */

        return (1);
    }

    /* Convert to 16k mask. */

    mmc1_prg_mask = (mmc1_prg_mask - 1);


    if (ROM_CHR_ROM_PAGES == 0)
    {
        /* No VROM is present. */

        mmc_no_vrom = TRUE;
    }
    else
    {
        mmc_no_vrom = FALSE;
    }


    if (ROM_CHR_ROM_PAGES != 0)
    {
        if (ROM_CHR_ROM_PAGES == 1) mmc1_chr_mask = 1;
        else if (ROM_CHR_ROM_PAGES == 2) mmc1_chr_mask = 2;
        else if (ROM_CHR_ROM_PAGES <= 4) mmc1_chr_mask = 4;
        else if (ROM_CHR_ROM_PAGES <= 8) mmc1_chr_mask = 8;
        /* max 128K CHR ROM */
        else mmc1_chr_mask = 16;

        /* Convert 8k mask to 4k mask. */

        mmc1_chr_mask = ((mmc1_chr_mask * 2) - 1);
    }



    mmc1_reset ();

    mmc_write = mmc1_write;
    return 0;
}
