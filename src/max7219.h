//***************************************************************************
//
//  File Name :		MAX7219.h
//
//  Project :		Arduino style libraries
//
//  Purpose :		Wrapper for the MAX7219 LED display driver
//
// The MIT License (MIT)
//
// Copyright (c) 2017-2023 Andy Burgess
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//  Revisions :
//
//      see rcs below
//
//***************************************************************************

#ifndef __MAX7219_H__
#define __MAX7219_H__


#include <SPI.h>

#ifndef _BV
#define _BV(x) (1<<x)
#endif

#define MAX7219_DIGIT0_REG 0x01
#define MAX7219_DIGIT1_REG 0x02
#define MAX7219_DIGIT2_REG 0x03
#define MAX7219_DIGIT3_REG 0x04
#define MAX7219_DIGIT4_REG 0x05
#define MAX7219_DIGIT5_REG 0x06
#define MAX7219_DIGIT6_REG 0x07
#define MAX7219_DIGIT7_REG 0x08
#define MAX7219_DECODE_REG 0x09
#define MAX7219_INTENSITY_REG 0x0A
#define MAX7219_SCANLIM_REG 0x0B
#define MAX7219_SHUTDOWN_REG 0x0C
#define MAX7219_DISPLAYTEST_REG 0x0F

// Definitions for decode register
#define MAX7219_DECODE_DIGIT0 _BV(0)
#define MAX7219_DECODE_DIGIT1 _BV(1)
#define MAX7219_DECODE_DIGIT2 _BV(2)
#define MAX7219_DECODE_DIGIT3 _BV(3)
#define MAX7219_DECODE_DIGIT4 _BV(4)
#define MAX7219_DECODE_DIGIT5 _BV(5)
#define MAX7219_DECODE_DIGIT6 _BV(6)
#define MAX7219_DECODE_DIGIT7 _BV(7)

// Definitions for shutdown register
#define MAX7219_SHUTDOWN_SHUTDOWN 0x00
#define MAX7219_SHUTDOWN_NORMAL 0x01

// Special characters
#define MAX7219_CHAR_DASH	0x0A
#define MAX7219_CHAR_E		0x0B
#define MAX7219_CHAR_H		0x0C
#define MAX7219_CHAR_L		0x0D
#define MAX7219_CHAR_P		0x0E
#define MAX7219_CHAR_BLANK	0x0F


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


//! \brief MAX7219/22 interface class
//! \param pin Pin to use for the SPI !SS
//! \details A template class to interface with one or more cascaded
//!	MAX7219.  When creating the object pass the number of cascaded devices
//! in the constructor, otherwise the default of 1 will be used.  All methods
//! that write to the device have their last parameter as the device to write to.
//! If not used, it will default to 0xFF which will write to all devices, or the
//! only device.
template <uint8_t pin>
class Max7219
{
//variables
public:
protected:
private:
    static const int spiClk = 1000000; // 1 MHz
    SPIClass& _spi;
	const uint8_t devices;
    uint8_t _ss;

//functions
public:
	//! \brief Initialises a new instance of the Max7219 class
	//! \param devices Specifies the number of devices in the cascade
	//! \details The Max7219 class can specify the number of devices in the cascade.
	//! If not specified, then the default is 1 device
	Max7219(uint8_t devices = 1, SPIClass& spi = SPI) : devices(devices), _spi(spi)
	{
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
	}
	//~max7219();

	//! \brief Set the device so that all digits are set to raw mode
	//! \param device The device to write to, if set; otherwise will default
	//! to all devices
	//! \details In raw mode the value passed in the write methods is a bit
	//! pattern of the led segments to be set.  This is equivalent to calling decodeDigits(0x00);
	inline void decodeNone(uint8_t device=0xFF) //__attribute__((always_inline))
	{
		writereg (MAX7219_DECODE_REG, 0x00, device);
	}

	//! \brief Set the device so that all digits are set to code-B
	//! \param display The device to write to, if set; otherwise will default to
	//! all devices
	//! \details In code-B, uses the chips build-in character generator to 
	//! decode 0..16 to 0..9, -, E, H, L, P, <blank>, DP. This is equivalent to calling decodeDigits(0xff);
	inline void decodeAll(uint8_t device=0xFF) //__attribute__((always_inline))
	{
		writereg (MAX7219_DECODE_REG, 0xff, device);
	}

	// Selectively decode some digits based on the bit pattern supplied
	//! \brief Sets the device so that specified digits are set to raw or b-code mode
	//! \param digits A bit pattern that specifies which of the digits should be set to b-code.
	//! A '1' specifies b-code decoding
	//! \param device The device to set the decoding for, if set; otherwise defaults
	//! to all devices
	//! \details In raw mode the value passed in the write methods is a bit pattern of the led
	//! segments to be set.  In b-code mode the internal build-in character generator to
	//! decode 0..16 to 0..0, -, E, H, L, P, <blank>, DP
	inline void decodeDigit (uint8_t digits,  uint8_t display=0xFF) //__attribute__((always_inline))
	{
		writereg (MAX7219_DECODE_REG, digits, display);		
	}


