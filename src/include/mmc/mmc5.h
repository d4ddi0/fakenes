

/* Mapper #5 (MMC5). */

/* This mapper is partially supported. */


static int mmc5_init (void);

static void mmc5_reset (void);


const MMC mmc_mmc5 =
{
    5, "MMC5 + ExSound",

    mmc5_init, mmc5_reset,


    "MMC5\0\0\0\0",

    null_save_state, null_load_state
};


#define MMC5_UPDATE_FILL_NAME       1
#define MMC5_UPDATE_FILL_ATTRIBUTE  2


/* 5102: xxxxxx10, 5103: xxxxxx01 to disable RAM write protect */
#define MMC5_RAM_WRITE_PROHIBITED \
 ((mmc5_5100[2] & 3) + ((mmc5_5100[3] & 3) << 2) != 6)

static INT8 mmc5_filled_name_table_needs_update;

/*static*/ UINT8 mmc5_5100[0x2C];
static UINT8 mmc5_5200[7];

static unsigned mmc5_multiply_result;
static INT8 mmc5_multiply_needs_update;


static UINT8 mmc5_wram[64 << 10];
static UINT8 mmc5_exram[1 << 10];
static UINT8 mmc5_filled_name_table[1 << 10];

static INT8 mmc5_wram_lut[8];
static INT8 background_patterns_last_mapped;


/* passed size in kbytes, defaults to 8k on invalid values */
static void mmc5_set_wram_size (int size)
{
 int i;
 switch (size)
 {
  case 64:
   for (i = 0; i < 8; i++) mmc5_wram_lut [i] = i;
   break;

  case 40:
   for (i = 0; i < 4; i++) mmc5_wram_lut [i] = i;
   for (i = 4; i < 8; i++) mmc5_wram_lut [i] = 4;
   break;

  case 32:
   for (i = 0; i < 4; i++) mmc5_wram_lut [i] = i;
   for (i = 4; i < 8; i++) mmc5_wram_lut [i] = -1;
   break;

  case 16:
   for (i = 0; i < 4; i++) mmc5_wram_lut [i] = 0;
   for (i = 4; i < 8; i++) mmc5_wram_lut [i] = 1;
   break;

  case 0:
   for (i = 0; i < 8; i++) mmc5_wram_lut [i] = -1;
   break;

  default: case 8:
   for (i = 0; i < 4; i++) mmc5_wram_lut [i] = 0;
   for (i = 4; i < 8; i++) mmc5_wram_lut [i] = -1;
 }
}


/* 5106: value returned for all names in 'filled' name table */
/* 5107: value returned for all attributes in 'filled' attribute table */
static void mmc5_update_filled_name_table (void)
{
/*
    if (mmc5_filled_name_table_in_use)
 */
    {
        if (mmc5_filled_name_table_needs_update & MMC5_UPDATE_FILL_NAME)
        {
            /* name table = 32x30 tiles */
            memset(mmc5_filled_name_table, mmc5_5100[6], (32 * 30));
        }

        if (mmc5_filled_name_table_needs_update & MMC5_UPDATE_FILL_ATTRIBUTE)
        {
            UINT8 attribute;

            /* attribute table = 16x16 tile-quads */
            attribute = mmc5_5100[7] & 3;
            attribute |= (attribute << 2) | (attribute << 4) | (attribute << 6);
            memset(mmc5_filled_name_table + (32 * 30), attribute,
                (32 * 32) / (2 * 2) / 4);
        }

        mmc5_filled_name_table_needs_update &=
            ~(MMC5_UPDATE_FILL_NAME | MMC5_UPDATE_FILL_ATTRIBUTE);
    }
}


static void mmc5_set_wram_banking_8k (UINT16 address, UINT8 bank)
{
    int index;

    if (mmc5_wram_lut[bank & 7] >= 0)
    {
        cpu_set_read_address_8k (address,
            mmc5_wram + (mmc5_wram_lut[bank & 7] << 13));
    }
    else
    {
        for (index = 0; index < (8 << 10); index += (2 << 10))
        {
            cpu_set_read_address_2k (address + index, dummy_read);
        }
    }

    if (mmc5_wram_lut[bank & 7] >= 0 && !MMC5_RAM_WRITE_PROHIBITED)
    {
        cpu_set_write_address_8k (address,
            mmc5_wram + (mmc5_wram_lut[bank & 7] << 13));
    }
    else
    {
        for (index = 0; index < (8 << 10); index += (2 << 10))
        {
            cpu_set_write_address_2k (address + index, dummy_write);
        }
    }
}


