/* FakeNES - A portable, Open Source NES and Famicom emulator.
 * Copyright © 2011-2012 Digital Carat Group
 * 
 * This is free software. See 'License.txt' for additional copyright and
 * licensing information. You must read and accept the license prior to any
 * modification or use of this software.
 * 
 * Original version by Charles Bilyue' (TRAC).
 */
#include "Common/Debug.h"
#include "Common/Global.h"
#include "Common/Types.h"
#include "CRC32.h"

namespace {

bool initialized = false;

const size_type TableSize = 256;
uint32 table[TableSize];

const uint32 seed = 0xFFFFFFFF;

discrete_function void Initialize() {
	for( size_type i = 0; i < TableSize; i++ ) {
		uint32 value = i;
		for( uint bit = 0; bit < 8; bit++ ) {
			if( value & 1 )
				value = (value >> 1) ^ 0xEDB88320;
			else
				value >>= 1;
		}

		table[i] = value;
	}

	initialized = true;
}

} // namespace anonymous

// --------------------------------------------------------------------------------
// PUBLIC INTERFACE
// --------------------------------------------------------------------------------

UINT32 crc32_start(void) {
	if( !initialized )
		Initialize();

	return seed;
}

void crc32_end(UINT32* crc32) {
	Safeguard( crc32 );
	*crc32 ^= seed;
}

void crc32_update(UINT32* crc32, const UINT8 data) {           
	Safeguard( crc32 );
	*crc32 = table[(*crc32 ^ data) & 0xFF] ^ ((*crc32 >> 8) & 0x00FFFFFF);
}

UINT32 calculate_crc32(const void* buffer, const SIZE size) {
	Safeguard( buffer );

	if( size == 0 )
		return 0;

	const uint8* data = (const uint8*)buffer;
	uint32 crc32 = crc32_start();
	for( size_type offset = 0; offset < size; offset++ )
		crc32_update(&crc32, data[offset]);

	crc32_end(&crc32);
	return crc32;
}
