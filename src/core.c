/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                         M6502.c                         **/
/**                                                         **/
/** This file contains implementation for 6502 CPU. Don't   **/
/** forget to provide Rd6502(), Wr6502(), Loop6502(), and   **/
/** possibly Op6502() functions to accomodate the emulated  **/
/** machine's architecture.                                 **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996                      **/
/**               Alex Krasivsky  1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/   
/**     changes to this file.                               **/
/*************************************************************/
/* 16.July     2002 stainless Added FAST_ZP.                 */
/* 08.July     2002 TRAC      Eliminated unused Run6502      */
/*                            return value.                  */
/* 13.June     2002 TRAC      Altered M_FIX_P()/M_UNFIX_P(). */
/*                            Added clear of jam on reset.   */
/* 11.June     2002 stainless Added emulation of JAM/HLT.    */
/* 11.June     2002 stainless Commented out unused Exec6502. */
/* 15.January  2002 TRAC      Added FAST_STACK.              */
/* 13.January  2002 TRAC      Fixed bugs where additive      */
/*                            cycle counting lost cycles.    */
/* 13.January  2002 TRAC      Added option to allow CPU      */
/*                            cycles to take more than one   */
/*                            cycle-counter cycle.           */
/* 08.January  2002 TRAC      Corrected RMW instruction bus  */
/*                            behavior.                      */
/* 07.January  2002 TRAC      Altered method of flag         */
/*                            emulation.                     */
/* 10.December 2001 TRAC      Added M_SLO.                   */
/* 08.December 2001 stainless Replaced unnecessary Op6502s.  */
/* 05.December 2001 TRAC      Added RETURN_ON_TRIP timing.   */
/* 04.December 2001 TRAC      Fixed Run6502 cycle loss bug.  */
/* 04.December 2001 TRAC      Indirect indexed and indexed   */
/*                            indirect no-wrap bugs added.   */
/* 02.December 2001 stainless Opcode fallback trace.         */
/* 01.December 2001 stainless D flag fix for interrupts.     */
/* 27.November 2001 stainless Additive cycle counting.       */
/* 27.November 2001 stainless Changed DEBUG to M_DEBUG.      */
/* 26.November 2001 stainless Removed decimal mode.          */
/* 25.November 2001 stainless Integrated into FakeNES.       */
/*************************************************************/

#include "core.h"
#include "core/tables.h"
#include <stdio.h>

/** INLINE ***************************************************/
/** Different compilers inline C functions differently.     **/
/*************************************************************/
#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE static
#endif

/** System-Dependent Stuff ***********************************/
/** This is system-dependent code put here to speed things  **/
/** up. It has to stay inlined to be fast.                  **/
/*************************************************************/
#ifdef INES
#define FAST_RDOP
extern byte *Page[];
INLINE byte Op6502(register word A) { return(Page[A>>13][A&0x1FFF]); }
#endif

/** FAST_RDOP ************************************************/
/** With this #define not present, Rd6502() should perform  **/
/** the functions of Op6502().                              **/
/*************************************************************/
#ifndef FAST_RDOP
#define Op6502(A) Rd6502(A)
#endif

/** FAST_STACK ***********************************************/
/** With this #define not present, Rd/Wr6502() should       **/
/** handle stack accesses.                                  **/
/*************************************************************/
#ifndef FAST_STACK
#define Rd6502Stack(A)   Rd6502(0x100+(A))
#define Wr6502Stack(A,D) Wr6502(0x100+(A),D)
#endif

/** FAST_ZP **************************************************/
/** With this #define not present, Rd/Wr6502() should       **/
/** handle zeropage accesses.                               **/
/*************************************************************/
#ifndef FAST_ZP
#define Rd6502zp(A)     Rd6502(A)
#define Wr6502zp(A,D)   Wr6502(A,D)
#endif

