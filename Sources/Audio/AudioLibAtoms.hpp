/* FakeNES - A free, portable, Open Source NES emulator.

   apu_int.h: Internal declarations for the APU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef Audio__AudioLibAtoms_hpp__included
#define Audio__AudioLibAtoms_hpp__included
#include "Common.hpp"

// Driver interfaces for the audio library.
class AudiolibDriver {
public:
   AudiolibDriver(void) { }
   virtual ~AudiolibDriver(void) { }

   virtual int initialize(void) { return 1; }
   virtual void deinitialize(void) { }
   virtual int openStream(void) { return 1; }
   virtual void closeStream(void) { }
   virtual void* getBuffer(void* buffer) { return null; }
   virtual void freeBuffer(void* buffer) { }
   virtual void suspend(void) { }
   virtual void resume(void) { }
};

class AudiolibAllegroDriver : public AudiolibDriver {
public:
   AudiolibAllegroDriver(void);

   int initialize(void);
   void deinitialize(void);
   int openStream(void);
   void closeStream(void);
   void* getBuffer(void* buffer);
   void freeBuffer(void* buffer);
   void suspend(void);
   void resume(void);

private:
   AUDIOSTREAM* stream;
};

#ifdef USE_OPENAL
class AudiolibOpenALDriver : public AudiolibDriver {
public:
   AudiolibOpenALDriver(void);

   int initialize(void);
   void deinitialize(void);
   int openStream(void);
   void closeStream(void);
   void* getBuffer(void* buffer);
   void freeBuffer(void* buffer);
   void suspend(void);
   void resume(void);

private:
   const UCHAR* getErrorStringAL(ALenum error);
   const UCHAR* getErrorStringALC(ALCenum error);

   ALCdevice* device;
   ALCcontext* context;
   ALuint format;

   ALuint* buffers;
   ALuint source;

   ALuint floatingBuffer;
};
#endif // USE_OPENAL

#endif // !Audio__AudioLibAtoms_hpp__included
