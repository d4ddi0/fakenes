/*

FakeNES - A portable, Open Source NES emulator.

core.c: Implementation of the RP2A03G CPU emulation

Copyright (c) 2002, Charles Bilyue' and Randy McDowell.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

 This file contains declarations necessary for the emulation core
functions for the Ricoh RP2A03G CPU, as well as preprocessor
definitions used to control the compilation and operation of
the emulation core.

*/

#ifndef CORE_H
#define CORE_H


                               /* Compilation options:       */
#define CYCLE_LENGTH 3         /* Number of cycles that one  */
                               /* CPU cycle uses.            */
/*  Uses FN2A03_Read, FN2A03_Write, FN2A03_Fetch #included */
/* from cpu.h */
#define INLINE_MEMORY_HANDLERS

#define FAST_STACK             /* Separate stack handlers    */
#define FAST_ZP                /* Separate zeropage handlers */

/* #define ALT_DEBUG */        /* Compile debugging version  */
/* #define LSB_FIRST */        /* Compile for low-endian CPU */

#define FN2A03_INT_NONE  0     /* No interrupt required      */
#define FN2A03_INT_NMI   1     /* Non-maskable interrupt     */
/* Maskable IRQ cleared after single acknowledgement */
#define FN2A03_INT_IRQ_SINGLE_SHOT 2
/* Maskable IRQs cleared via FN2A03_Clear_Interrupt() */
#define FN2A03_INT_IRQ_SOURCE(x) (FN2A03_INT_IRQ_SINGLE_SHOT + 1 + (x))
#define FN2A03_INT_IRQ_SOURCE_MAX (31 - 1)

                               /* 2A03 status flags:         */
#define	C_FLAG	  0x01         /* 1: Carry occured           */
#define	Z_FLAG	  0x02         /* 1: Result is zero          */
#define	I_FLAG	  0x04         /* 1: Interrupts disabled     */
#define	D_FLAG	  0x08         /* 1: Decimal mode            */
#define	B_FLAG	  0x10         /* Break [0 on stk after int] */
#define	R_FLAG	  0x20         /* Always 1                   */
#define	V_FLAG	  0x40         /* 1: Overflow occured        */
#define	N_FLAG	  0x80         /* 1: Result is negative      */

/*
 The following data types must be defined.
  UINT8     unsigned    sizeof(UINT8) == 1
  INT8      signed      sizeof(INT8) == 1
  UINT16    unsigned    sizeof(UINT16) == 2
  PAIR      union       sizeof(PAIR) == 2
   { UINT16 word; struct { UINT8 low, high } bytes; }
*/

#include "misc.h"

typedef struct
{
  /* CPU registers and program counter   */
  PAIR PC;
  UINT8 A,X,Y,S;
  UINT8 N,V,D,I,Z,C;   /* CPU status flags - Z flag set when Z == 0 */

  int ICount;         /*  FN2A03_Run will deduct cycles from */
                      /* here, executing while it is above   */
                      /* zero.                               */
  int Cycles;         /* Elapsed cycles since last cleared   */
  int IBackup;        /* Private, don't touch                */
  UINT16 IRequest;    /* Set to the INT_IRQ when pending IRQ */
  UINT16 Trap;        /* Set Trap to address to trace from   */
  UINT8 AfterCLI;     /* Private, don't touch                */
  UINT8 TrapBadOps;   /* Set to 1 to warn of illegal opcodes */
  UINT8 Trace;        /* Set Trace=1 to start tracing        */
  UINT8 Jammed;       /* Private, don't touch                */
} FN2A03;

/*
 FAST_STACK

  If this #define is present, FN2A03_Read_Stack() and
 FN2A03_Write_Stack() must be present.  If this #define is absent,
 FN2A03_Read() and FN2A03_Write() will handle stack accesses.
*/
#ifndef FAST_STACK
#define FN2A03_Read_Stack(A)    (FN2A03_Read((UINT16) 0x100+(UINT8 (A))))
#define FN2A03_Write_Stack(A,D) (FN2A03_Write((UINT16) 0x100+(UINT8 (A)), (D)))
#endif

/*
 FAST_ZP

  If this #define is present, FN2A03_Read_ZP() and
 FN2A03_Write_ZP() must be present.  If this #define is absent,
 FN2A03_Read() and FN2A03_Write() will handle zero page accesses.
*/
#ifndef FAST_ZP
#define FN2A03_Read_ZP(A)    (FN2A03_Read((UINT8 (A))))
#define FN2A03_Write_ZP(A,D) (FN2A03_Write((UINT8 (A)), (D)))
#endif


/*
 FN2A03_Init()

 This function performs any necessary initial core initialization.
*/
void FN2A03_Init(void);

/*
 FN2A03_Reset()

  This function is used to reset the CPU context to a state
 resembling hardware reset or power-on.  This function or
 FN2A03_Init() should be called at least once before any calls
 to FN2A03_Run() are made.
*/
void FN2A03_Reset(FN2A03 *R);

/*
 FN2A03_Exec()

  This function will execute a single RP2A03G opcode. It will
 then return next PC, and updated context in R.
*/
UINT16 FN2A03_Exec(FN2A03 *R);

/*
 FN2A03_Interrupt()

  This function requests an interrupt of the specified type.
 FN2A03_INT_NMI will raise a non-maskable interrupt.
 FN2A03_INT_IRQ_SINGLE_SHOT will raise a maskable interrupt to be cleared
 after a single acknowledgement, and FN2A03_INT_IRQ(x) will raise a
 maskable interrupt to be cleared later by FN2A03_Clear_Interrupt().
  No interrupts are acknowledged while the CPU is jammed.  Maskable
 interrupts will not be acknowledged until the I flag is clear.
*/
void FN2A03_Interrupt(FN2A03 *R,UINT8 Type);

/*
 FN2A03_Run()

  This function will execute RP2A03G code until the cycle
 counter expires.
*/
void FN2A03_Run(FN2A03 *R);

/*
 FN2A03_consume_cycles()

  This function will steal the requested count of clock cycles
 from the CPU.
*/
static INLINE void FN2A03_consume_cycles (FN2A03 *R, int cycles)
{
    R->ICount -= cycles * CYCLE_LENGTH;
    R->Cycles += cycles * CYCLE_LENGTH;
}

/*
 FN2A03_Read/Write/Fetch()

  These functions are called when a memory access occurs.
 Fetch is used for reading opcode bytes from executing code.

  These are not part of the CPU core and must be supplied by
 the user.
*/
#ifndef INLINE_MEMORY_HANDLERS
void FN2A03_Write(UINT16 Addr,UINT8 Value);
UINT8 FN2A03_Read(UINT16 Addr);
UINT8 FN2A03_Fetch(UINT16 Addr);

#ifdef FAST_STACK
UINT8 FN2A03_Read_Stack(UINT8 S);
void FN2A03_Write_Stack(UINT8 S,UINT8 Value);
#endif

#ifdef FAST_ZP
UINT8 FN2A03_Read_ZP(UINT8 S);
void FN2A03_Write_ZP(UINT8 S,UINT8 Value);
#endif

#else
#include "cpu.h"
#endif

/*
 FN2A03_Debug()

  This function should exist if ALT_DEBUG is #defined. When
 Trace!=0, it is called after each opcode executed by the
 CPU, and passed the RP2A03G context. Emulation exits if
 FN2A03_Debug() returns 0.
*/
UINT8 FN2A03_Debug(FN2A03 *R);

#endif /* CORE_H */