/** Addressing Methods ***************************************/
/** These macros calculate and return effective addresses.  **/
/*************************************************************/
#define MC_Ab(Rg)	M_LDWORD(Rg)
#define MC_Zp(Rg)       Rg.W=Op6502(R->PC.W);R->PC.W++;
#define MC_Zx(Rg)       Rg.B.l=Op6502(R->PC.W)+R->X;R->PC.W++;Rg.B.h=0
#define MC_Zy(Rg)       Rg.B.l=Op6502(R->PC.W)+R->Y;R->PC.W++;Rg.B.h=0
#define MC_Ax(Rg)	M_LDWORD(Rg);Rg.W+=R->X
#define MC_Ay(Rg)	M_LDWORD(Rg);Rg.W+=R->Y
#define MC_Ix(Rg)       K.B.l=Op6502(R->PC.W)+R->X;R->PC.W++;K.B.h=0; \
                        Rg.B.l=Rd6502zp(K.W);K.B.l++;Rg.B.h=Rd6502zp(K.W)
#define MC_Iy(Rg)       K.W=Op6502(R->PC.W);R->PC.W++; \
                        Rg.B.l=Rd6502zp(K.W);K.B.l++;Rg.B.h=Rd6502zp(K.W); \
                        Rg.W+=R->Y

/** Reading From Memory **************************************/
/** These macros calculate address and read from it.        **/
/*************************************************************/
#define MR_Ab(Rg)	MC_Ab(J);Rg=Rd6502(J.W)
#define MR_Im(Rg)       Rg=Op6502(R->PC.W);R->PC.W++
#define MR_Zp(Rg)   MC_Zp(J);Rg=Rd6502zp(J.W)
#define MR_Zx(Rg)   MC_Zx(J);Rg=Rd6502zp(J.W)
#define MR_Zy(Rg)   MC_Zy(J);Rg=Rd6502zp(J.W)
#define MR_Ax(Rg)       M_LDWORD(J); \
                        if (((int) J.B.l + R->X) > 0xFF) \
                        { \
                          J.B.l += R->X; \
                          Rd6502(J.W); \
                          R->ICount-=CYCLE_LENGTH;R->Cycles+=CYCLE_LENGTH; \
                          J.B.h++; \
                          Rg=Rd6502(J.W); \
                        } \
                        else \
                        { \
                          J.B.l += R->X; \
                          Rg=Rd6502(J.W); \
                        }
#define MR_Ay(Rg)       M_LDWORD(J); \
                        if (((int) J.B.l + R->Y) > 0xFF) \
                        { \
                          J.B.l += R->Y; \
                          Rd6502(J.W); \
                          R->ICount-=CYCLE_LENGTH;R->Cycles+=CYCLE_LENGTH; \
                          J.B.h++; \
                          Rg=Rd6502(J.W); \
                        } \
                        else \
                        { \
                          J.B.l += R->Y; \
                          Rg=Rd6502(J.W); \
                        }
#define MR_Ix(Rg)	MC_Ix(J);Rg=Rd6502(J.W)
#define MR_Iy(Rg)       K.W=Op6502(R->PC.W);R->PC.W++; \
                        J.B.l=Rd6502zp(K.W);K.B.l++; \
                        J.B.h=Rd6502zp(K.W); \
                        if (((int) J.B.l + R->Y) > 0xFF) \
                        { \
                          J.B.l += R->Y; \
                          Rd6502(J.W); \
                          R->ICount-=CYCLE_LENGTH;R->Cycles+=CYCLE_LENGTH; \
                          J.B.h++; \
                          Rg=Rd6502(J.W); \
                        } \
                        else \
                        { \
                          J.B.l += R->Y; \
                          Rg=Rd6502(J.W); \
                        }

