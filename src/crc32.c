

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

crc32.c: Implementation of the CRC32 calculation.

Copyright (c) 2003, Randy McDowell.
Copyright (c) 2003, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include "misc.h"

#include "crc32.h"


static const unsigned crc32_seed = 0xFFFFFFFF;
static unsigned crc32_table[256];

static int crc32_initialized = FALSE;


static unsigned crc32_update (unsigned char value, unsigned crc32)
{
    return crc32_table[(crc32 ^ value) & 0xFF] ^ ((crc32 >> 8) & 0x00FFFFFF);
}


static void crc32_init (void)
{
    int entry, bit;

    for (entry = 0; entry < 256; entry++)
    {
        unsigned value = entry;

        for (bit = 0; bit < 8; bit++)
        {
            if (value & 1)
            {
                value = (value >> 1) ^ 0xEDB88320;
            }
            else
            {
                value >>= 1;
            }
        }

        crc32_table[entry] = value;
    }

    crc32_initialized = TRUE;
}


static unsigned crc32_start (void)
{
    return crc32_seed;
}


static unsigned crc32_end (unsigned crc32)
{
    return crc32 ^ crc32_seed;
}


UINT32 crc32_calculate (const UINT8 *buffer, unsigned len)
{
    unsigned crc32;
    int pos;

    if (!crc32_initialized) crc32_init ();

    crc32 = crc32_start ();

    for (pos = 0; pos < len; pos++)
    {
        crc32 = crc32_update (buffer[pos], crc32);
    }

    crc32 = crc32_end (crc32);

    return crc32;
}
