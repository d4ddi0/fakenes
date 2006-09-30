#include "apu/shared.hpp"

/*
** Konami VRC6 ExSound by TAKEDA, toshiya
**
** original: s_vrc6.c in nezp0922
*/

/* Heavily modified for FakeNES by randilyn. */

/* --- Private functions. --- */

static INLINE void apu_vrc6s_square (apu_vrc6s_chan_t &chan)
{
   UINT32 output;
   INT32 output2;

   if (chan.update)
	{
      if (chan.update & (2 | 4))
		{
         chan.spd = (((chan.regs[2] & 0x0F) << 8) + chan.regs[1] + 1) << 18;
		}
      chan.update = 0;
	}

   if (!chan.spd)
   {
      chan.linear_output = 0;
      return;
   }

   chan.cycles -= chan.cps;
   while (chan.cycles < 0)
	{
      /* MAX() kludge here to fix a possible lock-up in some games. */
      chan.cycles += MAX(chan.spd, 1);
      chan.adr++;
	}
   chan.adr &= 0xF;

   if (!(chan.regs[2] & 0x80))
   {
      chan.linear_output = 0;
      return;
   }

   output = APU_LinearToLog(chan.regs[0] & 0x0F);
   if (!(chan.regs[0] & 0x80) && (chan.adr < ((chan.regs[0] >> 4) + 1)))
	{
      /* and array gate */
      chan.linear_output = 0;
      return;
	}

   output2 = APU_LogToLinear(output, APU_LOG_LIN_BITS - APU_LIN_BITS - 16 - 1);

   chan.linear_output = APU_TO_OUTPUT_24(output2);
}

static INLINE void apu_vrc6s_saw (apu_vrc6s_chan_t &chan)
{
   UINT32 output;
   INT32 output2;

   if (chan.update)
	{
      if (chan.update & (2 | 4))
		{
         chan.spd = (((chan.regs[2] & 0x0F) << 8) + chan.regs[1] + 1) << 18;
		}
      chan.update = 0;
	}

   if (!chan.spd)
   {
      chan.linear_output = 0;
      return;
   }

   chan.cycles -= chan.cps;
   while (chan.cycles < 0)
	{
      /* MAX() kludge here to fix a possible lock-up in some games. */
      chan.cycles += MAX(chan.spd, 1);
      chan.output += (chan.regs[0] & 0x3F);
      if (7 == ++chan.adr)
		{
         chan.adr = 0;
         chan.output = 0;
		}
	}

   if (!(chan.regs[2] & 0x80))
   {
      chan.linear_output = 0;
      return;
   }

   output = APU_LinearToLog((chan.output >> 3) & 0x1F);

   output2 = APU_LogToLinear(output, APU_LOG_LIN_BITS - APU_LIN_BITS - 16 - 1);

   chan.linear_output = APU_TO_OUTPUT_24(output2);
}

static INLINE void apu_vrc6s_update_square (apu_vrc6s_chan_t &chan)
{
   chan.cps = APU_DivFix((UINT32)ROUND(apu.base_frequency), (UINT32)ROUND(12 * apu.mixer.frequency), 18);
}

static INLINE void apu_vrc6s_update_saw (apu_vrc6s_chan_t &chan)
{
   chan.cps = APU_DivFix((UINT32)ROUND(apu.base_frequency), (UINT32)ROUND(24 * apu.mixer.frequency), 18);
}

/* --- Public functions. --- */

static void apu_vrc6s_write (UINT16, UINT8);

static void apu_vrc6s_reset (void)
{
   UINT16 address;

   /* Clear registers. */
   for (address = 0x9000; address < 0x9002; address++)
      apu_vrc6s_write (address, 0);

   for (address = 0xa000; address < 0xa002; address++)
      apu_vrc6s_write (address, 0);

   for (address = 0xb000; address < 0xb002; address++)
      apu_vrc6s_write (address, 0);
}

static void apu_vrc6s_update (void)
{
   apu_vrc6s_update_square (apu.vrc6s.square[0]);
   apu_vrc6s_update_square (apu.vrc6s.square[1]);
   apu_vrc6s_update_saw    (apu.vrc6s.saw);
}

