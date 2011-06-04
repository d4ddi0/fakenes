/* Mapper #4 (MMC3). */
/* This mapper is fully supported. */

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

static UINT8 mmc3_irq_last_table = 0;

static UINT8 mmc3_register_8000;
static UINT8 mmc3_sram_enable;

#define MMC3_PRG_ADDRESS_BIT 0x40
#define MMC3_CHR_ADDRESS_BIT 0x80

static void mmc3_irq_slave(void)
{
   const UINT8 saved_irq_counter = mmc3_irq_counter;

   /* When the scanline counter is clocked, the value will first be checked. If it is zero,
      it will be reloaded from the IRQ latch ($C000); otherwise, it will decrement. If the
      old value in the counter is nonzero and new value is zero (whether from decrementing or
      reloading), an IRQ will be fired if IRQ generation is enabled (by writing to $E001). */

   if(mmc3_irq_counter == 0)
      mmc3_irq_counter = mmc3_irq_latch;
   else
      mmc3_irq_counter--;

   if(!mmc3_disable_irqs && 
      ((saved_irq_counter != 0) && (mmc3_irq_counter == 0)))
      cpu_interrupt(CPU_INTERRUPT_IRQ_MMC);
}

static void mmc3_check_latches(const UINT16 address)
{
   /* The MMC3 scanline counter is based entirely on PPU A12, triggered on rising edges
      (after the line remains low for a sufficiently long period of time). */

   if(address > 0x1FFF)
      /* We only care about accesses to pattern table data. */
      return;

   /* The counter will not work properly unless you use different pattern tables for
      background and sprite data. */
   const int table = address / PPU__BYTES_PER_PATTERN_TABLE;
   if((table == 1) && (mmc3_irq_last_table == 0))
      mmc3_irq_slave();

   mmc3_irq_last_table = table;
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

         /* Disabled for Star Tropics(MMC6). */
         /*
         mmc3_sram_enable = value & 0x80;
         if(mmc3_sram_enable)
            cpu_enable_sram();
         else
            cpu_disable_sram();
         */

         break;
      }

      /* This register specifies the IRQ counter reload value. When the IRQ counter is
         zero (or a reload is requested through $C001), this value will be copied into the
         MMC3 IRQ counter at the end of the current scanline. */
      case 0xC000:
         mmc3_irq_latch = value;
         break;

      /* Writing any value to this register clears the MMC3 IRQ counter so that it will be
         reloaded at the end of the current scanline. */
      case 0xC001:
         mmc3_irq_counter = 0;
         break;

      /* Writing any value to this register will disable MMC3 interrupts AND acknowledge
         any pending interrupts. */
      case 0xE000: {
         mmc3_disable_irqs = TRUE;
         cpu_clear_interrupt(CPU_INTERRUPT_IRQ_MMC);
         break;
      }

      /* Writing any value to this register will enable MMC3 interrupts. */
      case 0xE001:
         mmc3_disable_irqs = FALSE;
         break;

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

   mmc3_irq_last_table = 0;
}

static int mmc3_init(void)
{
   cpu_set_write_handler_32k(0x8000, mmc3_write);

   mmc_check_latches = mmc3_check_latches;

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

   pack_putc(mmc3_irq_last_table, file);

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

   mmc3_irq_last_table = pack_getc(file);

   /* Restore banking */
   pack_fread(mmc3_prg_bank, MMC3_PRG_BANKS, file);
   mmc3_cpu_bank_sort();
   pack_fread(mmc3_chr_bank, MMC3_CHR_BANKS, file);
   mmc3_ppu_bank_sort();
}

