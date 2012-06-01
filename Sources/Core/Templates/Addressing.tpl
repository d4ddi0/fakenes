/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

// This is used for passing an index register to be used as an offset.
#define WithIndex	const uint8& index

/* This is used for passing an instruction template within an R-M-W sequence. The instruction
   template is expected to modify the data, then return a copy of it. For efficiency, the
   instruction template is embedded directly into the addressing mode template. */
#define WithInstruction template<uint8 (instruction)(uint8 data)>

/* This is used for passing an instruction template for the relative addressing mode. The
   instruction template determines whether a branch will be taken or not. Again, the instruction
   template is embedded directly into the addressing mode template. */
#define WithInstructor	template<bool (instructor)()>

// This is used for passing a register to be used as a data source.
#define register	_register
#define WithRegister	const uint8& register
// This is used for returning general-purpose data.
#define FromData	uint8
// This is used for returning registers.
#define FromRegister	const uint8&

// This returns true if a wrap-around will occur (A+B > $FF).
express_function constant_function bool T(Wrap)(const uint8 a, const uint8 b) {
	return (a + b) < a;
}

/* Note:
   Clocks with a plus sign (+) next to them are executed after the clock counter has already been
   incremented (e.g via reads or writes), but are still technically executing on the same clock.
   This is a simplification of the pipeline, and is entirely transparent.

   In other words, only reads and writes can affect the clock counter. Everything else is either
   before or after them in order, executing simultaneously (from a software viewpoint). */

/* Table of valid accessors for each addressing mode:
	-MODE-		-READ-	-RMW-	-WRITE-
	Implied		NA	NA	NA
	Accmulator	no	YES	no
	Absolute	YES	YES	YES
	Aboslute,X	YES	YES	YES
	Absolute,Y	YES	no	YES
	Immediate	YES	no	no
	Indirect,X	YES	yes*	YES
	Indirect,Y	YES	yes*	YES
	Relative	NA	NA	NA
	Zero Page	YES	YES	YES
	Zero Page,X	YES	YES	YES
	Zero Page,Y	YES	no	YES

   * This can only occur with undocumented opcodes.
*/

/* Accumulator or implied addressing:
    #  address R/W description
    2    PC     R  read next instruction byte (and throw it away) */
express_function void T(AddressImplied)() {
	T(DummyRead)(_PC);	// Clock 2
}

WithInstruction void T(AddressAccumulatorRMW)() {
	T(DummyRead)(_PC);	// Clock 2
	_A = instruction(_A);	// Clock 2+
}

/* Absolute addressing (Read):
    #  address R/W description
    2    PC     R  fetch low byte of address, increment PC
    3    PC     R  fetch high byte of address, increment PC
    4  address  R  read from effective address */
express_function FromData T(AddressAbsoluteRead)() {
	const uint16 address = T(FetchWord)();	// Clocks 2-3
	return T(Read)(address);		// Clock 4
}

/* Absolute addressing (Read-Modify-Write):
    #  address R/W description
    2    PC     R  fetch low byte of address, increment PC
    3    PC     R  fetch high byte of address, increment PC
    4  address  R  read from effective address
    5  address  W  write the value back to effective address, and do the operation on it
    6  address  W  write the new value to effective address */
WithInstruction void T(AddressAbsoluteRMW)() {
	const uint16 address = T(FetchWord)();	// Clocks 2-3
	uint8 data = T(Read)(address);		// Clock 4
	T(DummyWrite)(address, data);		// Clock 5
	data = instruction(data);		// Clock 5+
	T(Write)(address, data);		// Clock 6
}

/* Absolute addressing (Write):
    #  address R/W description
    2    PC     R  fetch low byte of address, increment PC
    3    PC     R  fetch high byte of address, increment PC
    4  address  W  write register to effective address */
express_function void T(AddressAbsoluteWrite)(WithRegister) {
	const uint16 address = T(FetchWord)();	// Clocks 2-3
	T(Write)(address, register);		// Clock 4
}

/* Absolute indexed addressing (Read):
    #   address  R/W description
    2     PC      R  fetch low byte of address, increment PC
    3     PC      R  fetch high byte of address, add index register to low address byte, increment PC
    4  address+I* R  read from effective address, fix the high byte of effective address
    5+ address+I  R  re-read from effective address */
express_function FromData T(AddressAbsoluteIndexedRead)(WithRegister) {
	byte_pair address;
	address.word = T(FetchWord)();		// Clocks 2-3
	if(T(Wrap)(address.bytes.low, register)) {
		uint16 savedAddress = address.word;
#if !defined(COREFast)
		address.bytes.low += register;	// Clock 3+
#endif
		T(DummyRead)(address.word);	// Clock 4
		savedAddress += register;	// Clock 4+
		return T(Read)(savedAddress);	// Clock 5
	} else {
		address.bytes.low += register;	// Clock 3
		return T(Read)(address.word);	// Clock 4
	}
}

