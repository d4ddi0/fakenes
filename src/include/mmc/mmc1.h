

/* Mapper #1 (MMC1). */

/* This mapper is only partially supported. */


static int mmc1_bit_stream; //could be a char

static int mmc1_bit_counter; //could be a char


//static int mmc1_port_counters [4]; //??

static int mmc1_register[3]; //could be a char

static int mmc1_cpu_bank[3];

static int mmc1_previous_address = 0x0000;

static int mmc1_banks_swapped; //could be a char

static int mmc1_256k_bank_num; //could be a char

static int mmc1_prgrom_size;

static int mmc1_lastbank;


static INLINE void mmc1_cpu_bank_sort (void)
{
  mmc_rom_banks[0] = ROM_PAGE_8K ((mmc1_256k_bank_num * 32) + mmc1_cpu_bank[0]);
  mmc_rom_banks[1] = ROM_PAGE_8K ((mmc1_256k_bank_num * 32) + mmc1_cpu_bank[1]);
  mmc_rom_banks[2] = ROM_PAGE_8K ((mmc1_256k_bank_num * 32) + mmc1_cpu_bank[2]);
  mmc_rom_banks[3] = ROM_PAGE_8K ((mmc1_256k_bank_num * 32) + mmc1_cpu_bank[3]);
}

