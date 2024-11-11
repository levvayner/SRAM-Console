#include "Console.h"


size_t Console::write(uint8_t data)
{
    write(data, true);
}

size_t Console::write(uint8_t data, Color color, bool clearBackround)
{
    if(data == 13){
        AdvanceCursor(true);
        return 1;
    }
    if(data == 10) return 0; //skip LF, only process RC
    if(data ==0x8 ){ //backspace
        ReverseCursor();
        _printChar(0);
        return -1;
    }
    
    int d = data - 32;
    byte savedColor = _color;
    _color = color.ToByte();
    if(d < 0 || d > sizeof(CHARS) / sizeof(CHARS[0])){
        //print space
        d = 0;
    }
   
    _printChar(d,clearBackround);    
    AdvanceCursor();
    _color = savedColor;
    return 1;
}

void Console::run()
{
    //byte buf[256];
    byte chr=0;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
    //DRAW BOTTOM SECTION
    DrawStatusBar();
    _cursorX = 0;
    _cursorY = 0;
        
    while(true){
        chr = Serial.read();
        if( chr == 255) continue;
        Serial.print("Read "); Serial.println(chr);
        
        if(chr == 18)
            break;
            
        write(chr);
    }
}



size_t Console::write(const uint8_t *buffer, size_t size)
{
    for(int idx = 0; idx < size; idx++)
        write(buffer[idx]);
}

size_t Console::println(void)
{
    return write(13);
}

void Console::AdvanceCursor(bool nextLine)
{
    //see if we can move over one pixel to the right
    if (_cursorX + _charWidth +1 < SCREEN_WIDTH && !nextLine)
    {
        _cursorX += _charWidth;
        return;
    }
    //otherwise advance to next available line or beginning
    _cursorX = 0;
    if(_cursorY + _charHeight * 2 < SCREEN_HEIGHT){
        _cursorY += _charHeight;
    } else {
        _cursorY = 0;
        _color -= 40;
        if(_color < 20) _color = 254;
    }
}

bool Console::ReverseCursor()
{
    if(_cursorX == 0 && _cursorY == 0)
        return false;

    if(_cursorX == 0){
        _cursorY -= _charHeight;
        _cursorX = ((SCREEN_WIDTH / _charWidth) -1 ) * _charWidth;
    } else{
        _cursorX -= _charWidth;
    }
    return true;
}

/// @brief status bar is one row high with top and bottom border (10px)
void Console::DrawStatusBar()
{

    
}

void Console::_printChar(uint8_t chr, bool clearBackground)
{
    //clear 6x9 pixel section, draw 5x8 pixel char
    for(uint8_t x = 0;x < PIXEL_WIDTH + 1;x ++){
       
        byte column = x < PIXEL_WIDTH ? CHARS[chr][x] : 0;
        for(int y = 0; y < PIXEL_HEIGHT; y++){
            byte isSet = (column & (1 << y)) > 0;
            if(!isSet && !clearBackground) continue;
            programmer.WriteByte(((y + _cursorY) << 8) + x + _cursorX, isSet ? _color : 0);
        }
        
        programmer.WriteByte(((PIXEL_HEIGHT + _cursorY) << 8) + x + _cursorX, 0);
        
    }
}

size_t Console::print(const char* str)
{
    return this->write(str);
}

size_t Console::println(const char* str)
{
    this->write(str);
    this->AdvanceCursor(true);
}
