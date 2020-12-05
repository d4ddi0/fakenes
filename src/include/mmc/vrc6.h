/* Mapper #24 and #26 (Konami VRC6 and VRC6V). */
/* These mappers are fully supported. */

#include "mmc/shared.h"

static int vrc6_init (void);
static void vrc6_reset (void);
static void vrc6_save_state (PACKFILE *, int);
static void vrc6_load_state (PACKFILE *, int);

static const MMC mmc_vrc6 =
{
   24, "Konami VRC6 + ExSound",
   vrc6_init, vrc6_reset,
   "VRC6\0\0\0\0",
   vrc6_save_state, vrc6_load_state
};

static int vrc6v_init (void);

static const MMC mmc_vrc6v =
{
   26, "Konami VRC6V + ExSound",
   vrc6v_init, vrc6_reset,
   "VRC6V\0\0\0",
   vrc6_save_state, vrc6_load_state
};

static const ENUM vrc6_mirroring_table[] =
{
   MIRRORING_VERTICAL,
   MIRRORING_HORIZONTAL,
   MIRRORING_ONE_SCREEN_2000,
   MIRRORING_ONE_SCREEN_2400
};

static const UINT8 vrc6_mirroring_mask = 0x0c;

static BOOL vrc6_enable_irqs = FALSE;
static UINT8 vrc6_irq_counter = 0;
static UINT8 vrc6_irq_latch = 0;

static UINT8 vrc6_prg_bank[2];
static UINT8 vrc6_chr_bank[8];

static int vrc6_irq_tick (int line)
{
   if (vrc6_enable_irqs & 0x02)
   {
      if (vrc6_irq_counter == 0xff)
      {
         vrc6_irq_counter = vrc6_irq_latch;
         return (CPU_INTERRUPT_IRQ);
      }
      else
      {
         vrc6_irq_counter++;
      }
   }

   return (CPU_INTERRUPT_NONE);
}

static BOOL vrc6_swap_address_pins = FALSE;

static void vrc6_update_prg_bank (int bank)
{
   switch (bank)
   {
      case 0:
      {
         /* 16k ROM page select. */
         cpu_set_read_address_16k_rom_block (0x8000, vrc6_prg_bank[0]);

         break;
      }

      case 1:
      {
         /* 8k ROM page select. */
         cpu_set_read_address_8k_rom_block (0xC000, vrc6_prg_bank[1]);

         break;
      }

      default:
         WARN_GENERIC();
   }
}

static void vrc6_update_chr_bank (int bank)
{
   if (ROM_CHR_ROM_PAGES <= 0)
      return;

   /* Set new VROM banking. */
   ppu_set_ram_1k_pattern_vrom_block ((bank << 10), vrc6_chr_bank[bank]);
}

static void vrc6_write (UINT16 address, UINT8 value)
{
   int major, minor;

   /* Swap address pins. */
   if (vrc6_swap_address_pins)
   {
      address = ((address & 0xfffc) | ((address >> 1) & 1) | ((address << 1)
         & 2));
   }

   /* Extract command indexes. */
   major = (address & 0xf000);
   minor = (address & 0x000f);

   switch (major)
   {
      case 0x8000:
      {
         if (minor != 0x0000)
            break;

         /* Set requested 16k ROM page at $8000. */

         vrc6_prg_bank[0] = value;
         vrc6_update_prg_bank (0);

         break;
      }

      case 0xb000:
      {
         /* Mirroring select. */

         if (minor != 0x0003)
            break;

         /* Discard unused bits. */
         value = ((value & vrc6_mirroring_mask) >> 2);

         /* Use value from LUT. */
         ppu_set_mirroring (vrc6_mirroring_table [value]);

         break;
      }

      case 0xc000:
      {
         if (minor != 0x0000)
            break;

         /* Set requested 8k ROM page at $C000. */

         vrc6_prg_bank[1] = value;
         vrc6_update_prg_bank (1);

         break;
      }

      case 0xd000:
      {
         if (minor >= 0x0004)
            break;

         /* Set requested 1k CHR-ROM page. */

         vrc6_chr_bank[minor] = value;
         vrc6_update_chr_bank (minor);

         break;
      }

      case 0xe000:
      {
         if (minor >= 0x0004)
            break;

         /* Set requested 1k CHR-ROM page. */
        
         vrc6_chr_bank[(minor + 4)] = value;
         vrc6_update_chr_bank ((minor + 4));

         break;
      }

      case 0xf000:
      {
         switch (minor)
         {
            case 0x0000:
            {
               /* Both (?) bytes of IRQ counter. */
               vrc6_irq_latch = value;

               break;
            }

            case 0x0001:
            {
               /* Enable/disable IRQs. */

               vrc6_enable_irqs = (value & 0x03);
   
               if (vrc6_enable_irqs & 0x02)
                   vrc6_irq_counter = vrc6_irq_latch;

               break;
            }

            case 0x0002:
            {
               /* ?? */
   
               if (vrc6_enable_irqs & 0x01)
                  vrc6_enable_irqs |= 0x02;
               else
                  vrc6_enable_irqs &= 0x01;
            }

            default:
               break;
         }

         break;
      }

      default:
         break;
   } 

   /* Write ExSound. */
   apu_ex_write (address, value);
}