express_function FromData T(AddressAbsoluteXRead)() { return T(AddressAbsoluteIndexedRead)(_X); }
express_function FromData T(AddressAbsoluteYRead)() { return T(AddressAbsoluteIndexedRead)(_Y); }

/* Absolute indexed addressing (Read-Modify-Write):
    #   address  R/W description
    2    PC       R  fetch low byte of address, increment PC
    3    PC       R  fetch high byte of address, add index register X to low address byte, increment PC
    4  address+X* R  read from effective address, fix the high byte of effective address
    5  address+X  R  re-read from effective address
    6  address+X  W  write the value back to effective address, and do the operation on it
    7  address+X  W  write the new value to effective address */
WithInstruction void T(AddressAbsoluteIndexedRMW)() {
	byte_pair address;
	address.word = T(FetchWord)();		// Clocks 2-3
	uint16 savedAddress = address.word;
#if !defined(COREFast)
	address.bytes.low += _X;		// Clock 3+
#endif
	T(DummyRead)(address.word);		// Clock 4
	savedAddress += _X;			// Clock 4+
	uint8 data = T(Read)(savedAddress);	// Clock 5
	T(DummyWrite)(savedAddress, data);	// Clock 6
	data = instruction(data);		// Clock 6+
	T(Write)(savedAddress, data);		// Clock 7
}

// Only the Absolute,X mode is avaiable for R-M-W instructions.
WithInstruction void T(AddressAbsoluteXRMW)() { T(AddressAbsoluteIndexedRMW)<instruction>(); }

/* Absolute indexed addressing (Write):
    #   address  R/W description
    2     PC      R  fetch low byte of address, increment PC
    3     PC      R  fetch high byte of address, add index register to low address byte, increment PC
    4  address+I* R  read from effective address, fix the high byte of effective address
    5  address+I  W  write to effective address */
express_function void T(AddressAbsoluteIndexedWrite)(WithIndex, WithRegister) {
	byte_pair address;
	address.word = T(FetchWord)();		// Clocks 2-3
	uint16 savedAddress = address.word;
#if !defined(COREFast)
	address.bytes.low += index;		// Clock 3+
#endif
	T(DummyRead)(address.word);		// Clock 4
	savedAddress += index;			// Clock 4+
	T(Write)(savedAddress, register);	// Clock 5
}

express_function void T(AddressAbsoluteXWrite)(WithRegister) { T(AddressAbsoluteIndexedWrite)(_X, register); }
express_function void T(AddressAbsoluteYWrite)(WithRegister) { T(AddressAbsoluteIndexedWrite)(_Y, register); }

/* Immediate addressing (Read):
   #  address R/W description
   2    PC     R  fetch value, increment PC */
express_function FromData T(AddressImmediateRead)() {
	return T(Fetch)();	// Clock 2
}

/* Indexed indirect addressing (Read):
    #    address   R/W description
    2      PC       R  fetch pointer address, increment PC
    3    pointer    R  read from the address, add X to it
    4   pointer+X   R  fetch effective address low
    5  pointer+X+1  R  fetch effective address high
    6    address    R  read from effective address */
express_function FromData T(AddressIndexedIndirectRead)() {
	uint8 pointer = T(Fetch)();				// Clock 2
	T(DummyRead)(pointer);					// Clock 3
	pointer += _X;						// Clock 3+
	const uint16 address = T(ReadZeroPageWord)(pointer);	// Clocks 4-5
	return T(Read)(address);				// Clock 6
}

express_function FromData T(AddressIndirectXRead)() { return T(AddressIndexedIndirectRead)(); }

/* Indexed indirect addressing (Read-Modify-Write):
    #    address   R/W description
    2      PC       R  fetch pointer address, increment PC
    3    pointer    R  read from the address, add X to it
    4   pointer+X   R  fetch effective address low
    5  pointer+X+1  R  fetch effective address high
    6    address    R  read from effective address
    7    address    W  write the value back to effective address, and do the operation on it
    8    address    W  write the new value to effective address */
WithInstruction void T(AddressIndexedIndirectRMW)() {
	uint8 pointer = T(Fetch)();				// Clock 2
	T(DummyRead)(pointer);					// Clock 3
	pointer += _X;						// Clock 3+
	const uint16 address = T(ReadZeroPageWord)(pointer);	// Clocks 4-5
	uint8 data = T(Read)(address);				// Clock 6
	T(DummyWrite)(address, data);				// Clock 7
	data = instruction(data);				// Clock 7+
	T(Write)(address, data);				// Clock 8
}

