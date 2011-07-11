/* Mapper #4 (MMC3). */
/* This mapper is fully supported. */

/* A variant known as MMC6 is used by StarTropics, which
   requires SRAM to be disabled by default. */

/* MMC3 IRQ depends on PPU internal timing. */
#include "ppu_int.h"

static int mmc3_init(void);
static void mmc3_reset(void);
static void mmc3_save_state(PACKFILE*, const int);
static void mmc3_load_state(PACKFILE*, const int);

static const MMC mmc_mmc3 = {
   4,
   "MMC3",
   mmc3_init, mmc3_reset,
   "MMC3\0\0\0\0",
   mmc3_save_state, mmc3_load_state,
   NULL, NULL, NULL, NULL
};

static int mmc3_command = 0;

static int mmc3_prg_address = 0x8000;
static int mmc3_chr_address = 0;

#define MMC3_PRG_BANKS 2
#define MMC3_CHR_BANKS 6
static UINT8 mmc3_prg_bank[MMC3_PRG_BANKS];
static UINT8 mmc3_chr_bank[MMC3_CHR_BANKS];

static UINT8 mmc3_irq_counter = 0;
static UINT8 mmc3_irq_latch = 0;
static BOOL mmc3_disable_irqs = TRUE;

static UINT8 mmc3_irq_bank = 0;
static BOOL mmc3_irq_queued = FALSE;

static UINT8 mmc3_register_8000;
static UINT8 mmc3_sram_enable;

#define MMC3_PRG_ADDRESS_BIT 0x40
#define MMC3_CHR_ADDRESS_BIT 0x80

/* Note: Don't save configuration variables to save states. */
static int mmc3_config_irq_mode = 0;
static int mmc3_config_sram = 1;

/* This needs reprediction when the following variables change:
      mmc3_config_irq_mode
      mmc3_disable_irqs
      mmc3_irq_counter
      mmc3_irq_latch
      mmc3_irq_queued */

static BOOL mmc3_irq_slave(const BOOL simulate)
{
   /* When the scanline counter is clocked, the value will first be checked. If it is zero,
      it will be reloaded from the IRQ latch ($C000); otherwise, it will decrement. If the
      old value in the counter is nonzero and new value is zero (whether from decrementing or
      reloading), an IRQ will be fired if IRQ generation is enabled (by writing to $E001). */

   /* There's a slight discrepancy with what happens when you set $C000 to $00, and so far,
      two behaviors are known to exist: 

      All MMC3A's and non-Sharp MMC3B's will generate only a single IRQ when $C000 is $00.
      This is because this version of the MMC3 generates IRQs when the scanline counter is
      decremented to 0. In addition, writing to $C001 with $C000 still at $00 will result
      in another single IRQ being generated. In the community, this is known as the
      "alternate" behavior. 

      All MMC3C's and Sharp MMC3B's will generate an IRQ on each scanline while $C000 is $00.
      This is because this version of the MMC3 generates IRQs when the scanline counter is
      equal to 0. In the community, this is known as the "normal" behavior. */

   BOOL trigger = FALSE;
   switch(mmc3_config_irq_mode) {
      case 0: {
         /* Normal behavior. */
         const UINT8 saved_irq_counter = mmc3_irq_counter;

         if(mmc3_irq_counter == 0)
            mmc3_irq_counter = mmc3_irq_latch;
         else
            mmc3_irq_counter--;

         if((saved_irq_counter != 0) && (mmc3_irq_counter == 0))
            trigger = TRUE;

         break;
      }

      case 1: {
         /* Alternate behavior. */
         if(mmc3_irq_counter == 0) {
            mmc3_irq_counter = mmc3_irq_latch;
         }
         else {
            mmc3_irq_counter--;
            if(mmc3_irq_counter == 0)
               trigger = TRUE;
         }

         break;
      }

      default:
         WARN_GENERIC();
         return FALSE;
   }

   if(!mmc3_disable_irqs &&
      (trigger || mmc3_irq_queued)) {
       /* Don't modify anything else when just simulating. */
      if(!simulate)
         mmc3_irq_queued = FALSE;

       /* Trigger an interrupt. */
      return TRUE;
   }

   /* Don't trigger an interrupt. */
   return FALSE;
}