	// Intensity register functions==========================================
	// sets the intensity from 1/32 to 31/32 (7219) or 1/16 to 15/16 (7221)
	// for values 0x00 to 0x0F
	// The high nibble is ignored by the chip
	//! \brief Sets the intensity of the device
	//! \param val The intensity to set.
	//! \param device The device to set, if specified; otherwise default to all devices
	//! \details For 7219 devices, the intensity is 1/32 to 31/32. For 7221 devices 1/16 to 15/16
	//! for intensity values of 0-15.
	inline void intensitySet (uint8_t val, uint8_t device=0xFF)
	{
		writereg (MAX7219_INTENSITY_REG, val & 0x0f, device);
	}


	// Scan limit register functions=========================================
	//! \brief Sets the number of digits to scan
	//! \param val The number of digits to scan
	//! \param device The device to set, if specified, otherwise default to all devices
	//! \details 1 = scan digit 0 only, 2 = scan digits 1 & 2 ... 8 = scan digits 0-7.  The 
	//! top 5 bits are ignored by the device
	inline void scanDigits (uint8_t val, uint8_t device=0xFF) //__attribute__((always_inline))
	{
		writereg (MAX7219_SCANLIM_REG, val-1, device);
	}
	

	// Digit registers functions===========================================
	
	//! \brief Writes a value to the led segments
	//! \param digit The digit to write to
	//! \param value The value to write
	//! \param device The device to write to, if set; otherwise default to all devices
	//! \detail When the digit is set to raw mode a value of 0-255 will specify the segments to
	//! write.  In b-code mode, a value of 0-15 will use the character generator to specify the segments
	//! to set. The characters are 0..9, -, E, H, L, P, <blank> for 0..15 and 0x8x for DP
	inline void write (uint8_t digit, uint8_t value, uint8_t device = 0xFF) //__attribute__((always_inline))
	{
		writereg (MAX7219_DIGIT0_REG + digit, value, device);
	}
	
	//! \brief Sets the supplied digit register to the hex representation of the supplied value
	//! \param digit The digit to write to
	//! \param value The value to write
	//! \param device The device to write to, if set; otherwise defaults to all devices
	//! \detail The digit must be set to raw for this method to operate correctly, using a lookup table
	//! to generate the led segments to set
	inline void writeHex (uint8_t digit, uint8_t value, uint8_t device = 0xFF) //__attribute__((always_inline))
	{
		if (digit >= 8) digit = 8;				// keep digit in range
		writereg (MAX7219_DIGIT0_REG + (digit & 0x0f), hexLookup[value & 0x0f], device);
	}


	// Shutdown register functions===========================================
	//! \brief Sets the device normal operating mode
	//! \param device The device to write to, if set; otherwise defaults to all devices
	//! \detail Returns the device to normal operating mode upon startup or if manually set
	//! to shutdown mode
	inline void powerOn (uint8_t device = 0xFF)
	{
		writereg (MAX7219_SHUTDOWN_REG, MAX7219_SHUTDOWN_NORMAL, device);
	}


	//! \brief Sets the device shutdown mode
	//! \param device The device to write to, if set; otherwise defaults to all devices
	//! \detail Sets the device to shutdown mode, blanking all segments on all digits
	inline void powerOff (uint8_t device = 0xFF)
	{
		writereg (MAX7219_SHUTDOWN_REG, MAX7219_SHUTDOWN_SHUTDOWN, device);
	}

	// Miscellaneous functions===============================================	
	//! \brief Clears the device
	//! \param device The device to write to, if set; otherwise defaults to all devices
	//! \detail In raw mode, all segments are cleared.  In b-code mode all digits will
	//! be set to 0
	void clear(uint8_t device = 0xFF)
	{
		for (uint8_t i = 0; i < 8; i++)
			writereg (MAX7219_DIGIT0_REG + i, 0x00, device);
	}


	protected:
	private:
		Max7219( const Max7219 &c );
		Max7219& operator=( const Max7219 &c );


	// Writes a register/value pair to the chip
	void writereg (uint8_t reg, uint8_t val, uint8_t device)
	{
		bool all = false;
		if ((devices > 1 && device >= devices) || (devices == 1))
			all = true;
			
        _spi.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
		for (uint8_t i = 0; i < devices; i++)
		{
			if (i == device || all == true)
			{
                digitalWrite(pin, LOW);
                _spi.transfer(reg);
                _spi.transfer(val);
                digitalWrite(pin, HIGH);
			}
			else
			{
                digitalWrite(pin, LOW);
                _spi.transfer((uint8_t)0);
                _spi.transfer((uint8_t)0);
                digitalWrite(pin, HIGH);
			}
		}
        _spi.endTransaction();
	}
}; //Max7219

#endif //__MAX7219_H__
