#ifndef _UI_H
#define _UI_H
#include "Arduino.h"
#include "KeyboardController.h"

// #define DEBUG	1
// #define VERIFY 1
#include "Console/Console.hpp"
#include "Editor/Editor.hpp"
#include "Screensaver.h"
#include "ScreensaverMandelbrot.h"
extern SRAM programmer;
extern VRAM graphics;
extern ProgramRom programRom;
extern Console console;
extern Editor editor;


// Initialize USB Controller
extern USBHost usb;

// Attach keyboard controller to USB
//extern KeyboardController keyboard;




class UI
{
public:
	UI();
	~UI();

    void begin();
	
	void blinkLED();

	
	void PrintMenu();

	void DumpRAM();
	void ClearScreen();
	void ProcessInput();
	//void GetChipCount();
	//void PrintOpCode(uint8_t opCode, bool condJump);
	void PrintOperations(uint8_t op);

private:
    template <typename TPort>
    void _processInput(TPort port);

    template <typename TPort>
    String _getResponse(TPort port);
private:
	bool ledState = 0;
	unsigned long lastToggle = 0;
	unsigned long toggleDuration = 500;
	bool needPrintMenu = true;
	
    unsigned long updateFrequency = 1000;
    unsigned long lastUpdated = 0;
    unsigned long checkingTime = 0;

};
#endif
extern UI ui;