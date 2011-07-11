/* Mapper #24 and #26 (Konami VRC6 and VRC6V). */
/* These mappers are fully supported. */
/* IRQ prediction is supported. */

#include "mmc/shared.h"

static int vrc6_init(void);
static int vrc6v_init(void);
static void vrc6_reset(void);
static void vrc6_save_state(PACKFILE*, const int);
static void vrc6_load_state(PACKFILE*, const int);

static const MMC mmc_vrc6 = {
   24,
   "Konami VRC6 + ExSound",
   vrc6_init, vrc6_reset,
   "VRC6\0\0\0\0",
   vrc6_save_state, vrc6_load_state,
   NULL, NULL, NULL, NULL
};

static const MMC mmc_vrc6v = {
   26,
   "Konami VRC6V + ExSound",
   vrc6v_init,vrc6_reset,
   "VRC6V\0\0\0",
   vrc6_save_state, vrc6_load_state,
   NULL, NULL, NULL, NULL
};

static const ENUM vrc6_mirroring_table[] = {
   PPU_MIRRORING_VERTICAL,
   PPU_MIRRORING_HORIZONTAL,
   PPU_MIRRORING_ONE_SCREEN_2000,
   PPU_MIRRORING_ONE_SCREEN_2400
};

static const UINT8 vrc6_mirroring_mask = 0x0C;

static BOOL vrc6_swap_address_pins = FALSE;

#define VRC6_PRG_BANKS 2
#define VRC6_CHR_BANKS 8
static UINT8 vrc6_prg_bank[VRC6_PRG_BANKS];
static UINT8 vrc6_chr_bank[VRC6_CHR_BANKS];

/* ~1.79MHz(~1.66MHz for PAL) clock derived directly from the CPU. */
static cpu_time_t vrc6_clock_counter = 0;
/* Buffer to hold unused clocks so we don't lose cycles. */
static cpu_time_t vrc6_clock_buffer = 0;

/* When in scanline mode ('M' bit clear), a prescaler divides the passing CPU cycles by 114, 114, then 113 (and repeats
   that order). This approximates 113+2/3 CPU cycles, which is one NTSC scanline */
static const UINT8 vrc6_irq_lut[3] = { 114, 114, 113 };
#define VRC6_IRQ_SEQUENCER_STEPS 3

static UINT8 vrc6_irq_timer = 0;
static UINT8 vrc6_irq_step = 0;

static UINT8 vrc6_irq_counter = 0x00;
static UINT8 vrc6_irq_latch = 0x00;
static UINT8 vrc6_irq_reg = 0x00;
static BOOL  vrc6_enable_irqs = FALSE;

/* IRQ prediction. */
static cpu_time_t vrc6_prediction_timestamp = 0;
static cpu_time_t vrc6_prediction_cycles = 0;

static void vrc6_update_irq_counter(const cpu_time_t cycles, const BOOL allow_irq)
{
   /* allow_irq setting:
         TRUE  - prediction mode, allows queue of IRQs
         FALSE - emulation mode, does not allow queueing of IRQs */

   cpu_time_t current;

   if(!vrc6_enable_irqs)
      return;

   for(current = 0; current < cycles; current++) {
      if(vrc6_irq_timer > 0) {
         vrc6_irq_timer--;
         if(vrc6_irq_timer > 0)
            continue;
      }

      if(vrc6_irq_reg & 0x04) {
         /* Cycle mode. */
         vrc6_irq_timer += 1;
      }
      else {
         /* Scanline mode. */
         vrc6_irq_timer += vrc6_irq_lut[vrc6_irq_step++];
         if(vrc6_irq_step > (VRC6_IRQ_SEQUENCER_STEPS - 1))
            vrc6_irq_step = 0;
      }

      if(vrc6_irq_counter == 0xFF) {
         vrc6_irq_counter = vrc6_irq_latch;

         if (allow_irq)
            cpu_queue_interrupt(CPU_INTERRUPT_IRQ_MAPPER,
                vrc6_prediction_timestamp + (current * CPU_CLOCK_MULTIPLIER));
      }
      else
         vrc6_irq_counter++;
   }
}

