/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

// For information on how this works, read the comments in 'Templates.hpp'.
#if defined(COREFast)
#	define CORESuffix Fast
#elif defined(COREAsynchronous)
#	define CORESuffix Asynchronous
#endif

/* This is used to templatize functions depending on the mode. The absurd apperance is because
   the C preprocessor doesn't expand tokenized arguments to macros. */
#ifdef CORESuffix
#	define T__(_Name, _Suffix) _Name##_Suffix
#	define T_(_Name, _Suffix) T__(_Name, _Suffix)
#	define T(_Name) T_(_Name, CORESuffix)
#else
#	define T(_Name) _Name
#endif

// Keep these in order, or things will break. :p
#include "Core/Templates/Context.tpl"
#include "Core/Templates/Memory.tpl"
#include "Core/Templates/Interrupts.tpl"
#include "Core/Templates/Instructions.tpl"
#include "Core/Templates/Addressing.tpl"
#include "Core/Templates/Opcodes.tpl"
#include "Core/Templates/Step.tpl"

// Clean up the namespace.
#undef T__
#undef T_
#undef T
#undef CORESuffix
