#ifndef _SRAM_H_
#define _SRAM_H_
#include "Arduino.h"
#include "Pins.h"

#define SRAM_SIZE 64*1024 //we will use lower 16 bits for now /* 1024*1024*/

#define BUFFER_SIZE 256
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
	uint8_t ReadByte(uint16_t addr);
	
	// void WriteFirstByte(uint8_t data, uint16_t offsetAddress = 0);
	// void WriteNextByte(uint8_t data);
	bool WriteByte(uint16_t addr, uint8_t data, uint8_t retryCount = RETRY_COUNT, bool showDebugData = true);
	bool WriteShort(uint16_t addr, uint16_t data, bool showDebugData = true);
    uint16_t WriteBytes(uint16_t addr, uint8_t* data, uint16_t length);

    void EraseRam();

private:
	uint16_t counter = 0;
	uint16_t _retries = 0;

protected:

	void SetAddress(uint16_t addr);
	void SetDataLines(uint8_t data);

	void BinToSerial(uint8_t var);
	
	DeviceState ramState;
};


#endif
