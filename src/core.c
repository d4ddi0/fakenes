/*

FakeNES - A portable, Open Source NES emulator.

core.c: Implementation of the RP2A03G CPU emulation

Copyright (c) 2002, Charles Bilyue' and Randy McDowell.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

 This file contains emulation core functions for the Ricoh RP2A03G
CPU, as used in the Nintendo Famicom (Family Computer) and NES
(Nintendo Entertainment System).

*/

#include <allegro.h>
#include <stdio.h>

#include "core.h"
#include "core/tables.h"


/* define some macros to help improve readability */

/* fetch an opcode byte */
#define Fetch(A)            (FN2A03_Fetch((A)))
/* read/write a data byte */
#define Read(A)             (FN2A03_Read((A)))
#define Write(A,D)          (FN2A03_Write((A),(D)))

/* read/write a stack byte */
#define Read_Stack(A)       (FN2A03_Read_Stack((A)))
#define Write_Stack(A,D)    (FN2A03_Write_Stack((A),(D)))

/* read/write a zero page byte */
#define Read_ZP(A)          (FN2A03_Read_ZP((A)))
#define Write_ZP(A,D)       (FN2A03_Write_ZP((A),(D)))

/* fetch two opcode bytes (absolute address) */
#define Fetch16(Rg) \
    Rg.bytes.low = Fetch(PC.word + 1); Rg.bytes.high = Fetch(PC.word + 2)

/* These macros are used for pushing and pulling data on the stack. */
#define Push(Rg)        Write_Stack(R->S,Rg); R->S--
#define Pull(Rg)        R->S++; Rg=Read_Stack(R->S)
#define Push16(Rg)      Push(Rg.bytes.high); Push(Rg.bytes.low)
#define Pull16(Rg)      Pull(Rg.bytes.low); Pull(Rg.bytes.high)


/* These macros pack flags into the CPU's own format (for push */
/* to the stack or display in a debugger) or unpack flags from */
/* the CPU's format (for pop from the stack). */

#define Pack_Flags()    ((R->N&N_FLAG)|(R->V?V_FLAG:0)|(R->D?D_FLAG:0)| \
                        (R->I?I_FLAG:0)|(R->Z?0:Z_FLAG)|(R->C?C_FLAG:0)|R_FLAG|B_FLAG)
#define Unpack_Flags(P) R->N=P&N_FLAG;R->V=P&V_FLAG;R->D=P&D_FLAG; \
                        R->I=P&I_FLAG;R->Z=P&Z_FLAG?0:1;R->C=P&C_FLAG;

/* This macro is used to set the Negative and Zero flags based */
/* on a result. */
#define Update_NZ(Value) (R->N=R->Z=Value)


/* Addressing mode macros */
#include "core/addr.h"
/* Instruction macros */
#include "core/insns.h"


/*
 FN2A03_Init()

 This function performs any necessary initial core initialization.
*/
void FN2A03_Init(void)
{
  int i;

  /* Initialize the instruction cycle count table. */
  for (i = 0; i < 256; i++)
  {
      Cycles[i] = BaseCycles[i] * CYCLE_LENGTH;
  }
}

/*
 FN2A03_Reset()

  This function is used to reset the CPU context to a state
 resembling hardware reset or power-on.  This function or
 FN2A03_Init() should be called at least once before any calls
 to FN2A03_Run() are made.
*/
void FN2A03_Reset(FN2A03 *R)
{
  FN2A03_Init();

  R->A=R->X=R->Y=0x00;

  R->N=R->V=R->D=R->I=R->C=0;
  R->Z=0;       /* 0 == set */

  R->S=0xFF;
  R->PC.bytes.low=Read(0xFFFC);
  R->PC.bytes.high=Read(0xFFFD);   
  R->ICount=0;
  R->IRequest=FN2A03_INT_NONE;
  R->AfterCLI=0;
  R->Jammed=0;
}

#ifdef DEBUG
static int opcode_count=0;
static UINT8 opcode_trace[10];
#endif

