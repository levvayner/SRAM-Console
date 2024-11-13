#include "Console.h"


size_t Console::write(uint8_t data)
{
    return write(data, Color::WHITE, true);
}

size_t Console::write(uint8_t data, Color color, bool clearBackround)
{
    Serial.print("Console: Writing data: 0x"); Serial.println(data, HEX);
    if(data == 13){
        AdvanceCursor(true);
        return 1;
    }
    if(data == 10) return 0; //skip LF, only process RC
    // if(data ==0x8 ){ //backspace
    //     _printChar(0, _cursorX, _cursorY); // get rid of cursor
    //     ReverseCursor();
    //     _printChar(0, _cursorX, _cursorY);
    //     return -1;
    // }
    // if(data == 54){
    //     AdvanceCursor();
    // }
    // if(data == 55){
    //     ReverseCursor();
    // }
    
    int d = data - 32;
    byte savedColor = _color;
    _color = color.ToByte();
    if(d < 0 || d > sizeof(CHARS) / sizeof(CHARS[0])){
        //print space
        d = 0;
    }
   
    _printChar(d, _cursorX, _cursorY, clearBackround);    
    AdvanceCursor();
    _color = savedColor;
    return 1;
}

void Console::run()
{
    _consoleRunning = true;
    //byte buf[256];
    byte chr=0;
    _cursorX = 0;
    _cursorY = 0;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
    //DRAW BOTTOM SECTION
    clear();
    DrawStatusBar();
   
    bool writable = true;
    while(true){
        writable = true;
        if(Serial.available()){
            chr = Serial.read();
            if( chr == 255) continue;
            
            Serial.print("Read 0x"); Serial.println(chr, HEX);
            if(chr == 0x1B){
                writable = false;
                //escape key. If not directly followed by other chars, treat as escape
                auto startTime = millis();
                while(Serial.available() < 2 && millis() - startTime < 500);
                if(!Serial.available())                
                    continue;
                
                char nextChar = Serial.read();

                //function keys
                if(chr == 0x4F ){
                    if(!Serial.available())  continue;
                    nextChar = Serial.read();
                    switch (nextChar)
                    {
                        case 0x50: //F1
                        case 0x51: //F2
                            break;
                        // ...
                    }
                    Serial.println("Pressed Function key F"); Serial.print(chr - 0x49);
                }
                if(nextChar == 0x32 ){ //insert, should be followed by 7E
                    if(Serial.available() && Serial.read() == 0x7E){
                        //do whatever insert does
                        Serial.println("Pressed insert");
                    }
                }
                if(nextChar == 0x33 ){ //delete, should be followed by 7E
                    if(Serial.available() && Serial.read() == 0x7E){
                        
                                Serial.println("Pressed delete");
                    }
                }

                if(nextChar == 0x5B ){
                    nextChar = Serial.read();
                    switch (nextChar)
                    {
                        
                            
                        case 0x7E: //delete, should have been preceeded by 0x33

                            break;
                        case 0x41: //up arrow
                            //Serial.println("Pressed up arrow");
                            MoveCursorUp();
                            break;   
                        case 0x42: //down arrow
                            //Serial.println("Pressed down arrow");
                            MoveCursorDown();
                            break;          
                        case 0x43: //right arrow                    
                            //Serial.println("Pressed right arrow");
                            MoveCursorRight();
                            break;
                        case 0x44: //left arrow
                            //Serial.println("Pressed left arrow");
                            MoveCursorLeft();
                            break;
                        case 0x46: //end
                            break;
                        case 0x48: //home
                            break;
                        default:
                            break;
                    }
                }

                if(nextChar == 0x65 ){
                    Serial.print("Backspace char 1..");
                    nextChar = Serial.read();
                    if(nextChar == 0x8){ //backspace
                        Serial.print("Backspace char 2..");
                        _printChar(0, _cursorX, _cursorY); // get rid of cursor
                        ReverseCursor();
                        //_printChar(0, _cursorX, _cursorY);
                    }
                    Serial.println();
                    continue;
                }
            }
            
            
            if(chr == 18) //ctrl + r
                break;
                
            if(!writable) continue;

            write(chr);
        }

        if(_cursorVisible){
            if(millis() - _lastCursorChange > 1200){
                _lastCursorChange = millis();
                _cursorState = !_cursorState;
                _drawCursor();
            }
        }
    }
    _consoleRunning = false;
}

void Console::processUSBKey()
{
    auto mods = keyboard.getModifiers();
    if(mods != 0){
        //process control keys
    } else{
        //regular keys
    }
    char keyVal = keyboard.getKey();

    _processKey(keyVal);

}