/* 5100: xxxxxxPP PRG banking mode, 5114-5117 is used for bank select */
/*  00: 32k 01: 16k 10: 16k/8k 11: 8k */
/*  bit 0 ignored in 16k select, bits 0 and 1 ignored in 32k select */
/*  the highest register for an address is used for any banking changes */
/*  RAM is available for mapping in banks not containing E000-FFFF, */
/*   by clearing bit 7 of bank value, bit 7 set requests ROM */
static void mmc5_update_prg_banking(void)
{
    int index;

    switch (mmc5_5100[0] & 3)
    {
    case 0: /* 32k banking */

        cpu_set_read_address_32k_rom_block (0x8000,
            (mmc5_5100[0x17] & ~0x80) >> 2);

        for (index = (32 << 10); index < (56 << 10); index += (2 << 10))
        {
            cpu_set_write_address_2k (index, dummy_write);
        }

        break;

    case 1: /* 16k banking */

        if (mmc5_5100[0x15] & 0x80)
        /* ROM */
        {
            cpu_set_read_address_16k_rom_block (0x8000,
                (mmc5_5100[0x15] & ~0x80) >> 1);

            for (index = (32 << 10); index < (48 << 10); index += (2 << 10))
            {
                cpu_set_write_address_2k (index, dummy_write);
            }
        }
        else
        /* RAM */
        {
            mmc5_set_wram_banking_8k (0x8000, mmc5_5100[0x15] & ~1);
            mmc5_set_wram_banking_8k (0xA000, mmc5_5100[0x15] | 1);
        }

        cpu_set_read_address_16k_rom_block (0xC000,
            (mmc5_5100[0x17] & ~0x80) >> 1);

        for (index = (48 << 10); index < (56 << 10); index += (2 << 10))
        {
            cpu_set_write_address_2k (index, dummy_write);
        }

        break;

    case 2: /* 16k/8k banking */

        if (mmc5_5100[0x15] & 0x80)
        /* ROM */
        {
            cpu_set_read_address_16k_rom_block (0x8000,
                (mmc5_5100[0x15] & ~0x80) >> 1);

            for (index = (32 << 10); index < (48 << 10); index += (2 << 10))
            {
                cpu_set_write_address_2k (index, dummy_write);
            }
        }
        else
        /* RAM */
        {
            mmc5_set_wram_banking_8k (0x8000, mmc5_5100[0x15] & ~1);
            mmc5_set_wram_banking_8k (0xA000, mmc5_5100[0x15] | 1);
        }

        if (mmc5_5100[0x16] & 0x80)
        /* ROM */
        {
            cpu_set_read_address_8k_rom_block (0xC000,
                (mmc5_5100[0x16] & ~0x80));

            for (index = (48 << 10); index < (56 << 10); index += (2 << 10))
            {
                cpu_set_write_address_2k (index, dummy_write);
            }
        }
        else
        /* RAM */
        {
            mmc5_set_wram_banking_8k (0xC000, mmc5_5100[0x16]);
        }

        cpu_set_read_address_8k_rom_block (0xE000,
            (mmc5_5100[0x17] & ~0x80));

        break;

    case 3: /* 8k banking */

        if (mmc5_5100[0x14] & 0x80)
        /* ROM */
        {
            cpu_set_read_address_8k_rom_block (0x8000,
                (mmc5_5100[0x14] & ~0x80));

            for (index = (32 << 10); index < (40 << 10); index += (2 << 10))
            {
                cpu_set_write_address_2k (index, dummy_write);
            }
        }
        else
        /* RAM */
        {
            mmc5_set_wram_banking_8k (0x8000, mmc5_5100[0x14]);
        }

        if (mmc5_5100[0x15] & 0x80)
        /* ROM */
        {
            cpu_set_read_address_8k_rom_block (0xA000,
                (mmc5_5100[0x15] & ~0x80));

            for (index = (40 << 10); index < (48 << 10); index += (2 << 10))
            {
                cpu_set_write_address_2k (index, dummy_write);
            }
        }
        else
        /* RAM */
        {
            mmc5_set_wram_banking_8k (0xA000, mmc5_5100[0x15]);
        }

        if (mmc5_5100[0x16] & 0x80)
        /* ROM */
        {
            cpu_set_read_address_8k_rom_block (0xC000,
                (mmc5_5100[0x16] & ~0x80));

            for (index = (48 << 10); index < (56 << 10); index += (2 << 10))
            {
                cpu_set_write_address_2k (index, dummy_write);
            }
        }
        else
        /* RAM */
        {
            mmc5_set_wram_banking_8k (0xC000, mmc5_5100[0x16]);
        }

        cpu_set_read_address_8k_rom_block (0xE000,
            (mmc5_5100[0x17] & ~0x80));

        break;

    }
}


