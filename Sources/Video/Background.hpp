/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Video__Background_hpp__included
#define Video__Background_hpp__included
#include "Local.hpp"
#include "Renderer.hpp"

namespace Renderer {
namespace Background {

extern R_LookupTable( FetchTable );
extern R_LookupTable( SequenceTable );

extern void Initialize();
extern void Frame();
extern void Line();
extern void Pixel(const bool rendering);
extern void PixelStub(const bool rendering);
extern void Clock();

} // namespace Background
} // namespace Renderer

#endif // !Video__Background_hpp__included
