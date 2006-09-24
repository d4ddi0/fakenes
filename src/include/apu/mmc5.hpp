#include "apu/shared.hpp"

/*
** Nintendo MMC3 ExSound by TAKEDA, toshiya
**
** original: s_apu.c in nezp0922
*/

/* Heavily modified for FakeNES by randilyn. */

/* --- Private functions. --- */

#define V(x) (x*64/60)
static UINT32 apu_mmc5s_vbl_length[32] =
{
	V(0x05), V(0x7F), V(0x0A), V(0x01), V(0x14), V(0x02), V(0x28), V(0x03),
	V(0x50), V(0x04), V(0x1E), V(0x05), V(0x07), V(0x06), V(0x0E), V(0x07),
	V(0x06), V(0x08), V(0x0C), V(0x09), V(0x18), V(0x0A), V(0x30), V(0x0B),
	V(0x60), V(0x0C), V(0x24), V(0x0D), V(0x08), V(0x0E), V(0x10), V(0x0F),
};
#undef V

#define V(x) ((x) << 19)
static const UINT32 apu_mmc5s_spd_limit[8] =
{
	V(0x3FF), V(0x555), V(0x666), V(0x71C), 
	V(0x787), V(0x7C1), V(0x7E0), V(0x7F0),
};
#undef V

static INLINE REAL apu_mmc5s_square (apu_mmc5s_chan_t &chan)
{
   UINT32 output;
   INT32 output2;

   if (chan.update)
	{
      if (chan.update & 1)
		{
         chan.duty = (chan.regs[0] >> 4) & 0x0C;
         if (chan.duty == 0) chan.duty = 2;
         chan.envspd = ((chan.regs[0] & 0x0F) + 1) << (19 + 7);
		}
      if (chan.update & 2)
		{
         chan.sweepspd = (((chan.regs[1] >> 4) & 0x07) + 1) << (19 + 8);
		}
      if (chan.update & (4 | 8))
		{
         chan.spd = (((chan.regs[3] & 7) << 8) + chan.regs[2] + 1) << 19;
		}
      if ((chan.update & 8) && (chan.key & 1))
		{
         chan.key &= ~2;
         chan.length = (apu_mmc5s_vbl_length[chan.regs[3] >> 3] * chan.freq) >> 6;
         chan.envadr = 0;
		}
      chan.update = 0;
	}

   if (chan.key == 0) return 0;

   chan.envphase -= chan.cps >> (13 - 7);
   if (chan.regs[0] & 0x20)
	{
      while (chan.envphase < 0)
		{
         /* MAX() kludge here to fix a possible lock-up in some games. */
         chan.envphase += MAX(chan.envspd, 1);
         chan.envadr++;
		}
      chan.envadr &= 0x0F;
	}
	else
	{
      while (chan.envphase < 0)
		{
         /* MAX() kludge here to fix a possible lock-up in some games. */
         chan.envphase += MAX(chan.envspd, 1);
         chan.envadr += (chan.envadr < 15);
		}
	}

   if (chan.length)
	{
      if (!(chan.regs[0] & 0x20)) chan.length--;
	}
	else
	{
      chan.key |= 2;
	}

   if ((chan.regs[1] & 0x80))
	{
      if (chan.regs[1] & 7)
		{
         chan.sweepphase -= chan.cps >> (14 - 8);
         if (chan.regs[1] & 8)
            while (chan.sweepphase < 0)
				{
               /* MAX() kludge here to fix a possible lock-up in some
                  games. */
               chan.sweepphase += MAX(chan.sweepspd, 1);
               chan.spd -= chan.spd >> (chan.regs[1] & 7);
				}
			else
            while (chan.sweepphase < 0)
				{
               /* MAX() kludge here to fix a possible lock-up in some
                  games. */
               chan.sweepphase += MAX(chan.sweepspd, 1);
               chan.spd += chan.spd >> (chan.regs[1] & 7);
				}
		}
	}

   if (chan.spd < (4 << 19)) return 0;
   if (!(chan.regs[1] & 8))
	{
      if (chan.spd > apu_mmc5s_spd_limit[chan.regs[1] & 7]) return 0;
	}

   chan.cycles -= chan.cps;
   while (chan.cycles < 0)
	{
      /* MAX() kludge here to fix a possible lock-up in some games. */
      chan.cycles += MAX(chan.spd, 1);
      chan.adr++;
	}
   chan.adr &= 0x0F;

   if (chan.key & 2)
	{
      if (chan.release < (31 << (APU_LOG_BITS + 1)))
         chan.release += 3 << (APU_LOG_BITS - 8 + 1);
	}
	else
	{
      chan.release = 0;
	}

   if (chan.regs[0] & 0x10) /* fixed volume */
      output = chan.regs[0] & 0x0F;
	else
      output = 15 - chan.envadr;

   output = APU_LinearToLog(output) + chan.release;
   output += (chan.adr < chan.duty);

   output2 = APU_LogToLinear(output, APU_LOG_LIN_BITS - APU_LIN_BITS - 16);

   return (APU_TO_OUTPUT_24(output2));
}

static INLINE REAL apu_mmc5s_da (apu_mmc5s_chan_t &chan)
{
   INT32 output;

   if (!chan.key)
      return (0);

   output = chan.output;
   output <= 8;   /* upshift to 16-bit. */
      
   return (APU_TO_OUTPUT(output));
}

