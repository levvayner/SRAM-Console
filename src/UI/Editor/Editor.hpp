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
    bool open(const char* filename);
    bool save();
    void stop();
    virtual inline void SetPosition(int x = 0, int y = 0, bool drawPosition = true){ Console::SetPosition(x,y); if(drawPosition && _isEditorRunning) {_drawCursorPosition();}}
    //virtual ConsoleKeyPress processPS2Key(uint8_t ps2KeyCode);

    bool AdvanceCursor(bool nextLine = false);
    bool ReverseCursor();
    bool MoveCursorDown();
    bool MoveCursorUp();
    inline bool IsEditorRunning(){ return _isEditorRunning;}

    void processKey(uint8_t keyCode);

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
    KeyInputMode _currentInputMode;

    //visual objects owned by editor
    GraphicsObject2D filenameView = GraphicsObject2D(new Rectangle2D(300, graphics.settings.screenHeight - STATUS_BAR_HEIGHT, graphics.settings.screenWidth - 305, 9,Fill), Color::YELLOW);
    GraphicsObject2D statusBar = GraphicsObject2D(new Rectangle2D(0, graphics.settings.screenHeight - STATUS_BAR_HEIGHT, graphics.settings.screenWidth, 9,Fill), Color::DARK_GREEN);
    GraphicsObject2D lineNo = GraphicsObject2D(new Rectangle2D(150, graphics.settings.screenHeight - 9, 32, 8,Fill),  Color::FromRGB(1,1,0).ToByte());
};

#endif