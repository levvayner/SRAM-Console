#ifndef _UI_H
#define _UI_H
#include "Arduino.h"
#include "KeyboardController.h"

// #define DEBUG	1
// #define VERIFY 1
#include "Console.h"
#include "Color.h"
#include "SRAM/VRAM.h"
extern SRAM programmer;
extern ProgramRom programRom;
extern Console console;

// Initialize USB Controller
extern USBHost usb;

// Attach keyboard controller to USB
extern KeyboardController keyboard;




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
