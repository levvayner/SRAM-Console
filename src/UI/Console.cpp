#include "Console.h"


size_t Console::write(uint8_t data)
{
    //Serial.println(data,HEX);
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
    //byte buf[256];
    byte chr=0;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
        
    while(true){
        chr = Serial.read();
        if( chr == 255) continue;
        Serial.print("Read "); Serial.println(chr);
        
        if(chr == 18)
            break;
            
        write(chr);
        //int bytesRead = Serial.readBytes(buf, 256);
        //while(!Serial.available() && lastEntered + 5000 > millis());

        //while(!Serial.available());
        // if(bytesRead <= 0) continue;
        // if(bytesRead >= 2 && buf[0] == ':' && buf[1] == 'q'){
        //     break;
        // }
        
        // write(buf, bytesRead);
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

void Console::_printChar(uint8_t chr)
{
    //clear 6x9 pixel section, draw 5x8 pixel char
    for(uint8_t x = 0;x < PIXEL_WIDTH + 1;x ++){
       
        byte column = x < PIXEL_WIDTH ? CHARS[chr][x] : 0;
        for(int y = 0; y < PIXEL_HEIGHT; y++){
            byte isSet = (column & (1 << y)) > 0;
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
