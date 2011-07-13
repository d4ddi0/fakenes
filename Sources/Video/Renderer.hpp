/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef Video__Renderer_hpp__included
#define Video__Renderer_hpp__included
#include "Internals.h"
#include "Local.hpp"

/* This macro helps with creating scanline-length arrays for storing the results of calculations
   for use on a per-clock basis to improve performance. */
#define R_LookupTable(_name) \
   int _name[1 + PPU_SCANLINE_CLOCKS]

// This macro helps with clearing a lookup table before use.
#define R_ClearLookupTable(_name) { \
   for(int _i = 0; _i < 1 + PPU_SCANLINE_CLOCKS; _i++) \
      _name[_i] = 0; \
}

// These macros write to the special background scanline buffer.
#define R_GetBackgroundPixel(_pixel) \
   ( ppu__background_pixels[render.pixel] )
#define R_PutBackgroundPixel(_pixel) \
   ( ppu__background_pixels[render.pixel] = (_pixel) )
#define R_ClearBackgroundPixel() \
   ( R_PutBackgroundPixel(0) )

// These macros write to the framebuffer.
#define R_PutFramePixel(_pixel) \
   ( render.buffer[render.pixel] = (_pixel) )
#define R_ClearFramePixel() \
   ( R_PutFramePixel(PPU__BACKGROUND_COLOR) )

// --------------------------------------------------------------------------------

namespace Renderer {

// These have to be here, rather than in a file, due to compiler limitations.
static const int SpritesPerLine = 8;						// Number of sprites per line (max)
static const int BytesPerSprite = PPU__BYTES_PER_SPRITE;			// Bytes consumed by each sprite in OAM
static const unsigned SecondaryOAMSize = BytesPerSprite * SpritesPerLine;	// Size of secondary OAM

extern const int TileWidth;		// Width of a tile
extern const int TileHeight;		// Height of a tile
extern const int DisplayWidth;		// Display width, in pixels
extern const int DisplayWidthTiles;	// Display width, in tiles
extern const int DisplayHeight;		// Display height, in pixels
extern const int DisplayHeightTiles;	// Display height, in tiles

typedef struct _RenderBackgroundContext {
   uint8 lowShift, highShift;	// Upper bytes of shift registers
   uint8 lowFeed, highFeed;	// Lower bytes of shift registers
   uint8 buffer, bufferTag;	// Attribute for current tile (+shift count)
   uint8 latch, latchTag;	// Attribute for next tile (+shift count)
   uint8 counter;		// Current pixel down-counter (7-0)

} RenderBackgroundContext;

typedef struct _RenderBackgroundEvaluation {
   uint8 name;			// Name table byte
   uint8 attribute, tag;	// Attribute table byte (+shift count)
   uint8 pattern1, pattern2;	// Pattern table bytes
   uint8 row;			// Fine Y scrolling offset (0-7)

} RenderBackgroundEvaluation;

typedef struct _RenderSpriteContext {
   uint8 index;			// Original sprite index from OAM (0-63)
   uint8 lowShift, highShift;	// Shift registers
   uint8 latch;			// Attribute byte
   uint8 counter;		// X-position down-counter (0-255)
   bool dead;			// Set if the sprite is disabled

} RenderSpriteContext;

typedef struct _RenderSpriteEvaluation {
   uint8 indices[SpritesPerLine];	// Original sprite index from OAM (0-63)
   uint8 state, substate;		// Evaluation state machine
   uint8 count;				// Number of sprites on the line so far (0-8)
   uint8 n, m;				// Current sprite and fetch byte
   BOOL locked;				// Set when too many sprites are on a line
   uint8 data;				// Data buffer for timing reads/writes

} RenderSpriteEvaluation;

typedef struct _RenderContext {
   uint16* buffer;	// Rendering framebuffer
   int16 line;		// Current scanline
   uint8 pixel;		// Current pixel position
   uint16 clock;	// Current clock cycle (1-341)
   bool isOddClock;	// Odd or even clock cycle flag

   RenderBackgroundContext background;
   RenderBackgroundEvaluation backgroundEvaluation;

   RenderSpriteContext sprites[SpritesPerLine];
   RenderSpriteEvaluation spriteEvaluation;
   uint8 secondaryOAM[SecondaryOAMSize];
   uint8 spriteCount;	// Number of sprites in secondary OAM (1-8)

} RenderContext;

extern RenderContext render;

// --------------------------------------------------------------------------------

extern void Initialize();
extern void Frame();
extern void Line(const int line);
extern void Pixel();
extern void Clock();
extern void Load(FILE_CONTEXT* file, const int version);
extern void Save(FILE_CONTEXT* file, const int version);

} // namespace Renderer

#endif // !Video__Renderer_hpp__included
