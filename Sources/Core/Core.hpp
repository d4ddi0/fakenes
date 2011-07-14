/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Core__Core_hpp__included
#define Core__Core_hpp__included
#include "Core/Local.hpp"

// Define this to enable some important sanity checks.
#define COREDebug

// Data types to hold execution times:
typedef uint32 CORETime;	// Absolute, used for timestamps, durations, etc.
typedef int32 CORETimeDelta;	// Relative, used for time deltas.

// Status register (P) bits:
enum {
	COREFlagCarry     = 1 << 0,	// C flag.
	COREFlagZero      = 1 << 1,	// Z flag.
	COREFlagInterrupt = 1 << 2,	// I flag.
	COREFlagDecimal   = 1 << 3,	// D flag (unused on the 2A03).
	COREFlagBreak     = 1 << 4,	// B pseudo-flag.
	COREFlagReserved  = 1 << 5,	// Reserved, always 1.
	COREFlagOverflow  = 1 << 6,	// V flag.
	COREFlagNegative  = 1 << 7	// N flag.
};

// Interrupt types:
enum COREInterruptType {
	COREInterruptNMI = 0,	// Non-maskable interrupt (NMI).
	COREInterruptIRQ,	// Interrupt request (IRQ), must be acknowledged.
	COREInterruptIRQ1,	// Additional IRQ sources (1-4). 
	COREInterruptIRQ2,	
	COREInterruptIRQ3,
	COREInterruptIRQ4
};

// Interrupt vectors:
enum {
	COREInterruptVectorNMI   = 0xFFFA,	// Non-maskable interrupt.
	COREInterruptVectorRESET = 0xFFFC,	// Reset interrupt.
	COREInterruptVectorIRQ   = 0xFFFE	// Interrupt request or BRK instruction.
};

// State context:
typedef struct _CORERegisters {
	byte_pair pc;		// Program counter.
	uint8 a, p, s, x, y;	// Accumulator, status, index registers, stack pointer.
} CORERegisters;

/* Note: The break (B) flag doesn't actually exist. The "break" bit (bit 4) in the
   cooresponding byte pushed onto the stack by the BRK and PHP instructions is
   simply always set, but is never set by a hardware interrupt. */
typedef struct _COREFlags {
	uint8 c;	// Carry flag.
	uint8 z;	// Zero flag.
	uint8 i;	// Interrupt flag.
	uint8 d;	// Decimal mode flag. 
	uint8 v;	// Overflow flag.
	uint8 n;	// Negative flag.
} COREFlags;

typedef struct _COREInterrupt {
	COREInterruptType type;	// Where it is coming from.
	CORETime time;		// When it will occur.
} COREInterrupt;

typedef std::list<COREInterrupt> COREInterruptQueue;

typedef struct _COREContext {
	CORETime time;			// Execution time.
	CORERegisters registers;	// Registers.
	COREFlags flags;		// Flags, in their expanded form.
	COREInterruptQueue interrupts;	// Interrupt queue.

	/* CLI does not clear the I flag immediately, it is delayed by one instruction.
           This is to help prevent inescapable loops from forming. */
	bool afterCLI;
} COREContext;

namespace CORE {

extern bool Initialize();
extern void Reset();
extern linear_function CORETime Execute(const CORETime time);
extern linear_function CORETime ExecuteFast(const CORETime time);
extern linear_function CORETime ExecuteAsynchronous(const CORETime time);
extern void Burn(const CORETime time);
extern void SetInterrupt(const COREInterruptType type, const CORETime time);
extern void ClearInterrupt(const COREInterruptType type);
extern void GetContext(COREContext& context);
extern void SetContext(const COREContext& context);
extern void ClearContext(COREContext& context);
extern CORETime GetTime();
extern CORETime GetTimeElapsed(const CORETime timestamp);
extern void BuildTimeTable();

} // namespace CORE

#endif // !Core__Core_hpp__included
