#include "Max7219.h"


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

Max7219::Max7219(uint8_t ssPin, uint8_t devices, SPIClass& spi) : _ssPin(ssPin), _devices(devices), _spi(spi)
{
    pinMode(ssPin, OUTPUT);
    digitalWrite(ssPin, HIGH);

    // We need this here in order to make any device calls in the constructors work
    _spi.begin();
}

void Max7219::decodeNone(uint8_t device)
{
    writereg (MAX7219_DECODE_REG, 0x00, device);
}

#if false
void Max7219::decodeAll(uint8_t device)
{
    writereg (MAX7219_DECODE_REG, 0xff, device);
}
#endif

#ifdef false
void Max7219::decodeDigit (uint8_t digits,  uint8_t display)
{
    writereg (MAX7219_DECODE_REG, digits, display);		
}
#endif

void Max7219::intensitySet (uint8_t val, uint8_t device)
{
    writereg (MAX7219_INTENSITY_REG, val & 0x0f, device);
}


void Max7219::scanDigits (uint8_t val, uint8_t device)
{
    writereg (MAX7219_SCANLIM_REG, val, device);
}


void Max7219::write (uint8_t digit, uint8_t value, uint8_t device)
{
	writereg (MAX7219_DIGIT0_REG + digit, value, device);
}


void Max7219::powerOn (uint8_t device)
{
    writereg (MAX7219_SHUTDOWN_REG, MAX7219_SHUTDOWN_NORMAL, device);
}


void Max7219:: powerOff (uint8_t device)
{
    writereg (MAX7219_SHUTDOWN_REG, MAX7219_SHUTDOWN_SHUTDOWN, device);
}




void Max7219::writereg (uint8_t reg, uint8_t val, uint8_t device)
{
    bool all = false;
    if ((_devices > 1 && device >= _devices) || (_devices == 1))
        all = true;
			
    _spi.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(_ssPin, LOW);
    for (uint8_t i = 0; i < _devices; i++)
    {
        if (i == device || all == true)
        {
            _spi.transfer(reg);
            _spi.transfer(val);
        }
        else
        {
            _spi.transfer((uint8_t)0);
            _spi.transfer((uint8_t)0);
        }
    }
    digitalWrite(_ssPin, HIGH);
    _spi.endTransaction();
}


