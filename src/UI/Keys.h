#pragma once
enum ConsoleKeyAction{
    None = 0,
    ASCII = 1,
    Control = 2,
    ColorChange = 3,
    Cursor = 4,
    Exit = 5
};

struct ConsoleKeyPress{
    ConsoleKeyAction action;
    bool isAltPressed = false;
    bool isCtrlPressed = false;
    bool isShiftPressed = false;
    bool isFunctionKey = false;
    bool isCaps = false;
    uint16_t keyCode;
    ConsoleKeyPress(){
        action = None;
        keyCode = 0;
    }
    ConsoleKeyPress(ConsoleKeyAction action, uint16_t keyCode){
        this->action = action;
        this->keyCode = keyCode;
    }
};
