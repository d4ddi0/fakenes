#include "apu/shared.h"

/*
** Konami VRC6 ExSound by TAKEDA, toshiya
**
** original: s_vrc6.c in nezp0922
*/

static INT32 APU_VRC6SoundSquareRender(APU_VRC6_SQUARE *ch)
{
    UINT32 output;

   RT_ASSERT(ch);

	if (ch->update)
	{
		if (ch->update & (2 | 4))
		{
			ch->spd = (((ch->regs[2] & 0x0F) << 8) + ch->regs[1] + 1) << 18;
		}
		ch->update = 0;
	}

	if (!ch->spd) return 0;

	ch->cycles -= ch->cps;
	while (ch->cycles < 0)
	{
		ch->cycles += ch->spd;
		ch->adr++;
	}
	ch->adr &= 0xF;

	if (ch->mute || !(ch->regs[2] & 0x80)) return 0;

   output = APU_LinearToLog(ch->regs[0] & 0x0F) + apu.vrc6s.mastervolume;
	if (!(ch->regs[0] & 0x80) && (ch->adr < ((ch->regs[0] >> 4) + 1)))
	{
#if 1
		return 0;	/* and array gate */
#else
		output++;	/* negative gate */
#endif
	}
   return APU_LogToLinear(output, APU_LOG_LIN_BITS - APU_LIN_BITS - 16 - 1);
}

static INT32 APU_VRC6SoundSawRender(APU_VRC6_SAW *ch)
{
    UINT32 output;

   RT_ASSERT(ch);

	if (ch->update)
	{
		if (ch->update & (2 | 4))
		{
			ch->spd = (((ch->regs[2] & 0x0F) << 8) + ch->regs[1] + 1) << 18;
		}
		ch->update = 0;
	}

	if (!ch->spd) return 0;

	ch->cycles -= ch->cps;
	while (ch->cycles < 0)
	{
		ch->cycles += ch->spd;
		ch->output += (ch->regs[0] & 0x3F);
		if (7 == ++ch->adr)
		{
			ch->adr = 0;
			ch->output = 0;
		}
	}

	if (ch->mute || !(ch->regs[2] & 0x80)) return 0;

   output = APU_LinearToLog((ch->output >> 3) & 0x1F) + apu.vrc6s.mastervolume;
   return APU_LogToLinear(output, APU_LOG_LIN_BITS - APU_LIN_BITS - 16 - 1);
}

static INT32 APU_VRC6SoundRender(void)
{
    INT32 accum = 0;
   /* output signed 16-bit */
   accum += APU_VRC6SoundSquareRender(&apu.vrc6s.square[0]) >> 8;
   accum += APU_VRC6SoundSquareRender(&apu.vrc6s.square[1]) >> 8;
   accum += APU_VRC6SoundSawRender(&apu.vrc6s.saw) >> 8;
	return accum;
}

static void APU_VRC6SoundVolume(UINT32 volume)
{
   apu.vrc6s.mastervolume = (volume << (APU_LOG_BITS - 8)) << 1;
}

static void APU_VRC6SoundWrite9000(UINT16 address, UINT8 value)
{
   apu.vrc6s.square[0].regs[address & 3] = value;
   apu.vrc6s.square[0].update |= 1 << (address & 3); 
}
static void APU_VRC6SoundWriteA000(UINT16 address, UINT8 value)
{
   apu.vrc6s.square[1].regs[address & 3] = value;
   apu.vrc6s.square[1].update |= 1 << (address & 3); 
}
static void APU_VRC6SoundWriteB000(UINT16 address, UINT8 value)
{
   apu.vrc6s.saw.regs[address & 3] = value;
   apu.vrc6s.saw.update |= 1 << (address & 3); 
}

static void APU_VRC6SoundWrite(UINT16 address, UINT8 value)
{
	switch(address & 0xF000) {
		case 0x9000:
         APU_VRC6SoundWrite9000(address, value);
			break;
		case 0xA000:
         APU_VRC6SoundWriteA000(address, value);
			break;
		case 0xB000:
         APU_VRC6SoundWriteB000(address, value);
			break;
	}
}

static void APU_VRC6SoundSquareReset(APU_VRC6_SQUARE *ch)
{
   RT_ASSERT(ch);

   ch->cps = APU_DivFix(APU_NES_BASECYCLES, 12 * APU_SAMPLE_RATE, 18);
}

static void APU_VRC6SoundSawReset(APU_VRC6_SAW *ch)
{
   RT_ASSERT(ch);

   ch->cps = APU_DivFix(APU_NES_BASECYCLES, 24 * APU_SAMPLE_RATE, 18);
}

static void APU_VRC6SoundReset(void)
{
   memset(&apu.vrc6s, 0, sizeof(APU_VRC6SOUND));
   APU_VRC6SoundSquareReset(&apu.vrc6s.square[0]);
   APU_VRC6SoundSquareReset(&apu.vrc6s.square[1]);
   APU_VRC6SoundSawReset(&apu.vrc6s.saw);
}

