/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.*/

/* This allows instruction code to be embedded within addressing mode templates.
   It is only needed for branch and R-M-W instructions. */
#define Embeddable	extern inline

#define WithData	const uint8 data	// Accept read-only data.
#define WithFlag	uint8& flag		// Accept a flag.
#define WithRegister	uint8& register		// Accept a register.
#define WithRegister1	uint8& register1	// Accept two registers (one of two).
#define WithRegister2	uint8& register2	// Accept two registers (two of two).
#define WithVariable	uint8 variable		// Accept modifiable data (R-M-W).

#define FromRegister	const uint8&		// Return a register.
#define FromVariable	uint8			// Return the modified data (R-M-W).

// Define some bit-masks to make things appear a little cleaner.
#define Bit0	_00000001b
#define Bit6	_01000000b
#define Bit7	_10000000b

/* Note:
   Clocks with a plus sign (+) next to them are executed after the clock counter has already been
   incremented (e.g via reads or writes), but are still technically executing on the same clock.
   This is a simplification of the pipeline, and is entirely transparent.

   In other words, only reads and writes can affect the clock counter. Everything else is either
   before or after them in order, executing simultaneously (from a software viewpoint).

   Clocks with a minus sign (-) next to them are part of a stack read; the clock is generated in
   the middle of the call to ReadStack(), after S has been incremented but before the read itself.
   This essentially means that stack reads end halfway through the next clock. */

/* *********** */
/* *** ADC *** */
/* *********** */
// Note: Decimal mode is not supported by the 2A03, and thus is not implemented here.
quick void T(InstructionADC)(WithData) {
	const uint16 result = data + _A + _CF;
	const uint8 masked = result & 0xFF;
	T(UpdateCF)(result > 0xFF);
	T(UpdateZF)(masked);
	T(UpdateVF)(!((_A ^ data) & Bit7) && ((_A ^ result) & Bit7));
	T(UpdateNF)(result);
	_A = masked;
}

/* *********** */
/* *** AND *** */
/* *********** */
quick void T(InstructionAND)(WithData) {
	_A = data & _A;
	T(UpdateZF)(_A);
	T(UpdateNF)(_A);
}

/* *********** */
/* *** ASL *** */
/* *********** */
Embeddable FromVariable T(InstructionASL)(WithVariable) {
	T(UpdateCF)(variable & Bit7);
	variable <<= 1;
	T(UpdateZF)(variable);
	T(UpdateNF)(variable);
	return variable;
}

/* ********************************************** */
/* *** BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS *** */
/* ********************************************** */
/* The actual logic is for branching handled by the relative addressing mode template; these
   instruction templates simply tell it whether to take the branch or not. */
Embeddable bool T(InstructionBxx)(const WithFlag, const uint8 state) {
	return (flag == state);
}

Embeddable bool T(InstructionBCC)() { T(InstructionBxx)(_CF, 0); }	// Carry clear.
Embeddable bool T(InstructionBCS)() { T(InstructionBxx)(_CF, 1); }	// Carry set.
Embeddable bool T(InstructionBNE)() { T(InstructionBxx)(_ZF, 0); }	// Zero clear.
Embeddable bool T(InstructionBEQ)() { T(InstructionBxx)(_ZF, 1); }	// Zero set.
Embeddable bool T(InstructionBPL)() { T(InstructionBxx)(_NF, 0); }	// Negative clear.
Embeddable bool T(InstructionBMI)() { T(InstructionBxx)(_NF, 1); }	// Negative set.
Embeddable bool T(InstructionBVC)() { T(InstructionBxx)(_VF, 0); }	// Overflow clear.
Embeddable bool T(InstructionBVS)() { T(InstructionBxx)(_VF, 1); }	// Overflow set.

/* *********** */
/* *** BIT *** */
/* *********** */
quick void T(InstructionBIT)(WithData) {
	T(UpdateZF)(data & _A);
	T(UpdateVF)(Bit6 & data);
	T(UpdateNF)(data);
}

/* *********** */
/* *** BRK *** */
/* *********** */
/* If a hardware interrupt (NMI or IRQ) occurs before the fourth (flags aving) cycle
   of BRK, the BRK instruction will be skipped, and the processor will jump to the
   hardware interrupt vector. This sequence will always take 7 cycles. */