/** Writing To Memory ****************************************/
/** These macros calculate address and write to it.         **/
/*************************************************************/
#define MW_Ab(Rg)	MC_Ab(J);Wr6502(J.W,Rg)
#define MW_Zp(Rg)   MC_Zp(J);Wr6502zp(J.W,Rg)
#define MW_Zx(Rg)   MC_Zx(J);Wr6502zp(J.W,Rg)
#define MW_Zy(Rg)   MC_Zy(J);Wr6502zp(J.W,Rg)
#define MW_Ax(Rg)       M_LDWORD(J); \
                        if (((int) J.B.l + R->X) > 0xFF) \
                        { \
                          J.B.l += R->X; \
                          Rd6502(J.W); \
                          J.B.h++; \
                          Wr6502(J.W,Rg); \
                        } \
                        else \
                        { \
                          J.B.l += R->X; \
                          Rd6502(J.W); \
                          Wr6502(J.W,Rg); \
                        }
#define MW_Ay(Rg)       M_LDWORD(J); \
                        if (((int) J.B.l + R->Y) > 0xFF) \
                        { \
                          J.B.l += R->Y; \
                          Rd6502(J.W); \
                          J.B.h++; \
                          Wr6502(J.W,Rg); \
                        } \
                        else \
                        { \
                          J.B.l += R->Y; \
                          Rd6502(J.W); \
                          Wr6502(J.W,Rg); \
                        }
#define MW_Ix(Rg)	MC_Ix(J);Wr6502(J.W,Rg)
#define MW_Iy(Rg)       K.W=Op6502(R->PC.W);R->PC.W++; \
                        J.B.l=Rd6502zp(K.W);K.B.l++; \
                        J.B.h=Rd6502zp(K.W); \
                        if (((int) J.B.l + R->Y) > 0xFF) \
                        { \
                          J.B.l += R->Y; \
                          Rd6502(J.W); \
                          J.B.h++; \
                          Wr6502(J.W,Rg); \
                        } \
                        else \
                        { \
                          J.B.l += R->Y; \
                          Wr6502(J.W,Rg); \
                        }

/** Modifying Memory *****************************************/
/** These macros calculate address and modify it.           **/
/*************************************************************/
#define MM_Ab(Cmd)      MC_Ab(J);I=Rd6502(J.W);Wr6502(J.W,I);Cmd(I);Wr6502(J.W,I)
#define MM_Zp(Cmd)      MC_Zp(J);I=Rd6502zp(J.W);Wr6502zp(J.W,I);Cmd(I);Wr6502zp(J.W,I)
#define MM_Zx(Cmd)      MC_Zx(J);I=Rd6502zp(J.W);Wr6502zp(J.W,I);Cmd(I);Wr6502zp(J.W,I)
#define MM_Ax(Cmd)      M_LDWORD(J); \
                        if (((int) J.B.l + R->X) > 0xFF) \
                        { \
                          J.B.l += R->X; \
                          Rd6502(J.W); \
                          J.B.h++; \
                          I=Rd6502(J.W); \
                        } \
                        else \
                        { \
                          J.B.l += R->X; \
                          I=Rd6502(J.W); \
                        } \
                        Wr6502(J.W,I); \
                        Cmd(I); \
                        Wr6502(J.W,I)

/** Other Macros *********************************************/
/** Calculating flags, stack, jumps, arithmetics, etc.      **/
/*************************************************************/
#define M_FIX_P()       ((R->N&N_FLAG)|(R->V?V_FLAG:0)|(R->D?D_FLAG:0)| \
                        (R->I?I_FLAG:0)|(R->Z?0:Z_FLAG)|(R->C?C_FLAG:0)|R_FLAG|B_FLAG)
#define M_UNFIX_P(P)    R->N=P&N_FLAG;R->V=P&V_FLAG;R->D=P&D_FLAG; \
                        R->I=P&I_FLAG;R->Z=P&Z_FLAG?0:1;R->C=P&C_FLAG;
#define M_FL(Rg)        R->N=R->Z=Rg
#define M_LDWORD(Rg)    Rg.B.l=Op6502(R->PC.W);Rg.B.h=Op6502(R->PC.W+1);R->PC.W+=2