void Console::_processKey(char keyVal){
    Serial.print("Processing usb key: "); Serial.println(keyVal);
    write(keyVal);
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

void Console::AdvanceCursor(bool nextLine)
{
    //see if we can move over one pixel to the right
    if (_cursorX + CHAR_WIDTH +1 < SCREEN_WIDTH && !nextLine)
    {
        _cursorX += CHAR_WIDTH + 1;
         _drawCursorPosition();
        return;
    }
    //otherwise advance to next available line or beginning
    _cursorX = 0;
    if(_cursorY + _charHeight * 2 < SCREEN_HEIGHT - STATUS_BAR_HEIGHT){
        _cursorY += _charHeight;
    } else {
        _cursorY = 0;
        _color -= 40;
        if(_color < 20) _color = 254;
    }
    _drawCursorPosition();
}

bool Console::ReverseCursor()
{
    if(_cursorX == 0 && _cursorY == 0)
        return false;

    if(_cursorX == 0){
        _cursorY -= _charHeight;
        _cursorX = ((SCREEN_WIDTH / CHAR_WIDTH) -1 ) * CHAR_WIDTH;
    } else{
        _cursorX -= (CHAR_WIDTH + 1);
    }
    _drawCursorPosition();
    return true;
}

/// @brief status bar is one row high with top and bottom border (10px)
void Console::DrawStatusBar()
{
    char buf[256];
    memset(buf,0,256);
    unsigned long startTime = millis();
    graphics.drawRect(0, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, SCREEN_WIDTH, 9,Color::DARK_GREEN);
    graphics.fillRect(2, SCREEN_HEIGHT - 9, 6* (CHAR_WIDTH + 1), 8,Color::GREEN);
    sprintf(buf,"Cursor");
    _printChars(buf,Color::WHITE, 2, SCREEN_HEIGHT - 9, false);     
    
    _drawCursorPosition();

    sprintf(buf, "Draw Status bar took %lu ms", millis() - startTime);
    Serial.println(buf);
    
}

bool Console::MoveCursorUp()
{
    if(_cursorY < CHAR_HEIGHT + 1) return false;
    //get rid of current
    if(_cursorState){
        _cursorState = false;
        _drawCursor();
    }

    _cursorY -= _charHeight;
    _cursorState = true;
    _drawCursor();
    _drawCursorPosition();
    return true;
}

bool Console::MoveCursorDown()
{
    if(_cursorY >= SCREEN_HEIGHT - STATUS_BAR_HEIGHT - _charHeight - 2 ) return false; //with 2px padding
    if(_cursorState){
        _cursorState = false;
        _drawCursor();
    }

    _cursorY += _charHeight;
    _cursorState = true;
    _drawCursor();
    _drawCursorPosition();
    return true;
}

bool Console::MoveCursorRight()
{
    if(_cursorX >= SCREEN_WIDTH - (CHAR_WIDTH + 1)) return false;
    if(_cursorState){
        _cursorState = false;
        _drawCursor();
    }

    _cursorX += CHAR_WIDTH + 1;
    _cursorState = true;
    _drawCursor();
    _drawCursorPosition();
    return true;
}

bool Console::MoveCursorLeft()
{
    if(_cursorX < (CHAR_WIDTH + 1)) return false;
    if(_cursorState){
        _cursorState = false;
        _drawCursor();
    }

    _cursorX -= (CHAR_WIDTH + 1);
    _cursorState = true;
    _drawCursor();
    _drawCursorPosition();
    return true;
}



void Console::_drawCursorPosition(){
    if(!_consoleRunning) return;
    if(_cursorX > SCREEN_WIDTH || _cursorY > SCREEN_HEIGHT ) return;
    char buf[9];
    
    //cursor x
    memset(buf,0,9);
    graphics.fillRect(6* (CHAR_WIDTH + 1) + 4, SCREEN_HEIGHT - 9, 18, 8,Color::LIME);    
    sprintf(buf,"%i",_cursorX); 
    _printChars(buf,Color::YELLOW, (6* (CHAR_WIDTH + 1) + 5), SCREEN_HEIGHT - 9, false);      
    
    
    //cursor y
    memset(buf,0,9);
    graphics.fillRect(10* (CHAR_WIDTH + 1), SCREEN_HEIGHT - 9, 18, 8,Color::LIME);    
    sprintf(buf,"%i",_cursorY);    
    _printChars(buf,Color::YELLOW, (9 * (CHAR_WIDTH + 1)) + 7, SCREEN_HEIGHT - 9, false);   

    sprintf(buf,"%i, %i", _cursorX, _cursorY);
    //Serial.print("Set cursor to "); Serial.println(buf);

}

void Console::_drawCursor()
{
    //ifnot visible, hide, otherwise if visible show, else hide
    graphics.drawLine(_cursorX, _cursorY + CHAR_HEIGHT, _cursorX + 6, _cursorY + CHAR_HEIGHT, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK);
}

void Console::_printChar(uint8_t chr, byte charX, byte charY, bool clearBackground)
{
    //clear 6x9 pixel section, draw 5x8 pixel char
    
    //byte drawBuffer[(CHAR_WIDTH + 1 )* CHAR_HEIGHT];

    for(uint8_t x = 0;x < CHAR_WIDTH + 1;x ++){
       
        byte column = x < CHAR_WIDTH ? CHARS[chr][x] : 0;
        for(int y = 0; y < CHAR_HEIGHT; y++){
            byte isSet = (column & (1 << y)) > 0;
            if(!isSet && !clearBackground) continue;
            programmer.WriteByte(((y + charY) << 8) + x + charX, isSet ? _color : 0);
        }
        if(clearBackground)
            programmer.WriteByte(((CHAR_HEIGHT + charY) << 8) + x + charX, 0);
        
    }
}

void Console::_printChars(const char* data, Color color, byte charX, byte charY, bool clearBackground)
{
    byte currentColor = _color;
    _color = color.ToByte();
    for(size_t idx = 0; idx < strlen(data);idx++)    
    {
        _printChar(byte((data[idx]) - 32), charX + (idx * (CHAR_WIDTH + 1)), charY, clearBackground);
    }
    _color = currentColor;
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

// size_t Console::print(uint8_t data)
// {
//     write(data + 48);
// }

size_t Console::print(char c)
{
    write(c + 48);
}
