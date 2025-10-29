#ifndef _UI_H
#define _UI_H
#include "Arduino.h"
#include "KeyboardController.h"

// #define DEBUG	1
// #define VERIFY 1
#include "Console/Console.hpp"
#include "Editor/Editor.hpp"
#include "sw/GPU.h"
#include "Screensaver/Screensaver.h"
#include "Screensaver/ScreensaverMandelbrot.h"
extern SRAM programmer;
extern VRAM graphics;
extern GPU gpu;
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

	
	void PrintMenu(bool force = false);

	void DumpRAM();
	void ClearScreen();
	void ProcessInput();
	//void GetChipCount();
	//void PrintOpCode(uint8_t opCode, bool condJump);
	void PrintOperations(uint8_t op);
    inline void setProgrammingMode(bool mode){ 
        _programmingMode = mode;
    }

     Graphics2D* blockObject = nullptr;
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
	
    unsigned long updateFrequency = 200;
    unsigned long lastUpdated = 0;
    unsigned long checkingTime = 0;
    bool _programmingMode = false;

   

};
#endif
extern UI ui;