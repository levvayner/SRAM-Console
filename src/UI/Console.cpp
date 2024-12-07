#include "Console.hpp"


size_t Console::write(uint8_t data, bool useFrameBuffer)
{
    return write(data, textColor, backgroundColor, true);
}
size_t Console::write(uint8_t data, byte color, byte backgroundColor, bool clearBackround, bool useFrameBuffer)
{
    auto pos = 1 << 19 | GetDataPos();
    EraseCursor();
    //Serial.print("Console: Writing data: "); Serial.print(data); Serial.print(" at position: 0x"); Serial.println(pos, HEX);
    if(data == 10 && _consoleRunning){
        if(!useFrameBuffer)
            programmer.WriteByte( pos,data);
        AdvanceCursor(true);
        return 1;
    }
    if(data == 13) return 0; 

    graphics.drawText(_cursorX,_cursorY, data, color, backgroundColor, clearBackround, useFrameBuffer, btVertical);
    
    if(_consoleRunning && !useFrameBuffer) {
        programmer.WriteByte( pos,data); //write to data space
        programmer.WriteByte( pos | (1 << 18),color); //store color info
    }
    AdvanceCursor();

    return 1;
}
size_t Console::write(uint8_t data, Color color, Color backgroundColor, bool clearBackround, bool useFrameBuffer)
{
    return write(data,color.ToByte(), backgroundColor.ToByte(), clearBackround, useFrameBuffer);
}

size_t Console::write(const char *buffer, size_t size, Color color, Color backgroundColor,  bool clearBackground, bool useFrameBuffer){
    for(size_t idx = 0; idx < size; idx++)
        write(buffer[idx], color, backgroundColor, clearBackground, useFrameBuffer);
    return size;
}

size_t Console::write(const uint8_t *buffer, size_t size)
{
    //we want to write to a buffer, and then push the buffer of pixels out
    for(size_t idx = 0; idx < size; idx++)
        write(buffer[idx], textColor, backgroundColor, true, true);

    
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
        textColor--;  
        return ColorChange;
    case PS2_KEY_F3:
        textColor++;  
        return ColorChange;
    case PS2_KEY_F4:
        _consoleRunning = false;
    default:
        Serial.print("Received unmapped ps2 key: 0x"); Serial.println(ps2KeyCode, HEX);
        break;
    }
}

void Console::_drawTextFromRam()
{
    uint32_t pos = _scrollOffset * (charsPerLine);
    uint32_t endPos = min(_lastIdx, (_windowHeight / graphics.settings.charHeight) * charsPerLine);
    
    //check how many bytes we can fit, updat end pos accordingly.
    char buf[charsPerLine + 1];
    byte colorBuf[charsPerLine];
    uint8_t lineBuf[graphics.settings.charHeight * graphics.settings.screenWidth];

    //programmer.ReadBytes(pos | (1 << 19), buf, endPos - pos + 1 );
    

    _consoleRunning = false;            
    clear();        
    char textBuf[128];

    for(uint32_t line = 0; line < (ceil(( endPos - pos + 1) / charsPerLine)); line++){      
        bool lineHasText = false;
        uint16_t lineLength =  min(endPos - pos, charsPerLine );
        memset(buf, 0, charsPerLine + 1);
        memset(lineBuf,backgroundColor, graphics.settings.charHeight * graphics.settings.screenWidth );
        //for(uint16_t idx = 0; idx < charsPerLine; idx++){
        programmer.ReadBytes((pos + (line * (charsPerLine + 1))) | (1 << 19), (uint8_t*)buf, lineLength);
        programmer.ReadBytes((pos + (line * (charsPerLine + 1))) | (3 << 18) , (uint8_t*)colorBuf, lineLength);
        
            //char val = programmer.ReadByte( (pos + (line * charsPerLine) + idx) | 1 << 19 );
            
            //Serial.print(isascii(val) ? val : ' ');
            // lineHasText |= isascii(val);
            // buf[idx] = val;
        //}
        for(uint16_t idx = 0; idx < lineLength; idx++){
            if((buf[idx] >= 32 && buf[idx] < 127) || buf[idx] == 10) lineHasText = true;
            //if( isascii(buf[idx]) && buf[idx] != 0) lineHasText = true;
            else buf[idx] = ' ';
        }
        if(lineHasText){
            
            sprintf(textBuf, "Writing %i chars from idx %d to %d on line %lu", strlen(buf), line * charsPerLine, (line * charsPerLine) + lineLength - 1, line);
            Serial.println(textBuf);
            Serial.println(buf);
            graphics.drawTextToBuffer(buf, colorBuf, lineBuf, graphics.settings.screenWidth);
            graphics.drawBuffer(0, line * graphics.settings.charHeight, graphics.settings.screenWidth, graphics.settings.charHeight, lineBuf);
        }
        //auto length = programmer.ReadBytes(1 << 19 | (pos + (line * charsPerLine)), (uint8_t*)buf,min(72, endPos - pos));  
        
    }

    // _cursorY = (ceil(( endPos - pos + 1) / charsPerLine)) * graphics.settings.charHeight;
    // _cursorX = ((endPos - pos + 1) % charsPerLine ) * graphics.settings.charWidth;

    
    
    //redraw the text from the scroll offset line
    // for(; pos < endPos ;pos+= BUFFER_SIZE){
    //     auto length = programmer.ReadBytes(1 << 19 | pos, (uint8_t*)buf,min(BUFFER_SIZE, endPos - pos));
    //     write((const char*)buf,length, Color(textColor), Color(backgroundColor) );
    // }
    //graphics.render();
    programmer.ReadByte(0x0); 
    //graphics.render();
    _consoleRunning = true;
}

