
#include "Max7219.h"

// Lookup table for hex characters
//   AAAA
//  F    B
//	F    B
//	 GGGG
//  E    C
//  E    C
//   DDDD 
static uint8_t hexLookup [] =
{
	//.ABCDEFG
	0b01111110,	// 0
	0b00110000,	// 1
	0b01101101,	// 2
	0b01111001,	// 3
	//.ABCDEFG
	0b00110011,	// 4
	0b01011011,	// 5
	0b01011111,	// 6
	0b01110000,	// 7
	//.ABCDEFG
	0b01111111,	// 8
	0b01111011,	// 9
	0b01110111,	// A
	0b00011111,	// b
	//.ABCDEFG
	0b01001110,	// C
	0b00111101,	// d
	0b01001111,	// E
	0b01000111,	// F
};

Max7219SevenSeg::Max7219SevenSeg(const uint8_t ssPin, const uint8_t devices, SPIClass& spi) : Max7219(ssPin, devices, spi)
{
    clear();
    decodeNone(0);
}


void Max7219SevenSeg::clear(uint8_t device)
{
    for (uint8_t i = 0; i < 8; i++)
        writeHex(i, 0);
}


void Max7219SevenSeg::writeHex (uint8_t digit, uint8_t value, uint8_t device)
{
	if (digit >= 8)
        return;
	writereg (MAX7219_DIGIT0_REG + (digit & 0x0f), hexLookup[value & 0x0f], device);
}

