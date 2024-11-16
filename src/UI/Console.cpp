#include "Console.hpp"


size_t Console::write(uint8_t data)
{
    return write(data, _fgColor, true);
}
size_t Console::write(uint8_t data, byte color, bool clearBackround)
{
    auto pos = 1 << 19 | GetDataPos();
    EraseCursor();
    //Serial.print("Console: Writing data: "); Serial.print(data); Serial.print(" at position: 0x"); Serial.println(pos, HEX);
    if(data == 10 && _consoleRunning){
        programmer.WriteByte( pos,data);
        AdvanceCursor(true);
        return 1;
    }
    if(data == 13) return 0; 

    graphics.drawText(_cursorX,_cursorY, data, color, clearBackround);
    if(_consoleRunning) 
        programmer.WriteByte( pos,data); //write to data space
    AdvanceCursor();

    return 1;
}
size_t Console::write(uint8_t data, Color color, bool clearBackround)
{
    return write(data,color.ToByte(),clearBackround);
}

size_t Console::write(const char *buffer, size_t size, Color color, bool clearBackground){
    for(size_t idx = 0; idx < size; idx++)
        write(buffer[idx], color, clearBackground);
    return size;
}

size_t Console::write(const uint8_t *buffer, size_t size)
{
    for(size_t idx = 0; idx < size; idx++)
        write(buffer[idx]);
    return size;
}

size_t Console::println(void)
{
    return write(13);
}

void Console::run(bool blocking )
{
    _consoleRunning = true;
    
    _cursorX = 0;
    _cursorY = 0;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
    //DRAW BOTTOM SECTION
    clear();
    clearData();    
   
    while(_consoleRunning && blocking){
        //ps2Controller.loop();
        checkPort(Serial);
        checkPort<PS2KeyAdvanced>(keyboardPs2);

        if(_cursorVisible){
            if(millis() - _lastCursorChange > 1200){
                _lastCursorChange = millis();
                _cursorState = !_cursorState;
                DrawCursor();
            }
        }
    }
    if(!_consoleRunning) clear();
}



void Console::processUSBKey()
{
    auto mods = keyboardUsb.getModifiers();
    if(mods != 0){
        //process control keys
    } else{
        //regular keys
    }
    char keyVal = keyboardUsb.getKey();

    _processKey(keyVal);

}
ConsoleKeyAction Console::processPS2Key(uint8_t ps2KeyCode)
{
   
    switch (ps2KeyCode)
    {
    case PS2_KEY_F2:
        _fgColor--; 
        return ColorChange;
    case PS2_KEY_F3:
        _fgColor++; 
        return ColorChange;
    case PS2_KEY_F4:
        _consoleRunning = false;
    default:
        Serial.print("Received unmapped ps2 key: 0x"); Serial.println(ps2KeyCode, HEX);
        break;
    }
}

void Console::_processKey(char keyVal){
    Serial.print("Processing usb key: "); Serial.println(keyVal);
    write(keyVal);
}




bool Console::AdvanceCursor(bool nextLine)
{
    //see if we can move over one pixel to the right
    if (_cursorX + graphics.settings.charWidth < graphics.settings.screenWidth && !nextLine)
    {
        _cursorX += graphics.settings.charWidth;
        return false;
    }
    //otherwise advance to next available line 
    if(!nextLine && _consoleRunning){
        Serial.print("Advancing to new line, injecting NL into data cache at address 0x"); Serial.println(GetDataPos(), HEX);
        programmer.WriteByte( 1 << 19 | (GetDataPos() + 1) ,10,1,false);      
        programmer.ReadByte(0); //turn off 19th bit     
    }

    _cursorX = 0;
    //if we need to scroll down
    if(_cursorY + graphics.settings.charHeight + 2  >= _windowHeight && _consoleRunning){
        _scrollOffset++;
        uint32_t pos = _scrollOffset * (graphics.settings.screenWidth / graphics.settings.charWidth);
        uint32_t endPos = GetDataPos();
        char nextChar;
        char buf[256];

        _consoleRunning = false;        
        _cursorY = 0;
        clear();        
        
        sprintf(buf,"Redrawing characters from offset %i to position %i", pos, endPos);
        Serial.println(buf);
        //redraw the text from the scroll offset line
        for(; pos < endPos ;pos++){
            nextChar = programmer.ReadByte(1 << 19 | pos);            
            write(nextChar);
        }
        programmer.ReadByte(0x0); 
        _consoleRunning = true;
    } else //otherwise move down one
        _cursorY += graphics.settings.charHeight;
    
    
    return true;
    
}

bool Console::ReverseCursor()
{
    if(_cursorX == 0 && _cursorY == 0)
        return false;

    if(_cursorX == 0){
        _cursorY -= graphics.settings.charHeight;
        _cursorX = ((graphics.settings.screenWidth / graphics.settings.charWidth) -1 ) * graphics.settings.charWidth;
        if(_cursorX % 6 != 0) _cursorX -= _cursorX % 6; // adjust to 6 pixel wide char grid
    } else{
        _cursorX -= (graphics.settings.charWidth);
    }
    return true;
}


bool Console::MoveCursorUp()
{
    if(_cursorY < graphics.settings.charHeight + 1) return false;
    //get rid of current
    if(_cursorState){
        _cursorState = false;
        DrawCursor();
    }

    _cursorY -= graphics.settings.charHeight;
    _cursorState = true;
    DrawCursor();
    return true;
}

bool Console::MoveCursorDown()
{
    if(_cursorY >= graphics.settings.screenHeight - graphics.settings.charHeight - 2 ) return false; //with 2px padding
    if(_cursorState){
        _cursorState = false;
        DrawCursor();
    }

    _cursorY += graphics.settings.charHeight;
    _cursorState = true;
    DrawCursor();
    return true;
}

bool Console::MoveCursorRight()
{
    if(_cursorX >= graphics.settings.screenWidth - (graphics.settings.charWidth)) return false;
    if(_cursorState){
        _cursorState = false;
        DrawCursor();
    }

    _cursorX += graphics.settings.charWidth;
    _cursorState = true;
    DrawCursor();
    return true;
}

bool Console::MoveCursorLeft()
{
    if(_cursorX < (graphics.settings.charWidth)) return false;
    if(_cursorState){
        _cursorState = false;
        DrawCursor();
    }

    _cursorX -= (graphics.settings.charWidth);
    _cursorState = true;
    DrawCursor();
    return true;
}



void Console::DrawCursor()
{
    //ifnot visible, hide, otherwise if visible show, else hide
    graphics.drawLine(_cursorX, _cursorY + graphics.settings.charHeight, _cursorX + 6, _cursorY + graphics.settings.charHeight, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK);
}

void Console::EraseCursor()
{
    graphics.drawLine(_cursorX, _cursorY + graphics.settings.charHeight, _cursorX + graphics.settings.charWidth + 1, _cursorY + graphics.settings.charHeight,  Color::BLACK);
}

size_t Console::print(const char* str)
{
    return this->write(str);
}

size_t Console::println(const char* str)
{
    this->write(str);
    this->AdvanceCursor(true);
    return strlen(str);
}

// size_t Console::print(uint8_t data)
// {
//     write(data + 48);
// }

size_t Console::print(char c)
{
    return write(c + 48);
}
