/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                         M6502.h                         **/
/**                                                         **/
/** This file contains declarations relevant to emulation   **/
/** of 6502 CPU.                                            **/
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
/* 13.June     2002 TRAC      Improved alignment of M6502    */
/*                            struct and eliminated obsolete */
/*                            variables.                     */
/* 11.June     2002 stainless Added emulation of JAM/HLT.    */
/* 15.January  2002 TRAC      Added FAST_STACK.              */
/* 13.January  2002 TRAC      Added option to allow CPU      */
/*                            cycles to take more than one   */
/*                            cycle-counter cycle.           */
/* 07.January  2002 TRAC      Altered context struct for     */
/*                            new method of flag emulation.  */
/* 11.December 2001 TRAC      Added INLINE_MEMORY_HANDLERS.  */
/* 05.December 2001 TRAC      Added RETURN_ON_TRIP timing.   */
/* 27.November 2001 stainless Additive cycle counting.       */
/* 27.November 2001 stainless Added 'misc.h' datatypes.      */
/* 27.November 2001 stainless Changed DEBUG to M_DEBUG.      */
/* 26.November 2001 stainless Integrated into FakeNES.       */
/*************************************************************/
#ifndef M6502_H
#define M6502_H


                               /* Compilation options:       */
#define CYCLE_LENGTH 3         /* Number of cycles that one  */
                               /* CPU cycle uses.            */
#define INLINE_MEMORY_HANDLERS /* Uses Rd6502/Wr6502/Op6502  */
                               /*  #include'd from cpu.h     */
#define RETURN_ON_TRIP         /* Run6502 returns on trip    */
#define FAST_RDOP              /* Separate Op6502()/Rd6502() */
#define FAST_STACK             /* Separate stack handlers    */
#define FAST_ZP                /* Separate zeropage handlers */

/* #define M_DEBUG */          /* Compile debugging version  */
/* #define LSB_FIRST */        /* Compile for low-endian CPU */

                               /* Loop6502() returns:        */
#define INT_NONE  0            /* No interrupt required      */
#define INT_IRQ	  1            /* Standard IRQ interrupt     */
#define INT_NMI	  2            /* Non-maskable interrupt     */
#define INT_QUIT  3            /* Exit the emulation         */

                               /* 6502 status flags:         */
#define	C_FLAG	  0x01         /* 1: Carry occured           */
#define	Z_FLAG	  0x02         /* 1: Result is zero          */
#define	I_FLAG	  0x04         /* 1: Interrupts disabled     */
#define	D_FLAG	  0x08         /* 1: Decimal mode            */
#define	B_FLAG	  0x10         /* Break [0 on stk after int] */
#define	R_FLAG	  0x20         /* Always 1                   */
#define	V_FLAG	  0x40         /* 1: Overflow occured        */
#define	N_FLAG	  0x80         /* 1: Result is negative      */

/** Simple Datatypes *****************************************/
/** NOTICE: sizeof(byte)=1 and sizeof(word)=2               **/
/*************************************************************/
// typedef unsigned char byte;
// typedef unsigned short word;
typedef signed char offset;

#include "misc.h"

#define byte    UINT8
#define word    UINT16

/** Structured Datatypes *************************************/
/** NOTICE: #define LSB_FIRST for machines where least      **/
/**         signifcant byte goes first.                     **/
/*************************************************************/
typedef union
{
#ifdef LSB_FIRST
  struct { byte l,h; } B;
#else
  struct { byte h,l; } B;
#endif
  word W;
} pair;

typedef struct
{
  /* CPU registers and program counter   */
  pair PC;
  byte A,X,Y,S;
  byte N,V,D,I,Z,C;   /* CPU status flags - Z flag set when Z == 0 */

  int IPeriod,ICount; /* Set IPeriod to number of CPU cycles */
                      /* between calls to Loop6502()         */
  int Cycles;         /* Elapsed cycles since last cleared   */
  int IBackup;        /* Private, don't touch                */
  word Trap;          /* Set Trap to address to trace from   */
  byte AfterCLI;      /* Private, don't touch                */
  byte IRequest;      /* Set to the INT_IRQ when pending IRQ */
  byte TrapBadOps;    /* Set to 1 to warn of illegal opcodes */
  byte Trace;         /* Set Trace=1 to start tracing        */
  byte Jammed;        /* Private, don't touch                */
} M6502;

/** Reset6502() **********************************************/
/** This function can be used to reset the registers before **/
/** starting execution with Run6502(). It sets registers to **/
/** their initial values.                                   **/
/*************************************************************/
void Reset6502(register M6502 *R);

/** Exec6502() ***********************************************/
/** This function will execute a single 6502 opcode. It     **/
/** will then return next PC, and current register values   **/
/** in R.                                                   **/
/*************************************************************/
word Exec6502(register M6502 *R);

/** Int6502() ************************************************/
/** This function will generate interrupt of a given type.  **/
/** INT_NMI will cause a non-maskable interrupt. INT_IRQ    **/
/** will cause a normal interrupt, unless I_FLAG set in R.  **/
/*************************************************************/
void Int6502(register M6502 *R,register byte Type);

/** Run6502() ************************************************/
/** This function will run 6502 code until Loop6502() call  **/
/** returns INT_QUIT. It will return the current register   **/
/** values in R.                                            **/
/*************************************************************/
void Run6502(register M6502 *R);

/** Rd6502()/Wr6502/Op6502() *********************************/
/** These functions are called when access to RAM occurs.   **/
/** They allow to control memory access. Op6502 is the same **/
/** as Rd6502, but used to read *opcodes* only, when many   **/
/** checks can be skipped to make it fast. It is only       **/
/** required if there is a #define FAST_RDOP.               **/
/************************************ TO BE WRITTEN BY USER **/
#ifndef INLINE_MEMORY_HANDLERS
void Wr6502(register word Addr,register byte Value);
byte Rd6502(register word Addr);
byte Op6502(register word Addr);
#ifdef FAST_STACK
byte Rd6502Stack(register byte S);
void Wr6502Stack(register byte S,register byte Value);
#endif
#ifdef FAST_ZP
byte Rd6502zp(register byte S);
void Wr6502zp(register byte S,register byte Value);
#endif
#else
#include "cpu.h"
#endif

/** Debug6502() **********************************************/
/** This function should exist if DEBUG is #defined. When   **/
/** Trace!=0, it is called after each command executed by   **/
/** the CPU, and given the 6502 registers. Emulation exits  **/
/** if Debug6502() returns 0.                               **/
/*************************************************************/
byte Debug6502(register M6502 *R);

/** Loop6502() ***********************************************/
/** 6502 emulation calls this function periodically to      **/
/** check if the system hardware requires any interrupts.   **/
/** This function must return one of following values:      **/
/** INT_NONE, INT_IRQ, INT_NMI, or INT_QUIT to exit the     **/
/** emulation loop.                                         **/
/************************************ TO BE WRITTEN BY USER **/
byte Loop6502(register M6502 *R);

#endif /* M6502_H */
