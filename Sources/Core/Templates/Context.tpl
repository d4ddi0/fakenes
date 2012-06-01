/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

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
express_function void T(PackFlags)() {
	_P = COREFlagReserved;
	_P |= _CF ? COREFlagCarry     : 0;
	_P |= _ZF ? COREFlagZero      : 0;
	_P |= _IF ? COREFlagInterrupt : 0;
	_P |= _DF ? COREFlagDecimal   : 0;
	_P |= _VF ? COREFlagOverflow  : 0;
	_P |= _NF ? COREFlagNegative  : 0;

}

// Unpacks the bits of the status register (P) into individual flags again.
express_function void T(UnpackFlags)() {
	SetFlag(_CF, _P & COREFlagCarry);
	SetFlag(_ZF, _P & COREFlagZero);
	SetFlag(_IF, _P & COREFlagInterrupt);
	SetFlag(_DF, _P & COREFlagDecimal);
	SetFlag(_VF, _P & COREFlagOverflow);
	SetFlag(_NF, _P & COREFlagNegative);
}

express_function void T(UpdateZF)(const uint8 result) { SetFlag(_ZF, (result == 0)); }
express_function void T(UpdateNF)(const uint8 result) { SetFlag(_NF, result & _10000000b); }
express_function void T(UpdateCF)(const bool condition) { SetFlag(_CF, condition); }
express_function void T(UpdateVF)(const bool condition) { SetFlag(_VF, condition); }
