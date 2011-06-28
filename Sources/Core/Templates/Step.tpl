/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.*/

// This executes a single instruction or interrupt.
quick void T(Step)() {
	// Check if an interupt is pending.
	if(T(InterruptPending)()) {
		// Get the profile of the first interrupt in the queue.
		const COREInterrupt& interrupt = core.interrupts.front();

		/* In order for the interrupt to be handled before the next
                   instruction, it needs to have occured *before* the last clock of
                   the previous instruction. */
		const CORETime lastClock = core.time - (1 * timeStep);
		if(interrupt.time < lastClock) {
			// Execute the interrupt sequence.
			T(Interrupt)(interrupt.type);
			// Delay instruction processing until the next call to Step().
			return;
		}
	}

	// Once interrupt processing is finished, we can safely handle CLI.
	if(core.afterCLI) {
		SetFlag(_IF, false);
		core.afterCLI = false;
	}

#if defined(COREDebug)
	// Save the current timestamp so we can verify the execution time later.
	const CORETime timestamp = core.time;
#endif

	// Fetch a single opcode and execute it.
	const uint8 opcode = T(Fetch)();
	T(ParseOpcode)(opcode);

#if defined(COREDebug)
	const CORETime time = (CORETimeDelta)core.time - (CORETimeDelta)timestamp;
	const CORETime idealTime = timeTable[opcode];
	
	if(time != idealTime) {
		log_printf("CORE: WARNING: Opcode $%02X took %d master clocks to execute, "
                           "but the ideal execution time was %d master clocks.\n", time, idealTime);
	}
#endif
}