static void apu_vrc6s_process (ENUM channel)
{
   switch (channel)
   {
      case APU_CHANNEL_EXTRA_1:
      {
         apu_vrc6s_square (apu.vrc6s.square[0]);
         break;
      }

      case APU_CHANNEL_EXTRA_2:
      {
         apu_vrc6s_square (apu.vrc6s.square[1]);
         break;
      }

      case APU_CHANNEL_EXTRA_3:
      {
         apu_vrc6s_saw (apu.vrc6s.saw);
         break;
      }

      default:
         WARN_GENERIC();
   }
}

static REAL apu_vrc6s_mix (void)
{
   REAL total = 0.0;

   total += apu.vrc6s.square[0].linear_output;
   total += apu.vrc6s.square[1].linear_output;
   total += apu.vrc6s.saw.linear_output;

   return (total);
}

static void apu_vrc6s_write (UINT16 address, UINT8 value)
{
   if ((address & 3) > 2)
   {
      /* Avoid register overflow. */
      return;
   }

   switch ((address & 0xf000))
   {
		case 0x9000:
      {
         apu.vrc6s.square[0].regs[address & 3] = value;
         apu.vrc6s.square[0].update |= 1 << (address & 3);

			break;
      }

      case 0xa000:
      {
         apu.vrc6s.square[1].regs[address & 3] = value;
         apu.vrc6s.square[1].update |= 1 << (address & 3); 

         break;
      }

      case 0xb000:
      {
         apu.vrc6s.saw.regs[address & 3] = value;
         apu.vrc6s.saw.update |= 1 << (address & 3);

			break;
      }

      default:
         break;
	}
}

static void apu_vrc6s_save_state (PACKFILE *file, int version)
{
   int index;
   apu_vrc6s_chan_t *chan;

   /* Square waves. */

   for (index = 0; index < 2; index++)
   {
      int subindex;

      chan = &apu.vrc6s.square[index];

      pack_iputl (chan->cps,    file);
      pack_iputl (chan->cycles, file);
      pack_iputl (chan->spd,    file);

      for (subindex = 0; subindex < 3; subindex++)
         pack_putc (chan->regs[subindex], file);

      pack_putc (chan->update, file);
      pack_putc (chan->adr,    file);
   }

   /* Saw wave. */

   chan = &apu.vrc6s.saw;

   pack_iputl (chan->cps,    file);
   pack_iputl (chan->cycles, file);
   pack_iputl (chan->spd,    file);

   for (index = 0; index < 3; index++)
      pack_putc (chan->regs[index], file);

   pack_putc  (chan->update, file);
   pack_putc  (chan->adr,    file);
   pack_iputl (chan->output, file);
}

static void apu_vrc6s_load_state (PACKFILE *file, int version)
{
   int index;
   apu_vrc6s_chan_t *chan;

   /* Square waves. */

   for (index = 0; index < 2; index++)
   {
      int subindex;

      chan = &apu.vrc6s.square[index];

      chan->cps    = pack_igetl (file);
      chan->cycles = pack_igetl (file);
      chan->spd    = pack_igetl (file);

      for (subindex = 0; subindex < 3; subindex++)
         chan->regs[subindex] = pack_getc (file);

      chan->update = pack_getc (file);
      chan->adr    = pack_getc (file);
   }

   /* Saw wave. */

   chan = &apu.vrc6s.saw;

   chan->cps    = pack_igetl (file);
   chan->cycles = pack_igetl (file);
   chan->spd    = pack_igetl (file);

   for (index = 0; index < 3; index++)
      chan->regs[index] = pack_getc (file);

   chan->update = pack_getc  (file);
   chan->adr    = pack_getc  (file);
   chan->output = pack_igetl (file);
}

static const APU_EXSOUND apu_vrc6s =
{
   (const UINT8 *)"VRC6S\0\0\0",
   apu_vrc6s_reset,
   apu_vrc6s_update,
   apu_vrc6s_process,
   apu_vrc6s_mix,
   apu_vrc6s_write,
   apu_vrc6s_save_state,
   apu_vrc6s_load_state
};
