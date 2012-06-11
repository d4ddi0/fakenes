This is the FakeNES source code package. It includes the neccessary program
source code, resources and other content needed to compile and run the software
on a number of supported platforms, including DOS, Windows, Unix and variants
of Unix such as Linux and Mac OS X. Only certain platforms are officially
supported, although the software may still compile (and run) on unsupported
platforms with some additional effort and know-how.

The only officially supported compilers are GCC and G++. We do not have plans
to support other compilers, as they rarely make an apperance on modern systems,
excepting Microsoft Visual C++. We prefer that this software only be built
using free (as in beer) tools, so we will not support MSVC for now.

GNU make, or a compatible make, will make the build process easier, but is not
strictly neccessary. The build system used by this software, CBuild, is capable
of compiling and running on any system with only a functional C compiler.
However, without make installed, compiling (and running) CBuild will have to be
done manually, and may require a minor investment of time.

For building for Linux, Mac OS X, and Windows, either the Allegro library
(version 4.2.0 or higher) or the SDL library must be available. The newer
Allegro 5 is not yet supported as it is still missing from some distributions.
OpenAL Soft, if present, is supported for sound output as an alternative to the
often problematic sound drivers included in Allegro and SDL.

!!! It is highly recommended to build with OpenAL output enabled on Linux !!!
!!! systems that utilize the PulseAudio sound server.                     !!!

To build for DOS, the software must be compiled from within an environment that
supports long filenames (LFN). Although the resulting executable(s) may be used
in an environment where LFNs are not supported, once compiled. Futhermore, a
very specific build environment is required: DJGPP (with G++ installed), and
Allegro 4.2.3, as later versions of Allegro 4.x removed support for DOS.

- Jun 11 2012
