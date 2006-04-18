#ifndef APU_SHARED_H_INCLUDED
#define APU_SHARED_H_INCLUDED
#include <math.h>

#define APU_NES_BASECYCLES 21477270

#define APU_LOG_BITS       12
#define APU_LIN_BITS       6
#define APU_LOG_LIN_BITS   30

static UINT32 apu_lineartbl[(1 << APU_LIN_BITS) + 1];
static UINT32 apu_logtbl[1 << APU_LOG_BITS];

UINT32 APU_LinearToLog(INT32 l)
{
   return (l < 0) ? (apu_lineartbl[-l] + 1) : apu_lineartbl[l];
}

INT32 APU_LogToLinear(UINT32 l, UINT32 sft)
{
    INT32 ret;
    UINT32 ofs;
   sft += (l >> 1) >> APU_LOG_BITS;
   if (sft >= APU_LOG_LIN_BITS) return 0;
   ofs = (l >> 1) & ((1 << APU_LOG_BITS) - 1);
   ret = apu_logtbl[ofs] >> sft;
	return (l & 1) ? -ret : ret;
}

void APU_LogTableInitialize(void)
{
    static INT32 initialized = 0;
    INT32 i;
	double a;
	if (initialized) return;
	initialized = 1;
   for (i = 0; i < (1 << APU_LOG_BITS); i++)
	{
      a = (1 << APU_LOG_LIN_BITS) / pow(2, i / (double)(1 << APU_LOG_BITS));
        apu_logtbl[i] = (UINT32)a;
	}
   apu_lineartbl[0] = APU_LOG_LIN_BITS << APU_LOG_BITS;
   for (i = 1; i < (1 << APU_LIN_BITS) + 1; i++)
	{
        UINT32 ua;
      a = i << (APU_LOG_LIN_BITS - APU_LIN_BITS);
        ua = (UINT32)((APU_LOG_LIN_BITS - (log(a) / log(2))) * (1 << APU_LOG_BITS));
      apu_lineartbl[i] = ua << 1;
	}
}

static UINT32 APU_DivFix(UINT32 p1, UINT32 p2, UINT32 fix)
{
    UINT32 ret;
   /* Fix for possible arithmetic exception on absolute 0. */
   p1 = MAX(p1, 1);
   p2 = MAX(p2, 1);
   ret = p1 / p2;
	p1  = p1 % p2;/* p1 = p1 - p2 * ret; */
	while (fix--)
	{
		p1 += p1;
		ret += ret;
		if (p1 >= p2)
		{
			p1 -= p2;
			ret++;
		}
	}
	return ret;
}

#endif   /* !APU_SHARED_H_INCLUDED */