#define OPCODE_PROLOG(x) \
    case x: {
#define OPCODE_EXIT      break;
#define OPCODE_EPILOG    OPCODE_EXIT }

#define OPCODE_PROLOG_DEFAULT \
    default: {

#if 0
/*
 FN2A03_Exec()

  This function will execute a single RP2A03G opcode. It will
 then return next PC, and updated context in R.
*/
UINT16 FN2A03_Exec(FN2A03 *R)
{
  UINT8 opcode, cycles;
  PAIR PC;

  PC.word=R->PC.word;
  opcode=Fetch(PC.word);
  cycles=Cycles[opcode];
  R->Cycles+=cycles;
  R->ICount-=cycles;
#ifdef DEBUG
  opcode_trace[opcode_count++]=opcode;
  if (opcode_count==10) opcode_count=0;
#endif
  switch(opcode)
  {
    PAIR address, temp_address, result;
    UINT8 zero_page_address, data;
#include "core/codes.h"
  }

  R->PC.word=PC.word;

  /* We are done */
  return(PC.word);
}
#endif

/*
 FN2A03_Interrupt()

  This function requests an interrupt of the specified type.
 FN2A03_INT_NMI will raise a non-maskable interrupt.
 FN2A03_INT_IRQ_SINGLE_SHOT will raise a maskable interrupt to be cleared
 after a single acknowledgement, and FN2A03_INT_IRQ_SOURCE(x) will raise
 a maskable interrupt to be cleared later by FN2A03_Clear_Interrupt().
  No interrupts are acknowledged while the CPU is jammed.  Maskable
 interrupts will not be acknowledged until the I flag is clear.
*/
void FN2A03_Interrupt(FN2A03 *R,UINT8 Type)
{
  UINT16 vector;

  if (R->Jammed) return;

  if((Type==FN2A03_INT_NMI)||((Type==FN2A03_INT_IRQ_SINGLE_SHOT)&&!(R->I)))
  {
    UINT8 P;
    R->ICount-=7*CYCLE_LENGTH;
    R->Cycles+=7*CYCLE_LENGTH;
    Push16(R->PC);
    P = Pack_Flags() & ~B_FLAG;
    Push(P);
    /* R->D=0; */
    if(Type==FN2A03_INT_NMI) vector=0xFFFA;
    else
    {
     R->I=1;
     vector=0xFFFE;
     R->IRequest=FN2A03_INT_NONE;
    }
    R->PC.bytes.low=Read(vector);
    R->PC.bytes.high=Read(vector+1);
  }
  else if ((Type==FN2A03_INT_IRQ_SINGLE_SHOT)&&(R->I))
  {
    R->IRequest=FN2A03_INT_IRQ_SINGLE_SHOT;
  }
}

/*
 FN2A03_Run()

  This function will execute RP2A03G code until the cycle
 counter expires.
*/
void FN2A03_Run(FN2A03 *R)
{
  if (R->Jammed) return;

  for(;;)
  {
    PAIR PC;
    PC.word=R->PC.word;

    while (R->ICount>0)
    {
      UINT8 opcode, cycles;

#ifdef ALT_DEBUG
      /* Turn tracing on when reached trap address */
      if(PC.word==R->Trap) R->Trace=1;
      /* Call single-step debugger, exit if requested */
      if(R->Trace)
        if(!FN2A03_Debug(R)) return;
#endif

      opcode=Fetch(PC.word);
      cycles=Cycles[opcode];
      R->Cycles+=cycles;
      R->ICount-=cycles;
#ifdef DEBUG
      opcode_trace[opcode_count++]=opcode;
      if (opcode_count==10) opcode_count=0;
#endif
      switch(opcode)
      {
        PAIR address, temp_address, result;
        UINT8 zero_page_address, data;
#include "core/codes.h"
      }
    }

    R->PC.word=PC.word;

    /* cycle counter expired, or we wouldn't be here */
    if (!R->AfterCLI) return;

    /* If we have come after CLI, get FN2A03_INT_? from IRequest */
    {
      int interrupt;

      interrupt=R->IRequest;    /* Get pending interrupt     */
      R->ICount+=R->IBackup-1;  /* Restore the ICount        */
      R->AfterCLI=0;            /* Done with AfterCLI state  */

      if (interrupt) FN2A03_Interrupt(R,interrupt);  /* Interrupt if needed */ 
    }
  }

  /* Execution stopped */
  return;
}