static BOOL mmc3_irq_predictor(const int line)
{
   BOOL trigger;

   /* The IRQ counter is only clocked when the PPU is rendering. */
   if(line > PPU_LAST_DISPLAYED_LINE)
      return FALSE;

   /* Save the IRQ counter since we're just simulating. */
   int saved_irq_counter = mmc3_irq_counter;

   /* Clock the IRQ counter. */
   trigger = mmc3_irq_slave(TRUE);

   /* Restore the IRQ counter from the backup. */
   mmc3_irq_counter = saved_irq_counter;

   return trigger;
}

static void mmc3_irq_handler(const int line)
{
   if(line > PPU_LAST_DISPLAYED_LINE)
      return;

   mmc3_irq_slave(FALSE);
}

static void mmc3_check_address_lines(const UINT16 address)
{
   /* The MMC3 scanline counter is based entirely on PPU A12, triggered on rising edges
      (after the line remains low for a sufficiently long period of time). */

   if(address > 0x1FFF)
      /* We only care about accesses to pattern table data. */
      return;

   /* The counter will not work properly unless you use different pattern tables for
      background and sprite data. */
   const int bank = address / PPU__BYTES_PER_PATTERN_TABLE;
   if((bank == 1) && (mmc3_irq_bank == 0))
      mmc3_irq_slave(FALSE);

   mmc3_irq_bank = bank;
}

static void mmc3_check_vram_banking(void)
{
   int background_bank, sprite_bank;

   mmc_hblank_start = NULL;
   mmc_virtual_hblank_start = NULL;

   mmc_hblank_prefetch_start = NULL;
   mmc_virtual_hblank_prefetch_start = NULL;

   mmc_check_address_lines = NULL;

   /* When the CPU is in unchained mode, we can do cycle-by-cycle processing. */
   if(cpu_get_execution_model() == CPU_EXECUTION_MODEL_UNCHAINED) {
      mmc_check_address_lines = mmc3_check_address_lines;
      return;
   }

   /* Otherwise, we have to use IRQ prediction. */

   /* If the BG uses $0000, and the sprites use $1000, then the IRQ will occur after PPU cycle 260
      (as in, a little after the visible part of the target scanline has ended). 

      If the BG uses $1000, and the sprites use $0000, then the IRQ will occur after PPU cycle 324
      of the previous scanline (as in, right before the target scanline is about to be drawn). */

   background_bank = (ppu__background_tileset >> 12) & 1;
   sprite_bank = (ppu__sprite_tileset >> 12) & 1;

   if((background_bank == 0) && (sprite_bank == 1)) {
      mmc_hblank_start = mmc3_irq_handler;
      mmc_virtual_hblank_start = mmc3_irq_predictor;
   }
   else if((background_bank == 1) && (sprite_bank == 0)) {
      mmc_hblank_prefetch_start = mmc3_irq_handler;
      mmc_virtual_hblank_prefetch_start = mmc3_irq_predictor;
   }

   ppu_repredict_interrupts(PPU_PREDICT_MMC_IRQ);
}

static void mmc3_cpu_bank_sort(void)
{
   /* set address for non-swappable page */
   const UINT16 address = (mmc3_prg_address == 0x8000) ? 0xC000 : 0x8000;
   cpu_set_read_address_8k(address, LAST_ROM_PAGE);

   /* set addresses for swappable pages */
   cpu_set_read_address_8k_rom_block(mmc3_prg_address, mmc3_prg_bank[0]);
   cpu_set_read_address_8k_rom_block(0xA000, mmc3_prg_bank[1]);
}

