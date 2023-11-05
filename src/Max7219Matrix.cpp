#include "Max7219.h"


Max7219Matrix::Max7219Matrix (const uint8_t ssPin, const uint8_t devices, SPIClass& spi) : Max7219(ssPin, devices, spi)
{ 
    _matrixData = (uint8_t*)malloc(8 * devices);
    clear();
    decodeNone(); 				// set no decoding
    scanDigits(0x07);           // we're a matrix, surely we want to see all rows
    powerOn();				  	// set no shutdown
}


void Max7219Matrix::clear(uint8_t device)
{
    memset(_matrixData, 0, 8 * _devices);
	for (uint8_t i = 0; i < 8; i++)
        spiTransfer(i);
}


bool Max7219Matrix::setRow(uint8_t row, uint32_t val) 
{
    if (row > 7) return false;
    for (uint8_t i = 0; i < _devices; i++)
    {
        _matrixData [row, i] = (uint8_t)(val & 0xff);
        val >>= 8;
    }
    spiTransfer(row);
    return true;
}


bool Max7219Matrix::setCol(uint8_t col, uint8_t val)
{
    Serial.print("col, val: "); Serial.print(col); Serial.print (" "); Serial.println(val);
    //if (col >= _devices * 8) return false;
    uint8_t device = col / 8;
    uint8_t bit = col % 8;
    uint8_t regVal = _BV(bit);
    for (uint8_t row = 0; row < 8; row++)
    {
        // calculate the bit position mask
        uint8_t rowMask = _BV(row);
        //for (uint8_t i = 0; i < 8; i++)
        //{
            //uint8_t colMask = val & _BV(i);
            if (val & rowMask)
                _matrixData[row, device] |= regVal;
            else
                _matrixData[row, device] &= ~regVal;
            spiTransfer(row);
        //}
    }
    return true;
}


bool Max7219Matrix::setLed(uint8_t row, uint8_t col, bool val)
{
    if (col > 7 || row > 7) return false;

    return true;
}


void Max7219Matrix::spiTransfer(uint8_t row)
{
    _spi.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(_ssPin, LOW);
    for (uint8_t i = 0; i < _devices; i++)
    {
        _spi.transfer(MAX7219_DIGIT0_REG + row);
        _spi.transfer(_matrixData[row, i]);
    }
    digitalWrite(_ssPin, HIGH);
    _spi.endTransaction();
}