static void vrc6_process (void)
{
   /* Call this before accessing the state of the mapper - before reads,
      writes, and state-sensetive emulation. */

   const cpu_time_t elapsed_cycles = cpu_get_elapsed_cycles(&vrc6_clock_counter) + vrc6_clock_buffer;
   if(elapsed_cycles == 0)
      return;

   /* Scale from master clock to CPU and buffer the remainder to avoid possibly losing cycles. */
   const cpu_time_t elapsed_cpu_cycles = elapsed_cycles / CPU_CLOCK_DIVIDER;
   vrc6_clock_buffer += elapsed_cycles - (elapsed_cpu_cycles * CPU_CLOCK_DIVIDER);

   if(elapsed_cpu_cycles == 0)
      return;

   /* *Don't* allow IRQs here, or it'll conflict with prediction. */
   vrc6_update_irq_counter(elapsed_cpu_cycles, FALSE);
}

static void vrc6_predict_irq_slave(const cpu_time_t cycles)
{
   /* Slave function used by IRQ prediction. */

   UINT8 saved_irq_timer, saved_irq_step;
   UINT8 saved_irq_counter;

   /* Clear any pending IRQs, just in case. */
   cpu_unqueue_interrupt(CPU_INTERRUPT_IRQ_MAPPER);

   if (!vrc6_enable_irqs)
      return;

   /* Save vars since we're just simulating. */
   saved_irq_timer = vrc6_irq_timer;
   saved_irq_step = vrc6_irq_step;
   saved_irq_counter = vrc6_irq_counter;

   /* Remember to allow IRQs here. */
   /* Just go through the motions... */
   vrc6_update_irq_counter(cycles, TRUE);

   /* Restore saved vars. */
   vrc6_irq_timer = saved_irq_timer;
   vrc6_irq_step = saved_irq_step;
   vrc6_irq_counter = saved_irq_counter;
}

static void vrc6_predict_irq(const cpu_time_t cycles)
{
   /* Sync state. */
   vrc6_process();

   /* Save parameters for re-prediction if a mid-scanline change occurs. */
   vrc6_prediction_timestamp = cpu_get_cycles();
   /* We'll actually emulate for a little bit longer than requested, since it doesn't hurt to do so. */
   vrc6_prediction_cycles = cycles + PREDICTION_BUFFER_CYCLES + (1 * CPU_CLOCK_MULTIPLIER);

   const cpu_time_t cpu_cycles = vrc6_prediction_cycles /  CPU_CLOCK_DIVIDER;
   if(cpu_cycles == 0)
      return;

   vrc6_predict_irq_slave(cpu_cycles);
}

static void vrc6_repredict_irq(void)
{
   /* This needs to be called whenver a possible mid-scanline change to the
      IRQ parameters occurs (to update prediction). */

   cpu_time_t timestamp;
   cpu_time_t cycles_elapsed;
   cpu_rtime_t cycles_remaining;

   /* Sync state. */
   vrc6_process();

   timestamp = cpu_get_cycles();

   cycles_elapsed = (cpu_rtime_t)timestamp - (cpu_rtime_t)vrc6_prediction_timestamp;
   cycles_remaining = (cpu_rtime_t)vrc6_prediction_cycles - cycles_elapsed;
   if(cycles_remaining <= 0)
      return;

   const cpu_time_t cpu_cycles_remaining = cycles_remaining / CPU_CLOCK_DIVIDER;
   if(cpu_cycles_remaining == 0)
      return;

   vrc6_predict_irq_slave(cpu_cycles_remaining);
}

