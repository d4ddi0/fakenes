/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "Core.hpp"
#include "CPU.h"
#include "Internals.h"
#include "Local.hpp"

namespace {

// Internal context, can be set via SetContext().
COREContext core;

/* This is the amount by which the clock counter is incremented each CPU clock.
   (The clock counter is always stored in master clock cycles.) */
CORETime timeStep = 0;

// Size of the time table; or more specifically the number of opcodes.
const size_type TimeTableSize = 0xFF + 1;

/* Table containing the execution times of each opcode. This is actually expanded into
   master clock cycles by BuildTimeTable() before use. */
const CORETime TimeTableBase[TimeTableSize] = {
	7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7,
	2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
	2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 5, 4, 4, 4, 4,
	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7,
	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 5, 5, 7, 7
};

CORETime timeTable[TimeTableSize];

/* We sort the interrupt queue by time, so that the first interrupt in the list always
   occurs first. This allows us to simplify checks for pending interrupts to only
   checking the first entry in the queue. */
constant_function bool InterruptQueueTimeSorter(const COREInterrupt first, const COREInterrupt second) {
	return first.time < second.time;
}

// Similarly, NMI generally takes priority over IRQ.
constant_function bool InterruptQueuePrioritySorter(const COREInterrupt first, const COREInterrupt second) {
	return (first.type == COREInterruptNMI);
}

} // namespace anonymous

// Public interface begins here.
namespace CORE {

/* Import and expand all of our templates into useable code. The generated functions
   occupy their own namespace, 'Templates', and are static. */
#include "Core/Templates.hpp"

/* At power-up:
    P = $34 (interrupt inhibit set) 
    A, X, Y = 0 
    S = $FD */
bool Initialize() {
	ClearContext(core);

	_PCL = cpu_read(COREInterruptVectorRESET);
	_PCH = cpu_read(COREInterruptVectorRESET + 1);
	_S = 0xFD;
	SetFlag(_IF, true);

	BuildTimeTable();

	return true;
}

/* After reset:
    A, X, Y were not affected 
    S was decremented by 3 (but nothing was written to the stack) 
    The I flag was set (status ORed with $04)  */
void Reset() {
	core.time = 0;
	core.interrupts.clear();
	core.afterCLI = false;

	_PCL = cpu_read(COREInterruptVectorRESET);
	_PCH = cpu_read(COREInterruptVectorRESET + 1);
	_S -= 3;
	SetFlag(_IF, true);
}

linear_function CORETime Execute(const CORETime time) {
	// Grab our initial timestamp so we can avoid emulating for too long.
	const CORETime timestamp = core.time;

	while(true) {
		// Execute a single instruction or interrupt.
		Templates::Step();

		// Check the time elapsed. If we've reached our limit, exit. 
		const CORETime timeElapsed = (CORETimeDelta)core.time - (CORETimeDelta)timestamp;
		if(timeElapsed >= time)
			return timeElapsed;
	}
}

#define ExecuteTemplate(_Suffix) \
linear_function CORETime Execute##_Suffix(const CORETime time) { \
	const CORETime timestamp = core.time; \
	while(true) { \
		Templates::Step##_Suffix(); \
		const CORETime timeElapsed = (CORETimeDelta)core.time - (CORETimeDelta)timestamp; \
		if(timeElapsed >= time) \
			return timeElapsed; \
	} \
}

// Generate the Execute() variants.
ExecuteTemplate(Fast)
ExecuteTemplate(Asynchronous)

/* This steals clocks from the CPU, causing external hardware to be "overclocked"
   while the processor is unable to do anything. As the amount is specified in master
   clock cycles, it is possible to steal partial CPU clocks. */
void Burn(const CORETime time) {
	core.time += time;
}

// This queues an interrupt of a given type to occur at 'time'.
void SetInterrupt(const COREInterruptType type, const CORETime time) {
	COREInterrupt interrupt;
	interrupt.type = type;
	interrupt.time = time;
	core.interrupts.push_back(interrupt);

	// Sort the interrupt queue by time (ascending) and priority (NMIs first).
	core.interrupts.sort(InterruptQueueTimeSorter);
	core.interrupts.sort(InterruptQueuePrioritySorter);
}

/* This clears all interrupts of a given type, even if more than one is set to occur
   at different times. The interrupts are both unqueued and acknowledged. */
void ClearInterrupt(const COREInterruptType type) {
	for(COREInterruptQueue::iterator i = core.interrupts.begin(); i != core.interrupts.end(); ) {
		COREInterrupt& interrupt = *i;
		if(interrupt.type != type) {
			i++;
			continue;
		}

		i = core.interrupts.erase(i);
	}
}

/* This returns a copy of the internal context (e.g for state saving).
   The flags are packed into the status register 'P'. */
void GetContext(COREContext& context) {
	Templates::PackFlags();
	context = core;
}

/* This sets the internal context to the contents of a user-provided one.
   The flags are unpacked from the status register 'P'. */
void SetContext(const COREContext& context) {
	core = context;
	Templates::UnpackFlags();
}

// This properly initializes a context object.
void ClearContext(COREContext& context) {
	context.time = 0;
	context.interrupts.clear();
	context.afterCLI = false;

	memset(&context.registers, 0, sizeof(CORERegisters));
	memset(&context.flags, 0, sizeof(COREFlags));
}

/* This returns the current execution time, in master clock cycles. It is not an
   infinite counter, so be careful of wrapping: calculations that compute deltas
   (differences between timestamps) must always cast both timestamps to the
   COREDeltaTime data type before subtracting. */
CORETime GetTime() {
	return core.time;
}

/* This offers a safe method of computing time deltas. This returns the number of
   clocks that have occured since 'timestamp'. */
CORETime GetTimeElapsed(const CORETime timestamp) {
	return (CORETimeDelta)core.time - (CORETimeDelta)timestamp;
}

/* This rebuilds the internal time table. you need to call this function any time
   CPU_CLOCK_MULTIPLIER changes (e.g from going from NTSC to PAL). */
void BuildTimeTable() {
	for(size_type i = 0; i < TimeTableSize; i++)
		timeTable[i] = TimeTableBase[i] * CPU_CLOCK_MULTIPLIER;

	timeStep = 1 * CPU_CLOCK_MULTIPLIER;
}

} // namespace CORE