static INLINE void apu_mmc5s_update_square (apu_mmc5s_chan_t &chan)
{
   chan.freq = (UINT32)ROUND(apu.mixer.mixing_frequency);
   chan.cps = APU_DivFix(APU_NES_BASECYCLES, 12 * chan.freq, 19);
}

/* --- Public functions. --- */

static void apu_mmc5s_write (UINT16, UINT8);

static void apu_mmc5s_reset (void)
{
   UINT16 address;

   /* Clear registers. */

   for (address = 0x5000; address < 0x5015; address++)
      apu_mmc5s_write (address, 0);
}

static void apu_mmc5s_update (void)
{
   apu_mmc5s_update_square (apu.mmc5s.square[0]);
   apu_mmc5s_update_square (apu.mmc5s.square[1]);
}

static REAL apu_mmc5s_process (ENUM channel)
{
   switch (channel)
   {
      case APU_CHANNEL_EXTRA_1:
         return (apu_mmc5s_square (apu.mmc5s.square[0]));

      case APU_CHANNEL_EXTRA_2:
         return (apu_mmc5s_square (apu.mmc5s.square[1]));

      case APU_CHANNEL_EXTRA_3:
         return (apu_mmc5s_da (apu.mmc5s.da));

      default:
         return (0);
   }
}

static void apu_mmc5s_write (UINT16 address, UINT8 value)
{
	if (0x5000 <= address && address <= 0x5015)
	{
		switch (address)
		{
			case 0x5000: case 0x5002: case 0x5003:
			case 0x5004: case 0x5006: case 0x5007:
				{
					int ch = address >= 0x5004;
					int port = address & 3;
               apu.mmc5s.square[ch].regs[port] = value;
               apu.mmc5s.square[ch].update |= 1 << port; 
				}
				break;
			case 0x5011:
                apu.mmc5s.da.output = ((signed int)(value & 0xff)) - 0x80;
				break;
			case 0x5010:
            apu.mmc5s.da.key = (value & 0x01);
				break;
			case 0x5015:
				if (value & 1)
               apu.mmc5s.square[0].key = 1;
				else
				{
               apu.mmc5s.square[0].key = 0;
               apu.mmc5s.square[0].length = 0;
				}
				if (value & 2)
               apu.mmc5s.square[1].key = 1;
				else
				{
               apu.mmc5s.square[1].key = 0;
               apu.mmc5s.square[1].length = 0;
				}
				break;
         default:
            break;
		}
	}
}

static void apu_mmc5s_save_state (PACKFILE *file, int version)
{
   int index;
   apu_mmc5s_chan_t *chan;

   /* Square waves. */              

   for (index = 0; index < 2; index++)
   {
      int subindex;

      chan = &apu.mmc5s.square[index];

      pack_iputl (chan->cps,        file);
      pack_iputl (chan->cycles,     file);
      pack_iputl (chan->sweepphase, file);
      pack_iputl (chan->envphase,   file);
      pack_iputl (chan->spd,        file);
      pack_iputl (chan->envspd,     file);
      pack_iputl (chan->sweepspd,   file);
      pack_iputl (chan->length,     file);
      pack_iputl (chan->freq,       file);
      pack_iputl (chan->release,    file);

      for (subindex = 0; subindex < 4; subindex++)
         pack_putc (chan->regs[subindex], file);

      pack_putc (chan->update, file);
      pack_putc (chan->key,    file);
      pack_putc (chan->adr,    file);
      pack_putc (chan->envadr, file);
      pack_putc (chan->duty,   file);
   }

   /* Digital audio. */

   chan = &apu.mmc5s.da;

   pack_putc  (chan->key,    file);
   pack_iputl (chan->output, file);
}

static void apu_mmc5s_load_state (PACKFILE *file, int version)
{
   int index;
   apu_mmc5s_chan_t *chan;

   /* Square waves. */              

   for (index = 0; index < 2; index++)
   {
      int subindex;

      chan = &apu.mmc5s.square[index];

      chan->cps        = pack_igetl (file);
      chan->cycles     = pack_igetl (file);
      chan->sweepphase = pack_igetl (file);
      chan->envphase   = pack_igetl (file);
      chan->spd        = pack_igetl (file);
      chan->envspd     = pack_igetl (file);
      chan->sweepspd   = pack_igetl (file);
      chan->length     = pack_igetl (file);
      chan->freq       = pack_igetl (file);
      chan->release    = pack_igetl (file);

      for (subindex = 0; subindex < 4; subindex++)
         chan->regs[subindex] = pack_getc (file);

      chan->update = pack_getc (file);
      chan->key    = pack_getc (file);
      chan->adr    = pack_getc (file);
      chan->envadr = pack_getc (file);
      chan->duty   = pack_getc (file);
   }

   /* Digital audio. */

   chan = &apu.mmc5s.da;

   chan->key    = pack_getc (file);
   chan->output = pack_igetl (file);
}

static const APU_EXSOUND apu_mmc5s =
{
   (const UINT8 *)"MMC5S\0\0\0",
   apu_mmc5s_reset,
   apu_mmc5s_update,
   apu_mmc5s_process,
   apu_mmc5s_write,
   apu_mmc5s_save_state, apu_mmc5s_load_state
};