static void vrc6_update_prg_bank(const int bank)
{
   switch(bank) {
      case 0: {
         /* 16k ROM page select. */
         cpu_set_read_address_16k_rom_block(0x8000, vrc6_prg_bank[0]);
         break;
      }

      case 1: {
         /* 8k ROM page select. */
         cpu_set_read_address_8k_rom_block(0xC000, vrc6_prg_bank[1]);
         break;
      }

      default: {
         WARN_GENERIC();
         break;
      }
   }
}

static void vrc6_update_chr_bank(const int bank)
{
   if(ROM_CHR_ROM_PAGES <= 0)
      return;

   /* Set new VROM banking. */
   ppu_set_1k_pattern_table_vrom_page(bank << 10, vrc6_chr_bank[bank]);
}

static void vrc6_write(UINT16 address, UINT8 value)
{
   int major, minor;

   /* Sync state. */
   vrc6_process();

   /* Swap address pins. */
   if(vrc6_swap_address_pins)
      address = (address & 0xFFFC) | ((address >> 1) & 1) | ((address << 1) & 2);

   /* Extract command indexes. */
   major = address & 0xF000;
   minor = address & 0x000F;

   switch (major) {
      case 0x8000: {
         if(minor != 0)
            break;

         /* Set requested 16k ROM page at $8000. */
         vrc6_prg_bank[0] = value;
         vrc6_update_prg_bank(0);

         break;
      }

      case 0xB000: {
         if(minor != 3)
            break;

         /* Mirroring select. */

         /* Discard unused bits. */
         value = (value & vrc6_mirroring_mask) >> 2;

         /* Use value from LUT. */
         ppu_set_mirroring(vrc6_mirroring_table[value]);

         break;
      }

      case 0xC000: {
         if(minor != 0)
            break;

         /* Set requested 8k ROM page at $C000. */
         vrc6_prg_bank[1] = value;
         vrc6_update_prg_bank(1);

         break;
      }

      case 0xD000: {
         if(minor >= 4)
            break;

         /* Set requested 1k CHR-ROM page. */
         vrc6_chr_bank[minor] = value;
         vrc6_update_chr_bank(minor);

         break;
      }

      case 0xE000: {
         if(minor >= 4)
            break;

         /* Set requested 1k CHR-ROM page. */
         vrc6_chr_bank[minor + 4] = value;
         vrc6_update_chr_bank(minor + 4);

         break;
      }

      case 0xF000: {
         switch(minor) {
            case 0: {
               /* Set the IRQ counter load value. */
               vrc6_irq_latch = value;

               /* Update prediction. */
               vrc6_repredict_irq();

               break;
            }

            case 1: {
               /* Save the value for future writes to $F002. */
               vrc6_irq_reg = value;

               /* Enable or disable the IRQ counter. */
               vrc6_enable_irqs = TRUE_OR_FALSE(value & 0x02);

               /* If enabled... */
               if(vrc6_enable_irqs) {
                  /* Load the counter with the value from the $F000 latch. */
                  vrc6_irq_counter = vrc6_irq_latch;

                  /* Reset the timer and sequencer. */
                  vrc6_irq_timer = 0;
                  vrc6_irq_step = 0;
               }

               /* Reset IRQ status. */
               cpu_clear_interrupt(CPU_INTERRUPT_IRQ_MAPPER);

               /* Update prediction. */
               vrc6_repredict_irq();

               break;
            }

            case 2: {
               /* Swap IRQ counter enable flags. */
               vrc6_enable_irqs = TRUE_OR_FALSE(vrc6_irq_reg & 0x01);

               /* Reset IRQ status. */
               cpu_clear_interrupt(CPU_INTERRUPT_IRQ_MAPPER);

               /* Update prediction. */
               vrc6_repredict_irq();

               break;
            } 

            default:
               break;
         }

         break;
      }

      default:
         break;
   } 

   /* Send to ExSound. */
   apu_write(address, value);
}