static void mmc3_ppu_bank_sort(void)
{
   if(ROM_CHR_ROM_PAGES == 0)
      return;

   ppu_set_1k_pattern_table_vrom_page(mmc3_chr_address << 10, mmc3_chr_bank[0] & ~1);
   ppu_set_1k_pattern_table_vrom_page((1 + mmc3_chr_address) << 10, mmc3_chr_bank[0] | 1);
   ppu_set_1k_pattern_table_vrom_page((2 + mmc3_chr_address) << 10, mmc3_chr_bank[1] & ~1);
   ppu_set_1k_pattern_table_vrom_page((3 + mmc3_chr_address) << 10, mmc3_chr_bank[1] | 1);
   ppu_set_1k_pattern_table_vrom_page((4 - mmc3_chr_address) << 10, mmc3_chr_bank[2]);
   ppu_set_1k_pattern_table_vrom_page((5 - mmc3_chr_address) << 10, mmc3_chr_bank[3]);
   ppu_set_1k_pattern_table_vrom_page((6 - mmc3_chr_address) << 10, mmc3_chr_bank[4]);
   ppu_set_1k_pattern_table_vrom_page((7 - mmc3_chr_address) << 10, mmc3_chr_bank[5]);
}

static void mmc3_write(UINT16 address, UINT8 value)
{
   int scrap;

   switch(address & 0xE001) {
      case 0x8000: {
         /* Bits 0 to 2 (command). */
         mmc3_register_8000 = value;
         mmc3_command = value & 0x07;

         /* Bits 6 and 7 (latches). */
         scrap = (value & MMC3_CHR_ADDRESS_BIT) ? 4 : 0;
         if((mmc3_chr_address != scrap) &&
            (ROM_CHR_ROM_PAGES > 0)) {
            mmc3_chr_address = scrap;
            mmc3_ppu_bank_sort();
         }

         scrap = (value & MMC3_PRG_ADDRESS_BIT) ? 0xC000 : 0x8000;
         if(mmc3_prg_address != scrap) {
            mmc3_prg_address = scrap;
            mmc3_cpu_bank_sort();
         }

         break;
      }

      case 0x8001: {
         switch(mmc3_command) {
            /* VROM swapping commands. */
 
            case 0:
            case 1: {
               /* 2 1k pages at $0000 or $0800. */
               mmc3_chr_bank[mmc3_command] = value;

               if(ROM_CHR_ROM_PAGES > 0) {
                  scrap = (mmc3_command * 2) ^ mmc3_chr_address;
                  ppu_set_1k_pattern_table_vrom_page(scrap << 10, value & ~1);
                  ppu_set_1k_pattern_table_vrom_page(++scrap << 10, value | 1);
               }

               break;
            }

            case 2:
            case 3:
            case 4:
            case 5: {
               /* 1 1k page at $1000 to $1c00. */
               mmc3_chr_bank[mmc3_command] = value;

               if(ROM_CHR_ROM_PAGES > 0) {
                  scrap = (mmc3_command + 2) ^ mmc3_chr_address;
                  ppu_set_1k_pattern_table_vrom_page(scrap << 10, value);
               }

               break;
            }

            /* ROM swapping commands. */

            case 6: {
               /* 1 8k page at $8000 or $C000. */
               mmc3_prg_bank[0] = value;
               cpu_set_read_address_8k_rom_block(mmc3_prg_address, value);

               break;
            }

            case 7: {
               /* 1 8k page at $A000. */
               mmc3_prg_bank[1] = value;
               cpu_set_read_address_8k_rom_block(0xA000, value);

               break;
            }

            default:
               break;
         }

         break;
      }

      case 0xA000: {
         /* Mirroring select. */
         if(!(global_rom.control_byte_1 & ROM_CTRL_FOUR_SCREEN))
            ppu_set_mirroring(((value & 0x01) ? PPU_MIRRORING_HORIZONTAL : PPU_MIRRORING_VERTICAL));

         break;
      }

      case 0xA001: {
         /* SRAM disable & enable. */
         /* Note: This needs to be disabled for Star Tropics(MMC6). */
         mmc3_sram_enable = value & 0x80;
         if(mmc3_sram_enable)
            cpu_enable_sram();
         else
            cpu_disable_sram();

         break;
      }

      /* This register specifies the IRQ counter reload value. When the IRQ counter is
         zero (or a reload is requested through $C001), this value will be copied into the
         MMC3 IRQ counter at the end of the current scanline. */
      case 0xC000: {
         mmc3_irq_latch = value;

         if((mmc3_config_irq_mode == 1) && (mmc3_irq_latch == 0))
            /* Force a single IRQ to be generated, even on latch=0. */
            mmc3_irq_queued = TRUE;

         /* IRQs are driven by the PPU. */
         ppu_repredict_interrupts(PPU_PREDICT_MMC_IRQ);

         break;
      }

      /* Writing any value to this register clears the MMC3 IRQ counter so that it will be
         reloaded at the end of the current scanline. */
      case 0xC001: {
         mmc3_irq_counter = 0;
         ppu_repredict_interrupts(PPU_PREDICT_MMC_IRQ);

         break;
      }

      /* Writing any value to this register will disable MMC3 interrupts AND acknowledge
         any pending interrupts. */
      case 0xE000: {
         cpu_clear_interrupt(CPU_INTERRUPT_IRQ_MAPPER_PROXY);
         mmc3_disable_irqs = TRUE;

         ppu_repredict_interrupts(PPU_PREDICT_MMC_IRQ);

         break;
      }

      /* Writing any value to this register will enable MMC3 interrupts. */
      case 0xE001: {
         mmc3_disable_irqs = FALSE;
         ppu_repredict_interrupts(PPU_PREDICT_MMC_IRQ);

         break;
      }

      default:
         break;
   }
}