WithInstruction void T(AddressIndirectXRMW)() { T(AddressIndexedIndirectRMW)<instruction>(); }

/* Indexed indirect addressing (Write):
    #    address   R/W description
    2      PC       R  fetch pointer address, increment PC
    3    pointer    R  read from the address, add X to it
    4   pointer+X   R  fetch effective address low
    5  pointer+X+1  R  fetch effective address high
    6    address    W  write to effective address */
express_function void T(AddressIndexedIndirectWrite)(WithRegister) {
	uint8 pointer = T(Fetch)();				// Clock 2
	T(DummyRead)(pointer);					// Clock 3
	pointer += _X;						// Clock 3+
	const uint16 address = T(ReadZeroPageWord)(pointer);	// Clocks 4-5
	T(Write)(address, register);				// Clock 6
}

express_function void T(AddressIndirectXWrite)(WithRegister) { T(AddressIndexedIndirectWrite)(register); }

/* Indirect indexed addressing (Read):
    #    address   R/W description
    2      PC       R  fetch pointer address, increment PC
    3    pointer    R  fetch effective address low
    4   pointer+1   R  fetch effective address high, add Y to low byte of effective address
    5   address+Y*  R  read from effective address, fix high byte of effective address
    6+  address+Y   R  read from effective address */
express_function FromData T(AddressIndirectIndexedRead)() {
	const uint8 pointer = T(Fetch)();		// Clock 2
	byte_pair address;
	address.word = T(ReadZeroPageWord)(pointer);	// Clocks 3-4
	if(T(Wrap)(address.bytes.low, _Y)) {
		uint16 savedAddress = address.word;
#if !defined(COREFast)
		address.bytes.low += _Y;		// Clock 4+
#endif
		T(DummyRead)(address.word);		// Clock 5
		savedAddress += _Y;			// Clock 5+
		return T(Read)(savedAddress);		// Clock 6
	} else {
		address.bytes.low += _Y;		// Clock 4+
		return T(Read)(address.word);		// Clock 5
	}
}

express_function FromData T(AddressIndirectYRead)() { return T(AddressIndirectIndexedRead)(); }

/* Indirect indexed addressing (Read-Modify-Write):
    #    address   R/W description
    2      PC       R  fetch pointer address, increment PC
    3    pointer    R  fetch effective address low
    4   pointer+1   R  fetch effective address high, add Y to low byte of effective address
    5   address+Y*  R  read from effective address, fix high byte of effective address
    6   address+Y   R  read from effective address
    7   address+Y   W  write the value back to effective address, and do the operation on it
    8   address+Y   W  write the new value to effective address */
WithInstruction void T(AddressIndirectIndexedRMW)() {
	const uint8 pointer = T(Fetch)();		// Clock 2
	byte_pair address;
	address.word = T(ReadZeroPageWord)(pointer);	// Clocks 3-4
	uint16 savedAddress = address.word;
#if !defined(COREFast)
	address.bytes.low += _Y;			// Clock 4+
#endif
	T(DummyRead)(address.word);			// Clock 5
	savedAddress += _Y;				// Clock 5+
	uint8 data = T(Read)(savedAddress);		// Clock 6
	T(DummyWrite)(savedAddress, data);		// Clock 7
	data = instruction(data);			// Clock 7+
	T(Write)(savedAddress, data);			// Clock 8
}

WithInstruction void T(AddressIndirectYRMW)() { T(AddressIndirectIndexedRMW)<instruction>(); }

/* Indirect indexed addressing (Write): 
    #    address   R/W description
    2      PC       R  fetch pointer address, increment PC
    3    pointer    R  fetch effective address low
    4   pointer+1   R  fetch effective address high, add Y to low byte of effective address
    5   address+Y*  R  read from effective address, fix high byte of effective address
    6   address+Y   W  write to effective address */
express_function void T(AddressIndirectIndexedWrite)(WithRegister) {
	const uint8 pointer = T(Fetch)();		// Clock 2
	byte_pair address;
	address.word = T(ReadZeroPageWord)(pointer);	// Clocks 3-4
	uint16 savedAddress = address.word;
#if !defined(COREFast)
	address.bytes.low += _Y;			// Clock 4+
#endif
	T(DummyRead)(address.word);			// Clock 5
	savedAddress += _Y;				// Clock 5+
	T(Write)(savedAddress, register);		// Clock 6
}

express_function void T(AddressIndirectYWrite)(WithRegister) { T(AddressIndirectIndexedWrite)(register); }

/* Relative addressing:
    #   address  R/W description
    2     PC      R  fetch operand, increment PC
    3     PC      R  Fetch opcode of next instruction, If branch is taken, add operand to PCL. Otherwise increment PC.
    4+    PC*     R  Fetch opcode of next instruction. Fix PCH. If it did not change, increment PC.
    5!    PC      R  Fetch opcode of next instruction, increment PC. */