/* #  address R/W description
   3  $0100,S  W  push PCH on stack, decrement S
   4  $0100,S  W  push PCL on stack, decrement S
   5  $0100,S  W  push P on stack (with B flag set), decrement S
   6   $FFFE   R  fetch PCL
   7   $FFFF   R  fetch PCH */
quick void T(InstructionBRK)() {
	T(WriteStack)(_PCH);			// Clock 3
	T(WriteStack)(_PCL);			// Clock 4
	const bool interrupted = T(InterruptPending)();
	T(PackFlags)();
	T(WriteStack)(_P | COREFlagBreak);	// Clock 5
	// BRK and IRQ both set the I flag.
	SetFlag(_IF, true);			// Clock (?)
	uint16 vector = COREInterruptVectorIRQ;
	if(interrupted) {
		/* IRQ and BRK behave identically, aside from the B flag being set,
                   so there is nothing special that needs to be done. However, if the
                   interrupt was an NMI, we have to override the vector. */
		if(core.interrupts.first().type == COREInterruptNMI) {
			// Since the NMI is actually occuring here, clear it.
			ClearInterrupt(COREInterruptNMI);
			// Redirect the interrupt vector to $FFFA.
			vector = COREInterruptVectorNMI;
		}
	}

	_PC = T(ReadWord)(vector);		// Clocks 6-7
}

/* ************************** */
/* *** CLC, CLD, CLI, CLV *** */
/* ************************** */
quick void T(FlagCLx)(WithFlag) {
	SetFlag(flag, false);
}

quick void T(InstructionCLC)() { T(FlagCLx)(_CF); }
quick void T(InstructionCLD)() { T(FlagCLx)(_DF); }
quick void T(InstructionCLV)() { T(FlagCLx)(_VF); }

// The effect of CLI is delayed by one instruction. See Core.hpp for more information.
quick void T(InstructionCLI)() {
	core.afterCLI = true;
}

/* ********************* */
/* *** CMP, CPX, CPY *** */
/* ********************* */
quick void T(RegisterCPx)(const WithRegister, WithData) {
	const uint16 result = register - data;
	T(UpdateZF)(result & 0xFF);
	T(UpdateNF)(result);
	T(UpdateCF)(result < 0x100);
}

quick void T(InstructionCMP)(WithData) { T(RegisterCPx)(_A, data); }
quick void T(InstructionCPX)(WithData) { T(RegisterCPx)(_X, data); }
quick void T(InstructionCPY)(WithData) { T(RegisterCPx)(_Y, data); }

/* ********************* */
/* *** DEC, DEX, DEY *** */
/* ********************* */
Embeddable FromVariable T(InstructionDEC)(WithVariable) {
	variable--;
	T(UpdateZF)(variable);
	T(UpdateNF)(variable);
	return variable;
}

quick void T(RegisterDEx)(WithRegister) {
	register--;
	T(UpdateZF)(register);
	T(UpdateNF)(register);
}

quick void T(InstructionDEX)() { T(RegisterDEx)(_X); }
quick void T(InstructionDEY)() { T(RegisterDEx)(_Y); }

/* *********** */
/* *** EOR *** */
/* *********** */
quick void T(InstructionEOR)(WithData) {
    	_A = data ^ _A;
	T(UpdateZF)(_A);
	T(UpdateNF)(_A);
}

/* ********************* */
/* *** INC, INX, INY *** */
/* ********************* */
Embeddable FromVariable T(InstructionINC)(WithVariable) {
	variable++;
	T(UpdateZF)(variable);
	T(UpdateNF)(variable);
	return variable;
}

quick void T(RegisterINx)(WithRegister) {
	register++;
	T(UpdateZF)(register);
	T(UpdateNF)(register);
}

quick void T(InstructionINX)() { T(RegisterINx)(_X); }
quick void T(InstructionINY)() { T(RegisterINx)(_Y); }

/* *********** */
/* *** JMP *** */
/* *********** */
/* Absolute jump:
    #  address R/W description
    2    PC     R  fetch low address byte, increment PC
    3    PC     R  copy low address byte to PCL, fetch high address byte to PCH */
