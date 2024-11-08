#include "Console.h"


size_t Console::write(uint8_t data)
{
    
    //clear 5x8 pixel section
    for(int x = 0;x < 5;x ++){
        byte column = CHARS[data][x];
        for(int y = 0; y < 8; y++){
            auto isSet = (column & (1 << y)) > 0;
            programmer.WriteByte(((y + _cursorY) << 8) + x + _cursorX, isSet ? 240 : 0);
        }
    }
    
    AdvanceCursor();
    
    return 1;
}

size_t Console::write(const uint8_t *buffer, size_t size)
{
    for(int idx = 0; idx < size; idx++)
        write(buffer[idx]);
}

void Console::AdvanceCursor()
{
    //see if we can move over one pixel to the right
    if (_cursorX + _charWidth * 2 < SCREEN_WIDTH)
    {
        _cursorX += _charWidth;
        return;
    }
    //otherwise advance to next available line or beginning
    _cursorX = 0;
    if(_cursorY + _charHeight * 2 < SCREEN_HEIGHT){
        _cursorY += _charHeight;
    } else _cursorY = 0;
}
