/* FakeNES - A free, portable, Open Source NES emulator.

   Copyright (c) 2001-2011, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use.*/

// For information on how this works, read the comments in 'Templates.hpp'.
#if defined(COREFast)
#	define CORESuffix Fast
#elif defined(COREAsynchronous)
#	define CORESuffix Asynchronous
#else
#	define CORESuffix
#endif

/* This is used to templatize functions depending on the mode. The absurd apperance is because
   the C preprocessor doesn't expand tokenized arguments to macros. */
#define T__(_Name, _Suffix) _Name##_Suffix
#define T_(_Name, _Suffix) T__(_Name, _Suffix)
#define T(_Name) T_(_Name, CORESuffix)

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
