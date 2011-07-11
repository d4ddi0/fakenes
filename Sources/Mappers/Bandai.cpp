/* Mapper #16 (Bandai). */
/* This mapper is only partially supported. */

/* Todo: Add EEPROM support. */

#include "mmc/shared.h"

static int bandai_init(void);
static void bandai_reset(void);
static void bandai_save_state(PACKFILE *, const int);
static void bandai_load_state(PACKFILE *, const int);

static const MMC mmc_bandai =
{
   16, "Bandai",
   bandai_init, bandai_reset,
   "BANDAI\0\0",
   bandai_save_state, bandai_load_state,
   NULL, NULL, NULL, NULL
};

static const ENUM bandai_mirroring_table[] = {
   PPU_MIRRORING_VERTICAL, PPU_MIRRORING_HORIZONTAL,
   PPU_MIRRORING_ONE_SCREEN_2000, PPU_MIRRORING_ONE_SCREEN_2400
};

static const UINT8 bandai_mirroring_mask = 0x03;

static BOOL bandai_enable_irqs = FALSE;
static int bandai_irq_counter = 0;
static PAIR bandai_irq_latch;

static UINT8 bandai_prg_bank;
static UINT8 bandai_chr_bank[8];

/* This needs repredicting when the following variables change:
      bandai_enable_irqs
      bandai_irq_counter */

static BOOL bandai_irq_slave(const BOOL simulate)
{
   if(!bandai_enable_irqs)
      return FALSE;

    bandai_irq_counter -= MMC_PSEUDO_CLOCKS_PER_SCANLINE;
    if(bandai_irq_counter <= 0) {
       /* Don't modify anything else when just simulating. */
       if(!simulate) {
          bandai_irq_counter = 0;
          bandai_enable_irqs = FALSE;
        }

       /* Trigger an interrupt. */
       return TRUE;
   }

   /* Don't trigger an interrupt. */
   return FALSE;
}

static BOOL bandai_irq_predictor(const int line)
{
   BOOL trigger;

   /* Save the IRQ counter since we're just simulating. */
   int saved_irq_counter = bandai_irq_counter;

   /* Clock the IRQ counter. */
   trigger = bandai_irq_slave(TRUE);

   /* Restore the IRQ counter from the backup. */
   bandai_irq_counter = saved_irq_counter;

   return trigger;
}

static void bandai_scanline_start(const int line)
{
   bandai_irq_slave(FALSE);
}

static INLINE void bandai_update_prg_bank(void)
{
   cpu_set_read_address_16k_rom_block(0x8000, bandai_prg_bank);
}

static INLINE void bandai_update_chr_bank(const int bank)
{
   if(ROM_CHR_ROM_PAGES > 0)
      /* set new VROM banking */
      ppu_set_1k_pattern_table_vrom_page(bank << 10, bandai_chr_bank[bank]);
}

static void bandai_write(UINT16 address, UINT8 value)
{
   /* Extract write port index. */
   address &= 0x000F;
   if(address > 0x000D)
      return;

   if(address <= 7) {
      /* Set requested 1k CHR-ROM page. */
      bandai_chr_bank[address] = value;
      bandai_update_chr_bank(address);
   }
   else {
      /* Convert $600X and $7FFX to $800X. */
      address += 0x8000;
      switch (address) {
         case 0x8008:
            /* Set requested 16k ROM page at $8000. */
            bandai_prg_bank = value;
            bandai_update_prg_bank();
            break;

         case 0x8009:
            /* Mirroring select. */
            /* Mask off upper 6 bits. */
            value &= bandai_mirroring_mask;
            /* Use value from LUT. */
            ppu_set_mirroring(bandai_mirroring_table[value]);
            break;

         case 0x800A: {
            /* Enable/disable IRQs. */
            cpu_clear_interrupt(CPU_INTERRUPT_IRQ_MAPPER_PROXY);
            bandai_enable_irqs = value & 0x01;
            bandai_irq_counter = bandai_irq_latch.word;

            /* IRQs are driven by the PPU. */
            ppu_repredict_interrupts(PPU_PREDICT_MMC_IRQ);

            break;
         }

         case 0x800B:
            /* Low byte of IRQ counter. */
            bandai_irq_latch.bytes.low = value;
            break;

         case 0x800C:
            /* High byte of IRQ counter. */
            bandai_irq_latch.bytes.high = value;
            break;
      }
   }
}

static void bandai_reset(void)
{
   int index;

   /* Set initial VROM mappings. */
   for(index = 0; index < 8; index++) {
       bandai_chr_bank[index] = index;
       bandai_update_chr_bank(index);
   }

   /* Select first 16k page in lower 16k. */
   bandai_prg_bank = 0;
   bandai_update_prg_bank();
   cpu_set_read_address_16k(0x8000, FIRST_ROM_PAGE);

   /* Select last 16k page in upper 16k. */
   cpu_set_read_address_16k(0xC000, LAST_ROM_PAGE);
}

static int bandai_init(void)
{
   /* Install write handlers. */
   cpu_set_write_handler_8k(0x6000, bandai_write);
   cpu_set_write_handler_32k(0x8000, bandai_write);

   /* Install IRQ tick handlers. */
   mmc_scanline_start = bandai_scanline_start;
   mmc_virtual_scanline_start = bandai_irq_predictor;

   bandai_reset();
   return 0;
}

static void bandai_save_state(PACKFILE* file, const int version) {
   RT_ASSERT(file);

   /* Save IRQ registers */
   pack_iputw(bandai_irq_counter, file);
   pack_iputw(bandai_irq_latch.word, file);
   pack_putc (bandai_enable_irqs, file);

   /* Save banking */
   pack_putc(bandai_prg_bank, file);
   pack_fwrite(bandai_chr_bank, 8, file);
}

static void bandai_load_state(PACKFILE* file, const int version) {
   int index;

   RT_ASSERT(file);

   /* Restore IRQ registers */
   bandai_irq_counter = pack_igetw(file);
   bandai_irq_latch.word = pack_igetw(file);
   bandai_enable_irqs = pack_getc(file);

   /* Restore banking */
   bandai_prg_bank = pack_getc(file);
   pack_fread(bandai_chr_bank, 8, file);

   bandai_update_prg_bank();

   for(index = 0; index < 8; index++)
      bandai_update_chr_bank(index);
}
