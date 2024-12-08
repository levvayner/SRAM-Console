#ifndef _EDITOR_H
#define _EDITOR_H
#include "Arduino.h"
#include "Color.h"
#include "Chars.h"
#include "Console.hpp"
#include "SRAM/SRAM.h"
#include "SRAM/VRAM.h"
#include "Programming/ProgramRom.h"
#include "KeyboardController.h"
#include "IO/ps2KeyboardController.h"

/*
Editor has two primary responsibilities
   1. Render the editor/editor window
   2. Store entered data into a data buffer in the RAM.

   A0-A15 (currently, soon A0-A18) are used for x and y coordinates.
   A19 high, or upper half the RAM, will be reserved for off the screen stuff, leaving 512kB for each.

*/

#define STATUS_BAR_HEIGHT 10

extern SRAM programmer;
extern VRAM graphics;
extern ProgramRom programRom;
extern KeyboardController keyboardUsb;
extern ps2KeyboardController ps2Controller;




class Editor : public Console{

    public:

    void run();
    virtual inline void SetPosition(int x = 0, int y = 0, bool drawPosition = true){ Console::SetPosition(x,y); if(drawPosition && _isEditorRunning) {_drawCursorPosition();}}
    //virtual ConsoleKeyPress processPS2Key(uint8_t ps2KeyCode);

    bool AdvanceCursor(bool nextLine = false);
    bool ReverseCursor();
    bool MoveCursorDown();
    bool MoveCursorUp();

    protected: 
    
    void DrawStatusBar();
    
    

    //bool MoveCursorUp();
    
    // bool MoveCursorRight();
    // bool MoveCursorLeft();
  
    

    private:

    
    void _drawCursorPosition();
    void _drawLineNo();
    void _drawColor();

    private: 
    bool _isEditorRunning;

    // void _processKey(char keyVal);
    // template< typename T>
    // void _checkPort(T & port);
};

#endif