void Console::_processKey(char keyVal)
{
    Serial.print("Processing usb key: "); Serial.println(keyVal);
    write(keyVal);
}

bool Console::AdvanceCursor(bool nextLine)
{
    _lastIdx = max(GetDataPos(), _lastIdx);
    //see if we can move over one pixel to the right
    if (_cursorX + graphics.settings.charWidth < graphics.settings.charWidth * charsPerLine && !nextLine)
    {
        _cursorX += graphics.settings.charWidth;
        return false;
    }
    //otherwise advance to next available line 
    if(!nextLine && _consoleRunning){
        Serial.print("Advancing to new line, injecting NL into data cache at address 0x"); Serial.println(GetDataPos(), HEX);
        programmer.WriteByte( 1 << 19 | (GetDataPos() + 1) ,10,1);      
        programmer.ReadByte(0); //turn off 19th bit     
    }

    _cursorX = 0;
    //if we need to scroll down
    if(_cursorY + 2* graphics.settings.charHeight  >= graphics.settings.screenHeight - STATUS_BAR_HEIGHT  && _consoleRunning){
        _scrollOffset++;
        _drawTextFromRam();
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
    if(_cursorY <= graphics.settings.charHeight + 1) 
    {
        if(_scrollOffset <= 0) return false;
        
        _scrollOffset--;
        _drawTextFromRam();
    }
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
    if(_cursorY >= _windowHeight- graphics.settings.charHeight - 2 ) return false; //with 2px padding
    if(_cursorState){
        _cursorState = false;
        DrawCursor();
    }

     //if we need to scroll down
    if(_cursorY + 2* graphics.settings.charHeight  >= graphics.settings.screenHeight - STATUS_BAR_HEIGHT  && _consoleRunning){
        _scrollOffset++;
        _drawTextFromRam();
    } else //otherwise move down one
        _cursorY += graphics.settings.charHeight;
    _cursorVisible = true;    
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
    _cursorVisible = true;
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
    _cursorVisible = true;
    DrawCursor();
    return true;
}


byte scratch[16];
void Console::DrawCursor()
{
    //ifnot visible, hide, otherwise if visible show, else hide
    
    memset(scratch, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, scratch, graphics.settings.charWidth);
    //graphics.drawLine(_cursorX, _cursorY + graphics.settings.charHeight, _cursorX + 6, _cursorY + graphics.settings.charHeight, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK);
}

void Console::EraseCursor()
{
    memset(scratch, 0, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, scratch, graphics.settings.charWidth);
    
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
