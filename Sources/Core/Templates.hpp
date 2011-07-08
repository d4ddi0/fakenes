/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Core__Templates_hpp__included
#define Core__Templates_hpp__included
#include "Core/Core.hpp"
#include "Core/Local.hpp"

namespace Templates {

/* The RP2A03G emulation code can be compiled in one of three modes:

	Fast mode
	Executes a whole opcode at a time, while throwing away any "pointless"
        operations, such as garbage fetches. The clock counter is not updated in the
        middle of instructions, and interrupts are simply handled as soon as possible.
        The APU and PPU must hook reads and writes and play "catch up" when they
        occur, but accuracy is haphazard. Interrupt prediction is required.

	Accurate mode (Default)
	Executes a whole opcode at a time, but the clock counter is updated in the
	middle of instructions. This allows the APU and PPU to achieve very good
        accuracy when catching up, as they will know exactly where the CPU is at.
	Interrupt prediction is still required in this mode, but interrupts are
	checked inside of instructions, and have proper timing.

	Asynchronous mode
	This is like normal mode, except that the APU and PPU are updated every time
	the clock counter is incremented. This achieves perfect synchronization with
	the APU and PPU, and interrupt prediction is not required, as CPU code runs
	in synchronization with APU and PPU processing. Mapper functionality that
	depends directly on address lines is enabled.

   These modes simply affect the code that is compiled into CORE::Execute*(). If all
   three modes are compiled and linked, they will all be useable from the same API
   simultaneously, simply by changing what execution routine is called. */

// Expand templates for fast mode.
#define COREFast
#include "Core/Templates/Main.tpl"
#undef COREFast

/* Expand templates for accurate mode. This is the default behavior, so we don't
   have to define anything to make it happen. */
#include "Core/Templates/Main.tpl"

// Exand templates for asynchronous mode.
#define COREAsynchronous
#include "Core/Templates/Main.tpl"
#undef COREAsynchronous

} // namespace Templates

#endif // !Core__Templates_hpp__included
