/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.*/

/* Note:
   Each read or write takes a single clock to complete, including stack access.
   Because of this, a word (16-bit) read or write takes two clocks to complete.

   During these times, the CPU can also perform a limited amount of other tasks,
   such as incrementing the program counter, or stack pointer. So these do not affect
   the clock counter directly, only by the memory access that occur on the same clock.
 
   For synchronization with other system components to work properly (the APU and PPU),
   the clock counter *must* be incremented after every operation, as external devices
   perform "catch up" on their state at the exact moment of any read or write to
   their address space. For example, if performing a 16-bit read, the clock counter
   must be incremented after each individual byte read. */

/* Generates a single clock, and updates the APU and PPU in asynchronous mode. All
   write access to the clock counter has to go through this. */
quick void T(Clock)() {
#if !defined(COREFast)
	core.time += timeStep;
#endif

#if defined(COREAsynchronous)
	apu_execute(timeStep);
	ppu_execute(timeStep);
#endif
}

/* Fetches a byte from the address contained in PC, then increments PC.
    Time taken: One clock. */
quick uint8 T(Fetch)() {
	const uint8 data = cpu_read(_PC);
	_PC++;
	T(Clock)();
	return data;
}

/* Fetches both bytes of a 16-bit word. The low byte is fetched first.
    Time taken: Two clocks. */
quick uint16 T(FetchWord)() {
	pair data;
	data.low = T(Fetch)();
	data.high = T(Fetch)();
	return data.word;
}

/* Fetches a byte from the address contained in PC, throws it away, and increments PC.
   If COREFast is defined, the fetch itself is eliminated.
    Time taken: One clock. */
quick void T(DummyFetch)() {
#if !defined(COREFast)
	cpu_read(_PC);
#endif
	_PC++;
	T(Clock)();
}

/* Reads a byte from an arbitrary address.
    Time taken: One clock. */
quick uint8 T(Read)(const uint16 address) {
	const uint8 data = cpu_read(address);
	T(Clock)();
	return data;
}

/* Reads both bytes of a 16-bit word. The low byte is read first.
    Time taken: Two clocks. */
quick uint16 T(ReadWord)(const uint16 address) {
 	pair data;
	data.bytes.low = T(Read)(address);
	data.bytes.high = T(Read)(address + 1);
	return data.word;
}

/* Reads a byte from an arbitrary address, then throws it away. If "COREFast" is
   defined, the read itself is eliminated.
    Time taken: One clock. */
quick void T(DummyRead)(const uint16 address) {
#if !defined(COREFast)
	cpu_read(address);
#endif
	T(Clock)();
}

/* Writes a byte to an arbitrary address.
    Time taken: One clock. */
quick void T(Write)(const uint16 address, const uint8 data) {
	cpu_write(address, data);
	T(Clock)();
}

/* Writes a byte to an arbitrary address. If "COREFast" is defined, the write
   itself is eliminated.
    Time taken: One clock. */
quick void T(DummyWrite)(const uint16 address, const uint8 data) {
#if !defined(COREFast)
	cpu_write(address, data);
#endif
	T(Clock)();
}

/* Reads a byte from the stack, using the stack pointer S. Note that the clock
   occurs after incrementing the stack pointer, before the read.
    Time taken: One clock+. */
quick uint8 T(ReadStack)() {
	_S++;
	T(Clock)();
	const uint8 data = cpu_read_direct(0x0100 + _S);
	return data;
}

/* Writes a byte to the stack, using the stack pointer S.
    Time taken: One clock. */
quick void T(WriteStack)(const uint8 data) {
	cpu_write_direct(0x0100 + _S, data);
	_S--;
	T(Clock)();
}

/* Reads a byte from zero page ($00-$FF). If an address larger than $FF is passed,
   it will be wrapped during the conversion to 8-bit.
    Time taken: One clock. */
quick uint8 T(ReadZeroPage)(const uint8 address) {
	const uint8 data = cpu_read_direct(address);
	T(Clock)();
	return data;
}

/* Reads both bytes of a 16-bit word from zero page ($00-$FF). If an address larger
   than $FF is passed, it will be wrapped during the conversion to 8-bit.
   As always, the low byte is read first.
    Time taken: Two clocks. */
quick uint16 T(ReadZeroPageWord)(const uint8 address) {
 	pair data;
	data.bytes.low = T(ReadZeroPage)(address);
	data.bytes.high = T(ReadZeroPage)(address + 1);
	return data.word;
}
/* Writes a byte to zero page ($00-$FF). If an address larger than $FF is passed,
   it will be wrapped during the conversion to 8-bit.
    Time taken: One clock. */
quick void T(WriteZeroPage)(const uint8 address, const uint8 data) {
	cpu_write_direct(address, data);
	T(Clock)();
}