quick void T(InstructionJMP1)() {
	const uint8 lowByte = T(Fetch)();	// Clock 2
	_PCL = lowByte;				// Clock 3
	_PCH = T(Read)(_PC);			// Clock 3
}

/* Absolute indirect jump:
    #   address  R/W description
    2     PC      R  fetch pointer address low, increment PC
    3     PC      R  fetch pointer address high, increment PC
    4   pointer   R  fetch low address to latch
    5  pointer+1* R  fetch PCH, copy latch to PCL */
quick void T(InstructionJMP2)() {
	pair pointer;
	pointer.bytes.low = T(Fetch)();			// Clock 2
	pointer.bytes.high = T(Fetch)();		// Clock 3
	const uint8 latch = T(Read)(pointer.word);	// Clock 4
	_PCH = T(Read)(pointer.word + 1);		// Clock 5
	_PCL = latch;					// Clock 5
}

/* *********** */
/* *** JSR *** */
/* *********** */
/* #  address R/W description
   2    PC     R  fetch low address byte, increment PC
   3  $0100,S  R  internal operation (predecrement S?)
   4  $0100,S  W  push PCH on stack, decrement S
   5  $0100,S  W  push PCL on stack, decrement S
   6    PC     R  copy low address byte to PCL, fetch high address byte to PCH
quick void T(InstructionJSR)() {	
	const uint8 lowByte = T(Fetch)();	// Clock 2
	T(Clock)();				// Clock 3
	T(WriteStack)(_PCH);			// Clock 4
	T(WriteStack)(_PCL);			// Clock 5
	_PCL = lowByte;				// Clock 6
	_PCH = T(Read)(_PC);			// Clock 6
}

/* ********************* */
/* *** LDA, LDX, LDY *** */
/* ********************* */
quick void T(RegisterLDx)(WithRegister, WithData) {
	register = data;
	T(UpdateZF)(register);
	T(UpdateNF)(register);
}

quick void T(InstructionLDA)(WithData) { T(RegisterLDx)(_A, data); }
quick void T(InstructionLDX)(WithData) { T(RegisterLDx)(_X, data); }
quick void T(InstructionLDY)(WithData) { T(RegisterLDx)(_Y, data); }

/* *********** */
/* *** LSR *** */
/* *********** */
Embeddable FromVariable T(InstructionLSR)(WithVariable) {
	T(UpdateCF)(variable & Bit0);
	variable >>= 1;
	T(UpdateZF)(variable);
	T(UpdateNF)(variable);
	return variable;
}

/* *********** */
/* *** NOP *** */
/* *********** */
quick void T(InstructionNOP)() {
}

/* *********** */
/* *** ORA *** */
/* *********** */
quick void T(InstructionORA)(WithData) {
	_A = data | _A;
	T(UpdateZF)(_A);
	T(UpdateNF)(_A);
}

/* **************** */
/* *** PHA, PHP *** */
/* **************** */
/* #  address R/W description
   3  $0100,S  W  push register on stack, decrement S */
quick void T(RegisterPHx)(const WithRegister) {
	T(WriteStack)(register);	// Clock 3
}

quick void T(InstructionPHA)() {
	T(RegisterPHx)(_A);
}

/* From blargg:
    PHP and BRK both always set bits 4 and 5 of the byte they push on the stack. NMI and IRQ
    both always clear bit 4 and set bit 5 of the byte they push on the stack. Thus, the
    status flags register only has 6 bits, not 8. */
quick void T(InstructionPHP)() {
	T(PackFlags)();
	T(RegisterPHx)(_P | COREFlagBreak); 
}

/* **************** */
/* *** PLA, PLP *** */
/* **************** */
/*  #  address R/W description
    3  $0100,S  R  increment S
    4  $0100,S  R  pull register from stack */
quick void T(RegisterPLx)(WithRegister) {
	register = T(ReadStack)();	// Clock 3,4-
	T(Clock)();			// Clock 4
}

quick void T(InstructionPLA)() {
	T(RegisterPLx)(_A);
	T(UpdateZF)(_A);
	T(UpdateNF)(_A);
}

quick void T(InstructionPLP)() {	
	T(RegisterPLx)(_P);
	T(UnpackFlags)();
}

