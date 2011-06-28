This directory contains the emulation for the NES's "2A03" processor.

Core.cpp, Core.hpp, Templates.hpp
*********************************
The RP2A03G CPU core which is also compatible with the PAL processor of a
slightly different model. This is a modified 6502 without decimal mode, and
with an APU (Audio Processing Unit) built-in. The APU is emulated separately.
It has no memory management, as that is handled by the CPU wrapper.

File summary:
Core.cpp   - The implementation of the core emulation ("the core").
Core.hpp   - The public interface to the core.
Templates* - Code generation templates used for optimizing the core for various
             execution methods, trading off performance for accuracy.

CPU.cpp, CPU.h, Internal.h
**************************
The CPU wrapper, which provides a C interface to the core, combined with memory
management, memory mapping, memory patching and state saving.

File summary:
CPU.cpp    - The implementation of the wrapper.
CPU.h      - The public interface to the wrapper, using C coding guidelines.
Internal.h - Allows external sources to access some internal components of the
             wrapper directly. This is primarily used by the core.

Note that the core emulation is deliberately kept separate from the wrapper; it
depends on the wrapper only for memory management, and they both keep separate
definitions that must either remain compatible, or be translated.

- Thur 16 June 2011
