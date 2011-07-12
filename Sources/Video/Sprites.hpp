/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Video__Sprites_hpp__included
#define Video__Sprites_hpp__included
#include "Local.hpp"

namespace Renderer {
namespace Sprites {

extern R_LookupTable( IndexTable );
extern R_LookupTable( SequenceTable );

extern void Initialize();
extern void Line();
extern void Pixel(const bool rendering);
extern void Clock();

} // namespace Sprites
} // namespace Renderer

#endif // !Video__Sprites_hpp__included
