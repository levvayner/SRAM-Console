#ifndef _UI_H
#define _UI_H
#include "Arduino.h"

#include "SRAM/SRAM.h"
#include "Programming/ProgramRom.h"
//#define DEBUG	1
// #define VERIFY 1
#define SCREEN_WIDTH 138
#define SCREEN_HEIGHT 150
extern SRAM programmer;
extern ProgramRom programRom;

class UI
{
public:
	UI();
	~UI();


	
	void blinkLED();

	
	void PrintMenu();

	void DumpRAM();
	void EraseRAM();
	void ProcessInput();
	//void GetChipCount();
	//void PrintOpCode(uint8_t opCode, bool condJump);
	void PrintOperations(uint8_t op);

private:
	bool ledState = 0;
	unsigned long lastToggle = 0;
	unsigned long toggleDuration = 500;
	bool needPrintMenu = true;
	//uint8_t chipCount = 2;

};
#endif