/* **************** */
/* *** ROL, ROR *** */
/* **************** */
Embeddable FromVariable T(InstructionROL)(WithVariable) {
	const uint16 result = (variable << 1) | _CF;
	variable = result & 0xFF;
	T(UpdateCF)(result > 0xFF);
	T(UpdateZF)(variable);
	T(UpdateNF)(variable);
	return variable;
}

Embeddable FromVariable T(InstructionROR)(WithVariable) {
	const bool carry = _CF;
	T(UpdateCF)(variable & Bit0);
	variable >>= 1;
	variable |= carry ? Bit7 : 0;
	T(UpdateZF)(variable);
	T(UpdateNF)(variable);
	return variable;
}

/* *********** */
/* *** RTI *** */
/* *********** */
/* #  address R/W description
   3  $0100,S  R  increment S
   4  $0100,S  R  pull P from stack, increment S
   5  $0100,S  R  pull PCL from stack, increment S
   6  $0100,S  R  pull PCH from stack */
quick void T(InstructionRTI)() {
	_P = T(ReadStack)();		// Clock 3,4-
	T(UnpackFlags)();
	_PCL = T(ReadStack)();		// Clock 4,5-
	_PCH = T(ReadStack)();		// Clock 5,6-
	T(Clock)();			// Clock 6
}

/* *********** */
/* *** RTS *** */
/* *********** */
/* #  address R/W description
   3  $0100,S  R  increment S
   4  $0100,S  R  pull PCL from stack, increment S
   5  $0100,S  R  pull PCH from stack
   6    PC     R  increment PC */
quick void T(InstructionRTS)() {
	_PCL = T(ReadStack)();		// Clock 3,4-
	_PCH = T(ReadStack)();		// Clock 4,5-
	T(Clock)();			// Clock 5
	_PC++;				// Clock 6
}

/* *********** */
/* *** SBC *** */
/* *********** */
// Note: Decimal mode is not supported by the 2A03, and thus is not implemented here.
quick void T(InstructionSBC)(WithData) {
	const uint16 result = _A - data - (_CF ? 0 : 1);
	const uint8 masked = result & 0xFF;
	T(UpdateCF)(result < 0x100);
	T(UpdateZF)(masked);
	T(UpdateVF)(((_A ^ result) & Bit7) && ((_A ^ data) & Bit7));
	T(UpdateNF)(result);
	_A = masked;
}
 
/* ********************* */
/* *** SEC, SED, SEI *** */
/* ********************* */
quick void T(FlagSEx)(WithFlag) {
	SetFlag(flag, true);
}

quick void T(InstructionSEC)() { T(FlagSEx)(_CF); }
quick void T(InstructionSED)() { T(FlagSEx)(_DF); }
quick void T(InstructionSEI)() { T(FlagSEx)(_IF); }

/* ********************* */
/* *** STA, STY, STX *** */
/* ********************* */
quick FromRegister T(InstructionSTA)() { return _A; };
quick FromRegister T(InstructionSTX)() { return _X; };
quick FromRegister T(InstructionSTY)() { return _Y; };

/* ******************************* */
/* *** TAX, TAY, TSX, TXA, TYA *** */
/* ******************************* */
quick void T(RegisterTxx)(const WithRegister1, WithRegister2) {
	register2 = register1;
	T(UpdateZF)(register2);
	T(UpdateNF)(register2);
}

quick void T(InstructionTAX)() { T(RegisterTxx)(_A, _X); }
quick void T(InstructionTAY)() { T(RegisterTxx)(_A, _Y); }
quick void T(InstructionTSX)() { T(RegisterTxx)(_S, _X); }
quick void T(InstructionTXA)() { T(RegisterTxx)(_X, _A); }
quick void T(InstructionTYA)() { T(RegisterTxx)(_Y, _A); }

/* *********** */
/* *** TXS *** */
/* *********** */
quick void T(InstructionTXS)() {
	_S = _X;
}

// Clean up the namespace.
#undef Embeddable

#undef WithData
#undef WithFlag
#undef WithRegister
#undef WithRegister1
#undef WithRegister2
#undef WithVariable

#undef FromRegister
#undef FromVariable

#undef Bit0
#undef Bit6
#undef Bit7
