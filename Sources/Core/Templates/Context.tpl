/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#define _PC	core.registers.pc.word
#define _PCL	core.registers.pc.bytes.low
#define _PCH	core.registers.pc.bytes.high
#define _A	core.registers.a
#define _P	core.registers.p
#define _S	core.registers.s
#define _X	core.registers.x
#define _Y	core.registers.y

#define _CF	core.flags.c
#define _ZF	core.flags.z
#define _IF	core.flags.i
#define _DF	core.flags.d
#define _VF	core.flags.v
#define _NF	core.flags.n

// This is needed as the flags may only be consuming one bit in memory.
#define SetFlag(_Flag, _Value) \
	_Flag = (_Value) ? 1 : 0

// Packs individual flags into bits of the status register (P).
quick void T(PackFlags)() {
	_P = COREFlagReserved;
	_P |= _CF ? COREFlagCarry     : 0;
	_P |= _ZF ? COREFlagZero      : 0;
	_P |= _IF ? COREFlagInterrupt : 0;
	_P |= _DF ? COREFlagDecimal   : 0;
	_P |= _VF ? COREFlagOverflow  : 0;
	_P |= _NF ? COREFlagNegative  : 0;

}

// Unpacks the bits of the status register (P) into individual flags again.
quick void T(UnpackFlags)() {
	SetFlag(_CF, _P & COREFlagCarry);
	SetFlag(_ZF, _P & COREFlagZero);
	SetFlag(_IF, _P & COREFlagInterrupt);
	SetFlag(_DF, _P & COREFlagDecimal);
	SetFlag(_VF, _P & COREFlagOverflow);
	SetFlag(_NF, _P & COREFlagNegative);
}

quick void T(UpdateZF)(const uint8 result) { SetFlag(_ZF, (result == 0)); }
quick void T(UpdateNF)(const uint8 result) { SetFlag(_NF, result & _10000000b); }
quick void T(UpdateCF)(const bool condition) { SetFlag(_CF, condition); }
quick void T(UpdateVF)(const bool condition) { SetFlag(_VF, condition); }