static void mmc3_reset(void)
{
   /* Reset address latches */
   mmc3_write(0x8000, 0x00);

   /* Enable SRAM */
   mmc3_write(0xA001, 0x80);

   /* Select last 16k page in upper 16k. */
   cpu_set_read_address_16k(0xC000, LAST_ROM_PAGE);

   /* Select first 16k page in lower 16k. */
   mmc3_prg_bank[0] = 0;
   mmc3_prg_bank[1] = 1;
   mmc3_cpu_bank_sort();

   /* Select first 8k CHR ROM page. */
   mmc3_chr_bank[0] = 0;
   mmc3_chr_bank[1] = 2;
   mmc3_chr_bank[2] = 4;
   mmc3_chr_bank[3] = 5;
   mmc3_chr_bank[4] = 6;
   mmc3_chr_bank[5] = 7;
   mmc3_ppu_bank_sort();

   mmc3_disable_irqs = TRUE;

   mmc3_irq_bank = 0;
   mmc3_irq_queued = FALSE;
}

static int mmc3_init(void)
{
   cpu_set_write_handler_32k(0x8000, mmc3_write);

   mmc3_check_vram_banking();
   mmc_check_vram_banking = mmc3_check_vram_banking;
   /* mmc_check_address_lines = mmc3_check_address_lines; */

   mmc3_reset();

   return 0;
}

static void mmc3_save_state(PACKFILE* file, const int version)
{
   RT_ASSERT(file);

   pack_putc(mmc3_register_8000, file);
   pack_putc(mmc3_sram_enable, file);

   pack_putc(mmc3_irq_counter, file);
   pack_putc(mmc3_irq_latch, file);
   pack_putc(BINARY(mmc3_disable_irqs), file);

   pack_putc(mmc3_irq_bank, file);
   pack_putc(BINARY(mmc3_irq_queued), file);

   pack_fwrite(mmc3_prg_bank, MMC3_PRG_BANKS, file);
   pack_fwrite(mmc3_chr_bank, MMC3_CHR_BANKS, file);
}

static void mmc3_load_state(PACKFILE* file, const int version)
{
   RT_ASSERT(file);

   /* Restore address latches */
   mmc3_write(0x8000, pack_getc(file));

   /* Restore SRAM status */
   mmc3_write(0xA001, pack_getc(file));

   /* Restore IRQ registers */
   mmc3_irq_counter = pack_getc(file);
   mmc3_irq_latch = pack_getc(file);
   mmc3_disable_irqs = BOOLEAN(pack_getc(file));

   mmc3_irq_bank = pack_getc(file);
   mmc3_irq_queued = BOOLEAN(pack_getc(file));

   /* Restore banking */
   pack_fread(mmc3_prg_bank, MMC3_PRG_BANKS, file);
   mmc3_cpu_bank_sort();
   pack_fread(mmc3_chr_bank, MMC3_CHR_BANKS, file);
   mmc3_ppu_bank_sort();
}