static void vrc6_reset(void)
{
   /* Reset PRG banking. */

   /* Select first 16k page in lower 16k. */
   vrc6_prg_bank[0] = 0;
   vrc6_update_prg_bank(0);

   /* Select last 16k page in upper 16k. */
   cpu_set_read_address_16k(0xC000, LAST_ROM_PAGE);

   vrc6_prg_bank[1] = (ROM_PRG_ROM_PAGES * 2) - 2;
   vrc6_update_prg_bank(1);

   /* Reset internal clock. */
   vrc6_clock_counter = 0;
   vrc6_clock_buffer = 0;

   /* Reset IRQ variables. */
   vrc6_irq_timer  = 0;
   vrc6_irq_step = 0;

   vrc6_irq_counter = 0x00;
   vrc6_irq_latch = 0x00;
   vrc6_irq_reg = 0x00;
   vrc6_enable_irqs = FALSE;

   /* Reset IRQ prediction. */
   vrc6_prediction_timestamp = 0;
   vrc6_prediction_cycles = 0;

   /* Reset ExSound. */
   apu_reset_exsound(APU_EXSOUND_VRC6);
}

static int vrc6_base_init (void)
{
   /* Install write handler. */
   cpu_set_write_handler_32k(0x8000, vrc6_write);

   /* Install IRQ predicter. */
   mmc_predict_asynchronous_irqs = vrc6_predict_irq;

   /* Select ExSound chip. */
   apu_enable_exsound(APU_EXSOUND_VRC6);

   /* Set initial mappings and reset variables. */
   vrc6_reset();

   /* Return success. */
   return 0;
}

static int vrc6_init (void)
{
   /* Disable address pin swap. */
   vrc6_swap_address_pins = FALSE;

   return vrc6_base_init();
}

static int vrc6v_init (void)
{
   /* Pins A0 and A1 are swapped in VRC6V. */
   vrc6_swap_address_pins = TRUE;

   return vrc6_base_init();
}

static void vrc6_save_state(PACKFILE* file, const int version)
{
   RT_ASSERT(file);

   /* Sync state. */
   vrc6_process();

   /* Save banking. */
   pack_fwrite(vrc6_prg_bank, VRC6_PRG_BANKS, file);
   pack_fwrite(vrc6_chr_bank, VRC6_CHR_BANKS, file);

   /* Save internal clock. */
   pack_iputl(vrc6_clock_counter, file);
   pack_iputl(vrc6_clock_buffer, file);

   /* Save IRQ status. */
   pack_putc(vrc6_irq_timer, file);
   pack_putc(vrc6_irq_step, file);
   pack_putc(vrc6_irq_counter, file);
   pack_putc(vrc6_irq_latch, file);
   pack_putc(vrc6_irq_reg, file);        
   pack_putc(BINARY(vrc6_enable_irqs), file);

   /* Save IRQ prediction. */
   pack_iputl(vrc6_prediction_timestamp, file);
   pack_iputl(vrc6_prediction_cycles, file);
}

static void vrc6_load_state(PACKFILE* file, const int version)
{
   int index;

   RT_ASSERT(file);

   /* Restore banking. */
   pack_fread(vrc6_prg_bank, VRC6_PRG_BANKS, file);
   pack_fread(vrc6_chr_bank, VRC6_CHR_BANKS, file);

   for(index = 0; index < VRC6_PRG_BANKS; index++)
      vrc6_update_prg_bank(index);
   for(index = 0; index < VRC6_CHR_BANKS; index++)
      vrc6_update_chr_bank(index);

   /* Restore internal clock. */
   vrc6_clock_counter = pack_igetl(file);
   vrc6_clock_buffer = pack_igetl(file);

   /* Restore IRQ status. */
   vrc6_irq_timer = pack_getc(file);
   vrc6_irq_step = pack_getc(file);
   vrc6_irq_counter = pack_getc(file);
   vrc6_irq_latch = pack_getc(file);
   vrc6_irq_reg = pack_getc(file);
   vrc6_enable_irqs = BOOLEAN(pack_getc(file));

   /* Load IRQ prediction. */
   vrc6_prediction_timestamp = pack_igetl(file);
   vrc6_prediction_cycles = pack_igetl(file);
}
