/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

/* Checks if an interrupt is pending, either NMI or IRQ. This will not signal an IRQ as pending
   if the interrupt inhibit (I) flag is set in the processor status register. */
express_function bool T(InterruptPending)() {
	// Check if any interrupts are queued, and return if the queue is empty.
	if(core.interrupts.size() == 0)
		return false;

	/* Get the profile of the first interrupt in the queue. The interrupt queue is presorted
           for time and priority, so they must be executed in linear order. */
	const COREInterrupt& interrupt = core.interrupts.front();

	// Check if the alotted time has passed.
	if(interrupt.time >= core.time) {
		if(interrupt.type == COREInterruptNMI) {
			// NMIs cannot be inhibited.
			return true;
		} else {
			// The I flag prevents IRQs from occuring as long as it is set.
			return !_IF;
		}
	}

	// No interrupt is pending at this time.
	return false;
}

/* Executes a single interrupt and clears it if NMI or single-shot IRQ. Note that if the
   interrupt is an IRQ, it will be executed regardless of I flag state. */
express_function void T(Interrupt)(const COREInterruptType type) {
	// Check if this is a NMI or single-shot IRQ.
	if((type == COREInterruptNMI) || (type == COREInterruptIRQ)) {
		// Clear and acknowledge the interrupt.
		ClearInterrupt(type);
	}

	/* Interrupt timing is similar to BRK...
	    #  address R/W description
	    1    PC     R  fetch opcode, increment PC
	    2    PC     R  read next instruction byte (and throw it away), increment PC
	    3  $0100,S  W  push PCH on stack, decrement S
	    4  $0100,S  W  push PCL on stack, decrement S
	    5  $0100,S  W  push P on stack, decrement S
	    6   $FFFE   R  fetch PCL
	    7   $FFFF   R  fetch PCH */
	T(DummyFetch)();			// Clock 1
	T(DummyFetch)();			// Clock 2
	T(WriteStack)(_PCH);			// Clock 3
	T(WriteStack)(_PCL);			// Clock 4
	T(PackFlags)();
	T(WriteStack)(_P);			// Clock 5
	uint16 vector;
	if(type == COREInterruptNMI) {
		vector = COREInterruptVectorNMI;
	} else {
		// BRK and IRQ set the I flag, while NMI does not.
		SetFlag(_IF, true);		// Clock (?)
		vector = COREInterruptVectorIRQ;
	}	

	_PC = T(ReadWord)(vector);		// Clocks 6-7

#if defined(COREFast)
	// Adjust the clock counter manually.
	core.time += timeStep * 7;
#endif
}
