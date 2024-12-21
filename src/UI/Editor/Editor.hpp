#ifndef _EDITOR_H
#define _EDITOR_H
#include "Arduino.h"
#include "../Console/Console.hpp"
#include "Programming/ProgramRom.h"
#include "KeyboardController.h"

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
    void open(const char* filename);
    bool save();
    virtual inline void SetPosition(int x = 0, int y = 0, bool drawPosition = true){ Console::SetPosition(x,y); if(drawPosition && _isEditorRunning) {_drawCursorPosition();}}
    //virtual ConsoleKeyPress processPS2Key(uint8_t ps2KeyCode);

    bool AdvanceCursor(bool nextLine = false);
    bool ReverseCursor();
    bool MoveCursorDown();
    bool MoveCursorUp();

    protected: 
    
    void DrawStatusBar();
    private:

    
    void _drawCursorPosition();
    void _drawLineNo();
    void _drawColor();
    void _drawFilename();

    private: 
    bool _isEditorRunning;
    char* _fileName = nullptr;
    bool _isNewFile = false;
};

#endif