static void mmc5_update_ram_protection (void)
{
    mmc5_set_wram_banking_8k (0x6000, mmc5_5100[0x13]);
    mmc5_update_prg_banking ();
}


/* 5101: xxxxxxPP CHR banking mode */
/*  00: 8k 01: 4k 10: 2k 11: 1k */
/*  the highest register for an address is used for any banking changes */
/*  5120-5127 is used for SPR CHR bank select */
/*  5128-512B is used for BG CHR bank select, if 8k is disabled, high */
/*   and low 4k is same */
/*  the last type of register to be written to, BG or SPR, determines which */
/*   is accessed by PPU read */
static void mmc5_update_chr_banking(void)
{
    int index;

    int sprites_map_type = PPU_MAP_SPRITES |
        (!background_patterns_last_mapped ? PPU_MAP_RAM : 0);

    int background_map_type = PPU_MAP_BACKGROUND |
        (background_patterns_last_mapped ? PPU_MAP_RAM : 0);


    switch (mmc5_5100[1] & 3)
    {
        case 0: /* 8k banking */

        for (index = 0; index < 8; index++)
        {
            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x27] + index, sprites_map_type);

            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x2B] + index, background_map_type);
        }

        break;


    case 1: /* 4k banking */

        for (index = 0; index < 8; index++)
        {
            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x23 + (index & 4)] + (index & 3),
                sprites_map_type);

            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x2B] + (index & 3), background_map_type);
        }

        break;


    case 2: /* 2k banking */

        for (index = 0; index < 8; index++)
        {
            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x21 + (index & 6)] + (index & 1),
                sprites_map_type);

            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x29 + (index & 2)] + (index & 1),
                background_map_type);
        }

        break;


    case 3: /* 1k banking */

        for (index = 0; index < 8; index++)
        {
            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x20 + index], sprites_map_type);

            ppu_set_ram_1k_pattern_vrom_block_ex (index << 10,
                mmc5_5100[0x28 + (index & 3)], background_map_type);
        }

        break;

    }
}


static int mmc5_irq_line_counter = 0;
static UINT8 mmc5_irq_line_requested = 0;

static UINT8 mmc5_irq_status = 0;

static UINT8 mmc5_disable_irqs = TRUE;


static int mmc5_irq_tick (int line)
{
    if (line == 0)
    {
        mmc5_irq_line_counter = 0;
    }

    if (mmc5_irq_line_counter >= 238)
    {
        mmc5_irq_status |= 0x40;
    }

    if (mmc5_irq_line_counter < 245)
    {
        if (++mmc5_irq_line_counter == mmc5_irq_line_requested)
        {
            mmc5_irq_status |= 0x80;
            if (!mmc5_disable_irqs) return CPU_INTERRUPT_IRQ_MMC;
        }
    }


    return CPU_INTERRUPT_NONE;
}