#define M_PUSH(Rg)      Wr6502Stack(R->S,Rg);R->S--
#define M_POP(Rg)       R->S++;Rg=Rd6502Stack(R->S)
#define M_JR            I=Op6502(R->PC.W); \
                        R->PC.W++; \
                        R->ICount-=CYCLE_LENGTH;R->Cycles+=CYCLE_LENGTH; \
                        if ((I & 0x80) != \
                            ((((int) R->PC.B.l + I) > 0xFF) ? 0x80 : 0)) \
                        { \
                          R->PC.B.l += I; \
                          Op6502(R->PC.W); \
                          R->ICount-=CYCLE_LENGTH;R->Cycles+=CYCLE_LENGTH; \
                          if (I & 0x80) R->PC.B.h--; \
                          else R->PC.B.h++; \
                        } \
                        else \
                        { \
                          R->PC.B.l += I; \
                        }

#define M_ADC(Rg) \
    K.W=R->A+Rg+(R->C?1:0); \
    R->V=(~(R->A^Rg) & (R->A^K.B.l) & 0x80); \
    R->C=K.B.h; \
    M_FL(K.B.l); \
    R->A=K.B.l;

/* Warning! C_FLAG is inverted before SBC and after it */
#define M_SBC(Rg) \
    K.W=R->A-Rg-(R->C?0:1); \
    R->V=((R->A^Rg) & (R->A^K.B.l) & 0x80); \
    R->C=K.B.h + 1; \
    M_FL(K.B.l); \
    R->A=K.B.l;

#define M_CMP(Rg1,Rg2) \
  K.W=Rg1-Rg2; \
  R->C=K.B.h + 1; \
  M_FL(K.B.l);

#define M_BIT(Rg) \
  R->N=Rg; \
  R->V=Rg&V_FLAG; \
  R->Z=Rg&R->A;

#define M_AND(Rg)	R->A&=Rg;M_FL(R->A)
#define M_ORA(Rg)	R->A|=Rg;M_FL(R->A)
#define M_EOR(Rg)	R->A^=Rg;M_FL(R->A)
#define M_INC(Rg)	Rg++;M_FL(Rg)
#define M_DEC(Rg)	Rg--;M_FL(Rg)

#define M_SLO(Rg)       M_ASL(Rg);M_ORA(Rg)
#define M_ASL(Rg)       R->C=Rg&0x80;Rg<<=1;M_FL(Rg)
#define M_LSR(Rg)       R->C=Rg&1;Rg>>=1;M_FL(Rg)
#define M_ROL(Rg)       K.B.l=(Rg<<1)|(R->C?1:0); \
                        R->C=Rg&0x80;Rg=K.B.l; \
			M_FL(Rg)
#define M_ROR(Rg)       K.B.l=(Rg>>1)|(R->C?0x80:0); \
                        R->C=Rg&1;Rg=K.B.l; \
			M_FL(Rg)

/** Reset6502() **********************************************/
/** This function can be used to reset the registers before **/
/** starting execution with Run6502(). It sets registers to **/
/** their initial values.                                   **/
/*************************************************************/
void Reset6502(M6502 *R)
{
  int i;

  /* Initialize the instruction cycle count table. */
  for (i = 0; i < 256; i++)
  {
      Cycles[i] = BaseCycles[i] * CYCLE_LENGTH;
  }

  R->A=R->X=R->Y=0x00;

  R->N=R->V=R->D=R->I=R->C=0;
  R->Z=0;       /* 0 == set */

  R->S=0xFF;
  R->PC.B.l=Rd6502(0xFFFC);
  R->PC.B.h=Rd6502(0xFFFD);   
  R->ICount=R->IPeriod;
  R->IRequest=INT_NONE;
  R->AfterCLI=0;
  R->Jammed=0;
}

#ifdef DEBUG
static int opcode_count=0;
static byte opcode_trace[10];
#endif

