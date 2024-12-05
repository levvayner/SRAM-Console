#ifndef _SRAM_H_
#define _SRAM_H_
#include "Arduino.h"
#include "Pins.h"

#define SRAM_SIZE (unsigned long)1048576 //20 bit address

#define BUFFER_SIZE 1 << 10
#define RETRY_COUNT 3
#define ERASE_BYTE 0 //value denoting an erased byte

enum DeviceState {
	dsOff = 0,
	dsOutput = 1,
	dsWrite = 2
};
class SRAM
{
public:
	SRAM();
	~SRAM();


	void DeviceOff();
	void DeviceOutput();
	void DeviceWrite();
	//MAX ADDR is 2048 with 11 address lines

	//construct byte from data bits
	uint8_t ReadByte(uint32_t addr);
    size_t ReadBytes(uint32_t addr, uint8_t* buffer, uint32_t length);
	
	// void WriteFirstByte(uint8_t data, uint16_t offsetAddress = 0);
	// void WriteNextByte(uint8_t data);
	bool WriteByte(uint32_t addr, uint8_t data, uint8_t retryCount = RETRY_COUNT, bool showDebugData = true);
	bool WriteShort(uint32_t addr, uint16_t data, bool showDebugData = true);
    uint16_t WriteBytes(uint32_t addr, uint8_t* data, uint32_t length);

    void Erase(uint32_t startAddress = 0x0, uint32_t length = SRAM_SIZE);

private:
	uint16_t counter = 0;
	uint16_t _retries = 0;
    // uint8_t _portAddressLower = PORTA;
    // uint8_t _portAddressUpper = PORTC;
    // uint8_t _portData = PORTL;

protected:

	void SetAddress(uint32_t addr);
	void SetDataLines(uint8_t data);

	void BinToSerial(uint8_t var);
	
	DeviceState ramState;
};


#endif