static void mmc1_write (UINT16 address, UINT8 value)
{
    int mmc1_current_register;
        
    /* just in case the write is to a different register */
    if ((address & 0x6000) != (mmc1_previous_address & 0x6000))
        {
	mmc1_bit_counter = 0;
	mmc1_bit_stream = 0x00;
	}
    mmc1_previous_address = address;

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
    mmc1_current_register = (address & 0x7fff) >> 13;
    mmc1_register[mmc1_current_register] = mmc1_bit_stream;
    
    /* clean up buffer & counter */
    mmc1_bit_stream = 0x00;
    mmc1_bit_counter = 0;

    switch(mmc1_current_register)
        {
	case 0:
	    {
	    /* multi-mirroring? */
	    if (mmc1_register[0] & 0x02)  
	        {
		/* which way then? */
		set_ppu_mirroring ((mmc1_register[0] & 0x01) ? MIRRORING_HORIZONTAL : MIRRORING_VERTICAL);
		}
	    else
	        {
	        /* which page then? */
		set_ppu_mirroring ((mmc1_register[0] & 0x01) ? MIRRORING_ONE_SCREEN_2400 : MIRRORING_ONE_SCREEN_2000);
		}
	    return;
	    }
	
	case 1:
	    {
	    int mmc1_bank_number = mmc1_register[1];
	       if (mmc1_prgrom_size == 1024)
		    {
	            if (mmc1_register[0] & 0x10)
	                {
		        if (mmc1_banks_swapped)
		            {
			    mmc1_256k_bank_num = (mmc1_register[1] & 0x10) >> 4;
			    if (mmc1_register[0] & 0x08)
			        {
			        mmc1_256k_bank_num |= ((mmc1_register[2] & 0x10) >> 3);
				}
			    mmc1_cpu_bank_sort();
			    mmc1_banks_swapped = 0;
		            }
			else
			    {
			    mmc1_banks_swapped = 1;
			    }
			}
		    else
		        {
			mmc1_256k_bank_num = (mmc1_register[1] & 0x10) ? 3 : 0;
			mmc1_cpu_bank_sort();
			}
		    }
		else if ((mmc1_prgrom_size == 512) && (!ROM_CHR_ROM_PAGES))
		    {
		    mmc1_256k_bank_num = (mmc1_register[1] & 0x10) >> 4;
		    mmc1_cpu_bank_sort();
		    }
		else if (ROM_CHR_ROM_PAGES > 0)
		    {
		      if (mmc1_register[0] & 0x10)
			  {
			  /* swap 4k of CHR-ROM */ //works!!!
			  mmc1_bank_number <<= 2;
			  mmc_vrom_banks[0] = VROM_PAGE_1K (mmc1_bank_number);
			  mmc_vrom_banks[1] = VROM_PAGE_1K (mmc1_bank_number + 1);
			  mmc_vrom_banks[2] = VROM_PAGE_1K (mmc1_bank_number + 2);
			  mmc_vrom_banks[3] = VROM_PAGE_1K (mmc1_bank_number + 3);
			  }
		      else
			  {
			  /* swap 8k of CHR-ROM */ //never called??
			  mmc1_bank_number <<= 2;
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
		    if (mmc1_register[0] & 0x10)
		        {
			  printf("MMC1 VRAM SWAP 0-3 NOT IMPLEMENTED!!!\n");
		    //    mmc1_bank_number <<= 2;
		    //    mmc_vram_banks[0] = mmc1_bank_number;
		    //    mmc_vram_banks[1] = (mmc1_bank_number + 1);
		    //    mmc_vram_banks[2] = (mmc1_bank_number + 2);
		    //    mmc_vram_banks[3] = (mmc1_bank_number + 3);
		        }
		    }
	       return;
	    }

	  case 2:
	      {
	      int mmc1_bank_number = mmc1_register[2];
	      if ((mmc1_prgrom_size == 1024) && (mmc1_register[0] & 0x08))
		  {
		  if (mmc1_banks_swapped)
		      {
		      mmc1_256k_bank_num = (mmc1_register[1] & 0x10) >> 4;
		      mmc1_256k_bank_num |= ((mmc1_register[2] & 0x10) >> 3);
		      mmc1_cpu_bank_sort();
		      mmc1_banks_swapped = 0;
		      }
		  else
		      {
		      mmc1_banks_swapped = 1;
		      }
		  }

	      if (ROM_CHR_ROM_PAGES == 0)
	          {
		  if (mmc1_register[0] & 0x10)
		        {
			printf("MMC1 VRAM SWAP 4-7 NOT IMPLEMENTED!!!\n");
		//      mmc1_bank_number <<= 2;
		//      mmc_vram_banks[4] = mmc1_bank_number;
		//      mmc_vram_banks[5] = (mmc1_bank_number + 1);
		//      mmc_vram_banks[6] = (mmc1_bank_number + 2);
		//      mmc_vram_banks[7] = (mmc1_bank_number + 3);
		        return;
		        }  
		  }

	      if (mmc1_register[0] & 0x10)
		  {
		  /* swap other 4k of CHR ROM */ //works!!!
		  mmc1_bank_number <<= 2;
		  mmc_vrom_banks[4] = VROM_PAGE_1K (mmc1_bank_number);
		  mmc_vrom_banks[5] = VROM_PAGE_1K (mmc1_bank_number + 1);
		  mmc_vrom_banks[6] = VROM_PAGE_1K (mmc1_bank_number + 2);
		  mmc_vrom_banks[7] = VROM_PAGE_1K (mmc1_bank_number + 3);
		  }
	      return;
	      }

	  case 3:
	      {
	      int mmc1_bank_number = mmc1_register[3];
	      if (mmc1_register[0] & 0x08)
		  {
		  mmc1_bank_number <<= 1;
		  if (mmc1_register[0] & 0x04)
		      {
		      mmc1_cpu_bank [0] = mmc1_bank_number;
                      mmc1_cpu_bank [1] = mmc1_bank_number + 1;
		      mmc1_cpu_bank [2] = mmc1_lastbank - 1;
		      mmc1_cpu_bank [3] = mmc1_lastbank;
		      }
		  else
		      {
		      if (mmc1_prgrom_size < 512)
		          {
		          mmc1_cpu_bank [0] = 0;
		          mmc1_cpu_bank [1] = 1;
		          mmc1_cpu_bank [2] = mmc1_bank_number;
                          mmc1_cpu_bank [3] = mmc1_bank_number + 1;
		          }
		      }
		  }
	      else
		  {
		  mmc1_bank_number <<= 1;
		  mmc1_cpu_bank [0] = mmc1_bank_number;
                  mmc1_cpu_bank [1] = mmc1_bank_number + 1;
		  if (mmc1_prgrom_size < 512)
		      {
		      mmc1_cpu_bank [2] = mmc1_bank_number + 2;
                      mmc1_cpu_bank [3] = mmc1_bank_number + 3;
		      }
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

    /* Select first 16k page in lower 16k. */

    mmc_rom_banks [0] = ROM_PAGE_8K (0);

    mmc_rom_banks [1] = ROM_PAGE_8K (1);


    /* Select last 16k page in upper 16k. */

    mmc_rom_banks [2] = LAST_ROM_PAGE;

    mmc_rom_banks [3] = (LAST_ROM_PAGE + 0x2000);

    
    if (!ROM_CHR_ROM_PAGES)
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
    printf ("Using memory mapper #1 (MMC1) "
        "(%d PRG, %d CHR).\n\n", ROM_PRG_ROM_PAGES, ROM_CHR_ROM_PAGES);


    printf ("Warning: This mapper is only partially supported.\n");

    printf ("Press any key to continue...\n");


    while (!keypressed ());
    readkey ();

    mmc1_prgrom_size = (ROM_PRG_ROM_PAGES * 16);

    if (((ROM_PRG_ROM_PAGES * 2) - 1) > 0x1f)
         { 
	 mmc1_lastbank = 0x1f; 
	 } 
    else 
         { 
	 mmc1_lastbank = (ROM_PRG_ROM_PAGES * 2) - 1; 
	 } 

    mmc1_reset ();

    mmc_write = mmc1_write;
    return 0;
}