#define OPCODE_PROLOG(x) \
    case x: {
#define OPCODE_EXIT      break;
#define OPCODE_EPILOG    OPCODE_EXIT }

#define OPCODE_PROLOG_DEFAULT \
    default: {

#if 0
/** Exec6502() ***********************************************/
/** This function will execute a single 6502 opcode. It     **/
/** will then return next PC, and current register values   **/
/** in R.                                                   **/
/*************************************************************/
word Exec6502(M6502 *R)
{
  byte opcode, cycles;

  opcode=Op6502(R->PC.W++);
  cycles=Cycles[opcode];
  R->Cycles+=cycles;
  R->ICount-=cycles;
#ifdef DEBUG
  opcode_trace[opcode_count++]=opcode;
  if (opcode_count==10) opcode_count=0;
#endif
  switch(opcode)
  {
    pair J,K;
    byte I;
#include "core/codes.h"
  }

  /* We are done */
  return(R->PC.W);
}
#endif

/** Int6502() ************************************************/
/** This function will generate interrupt of a given type.  **/
/** INT_NMI will cause a non-maskable interrupt. INT_IRQ    **/
/** will cause a normal interrupt, unless I_FLAG set in R.  **/
/*************************************************************/
void Int6502(M6502 *R,byte Type)
{
  pair J;

  if (R->Jammed) return;

  if((Type==INT_NMI)||((Type==INT_IRQ)&&!(R->I)))
  {
    byte P;
    R->ICount-=7*CYCLE_LENGTH;
    R->Cycles+=7*CYCLE_LENGTH;
    M_PUSH(R->PC.B.h);
    M_PUSH(R->PC.B.l);
    P = M_FIX_P() & ~B_FLAG;
    M_PUSH(P);
    /* R->D=0; */
    if(Type==INT_NMI) J.W=0xFFFA;
    else
    {
     R->I=1;
     J.W=0xFFFE;
     R->IRequest=INT_NONE;
    }
    R->PC.B.l=Rd6502(J.W++);
    R->PC.B.h=Rd6502(J.W);
  }
  else if ((Type==INT_IRQ)&&(R->I))
  {
    R->IRequest=INT_IRQ;
  }
}

/** Run6502() ************************************************/
/** This function will run 6502 code until Loop6502() call  **/
/** returns INT_QUIT. It will return the current register   **/
/** values in R.                                            **/
/*************************************************************/
void Run6502(M6502 *R)
{
  if (R->Jammed) return;

  for(;;)
  {
    while (R->ICount>0)
    {
      byte opcode, cycles;

#ifdef M_DEBUG
      /* Turn tracing on when reached trap address */
      if(R->PC.W==R->Trap) R->Trace=1;
      /* Call single-step debugger, exit if requested */
      if(R->Trace)
        if(!Debug6502(R)) return;
#endif

      opcode=Op6502(R->PC.W++);
      cycles=Cycles[opcode];
      R->Cycles+=cycles;
      R->ICount-=cycles;
#ifdef DEBUG
      opcode_trace[opcode_count++]=opcode;
      if (opcode_count==10) opcode_count=0;
#endif
      switch(opcode)
      {
        pair J,K;
        byte I;
#include "core/codes.h"
      }
    }

    /* cycle counter expired, or we wouldn't be here */
    {
      int interrupt;

      /* If we have come after CLI, get INT_? from IRequest */
      /* Otherwise, get it from the loop handler            */
      if(R->AfterCLI)
      {
        interrupt=R->IRequest;    /* Get pending interrupt     */
        R->ICount+=R->IBackup-1;  /* Restore the ICount        */
        R->AfterCLI=0;            /* Done with AfterCLI state  */
      }
      else
      {
#ifdef RETURN_ON_TRIP
        return;
#else
        interrupt=Loop6502(R);    /* Call the periodic handler */
        R->ICount+=R->IPeriod;    /* Reset the cycle counter   */
#endif
      }

      if(interrupt==INT_QUIT) return;          /* Exit if INT_QUIT     */
      if(interrupt) Int6502(R,interrupt);      /* Interrupt if needed  */ 
    }
  }

  /* Execution stopped */
  return;
}
