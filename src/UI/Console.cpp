#include "Console.h"


size_t Console::write(uint8_t data)
{
    return write(data, _fgColor, true);
}

size_t Console::write(uint8_t data, Color color, bool clearBackround)
{
    Serial.print("Console: Writing data: 0x"); Serial.println(data, HEX);
    if(data == 10){
        AdvanceCursor(true);
        return 1;
    }
    if(data == 13) return 0; //skip CR, only process LF
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
    byte savedColor = _fgColor;
    _fgColor = color.ToByte();
    if(d < 0 || d > sizeof(CHARS) / sizeof(CHARS[0])){
        //print space
        d = 0;
    }
   
    _printChar(d, _cursorX, _cursorY, clearBackround);    
    programmer.WriteByte( 1 << 19 | GetDataPos(),d,1,false); //write to data space
    AdvanceCursor();
    _fgColor = savedColor;
    return 1;
}

void Console::run()
{
    _consoleRunning = true;
    
    _cursorX = 0;
    _cursorY = 0;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
    //DRAW BOTTOM SECTION
    clear();
    clearData();
    DrawStatusBar();
   
    while(_consoleRunning){
        //ps2Controller.loop();
        _checkPort(Serial);
        _checkPort<PS2KeyAdvanced>(keyboardPs2);

        if(_cursorVisible){
            if(millis() - _lastCursorChange > 1200){
                _lastCursorChange = millis();
                _cursorState = !_cursorState;
                _drawCursor();
            }
        }
    }
    _consoleRunning = false;
    clear();
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

void Console::_processKey(char keyVal){
    Serial.print("Processing usb key: "); Serial.println(keyVal);
    write(keyVal);
}

template< typename T>
void Console::_checkPort(T &port)
{
    byte chr=0;
    bool writable = true;

    if(port.available()){
            chr = port.read();
            if( chr == 255) return;
            
            //Serial.print("Read 0x"); Serial.println(chr, HEX);
            if(chr == 0x1B){
                writable = false;
                //escape key. If not directly followed by other chars, treat as escape
                auto startTime = millis();
                while(port.available() < 2 && millis() - startTime < 500);
                if(!port.available())                
                    return;
                
                char nextChar = port.read();
                //Serial.print("Read 0x"); Serial.println(nextChar, HEX);
                //function keys
                if(nextChar == 0x4F ){
                    if(!port.available())  return;
                    nextChar = port.read();
                    switch (nextChar)
                    {
                        case 0x50: //F1
                        case 0x51: //F2
                        {
                            _fgColor++;   
                            _drawColor();                         
                        }
                            break;
                        case 0x53: //F4
                        {
                            //dump data to serial
                            byte buf[BUFFER_SIZE];
                            uint16_t pos = GetDataPos();
                            if(pos <=  0) return;
                            Serial.println("Dumping data contents... ");
                            for(int idx = 0; idx < GetDataPos();idx++){
                                nextChar = programmer.ReadByte(1 << 19 | idx);
                                Serial.print(nextChar);
                            }
                        }
                            return;                        
                        // ...
                    }
                   
                }
                if(nextChar == 0x32 ){ //insert, should be followed by 7E
                    if(port.available() && port.read() == 0x7E){
                        //do whatever insert does
                        Serial.println("Pressed insert");
                    }
                }
                if(nextChar == 0x33 ){ //delete, should be followed by 7E
                    if(port.available() && port.read() == 0x7E){
                        
                                Serial.println("Pressed delete");
                    }
                }

                if(nextChar == 0x5B ){
                    nextChar = port.read();
                    switch (nextChar)
                    {
                        
                            
                        case 0x7E: //delete, should have been preceeded by 0x33
                            Serial.println("Pressed delete");
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
                    //Serial.print("Backspace char 1..");
                    nextChar = port.read();
                    if(nextChar == 0x8){ //backspace
                        //Serial.print("Backspace char 2..");
                        _printChar(0, _cursorX, _cursorY); // get rid of cursor
                        ReverseCursor();
                        //_printChar(0, _cursorX, _cursorY);
                    }
                    //Serial.println();
                    return;
                }
            }
            
            if(chr == 0x8){                
                _printChar(0, _cursorX, _cursorY); // get rid of cursor
                ReverseCursor();
                _printChar(0, _cursorX, _cursorY); // get rid of last char
                return;
            }
            
            if(chr == 18){ //ctrl + r
                _consoleRunning = false;
                return;
            }
             
                
            if(writable) 

                write(chr);
        }
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
    if (_cursorX + graphics.settings.charWidth < graphics.settings.screenWidth && !nextLine)
    {
        _cursorX += graphics.settings.charWidth;
         _drawCursorPosition();
        return;
    }
    //otherwise advance to next available line or beginning
    _cursorX = 0;
    if(_cursorY + _charHeight * 2 < graphics.settings.screenHeight - STATUS_BAR_HEIGHT){
        _cursorY += _charHeight;
    } else {
        _cursorY = 0;
        _fgColor -= 40;
        if(_fgColor < 20) _fgColor = 254;
    }
    _drawCursorPosition();
}

bool Console::ReverseCursor()
{
    if(_cursorX == 0 && _cursorY == 0)
        return false;

    if(_cursorX == 0){
        _cursorY -= _charHeight;
        _cursorX = ((graphics.settings.screenWidth / graphics.settings.charWidth) -1 ) * graphics.settings.charWidth;
        if(_cursorX % 6 != 0) _cursorX -= _cursorX % 6; // adjust to 6 pixel wide char grid
    } else{
        _cursorX -= (graphics.settings.charWidth);
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
    graphics.drawRect(0, graphics.settings.screenHeight - STATUS_BAR_HEIGHT, graphics.settings.screenWidth, 9,Color::DARK_GREEN);
    graphics.fillRect(2, graphics.settings.screenHeight - 9, 6* (graphics.settings.charWidth), 8,Color::FromRGB(2,3,2));
    sprintf(buf,"Cursor");
    _printChars(buf,Color::WHITE, 2, graphics.settings.screenHeight - 9, false);     
    
    _drawCursorPosition();

    graphics.fillRect(79, graphics.settings.screenHeight - 9, graphics.settings.screenWidth - 80, 8, Color::FromRGB(2,3,0));
    
    sprintf(buf,"F2:color     F4:Quit");
    _printChars(buf,Color::WHITE, 80, graphics.settings.screenHeight - 9, false);     
    //see where we end up with color text

    _drawColor();

    sprintf(buf, "Draw Status bar took %lu ms", millis() - startTime);
    Serial.println(buf);
    
}

bool Console::MoveCursorUp()
{
    if(_cursorY < graphics.settings.charHeight + 1) return false;
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
    if(_cursorY >= graphics.settings.screenHeight - STATUS_BAR_HEIGHT - _charHeight - 2 ) return false; //with 2px padding
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
    if(_cursorX >= graphics.settings.screenWidth - (graphics.settings.charWidth)) return false;
    if(_cursorState){
        _cursorState = false;
        _drawCursor();
    }

    _cursorX += graphics.settings.charWidth;
    _cursorState = true;
    _drawCursor();
    _drawCursorPosition();
    return true;
}

bool Console::MoveCursorLeft()
{
    if(_cursorX < (graphics.settings.charWidth)) return false;
    if(_cursorState){
        _cursorState = false;
        _drawCursor();
    }

    _cursorX -= (graphics.settings.charWidth);
    _cursorState = true;
    _drawCursor();
    _drawCursorPosition();
    return true;
}



void Console::_drawCursorPosition(){
    if(!_consoleRunning) return;
    if(_cursorX > graphics.settings.screenWidth || _cursorY > graphics.settings.screenHeight ) return;
    char buf[9];
    
    //cursor x
    memset(buf,0,9);
    graphics.fillRect(6* (graphics.settings.charWidth) + 4, graphics.settings.screenHeight - 9, 18, 8,Color::LIME);    
    sprintf(buf,"%i",_cursorX); 
    _printChars(buf,Color::YELLOW, (6* (graphics.settings.charWidth) + 5), graphics.settings.screenHeight - 9, false);      
    
    
    //cursor y
    memset(buf,0,9);
    graphics.fillRect(10* (graphics.settings.charWidth), graphics.settings.screenHeight - 9, 18, 8,Color::LIME);    
    sprintf(buf,"%i",_cursorY);    
    _printChars(buf,Color::YELLOW, (9 * (graphics.settings.charWidth)) + 7, graphics.settings.screenHeight - 9, false);   

    sprintf(buf,"%i, %i", _cursorX, _cursorY);
    //Serial.print("Set cursor to "); Serial.println(buf);

}

void Console::_drawColor()
{
    char buf[3];
    graphics.fillRect(130, graphics.settings.screenHeight - 9, 24, 8, Color::FromRGB(1,1,0));
     sprintf(buf,"%i", _fgColor);
    _printChars(buf,Color::WHITE, 131, graphics.settings.screenHeight - 9, false);  
    graphics.fillRect(150, graphics.settings.screenHeight - 8, 3, 6, _fgColor);
}

void Console::_drawCursor()
{
    //ifnot visible, hide, otherwise if visible show, else hide
    graphics.drawLine(_cursorX, _cursorY + graphics.settings.charHeight, _cursorX + 6, _cursorY + graphics.settings.charHeight, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK);
}

void Console::_printChar(uint8_t chr, byte charX, byte charY, bool clearBackground)
{
    //clear 6x9 pixel section, draw 5x8 pixel char
    
    //byte drawBuffer[(graphics.settings.charWidth )* graphics.settings.charHeight];

    for(uint8_t x = 0;x < graphics.settings.charWidth - 1;x ++){
       
        byte column = x < graphics.settings.charWidth ? CHARS[chr][x] : 0;
        for(int y = 0; y < graphics.settings.charHeight; y++){
            byte isSet = (column & (1 << y)) > 0;
            if(!isSet && !clearBackground) continue;
            programmer.WriteByte(((y + charY) << 8) + x + charX, isSet ? _fgColor : 0);
        }
        if(clearBackground)
            programmer.WriteByte(((graphics.settings.charHeight + charY) << 8) + x + charX, 0);
        
    }
}

void Console::_printChars(const char* data, Color color, byte charX, byte charY, bool clearBackground)
{
    byte currentColor = _fgColor;
    _fgColor = color.ToByte();
    for(size_t idx = 0; idx < strlen(data);idx++)    
    {
        _printChar(byte((data[idx]) - 32), charX + (idx * (graphics.settings.charWidth)), charY, clearBackground);
    }
    _fgColor = currentColor;
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
