/* FakeNES - Portable NES Emulator

   Copyright (c) 2001-2008 FakeNES Team
   Copyright (c) 2011 Digital Carat

   This software is provided 'as-is', without any express or implied warranty.
   In no event will the authors be held liable for any damages arising from the
   use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

       1. The origin of this software must not be misrepresented; you must not
          claim that you wrote the original software. If you use this software
          in a product, an acknowledgment in the product documentation would be
          appreciated but is not required.

       2. Altered source versions must be plainly marked as such, and must not
          be misrepresented as being the original software.

       3. This notice may not be removed or altered from any source
          distribution.
*/
#ifndef _BINARY_H
#define _BINARY_H

#define _00000000b 0x00
#define _00000001b 0x01
#define _00000010b 0x02
#define _00000011b 0x03
#define _00000100b 0x04
#define _00000101b 0x05
#define _00000110b 0x06
#define _00000111b 0x07
#define _00001000b 0x08
#define _00001001b 0x09
#define _00001010b 0x0A
#define _00001011b 0x0B
#define _00001100b 0x0C
#define _00001101b 0x0D
#define _00001110b 0x0E
#define _00001111b 0x0F
#define _00010000b 0x10
#define _00010001b 0x11
#define _00010010b 0x12
#define _00010011b 0x13
#define _00010100b 0x14
#define _00010101b 0x15
#define _00010110b 0x16
#define _00010111b 0x17
#define _00011000b 0x18
#define _00011001b 0x19
#define _00011010b 0x1A
#define _00011011b 0x1B
#define _00011100b 0x1C
#define _00011101b 0x1D
#define _00011110b 0x1E
#define _00011111b 0x1F
#define _00100000b 0x20
#define _00100001b 0x21
#define _00100010b 0x22
#define _00100011b 0x23
#define _00100100b 0x24
#define _00100101b 0x25
#define _00100110b 0x26
#define _00100111b 0x27
#define _00101000b 0x28
#define _00101001b 0x29
#define _00101010b 0x2A
#define _00101011b 0x2B
#define _00101100b 0x2C
#define _00101101b 0x2D
#define _00101110b 0x2E
#define _00101111b 0x2F
#define _00110000b 0x30
#define _00110001b 0x31
#define _00110010b 0x32
#define _00110011b 0x33
#define _00110100b 0x34
#define _00110101b 0x35
#define _00110110b 0x36
#define _00110111b 0x37
#define _00111000b 0x38
#define _00111001b 0x39
#define _00111010b 0x3A
#define _00111011b 0x3B
#define _00111100b 0x3C
#define _00111101b 0x3D
#define _00111110b 0x3E
#define _00111111b 0x3F
#define _01000000b 0x40
#define _01000001b 0x41
#define _01000010b 0x42
#define _01000011b 0x43
#define _01000100b 0x44
#define _01000101b 0x45
#define _01000110b 0x46
#define _01000111b 0x47
#define _01001000b 0x48
#define _01001001b 0x49
#define _01001010b 0x4A
#define _01001011b 0x4B
#define _01001100b 0x4C
#define _01001101b 0x4D
#define _01001110b 0x4E
#define _01001111b 0x4F
#define _01010000b 0x50
#define _01010001b 0x51
#define _01010010b 0x52
#define _01010011b 0x53
#define _01010100b 0x54
#define _01010101b 0x55
#define _01010110b 0x56
#define _01010111b 0x57
#define _01011000b 0x58
#define _01011001b 0x59
#define _01011010b 0x5A
#define _01011011b 0x5B
#define _01011100b 0x5C
#define _01011101b 0x5D
#define _01011110b 0x5E
#define _01011111b 0x5F
#define _01100000b 0x60
#define _01100001b 0x61
#define _01100010b 0x62
#define _01100011b 0x63
#define _01100100b 0x64
#define _01100101b 0x65
#define _01100110b 0x66
#define _01100111b 0x67
#define _01101000b 0x68
#define _01101001b 0x69
#define _01101010b 0x6A
#define _01101011b 0x6B
#define _01101100b 0x6C
#define _01101101b 0x6D
#define _01101110b 0x6E
#define _01101111b 0x6F
#define _01110000b 0x70
#define _01110001b 0x71
#define _01110010b 0x72
#define _01110011b 0x73
#define _01110100b 0x74
#define _01110101b 0x75
#define _01110110b 0x76
#define _01110111b 0x77
#define _01111000b 0x78
#define _01111001b 0x79
#define _01111010b 0x7A
#define _01111011b 0x7B
#define _01111100b 0x7C
#define _01111101b 0x7D
#define _01111110b 0x7E
#define _01111111b 0x7F
#define _10000000b 0x80
#define _10000001b 0x81
#define _10000010b 0x82
#define _10000011b 0x83
#define _10000100b 0x84
#define _10000101b 0x85
#define _10000110b 0x86
#define _10000111b 0x87
#define _10001000b 0x88
#define _10001001b 0x89
#define _10001010b 0x8A
#define _10001011b 0x8B
#define _10001100b 0x8C
#define _10001101b 0x8D
#define _10001110b 0x8E
#define _10001111b 0x8F
#define _10010000b 0x90
#define _10010001b 0x91
#define _10010010b 0x92
#define _10010011b 0x93
#define _10010100b 0x94
#define _10010101b 0x95
#define _10010110b 0x96
#define _10010111b 0x97
#define _10011000b 0x98
#define _10011001b 0x99
#define _10011010b 0x9A
#define _10011011b 0x9B
#define _10011100b 0x9C
#define _10011101b 0x9D
#define _10011110b 0x9E
#define _10011111b 0x9F
#define _10100000b 0xA0
#define _10100001b 0xA1
#define _10100010b 0xA2
#define _10100011b 0xA3
#define _10100100b 0xA4
#define _10100101b 0xA5
#define _10100110b 0xA6
#define _10100111b 0xA7
#define _10101000b 0xA8
#define _10101001b 0xA9
#define _10101010b 0xAA
#define _10101011b 0xAB
#define _10101100b 0xAC
#define _10101101b 0xAD
#define _10101110b 0xAE
#define _10101111b 0xAF
#define _10110000b 0xB0
#define _10110001b 0xB1
#define _10110010b 0xB2
#define _10110011b 0xB3
#define _10110100b 0xB4
#define _10110101b 0xB5
#define _10110110b 0xB6
#define _10110111b 0xB7
#define _10111000b 0xB8
#define _10111001b 0xB9
#define _10111010b 0xBA
#define _10111011b 0xBB
#define _10111100b 0xBC
#define _10111101b 0xBD
#define _10111110b 0xBE
#define _10111111b 0xBF
#define _11000000b 0xC0
#define _11000001b 0xC1
#define _11000010b 0xC2
#define _11000011b 0xC3
#define _11000100b 0xC4
#define _11000101b 0xC5
#define _11000110b 0xC6
#define _11000111b 0xC7
#define _11001000b 0xC8
#define _11001001b 0xC9
#define _11001010b 0xCA
#define _11001011b 0xCB
#define _11001100b 0xCC
#define _11001101b 0xCD
#define _11001110b 0xCE
#define _11001111b 0xCF
#define _11010000b 0xD0
#define _11010001b 0xD1
#define _11010010b 0xD2
#define _11010011b 0xD3
#define _11010100b 0xD4
#define _11010101b 0xD5
#define _11010110b 0xD6
#define _11010111b 0xD7
#define _11011000b 0xD8
#define _11011001b 0xD9
#define _11011010b 0xDA
#define _11011011b 0xDB
#define _11011100b 0xDC
#define _11011101b 0xDD
#define _11011110b 0xDE
#define _11011111b 0xDF
#define _11100000b 0xE0
#define _11100001b 0xE1
#define _11100010b 0xE2
#define _11100011b 0xE3
#define _11100100b 0xE4
#define _11100101b 0xE5
#define _11100110b 0xE6
#define _11100111b 0xE7
#define _11101000b 0xE8
#define _11101001b 0xE9
#define _11101010b 0xEA
#define _11101011b 0xEB
#define _11101100b 0xEC
#define _11101101b 0xED
#define _11101110b 0xEE
#define _11101111b 0xEF
#define _11110000b 0xF0
#define _11110001b 0xF1
#define _11110010b 0xF2
#define _11110011b 0xF3
#define _11110100b 0xF4
#define _11110101b 0xF5
#define _11110110b 0xF6
#define _11110111b 0xF7
#define _11111000b 0xF8
#define _11111001b 0xF9
#define _11111010b 0xFA
#define _11111011b 0xFB
#define _11111100b 0xFC
#define _11111101b 0xFD
#define _11111110b 0xFE
#define _11111111b 0xFF

#endif // !_BINARY_H
