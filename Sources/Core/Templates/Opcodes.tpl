/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

// We certainly aren't going to win a beauty contest with this, but it works.
#define IMPLIED_T(_INSTRUCTION) { \
	T(AddressImplied)(); \
	T(Instruction## _INSTRUCTION)(); \
}

#define IMPLIED(_INSTRUCTION) IMPLIED_T(_INSTRUCTION)

#define READ_T(_MODE, _INSTRUCTION) { \
	const uint8 data = T(Address## _MODE ##Read)(); \
	T(Instruction## _INSTRUCTION)(data); \
}

#define READ(_MODE, _INSTRUCTION) READ_T(_MODE, _INSTRUCTION)

#define RMW_T(_MODE, _INSTRUCTION) \
	T(Address## _MODE ##RMW)<T(Instruction## _INSTRUCTION)>();

#define RMW(_MODE, _INSTRUCTION) RMW_T(_MODE, _INSTRUCTION)

#define WRITE_T(_MODE, _INSTRUCTION) { \
	const uint8 data = T(Instruction## _INSTRUCTION)(); \
	T(Address## _MODE ##Write)(data); \
}

#define WRITE(_MODE, _INSTRUCTION) WRITE_T(_MODE, _INSTRUCTION)

#define BRANCH_T(_INSTRUCTION) \
	T(AddressRelative)<T(Instruction## _INSTRUCTION)>();

#define BRANCH(_INSTRUCTION) BRANCH_T(_INSTRUCTION)

// Jumps perform all of their addressing in the instruction template.
#define JUMP_T(_INSTRUCTION) \
	T(Instruction## _INSTRUCTION)();

#define JUMP(_INSTRUCTION) JUMP_T(_INSTRUCTION)

#define ACCUMULATOR	Accumulator
#define ABSOLUTE	Absolute
#define ABSOLUTE_X	AbsoluteX
#define ABSOLUTE_Y	AbsoluteY
#define IMMEDIATE	Immediate
#define INDIRECT_X	IndirectX
#define INDIRECT_Y	IndirectY
#define ZERO_PAGE	ZeroPage
#define ZERO_PAGE_X	ZeroPageX
#define ZERO_PAGE_Y	ZeroPageY

#define BEGIN(_Code)	case _Code: {
#define END		break; }

static discrete_function void T(ParseOpcode)(const uint8 opcode) {
	switch(opcode) {
		/* ADC (ADd with Carry)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     ADC #$44      $69  2   2
		   Zero Page     ADC $44       $65  2   3
		   Zero Page,X   ADC $44,X     $75  2   4
		   Absolute      ADC $4400     $6D  3   4
		   Absolute,X    ADC $4400,X   $7D  3   4+
		   Absolute,Y    ADC $4400,Y   $79  3   4+
		   Indirect,X    ADC ($44,X)   $61  2   6
		   Indirect,Y    ADC ($44),Y   $71  2   5+ */
		BEGIN(0x69) READ(IMMEDIATE,   ADC) END
		BEGIN(0x65) READ(ZERO_PAGE,   ADC) END
		BEGIN(0x75) READ(ZERO_PAGE_X, ADC) END
		BEGIN(0x6D) READ(ABSOLUTE,    ADC) END
		BEGIN(0x7D) READ(ABSOLUTE_X,  ADC) END
		BEGIN(0x79) READ(ABSOLUTE_Y,  ADC) END
		BEGIN(0x61) READ(INDIRECT_X,  ADC) END
		BEGIN(0x71) READ(INDIRECT_Y,  ADC) END

		/* AND (bitwise AND with accumulator)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     AND #$44      $29  2   2
		   Zero Page     AND $44       $25  2   2
		   Zero Page,X   AND $44,X     $35  2   3
		   Absolute      AND $4400     $2D  3   4
		   Absolute,X    AND $4400,X   $3D  3   4+
		   Absolute,Y    AND $4400,Y   $39  3   4+
		   Indirect,X    AND ($44,X)   $21  2   6
		   Indirect,Y    AND ($44),Y   $31  2   5+ */
		BEGIN(0x29) READ(IMMEDIATE,   AND) END
		BEGIN(0x25) READ(ZERO_PAGE,   AND) END
		BEGIN(0x35) READ(ZERO_PAGE_X, AND) END
		BEGIN(0x2D) READ(ABSOLUTE,    AND) END
		BEGIN(0x3D) READ(ABSOLUTE_X,  AND) END
		BEGIN(0x39) READ(ABSOLUTE_Y,  AND) END
		BEGIN(0x21) READ(INDIRECT_X,  AND) END
		BEGIN(0x31) READ(INDIRECT_Y,  AND) END

		/* ASL (Arithmetic Shift Left)
		   MODE           SYNTAX       HEX LEN TIM
		   Accumulator   ASL A         $0A  1   2
		   Zero Page     ASL $44       $06  2   5
		   Zero Page,X   ASL $44,X     $16  2   6
		   Absolute      ASL $4400     $0E  3   6
		   Absolute,X    ASL $4400,X   $1E  3   7 */
		BEGIN(0x0A) RMW(ACCUMULATOR, ASL) END
		BEGIN(0x06) RMW(ZERO_PAGE,   ASL) END
		BEGIN(0x16) RMW(ZERO_PAGE_X, ASL) END
		BEGIN(0x0E) RMW(ABSOLUTE,    ASL) END
		BEGIN(0x1E) RMW(ABSOLUTE_X,  ASL) END

		/* BIT (test BITs)
		   MODE           SYNTAX       HEX LEN TIM
		   Zero Page     BIT $44       $24  2   3
		   Absolute      BIT $4400     $2C  3   4 */
		BEGIN(0x24) READ(ZERO_PAGE, BIT) END
		BEGIN(0x2C) READ(ABSOLUTE,  BIT) END

		/* Branch Instructions
		   MNEMONIC                       HEX
		   BPL (Branch on PLus)           $10
		   BMI (Branch on MInus)          $30
		   BVC (Branch on oVerflow Clear) $50
		   BVS (Branch on oVerflow Set)   $70
		   BCC (Branch on Carry Clear)    $90
		   BCS (Branch on Carry Set)      $B0
		   BNE (Branch on Not Equal)      $D0
		   BEQ (Branch on EQual)          $F0 */
		BEGIN(0x10) BRANCH(BPL) END
		BEGIN(0x30) BRANCH(BMI) END
		BEGIN(0x50) BRANCH(BVC) END
		BEGIN(0x70) BRANCH(BVS) END
		BEGIN(0x90) BRANCH(BCC) END
		BEGIN(0xB0) BRANCH(BCS) END
		BEGIN(0xD0) BRANCH(BNE) END
		BEGIN(0xF0) BRANCH(BEQ) END

		/* BRK (BReaK)
		   MODE           SYNTAX       HEX LEN TIM
		   Implied       BRK           $00  1   7 */
		BEGIN(0x00) IMPLIED(BRK) END

		/* CMP (CoMPare accumulator)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     CMP #$44      $C9  2   2
		   Zero Page     CMP $44       $C5  2   3
		   Zero Page,X   CMP $44,X     $D5  2   4
		   Absolute      CMP $4400     $CD  3   4
		   Absolute,X    CMP $4400,X   $DD  3   4+
		   Absolute,Y    CMP $4400,Y   $D9  3   4+
		   Indirect,X    CMP ($44,X)   $C1  2   6
		   Indirect,Y    CMP ($44),Y   $D1  2   5+ */
		BEGIN(0xC9) READ(IMMEDIATE,   CMP) END
		BEGIN(0xC5) READ(ZERO_PAGE,   CMP) END
		BEGIN(0xD5) READ(ZERO_PAGE_X, CMP) END
		BEGIN(0xCD) READ(ABSOLUTE,    CMP) END
		BEGIN(0xDD) READ(ABSOLUTE_X,  CMP) END
		BEGIN(0xD9) READ(ABSOLUTE_Y,  CMP) END
		BEGIN(0xC1) READ(INDIRECT_X,  CMP) END
		BEGIN(0xD1) READ(INDIRECT_Y,  CMP) END

		/* CPX (ComPare X register)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     CPX #$44      $E0  2   2
		   Zero Page     CPX $44       $E4  2   3
		   Absolute      CPX $4400     $EC  3   4 */
		BEGIN(0xE0) READ(IMMEDIATE, CPX) END
		BEGIN(0xE4) READ(ZERO_PAGE, CPX) END
		BEGIN(0xEC) READ(ABSOLUTE,  CPX) END

		/* CPY (ComPare Y register)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     CPY #$44      $C0  2   2
		   Zero Page     CPY $44       $C4  2   3
		   Absolute      CPY $4400     $CC  3   4 */
		BEGIN(0xC0) READ(IMMEDIATE, CPY) END
		BEGIN(0xC4) READ(ZERO_PAGE, CPY) END
		BEGIN(0xCC) READ(ABSOLUTE,  CPY) END

		/* DEC (DECrement memory)
		   MODE           SYNTAX       HEX LEN TIM
		   Zero Page     DEC $44       $C6  2   5
		   Zero Page,X   DEC $44,X     $D6  2   6
		   Absolute      DEC $4400     $CE  3   6
		   Absolute,X    DEC $4400,X   $DE  3   7 */
		BEGIN(0xC6) RMW(ZERO_PAGE,   DEC) END
		BEGIN(0xD6) RMW(ZERO_PAGE_X, DEC) END
		BEGIN(0xCE) RMW(ABSOLUTE,    DEC) END
		BEGIN(0xDE) RMW(ABSOLUTE_X,  DEC) END

		/* EOR (bitwise Exclusive OR)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     EOR #$44      $49  2   2
		   Zero Page     EOR $44       $45  2   3
		   Zero Page,X   EOR $44,X     $55  2   4
		   Absolute      EOR $4400     $4D  3   4
		   Absolute,X    EOR $4400,X   $5D  3   4+
		   Absolute,Y    EOR $4400,Y   $59  3   4+
		   Indirect,X    EOR ($44,X)   $41  2   6
		   Indirect,Y    EOR ($44),Y   $51  2   5+ */
		BEGIN(0x49) READ(IMMEDIATE,   EOR) END
		BEGIN(0x45) READ(ZERO_PAGE,   EOR) END
		BEGIN(0x55) READ(ZERO_PAGE_X, EOR) END
		BEGIN(0x4D) READ(ABSOLUTE,    EOR) END
		BEGIN(0x5D) READ(ABSOLUTE_X,  EOR) END
		BEGIN(0x59) READ(ABSOLUTE_Y,  EOR) END
		BEGIN(0x41) READ(INDIRECT_X,  EOR) END
		BEGIN(0x51) READ(INDIRECT_Y,  EOR) END

		/* Flag (Processor Status) Instructions
		   MNEMONIC                       HEX
		   CLC (CLear Carry)              $18
		   SEC (SEt Carry)                $38
		   CLI (CLear Interrupt)          $58
		   SEI (SEt Interrupt)            $78
		   CLV (CLear oVerflow)           $B8
		   CLD (CLear Decimal)            $D8
		   SED (SEt Decimal)              $F8 */
		BEGIN(0x18) IMPLIED(CLC) END
		BEGIN(0x38) IMPLIED(SEC) END
		BEGIN(0x58) IMPLIED(CLI) END
		BEGIN(0x78) IMPLIED(SEI) END
		BEGIN(0xB8) IMPLIED(CLV) END
		BEGIN(0xD8) IMPLIED(CLD) END
		BEGIN(0xF8) IMPLIED(SED) END

		/* INC (INCrement memory)
		   MODE           SYNTAX       HEX LEN TIM
		   Zero Page     INC $44       $E6  2   5
		   Zero Page,X   INC $44,X     $F6  2   6
		   Absolute      INC $4400     $EE  3   6
		   Absolute,X    INC $4400,X   $FE  3   7 */
		BEGIN(0xE6) RMW(ZERO_PAGE,   INC) END
		BEGIN(0xF6) RMW(ZERO_PAGE_X, INC) END
		BEGIN(0xEE) RMW(ABSOLUTE,    INC) END
		BEGIN(0xFE) RMW(ABSOLUTE_X,  INC) END

		/* JMP (JuMP)
		   MODE           SYNTAX       HEX LEN TIM
		   Absolute      JMP $5597     $4C  3   3 
		   Indirect      JMP ($5597)   $6C  3   5 */
		BEGIN(0x4C) JUMP(JMP1) END
		BEGIN(0x6C) JUMP(JMP2) END

		/* JSR (Jump to SubRoutine)
		   MODE           SYNTAX       HEX LEN TIM
		   Absolute      JSR $5597     $20  3   6 */
		BEGIN(0x20) JUMP(JSR) END

		/* LDA (LoaD Accumulator)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     LDA #$44      $A9  2   2
		   Zero Page     LDA $44       $A5  2   3
		   Zero Page,X   LDA $44,X     $B5  2   4
		   Absolute      LDA $4400     $AD  3   4
		   Absolute,X    LDA $4400,X   $BD  3   4+
		   Absolute,Y    LDA $4400,Y   $B9  3   4+
		   Indirect,X    LDA ($44,X)   $A1  2   6
		   Indirect,Y    LDA ($44),Y   $B1  2   5+ */
		BEGIN(0xA9) READ(IMMEDIATE,   LDA) END
		BEGIN(0xA5) READ(ZERO_PAGE,   LDA) END
		BEGIN(0xB5) READ(ZERO_PAGE_X, LDA) END
		BEGIN(0xAD) READ(ABSOLUTE,    LDA) END
		BEGIN(0xBD) READ(ABSOLUTE_X,  LDA) END
		BEGIN(0xB9) READ(ABSOLUTE_Y,  LDA) END
		BEGIN(0xA1) READ(INDIRECT_X,  LDA) END
		BEGIN(0xB1) READ(INDIRECT_Y,  LDA) END

		/* LDX (LoaD X register)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     LDX #$44      $A2  2   2
		   Zero Page     LDX $44       $A6  2   3
		   Zero Page,Y   LDX $44,Y     $B6  2   4
		   Absolute      LDX $4400     $AE  3   4
		   Absolute,Y    LDX $4400,Y   $BE  3   4+ */
		BEGIN(0xA2) READ(IMMEDIATE,   LDX) END
		BEGIN(0xA6) READ(ZERO_PAGE,   LDX) END
		BEGIN(0xB6) READ(ZERO_PAGE_Y, LDX) END
		BEGIN(0xAE) READ(ABSOLUTE,    LDX) END
		BEGIN(0xBE) READ(ABSOLUTE_Y,  LDX) END

		/* LDY (LoaD Y register)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     LDY #$44      $A0  2   2
		   Zero Page     LDY $44       $A4  2   3
		   Zero Page,X   LDY $44,X     $B4  2   4
		   Absolute      LDY $4400     $AC  3   4
		   Absolute,X    LDY $4400,X   $BC  3   4+ */
		BEGIN(0xA0) READ(IMMEDIATE,   LDY) END
		BEGIN(0xA4) READ(ZERO_PAGE,   LDY) END
		BEGIN(0xB4) READ(ZERO_PAGE_X, LDY) END
		BEGIN(0xAC) READ(ABSOLUTE,    LDY) END
		BEGIN(0xBC) READ(ABSOLUTE_X,  LDY) END

		/* LSR (Logical Shift Right)
		   MODE           SYNTAX       HEX LEN TIM
		   Accumulator   LSR A         $4A  1   2
		   Zero Page     LSR $44       $46  2   5
		   Zero Page,X   LSR $44,X     $56  2   6
		   Absolute      LSR $4400     $4E  3   6
		   Absolute,X    LSR $4400,X   $5E  3   7 */
		BEGIN(0x4A) RMW(ACCUMULATOR, LSR) END
		BEGIN(0x46) RMW(ZERO_PAGE,   LSR) END
		BEGIN(0x56) RMW(ZERO_PAGE_X, LSR) END
		BEGIN(0x4E) RMW(ABSOLUTE,    LSR) END
		BEGIN(0x5E) RMW(ABSOLUTE_X,  LSR) END

		/* NOP (No OPeration)
		   MODE           SYNTAX       HEX LEN TIM
		   Implied       NOP           $EA  1   2 */
		BEGIN(0xEA) IMPLIED(NOP) END

		/* ORA (bitwise OR with Accumulator)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     ORA #$44      $09  2   2
		   Zero Page     ORA $44       $05  2   2
		   Zero Page,X   ORA $44,X     $15  2   3
		   Absolute      ORA $4400     $0D  3   4
		   Absolute,X    ORA $4400,X   $1D  3   4+
		   Absolute,Y    ORA $4400,Y   $19  3   4+
		   Indirect,X    ORA ($44,X)   $01  2   6
		   Indirect,Y    ORA ($44),Y   $11  2   5+ */
		BEGIN(0x09) READ(IMMEDIATE,   ORA) END
		BEGIN(0x05) READ(ZERO_PAGE,   ORA) END
		BEGIN(0x15) READ(ZERO_PAGE_X, ORA) END
		BEGIN(0x0D) READ(ABSOLUTE,    ORA) END
		BEGIN(0x1D) READ(ABSOLUTE_X,  ORA) END
		BEGIN(0x19) READ(ABSOLUTE_Y,  ORA) END
		BEGIN(0x01) READ(INDIRECT_X,  ORA) END
		BEGIN(0x11) READ(INDIRECT_Y,  ORA) END

		/* Register Instructions
		   MNEMONIC                 HEX
		   TAX (Transfer A to X)    $AA
		   TXA (Transfer X to A)    $8A
		   DEX (DEcrement X)        $CA
		   INX (INcrement X)        $E8
		   TAY (Transfer A to Y)    $A8
		   TYA (Transfer Y to A)    $98
		   DEY (DEcrement Y)        $88
		   INY (INcrement Y)        $C8 */
		BEGIN(0xAA) IMPLIED(TAX) END
		BEGIN(0x8A) IMPLIED(TXA) END
		BEGIN(0xCA) IMPLIED(DEX) END
		BEGIN(0xE8) IMPLIED(INX) END
		BEGIN(0xA8) IMPLIED(TAY) END
		BEGIN(0x98) IMPLIED(TYA) END
		BEGIN(0x88) IMPLIED(DEY) END
		BEGIN(0xC8) IMPLIED(INY) END

		/* ROL (ROtate Left)
		   MODE           SYNTAX       HEX LEN TIM
		   Accumulator   ROL A         $2A  1   2
		   Zero Page     ROL $44       $26  2   5
		   Zero Page,X   ROL $44,X     $36  2   6
		   Absolute      ROL $4400     $2E  3   6
		   Absolute,X    ROL $4400,X   $3E  3   7 */
		BEGIN(0x2A) RMW(ACCUMULATOR, ROL) END
		BEGIN(0x26) RMW(ZERO_PAGE,   ROL) END
		BEGIN(0x36) RMW(ZERO_PAGE_X, ROL) END
		BEGIN(0x2E) RMW(ABSOLUTE,    ROL) END
		BEGIN(0x3E) RMW(ABSOLUTE_X,  ROL) END

		/* ROR (ROtate Right)
		   MODE           SYNTAX       HEX LEN TIM
		   Accumulator   ROR A         $6A  1   2
		   Zero Page     ROR $44       $66  2   5
		   Zero Page,X   ROR $44,X     $76  2   6
		   Absolute      ROR $4400     $6E  3   6
		   Absolute,X    ROR $4400,X   $7E  3   7 */
		BEGIN(0x6A) RMW(ACCUMULATOR, ROR) END
		BEGIN(0x66) RMW(ZERO_PAGE,   ROR) END
		BEGIN(0x76) RMW(ZERO_PAGE_X, ROR) END
		BEGIN(0x6E) RMW(ABSOLUTE,    ROR) END
		BEGIN(0x7E) RMW(ABSOLUTE_X,  ROR) END

		/* RTI (ReTurn from Interrupt)
		   MODE           SYNTAX       HEX LEN TIM
		   Implied       RTI           $40  1   6 */
		BEGIN(0x40) IMPLIED(RTI) END

		/* RTS (ReTurn from Subroutine)
		   MODE           SYNTAX       HEX LEN TIM
		   Implied       RTS           $60  1   6  */
		BEGIN(0x60) IMPLIED(RTS) END

		/* SBC (SuBtract with Carry)
		   MODE           SYNTAX       HEX LEN TIM
		   Immediate     SBC #$44      $E9  2   2
		   Zero Page     SBC $44       $E5  2   3
		   Zero Page,X   SBC $44,X     $F5  2   4
		   Absolute      SBC $4400     $ED  3   4
		   Absolute,X    SBC $4400,X   $FD  3   4+
		   Absolute,Y    SBC $4400,Y   $F9  3   4+
		   Indirect,X    SBC ($44,X)   $E1  2   6
		   Indirect,Y    SBC ($44),Y   $F1  2   5+ */
		BEGIN(0xE9) READ(IMMEDIATE,   SBC) END
		BEGIN(0xE5) READ(ZERO_PAGE,   SBC) END
		BEGIN(0xF5) READ(ZERO_PAGE_X, SBC) END
		BEGIN(0xED) READ(ABSOLUTE,    SBC) END
		BEGIN(0xFD) READ(ABSOLUTE_X,  SBC) END
		BEGIN(0xF9) READ(ABSOLUTE_Y,  SBC) END
		BEGIN(0xE1) READ(INDIRECT_X,  SBC) END
		BEGIN(0xF1) READ(INDIRECT_Y,  SBC) END

		/* STA (STore Accumulator)
		   MODE           SYNTAX       HEX LEN TIM
		   Zero Page     STA $44       $85  2   3
		   Zero Page,X   STA $44,X     $95  2   4
		   Absolute      STA $4400     $8D  3   4
		   Absolute,X    STA $4400,X   $9D  3   5
		   Absolute,Y    STA $4400,Y   $99  3   5
		   Indirect,X    STA ($44,X)   $81  2   6
		   Indirect,Y    STA ($44),Y   $91  2   6 */
		BEGIN(0x85) WRITE(ZERO_PAGE,   STA) END
		BEGIN(0x95) WRITE(ZERO_PAGE_X, STA) END
		BEGIN(0x8D) WRITE(ABSOLUTE,    STA) END
		BEGIN(0x9D) WRITE(ABSOLUTE_X,  STA) END
		BEGIN(0x99) WRITE(ABSOLUTE_Y,  STA) END
		BEGIN(0x81) WRITE(INDIRECT_X,  STA) END
		BEGIN(0x91) WRITE(INDIRECT_Y,  STA) END

		/* Stack Instructions
		   MNEMONIC                        HEX TIM
		   TXS (Transfer X to Stack ptr)   $9A  2 
		   TSX (Transfer Stack ptr to X)   $BA  2 
		   PHA (PusH Accumulator)          $48  3 
		   PLA (PuLl Accumulator)          $68  4 
		   PHP (PusH Processor status)     $08  3 
		   PLP (PuLl Processor status)     $28  4  */
		BEGIN(0x9A) IMPLIED(TXS) END
		BEGIN(0xBA) IMPLIED(TSX) END
		BEGIN(0x48) IMPLIED(PHA) END
		BEGIN(0x68) IMPLIED(PLA) END
		BEGIN(0x08) IMPLIED(PHP) END
		BEGIN(0x28) IMPLIED(PLP) END

		/* STX (STore X register)
		   MODE           SYNTAX       HEX LEN TIM
		   Zero Page     STX $44       $86  2   3
		   Zero Page,Y   STX $44,Y     $96  2   4
		   Absolute      STX $4400     $8E  3   4 */
		BEGIN(0x86) WRITE(ZERO_PAGE,   STX) END
		BEGIN(0x96) WRITE(ZERO_PAGE_Y, STX) END
		BEGIN(0x8E) WRITE(ABSOLUTE,    STX) END

		/* STY (STore Y register)
		   MODE           SYNTAX       HEX LEN TIM
		   Zero Page     STY $44       $84  2   3
		   Zero Page,X   STY $44,X     $94  2   4
		   Absolute      STY $4400     $8C  3   4 */
		BEGIN(0x84) WRITE(ZERO_PAGE,   STY) END
		BEGIN(0x94) WRITE(ZERO_PAGE_X, STY) END
		BEGIN(0x8C) WRITE(ABSOLUTE,    STY) END

		default:
			log_printf("CORE: Unsupported opcode ($%02X) at PC=$%04X\n", opcode, _PC);
			break;
	}

#if defined(COREFast)
	/* Normally this is set incrementally throughout the execution of the opcode,
           however in fast mode the clock counter is not modified mid-instruction so 
           we have to do it here at the end of the opcode. */
	core.time += timeTable[opcode];
#endif
}

// Clean up.
#undef IMPLIED_T
#undef IMPLIED
#undef READ_T
#undef READ
#undef RMW_T
#undef RMW
#undef WRITE_T
#undef WRITE
#undef BRANCH_T
#undef BRANCH
#undef JUMP_T
#undef JUMP

#undef ACCUMULATOR
#undef ABSOLUTE
#undef ABSOLUTE_X
#undef ABSOLUTE_Y
#undef IMMEDIATE
#undef INDIRECT_X
#undef INDIRECT_Y
#undef ZERO_PAGE
#undef ZERO_PAGE_X
#undef ZERO_PAGE_Y

#undef BEGIN
#undef END
