#include "Console.h"


size_t Console::write(uint8_t data)
{
    //Serial.println(data,HEX);
    if(data == 13){
        AdvanceCursor(true);
        return 1;
    }
    if(data ==0x8 ){ //backspace
        ReverseCursor();
        _printChar(0);
        return -1;
    }
    int d = data - 32;
    if(d < 0 || d > sizeof(CHARS) / sizeof(CHARS[0])){
        //print space
        d = 0;
    }
   
    _printChar(d);    
    AdvanceCursor();
    
    return 1;
}

void Console::run()
{
    byte buf[256];
    byte * chr;
    byte yOffset = 2;
    byte xOffset = 2;
    unsigned long lastEntered = millis();
    Serial.println("Enter text to render. :q to quit");
        
    while(true){
        int bytesRead = Serial.readBytes(buf, 256);
        //while(!Serial.available() && lastEntered + 5000 > millis());

        //while(!Serial.available());
        if(bytesRead <= 0) continue;
        if(bytesRead >= 2 && buf[0] == ':' && buf[1] == 'q'){
            break;
        }
        
        write(buf, bytesRead);
        lastEntered = millis();
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
    if (_cursorX + _charWidth * 2 < SCREEN_WIDTH && !nextLine)
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

void Console::_printChar(uint8_t chr)
{
    //clear 5x8 pixel section
    for(int x = 0;x < 5;x ++){
        byte column = CHARS[chr][x];
        for(int y = 0; y < 8; y++){
            auto isSet = (column & (1 << y)) > 0;
            programmer.WriteByte(((y + _cursorY) << 8) + x + _cursorX, isSet ? _color : 0);
        }
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