/* 5000, 5004: channel 1/2 pulse control */
/* 5002, 5006: channel 1/2 frequency low */
/* 5003, 5007: channel 1/2 frequency high */
/* 5010: xPxxxxxx P = disable PCM output */
/* 5011: PCM wave height */
/* 5015: MMC5 PSG ??? */
/* 5100: xxxxxxPP PRG banking mode, 5114-5117 is used for bank select */
/*  00: 32k 01: 16k 10: 16k/8k 11: 8k */
/*  bit 0 ignored in 16k select, bits 0 and 1 ignored in 32k select */
/*  the highest register for an address is used for any banking changes */
/*  RAM is available for mapping in banks not containing E000-FFFF, */
/*   by clearing bit 7 of bank value, bit 7 set requests ROM */
/* 5101: xxxxxxPP CHR banking mode */
/*  00: 8k 01: 4k 10: 2k 11: 1k */
/*  the highest register for an address is used for any banking changes */
/*  5120-5127 is used for SPR CHR bank select */
/*  5128-512B is used for BG CHR bank select, if 8k is disabled, high */
/*   and low 4k is same */
/*  the last type of register to be written to, BG or SPR, determines which */
/*   is accessed by PPU read */
/* 5102: xxxxxx10, 5103: xxxxxx01 to disable RAM write protect */
/* 5104: EXRAM/split mode control ??? */
/* 5105: nametable selection CC884400 CC = 2C00, etc. */
/*  00 = PPU $2000, 01 = PPU $2400, 10 = EXRAM @ 5C00-5FFF, 11 = filled */
/* 5106: value returned for all names in 'filled' name table */
/* 5107: value returned for all attributes in 'filled' attribute table */
/* 5113: RAM bank select for 6000-7FFF */
/*  xxxxxCBB C = chip select, B = bank select */
/*  8k = chip 1 open bus, bank select ignored */
/*  16k = bank select ignored */
/*  32k = chip 1 open bus */
/* 5114: PRG bank select for 8000-9FFF */
/* 5115: PRG bank select for A000-BFFF */
/* 5116: PRG bank select for C000-DFFF */
/* 5117: PRG bank select for E000-FFFF */
/* 5120: SPR CHR bank select for 0000-03FF */
/* 5121: SPR CHR bank select for 0400-07FF */
/* 5122: SPR CHR bank select for 0800-0BFF */
/* 5123: SPR CHR bank select for 0C00-0FFF */
/* 5124: SPR CHR bank select for 1000-13FF */
/* 5125: SPR CHR bank select for 1400-17FF */
/* 5126: SPR CHR bank select for 1800-1BFF */
/* 5127: SPR CHR bank select for 1C00-1FFF */
/* 5128: BG CHR bank select for 0000-03FF 1000-13FF */
/* 5129: BG CHR bank select for 0400-07FF 1400-17FF */
/* 512A: BG CHR bank select for 0800-0BFF 1800-1BFF */
/* 512B: BG CHR bank select for 0C00-0FFF 1C00-1FFF */
/* 5200-5202 = split mode control ??? */
/* 5203: IRQ scanline select */
/* 5204: IRQ enable/status register */
/* 5205/5206 = hardware 8x8=16 multiply */
/*  result read from 5205/5206 (low/high) */


static UINT8 mmc5_read (UINT16 address)
{
    switch (address)
    {
/* 5204 = IRQ enable/status register */
        case 0x5204:
        {
            UINT8 read_value = mmc5_irq_status;

            cpu_clear_interrupt (CPU_INTERRUPT_IRQ_MMC);

            mmc5_irq_status &= 0x40;

            if (mmc5_disable_irqs)
            {
                mmc5_irq_status = 0;
            }

            return read_value;
        }
/* 5205/5206 = hardware 8x8=16 multiply */
/*  result read from 5205/5206 (low/high) */
        case 0x5205:

            if (mmc5_multiply_needs_update)
            {
                mmc5_multiply_result = (unsigned) mmc5_5200[5] *
                    (unsigned) mmc5_5200[6];

                mmc5_multiply_needs_update = FALSE;
            }

            return (mmc5_multiply_result & 0xFF);


/* 5205/5206 = hardware 8x8=16 multiply */
/*  result read from 5205/5206 (low/high) */
        case 0x5206:

            if (mmc5_multiply_needs_update)
            {
                mmc5_multiply_result = (unsigned) mmc5_5200[5] *
                    (unsigned) mmc5_5200[6];

                mmc5_multiply_needs_update = FALSE;
            }

            return ((mmc5_multiply_result >> 8) & 0xFF);


        default:

            break;
    }

    return 0;
}