WithInstructor void T(AddressRelative)() {
	const uint8 operand = T(Fetch)();	// Clock 2
	T(DummyRead)(_PC);			// Clock 3
	if(instructor()) {
		const uint16 savedAddress = _PC;
		_PCL += operand;		// Clock 3+
		T(DummyRead)(_PC);		// Clock 4
		const uint8 highByte = _PCH;
		_PC = savedAddress + operand;	// Clock 4+
		if(_PCH == highByte)
			_PC++;			// Clock 4+
	} else {
		_PC += 2;			// Clocks 3-4
	}

	T(DummyFetch)();			// Clock 5
}

/* Zero page addressing (Read):
    #  address R/W description
    2    PC     R  fetch address, increment PC
    3  address  R  read from effective address */
express_function FromData T(AddressZeroPageRead)() {
	const uint8 address = T(Fetch)();	// Clock 2
	return T(ReadZeroPage)(address);	// Clock 3
}

/* Zero page addressing (Read-Modify-Write):
    #  address R/W description
    2    PC     R  fetch address, increment PC
    3  address  R  read from effective address
    4  address  W  write the value back to effective address, and do the operation on it
    5  address  W  write the new value to effective address */
WithInstruction void T(AddressZeroPageRMW)() {
	const uint8 address = T(Fetch)();	// Clock 2
	uint8 data = T(ReadZeroPage)(address);	// Clock 3
	T(DummyWrite)(address, data);		// Clock 4
	data = instruction(data);		// Clock 4+
	T(WriteZeroPage)(address, data);	// Clock 5
}

/* Zero page addressing (Write):
    #  address R/W description
    2    PC     R  fetch address, increment PC
    3  address  W  write register to effective address */
express_function void T(AddressZeroPageWrite)(WithRegister) {
	const uint8 address = T(Fetch)();	// Clock 2
	T(WriteZeroPage)(address, register);	// Clock 3
}

/* Zero page indexed addressing (Read):
    #   address  R/W description
    2     PC      R  fetch address, increment PC
    3   address   R  read from address, add index register to it
    4  address+I* R  read from effective address */
express_function FromData T(AddressZeroPageIndexedRead)(WithRegister) {
	uint8 address = T(Fetch)();		// Clock 2
	T(DummyRead)(address);			// Clock 3
	address += register;			// Clock 3+
	return T(ReadZeroPage)(address);	// Clock 4
}

express_function FromData T(AddressZeroPageXRead)() { return T(AddressZeroPageIndexedRead)(_X); }
express_function FromData T(AddressZeroPageYRead)() { return T(AddressZeroPageIndexedRead)(_Y); }

/* Zero page indexed addressing (Read-Modify-Write):
    #   address  R/W description
    2     PC      R  fetch address, increment PC
    3   address   R  read from address, add index register X to it
    4  address+X* R  read from effective address
    5  address+X* W  write the value back to effective address, and do the operation on it
    6  address+X* W  write the new value to effective address */
WithInstruction void T(AddressZeroPageIndexedRMW)() {
	uint8 address = T(Fetch)();		// Clock 2
	T(DummyRead)(address);			// Clock 3
	address += _X;				// Clock 3+
	uint8 data = T(ReadZeroPage)(address);	// Clock 4
	T(DummyWrite)(address, data);		// Clock 5
	data = instruction(data);		// Clock 5+
	T(WriteZeroPage)(address, data);	// Clock 6
}

// Only the Zero Page,X mode is avaiable for R-M-W instructions.
WithInstruction void T(AddressZeroPageXRMW)() { T(AddressZeroPageIndexedRMW)<instruction>(); }

/* Zero page addressing (Write):
    #   address  R/W description
    2     PC      R  fetch address, increment PC
    3   address   R  read from address, add index register to it
    4  address+I* W  write to effective address */
express_function void T(AddressZeroPageIndexedWrite)(WithIndex, WithRegister) {
	uint8 address = T(Fetch)();		// Clock 2
	T(DummyRead)(address);			// Clock 3
	address += index;			// Clock 3+
	T(WriteZeroPage)(address, register);	// Clock 4
}

express_function void T(AddressZeroPageXWrite)(WithRegister) { T(AddressZeroPageIndexedWrite)(_X, register); }
express_function void T(AddressZeroPageYWrite)(WithRegister) { T(AddressZeroPageIndexedWrite)(_Y, register); }

// Clean up the namespace.
#undef WithIndex
#undef WithInstruction
#undef WithInstructor
#undef WithRegister

#undef FromData
#undef FromRegister