static void vrc6_reset (void)
{
   /* Reset PRG banking. */

   /* Select first 16k page in lower 16k. */
   vrc6_prg_bank[0] = 0;
   vrc6_update_prg_bank (0);

   /* Select last 16k page in upper 16k. */
   cpu_set_read_address_16k (0xc000, LAST_ROM_PAGE);

   vrc6_prg_bank[1] = ((ROM_PRG_ROM_PAGES * 2) - 2);
   vrc6_update_prg_bank (1);
}

static int vrc6_base_init (void)
{
   int index;

   /* Set initial mappings. */
   vrc6_reset ();

   /* Install write handler. */
   cpu_set_write_handler_32k (0x8000, vrc6_write);

   /* Install IRQ tick handler. */
   mmc_scanline_end = vrc6_irq_tick;

   /* Select ExSound chip. */
   apu_set_exsound (APU_EXSOUND_VRC6);

   /* Return success. */
   return (0);
}

static int vrc6_init (void)
{
   /* Disable address pin swap. */
   vrc6_swap_address_pins = FALSE;

   return (vrc6_base_init ());
}

static int vrc6v_init (void)
{
   /* Pins A0 and A1 are swapped in VRC6V. */
   vrc6_swap_address_pins = TRUE;

   return (vrc6_base_init ());
}

static void vrc6_save_state (PACKFILE *file, int version)
{
   PACKFILE *chunk;

   RT_ASSERT(file);

   /* Open chunk. */
   chunk = pack_fopen_chunk (file, FALSE);
   if (!chunk)
      WARN_BREAK_GENERIC();

   /* Save data. */

   /* Save IRQ registers. */
   pack_putc (vrc6_irq_counter, chunk);
   pack_putc (vrc6_irq_latch,   chunk);
   pack_putc (vrc6_enable_irqs, chunk);

   /* Save banking. */
   pack_fwrite (vrc6_prg_bank, 2, chunk);
   pack_fwrite (vrc6_chr_bank, 8, chunk);

   /* Close chunk. */
   pack_fclose_chunk (chunk);
}

static void vrc6_load_state (PACKFILE *file, int version)
{
   PACKFILE *chunk;
   int index;

   RT_ASSERT(file);

   /* Open chunk. */
   chunk = pack_fopen_chunk (file, FALSE);
   if (!chunk)
      WARN_BREAK_GENERIC();

   /* Load data. */

   /* Restore IRQ registers */
   vrc6_irq_counter = pack_getc (chunk);
   vrc6_irq_latch   = pack_getc (chunk);
   vrc6_enable_irqs = pack_getc (chunk);

   /* Restore banking */
   pack_fread (vrc6_prg_bank, 2, chunk);
   pack_fread (vrc6_chr_bank, 8, chunk);

   vrc6_update_prg_bank (0);
   vrc6_update_prg_bank (1);

   for (index = 0; index < 8; index++)
      vrc6_update_chr_bank (index);

   /* Close chunk. */
   pack_fclose_chunk (chunk);
}