static void mmc5_write (UINT16 address, UINT8 value)
{
    int scrap;


    if (address < 0x5100)
    {
        papu_exwrite (address, value);
        return;
    }

    switch (address)
    {
        case 0x5100:

            /* PRG banking mode (bits 0-1). */

            if (!((mmc5_5100[0] ^ value) & 3)) break;

            mmc5_5100[0] = value;

            mmc5_update_prg_banking ();

            break;


        case 0x5101:

            /* CHR banking mode (bits 0-1). */

            if (!((mmc5_5100[1] ^ value) & 3)) break;

            mmc5_5100[1] = value;

            mmc5_update_chr_banking ();

            break;


        case 0x5102:

            /* RAM write protect register 1 (bits 0-1). */

        case 0x5103:

            /* RAM write protect register 2 (bits 0-1). */

            if (!((mmc5_5100[address & 3] ^ value) & 3)) break;

            mmc5_5100[address & 3] = value;
            mmc5_update_ram_protection ();

            break;


        case 0x5104:

            /* EXRAM/split mode control ??? */

            mmc5_5100[4] = value;

            break;


        case 0x5105:

            /* name table selection */

            if (!(mmc5_5100[5] ^ value)) break;

            for (scrap = 0; scrap < 4; scrap++)
            {
                if (!(((mmc5_5100[5] ^ value) >> (scrap * 2)) & 3))
                    continue;

                switch ((value >> (scrap * 2)) & 3)
                {
                    case 0: /* PPU $2000 */
                        ppu_set_name_table_internal (scrap, 0);
                        break;
                        
                    case 1: /* PPU $2400 */
                        ppu_set_name_table_internal (scrap, 1);
                        break;
                        
                    case 2: /* EXRAM */
                        ppu_set_name_table_address (scrap, mmc5_exram);
                        break;
                        
                    case 3: /* filled, special */
                        ppu_set_name_table_address_rom (scrap,
                            mmc5_filled_name_table);
/*                      mmc5_update_filled_name_table ();*/
                        break;
                        
                }
            }

            mmc5_5100[5] = value;

            break;

        case 0x5106: /* 'filled' name table value */

            if (!(mmc5_5100[6] ^ value)) break;

            mmc5_5100[6] = value;

            mmc5_filled_name_table_needs_update |= MMC5_UPDATE_FILL_NAME;

            mmc5_update_filled_name_table ();

            break;

        case 0x5107: /* 'filled' attribute table value */

            if (!((mmc5_5100[7] ^ value) & 3)) break;

            mmc5_5100[7] = value;

            mmc5_filled_name_table_needs_update |= MMC5_UPDATE_FILL_ATTRIBUTE;

            mmc5_update_filled_name_table ();

            break;


        case 0x5113: /* RAM bank select for 6000-7FFF */

            if (!((mmc5_5100[0x13] ^ value) & 7)) break;

            mmc5_5100[0x13] = value;

            mmc5_set_wram_banking_8k (0x6000, value);

            break;


        case 0x5114: /* Bank select for 8000-9FFF */

            if (!(mmc5_5100[0x14] ^ value)) break;

            mmc5_5100[0x14] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_prg_banking ();

            break;


        case 0x5115: /* Bank select for A000-BFFF */

            if (!(mmc5_5100[0x15] ^ value)) break;

            mmc5_5100[0x15] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_prg_banking ();

            break;


        case 0x5116: /* Bank select for C000-DFFF */

            if (!(mmc5_5100[0x16] ^ value)) break;

            mmc5_5100[0x16] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_prg_banking ();

            break;


        case 0x5117: /* ROM bank select for E000-FFFF */

            if (!(mmc5_5100[0x17] ^ value)) break;

            mmc5_5100[0x17] = value;

            /* always affects banking */
            mmc5_update_prg_banking ();

            break;


/* 5120: SPR CHR bank select for 0000-03FF */
/* 5121: SPR CHR bank select for 0400-07FF */
/* 5122: SPR CHR bank select for 0800-0BFF */
/* 5123: SPR CHR bank select for 0C00-0FFF */
/* 5124: SPR CHR bank select for 1000-13FF */
/* 5125: SPR CHR bank select for 1400-17FF */
/* 5126: SPR CHR bank select for 1800-1BFF */
/* 5127: SPR CHR bank select for 1C00-1FFF */

        case 0x5120:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x20] ^ value)) break;

            mmc5_5100[0x20] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5121:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x21] ^ value)) break;

            mmc5_5100[0x21] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5122:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x22] ^ value)) break;

            mmc5_5100[0x22] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5123:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x23] ^ value)) break;

            mmc5_5100[0x23] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5124:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x24] ^ value)) break;

            mmc5_5100[0x24] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5125:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x25] ^ value)) break;

            mmc5_5100[0x25] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5126:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x26] ^ value)) break;

            mmc5_5100[0x26] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5127:

            background_patterns_last_mapped = FALSE;

            if (!(mmc5_5100[0x27] ^ value)) break;

            mmc5_5100[0x27] = value;

            /* always affects banking */
            mmc5_update_chr_banking ();

            break;


/* 5128: BG CHR bank select for 0000-03FF 1000-13FF */
/* 5129: BG CHR bank select for 0400-07FF 1400-17FF */
/* 512A: BG CHR bank select for 0800-0BFF 1800-1BFF */
/* 512B: BG CHR bank select for 0C00-0FFF 1C00-1FFF */
        case 0x5128:

            background_patterns_last_mapped = TRUE;

            if (!(mmc5_5100[0x28] ^ value)) break;

            mmc5_5100[0x28] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x5129:

            background_patterns_last_mapped = TRUE;

            if (!(mmc5_5100[0x29] ^ value)) break;

            mmc5_5100[0x29] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x512A:

            background_patterns_last_mapped = TRUE;

            if (!(mmc5_5100[0x2A] ^ value)) break;

            mmc5_5100[0x2A] = value;

            /* to do: determine if banking affected by write */
            mmc5_update_chr_banking ();

            break;


        case 0x512B:

            background_patterns_last_mapped = TRUE;

            if (!(mmc5_5100[0x2B] ^ value)) break;

            mmc5_5100[0x2B] = value;

            /* always affects banking */
            mmc5_update_chr_banking ();

            break;


/* 5203: IRQ scanline select */
        case 0x5203:

            cpu_clear_interrupt (CPU_INTERRUPT_IRQ_MMC);

            mmc5_irq_line_requested = value;

            break;

/* 5204: IRQ enable/status register */
        case 0x5204:

            cpu_clear_interrupt (CPU_INTERRUPT_IRQ_MMC);

            mmc5_disable_irqs = (~value & 0x80);

            break;

/* 5205/5206 = hardware 8x8=16 multiply */
        case 0x5205:
        case 0x5206:

            if (!(mmc5_5200[address & 0x07] ^ value)) break;

            mmc5_5200[address & 0x07] = value;

            mmc5_multiply_needs_update = TRUE;

            break;


        default:

            break;
    }
}


static UINT8 mmc5_exram_read (UINT16 address)
{
    if (address < 0x5C00) return 0;

    return mmc5_exram[address - 0x5C00];
}


static void mmc5_exram_write (UINT16 address, UINT8 value)
{
    if (address < 0x5C00) return;

    mmc5_exram[address - 0x5C00] = value;
}


static void mmc5_reset (void)
{
    int index;

    /* Set everything up. */
    mmc5_5100[0x00] = mmc5_5100[0x01] = mmc5_5100[0x02] = mmc5_5100[0x03] =
    mmc5_5100[0x05] = mmc5_5100[0x06] = mmc5_5100[0x07] = mmc5_5100[0x13] =
    mmc5_5100[0x14] = mmc5_5100[0x15] = mmc5_5100[0x16] = mmc5_5100[0x17] =
    mmc5_5100[0x20] = mmc5_5100[0x21] = mmc5_5100[0x22] = mmc5_5100[0x23] =
    mmc5_5100[0x24] = mmc5_5100[0x25] = mmc5_5100[0x26] = mmc5_5100[0x27] =
    mmc5_5100[0x28] = mmc5_5100[0x29] = mmc5_5100[0x2A] = mmc5_5100[0x2B] =
        ~0;


    mmc5_filled_name_table_needs_update =
        MMC5_UPDATE_FILL_NAME | MMC5_UPDATE_FILL_ATTRIBUTE;
    mmc5_update_filled_name_table ();

    for (index = 0; index < 4; index++)
    {
        ppu_set_name_table_address_rom (index, mmc5_filled_name_table);
    }


    mmc5_set_wram_banking_8k (0x6000, mmc5_5100[0x13]);
    mmc5_update_prg_banking ();
    mmc5_update_chr_banking ();
}


static int mmc5_init (void)
{
    int index;

    if (mmc_pattern_vram_in_use)
    {
        /* No VROM is present. */
        return (1);
    }

    mmc_name_table_count = 4;


    mmc5_set_wram_size (8);
    mmc5_reset ();

    cpu_set_write_handler_2k (0x5000, mmc5_write);
    cpu_set_read_handler_2k (0x5000, mmc5_read);
    cpu_set_read_handler_2k (0x5800, mmc5_exram_read);
    cpu_set_write_handler_2k (0x5800, mmc5_exram_write);

    mmc5_multiply_needs_update = TRUE;

    mmc_scanline_end = mmc5_irq_tick;
    mmc5_disable_irqs = TRUE;


    /* Select ExSound chip. */

    papu_set_exsound (PAPU_EXSOUND_MMC5);


    return (0);
}
