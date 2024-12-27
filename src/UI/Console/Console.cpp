#include "Console.hpp"
#include "../Editor/Editor.hpp"
#include "ConsoleCommands.h"
extern Editor editor;
extern Console console;


void consoleProcessKey(uint8_t data){
    //Serial.print("Received key 0x"); Serial.println(data, HEX);
    console.processKey(data);
    
}
void consoleDrawCursor(){
    //Serial.println("Console draw cursor");
    console.ToggleCursor();
    console.DrawCursor();
}

size_t Console::write(uint8_t data, bool useFrameBuffer)
{
    return write(data, textColor, backgroundColor, true, useFrameBuffer);
}
size_t Console::write(uint8_t data, byte color, byte backgroundColor, bool clearBackround, bool useFrameBuffer)
{
    auto pos = 1 << 19 | GetDataPos();
    EraseCursor();
    //Serial.print("Console: Writing data: "); Serial.print(data); Serial.print(" at position: 0x"); Serial.println(pos, HEX);
    if(data == 10 /* && _consoleRunning*/ ){
        programmer.WriteByte( pos,data); //write to data space
        AdvanceCursor(true);
        return 1;
    }
    if(data == 13) {
        //Serial.print("Processing last char of cmd");
        return 0; 
        
    }

    graphics.drawText(_cursorX,_cursorY, (char)data, color, backgroundColor, clearBackround, useFrameBuffer);
    
    if(_consoleRunning && !useFrameBuffer) {
        //Serial.print("Stored command to 0x"); Serial.println(pos,HEX);
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
        write(buffer[idx], textColor, backgroundColor, true, false);

    
    return size;
}



void Console::run(bool blocking )
{
    _scrollOffset = 0;
    _consoleRunning = blocking;
    if(blocking){
        //keyboard.SetMode(true);
        commands.clearCommands(CONTEXT_CONSOLE);
        registerConsoleCommands();
        _commandMode = true;
    }
    keyboard.onKeyDown = consoleProcessKey;
    
    //    _cursorTimer = Timer.getAvailable().attachInterrupt(consoleDrawCursor);
    Serial.println("Enter text to render. Ctrl+R to quit");
    charsPerLine = graphics.settings.screenWidth / graphics.settings.charWidth;
    clear();    
    if(blocking){
        _echoPrompt = true;
        if(!_initialized)
            _initSD();
        clearData(); 
        _cursorX = 0;
        _cursorY = 0;
        //SetEchoMode(false);
        //SetEchoMode(true);
    }
    _needEcho = true;
    mouse.begin();
    ShowCursor();
}

void Console::loop()
{
    if(!_consoleRunning) return;
    if(_needEcho){
        _printEcho();
        _needEcho = false;
    }
    if(_commandReady){
        _echoPrompt = false;
        _cursorState = false;
        _commandMode = false;
        EraseCursor();

        //Serial.print("Receieved command: "); Serial.println(cmdBuf);
        //check if registered command
        auto command = commands.buildCommand(_cmdBuf);
        HideCursor();
        AdvanceCursor(true);
        if(command.valid){     
            command.onExecute(command);
        }else{            
            println("Invalid command");           
        }
        AdvanceCursor(true);
        ShowCursor();
        memset(_cmdBuf,0,sizeof(_cmdBuf));
        _cmdBufIdx = 0;
        _commandReady = false;


        _echoPrompt = true;
        _commandMode = true;
        _needEcho = true;
    }
    mouse.update();
}

void Console::stop()
{
    _consoleRunning = false;
    _commandMode = false;
    commands.clearCommands(CONTEXT_CONSOLE);
    keyboard.onKeyDown = nullptr;
    //keyboard.SetMode(false);
    if(_cursorVisible){
        _cursorTimer->detachInterrupt();
        _cursorTimer = nullptr;
    }
    mouse.end();
}

void Console::_drawTextFromRam()
{
    uint32_t pos = _scrollOffset * (charsPerLine);
    uint32_t endPos = min(_lastIdx, ((_windowHeight / graphics.settings.charHeight) * charsPerLine));
    uint16_t startLine = 0;// _scrollOffset;
    uint16_t endLine = /*_scrollOffset + */(graphics.settings.screenHeight / graphics.settings.charHeight);
    if(endPos < pos) return; // nothing to print
    
    //check how many bytes we can fit, updat end pos accordingly.
    char buf[charsPerLine + 1];
    byte colorBuf[charsPerLine];
    uint8_t lineBuf[graphics.settings.charHeight * graphics.settings.screenWidth];
    bool consoleState = _consoleRunning;
    _consoleRunning = false;            
    clear();        
    // char textBuf[128];
    // sprintf(textBuf, "Drawing from position %lu to %lu on lines %u to %u",
    //     pos, endPos, startLine, endLine 
    // );
    // Serial.println(textBuf);

    for(uint32_t line = startLine; line < endLine; line++){      
        if((line * charsPerLine) > endPos){
            //Serial.print("Line is ahead of endpos, breaking");
            break; //done
        }
        //Serial.print("Drawing line "); Serial.println(line);
        bool lineHasText = false;
        uint16_t lineLength =  min(endPos - pos, charsPerLine );
        memset(buf, 0, charsPerLine + 1);
        memset(lineBuf,backgroundColor, graphics.settings.charHeight * graphics.settings.screenWidth );
        memset(colorBuf, textColor,sizeof(colorBuf));
        //for(uint16_t idx = 0; idx < charsPerLine; idx++){
        programmer.ReadBytes((pos + (line * (charsPerLine ))) | (1 << 19), (uint8_t*)buf, lineLength);
        //if(!consoleState) //we don't do colors in console
        programmer.ReadBytes((pos + (line * (charsPerLine ))) | (3 << 18) , (uint8_t*)colorBuf, lineLength);
        
            
        for(uint16_t idx = 0; idx < lineLength; idx++){
            if((buf[idx] >= 32 && buf[idx] < 127)) lineHasText = true;
            //if( isascii(buf[idx]) && buf[idx] != 0) lineHasText = true;
            else buf[idx] = ' ';
        }
        if(lineHasText){
            
            // sprintf(textBuf, "Writing %u chars from idx %u to %lu on line %lu", strlen(buf), line * charsPerLine, (line * charsPerLine) + lineLength - 1, line);
            // Serial.println(textBuf);
            // Serial.println(buf);
            //graphics.drawText(0, (line - _scrollOffset) * graphics.settings.charHeight, buf, textColor, backgroundColor,true, false, btVolatile);
            graphics.drawTextToBuffer(buf, colorBuf, lineBuf, graphics.settings.screenWidth);
            graphics.drawBuffer(0, line * graphics.settings.charHeight, graphics.settings.screenWidth, graphics.settings.charHeight, lineBuf, btVolatile);
        }
    }
    //Serial.println("-----------------------");
    programmer.ReadByte(0x0); 
    //graphics.render();
    _consoleRunning = consoleState;
}

void Console::_printEcho()
{
    
    if(_consoleRunning && _commandMode && _echoPrompt){
        char buf[256];
        // sprintf(
        //     buf,
        //     "Running: %s, Mode: %s, Echo: %s",
        //     _consoleRunning ? "Yes" : "No", _commandMode ? "Command" : "Text", _echoPrompt ? "Yes" : "No"
        // );
        // Serial.println(buf);
        echoY = _cursorY;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%s |>", _path.c_str());
        // Serial.print("Path: "); Serial.println(buf);
        _promptLength = strlen(buf);
        // Serial.print("printing to "); Serial.print(_cursorX);Serial.print(", ");Serial.println(_cursorY);
        //graphics.drawText(_cursorX, _cursorY, buf, 255, backgroundColor);
        byte color = textColor;
        textColor = 255;
        write((const char*)buf,_promptLength);
        textColor = color;
        //_cursorX += strlen(buf) * graphics.settings.charWidth;
        
        //write((const char*)buf);
    }
}

void Console::_initSD()
{
   
    if (!card.init(SPI_HALF_SPEED, 10) && this->_commandMode) {
        println("initialization failed. Things to check:");
        println("* is a card inserted?");
        println("* is your wiring correct?");
        println("* did you change the chipSelect pin to match your shield or module?");
        return;
    }
   
    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card) && _commandMode) {
        println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
        return;
    }
    //root.close();
    _initialized = true;
}

void Console::_initCurrentCommand()
{
    memset(_currentCommand, 0, sizeof(_currentCommand));
    _currentCommandIdx = 0;
    _currentInputMode = Text;
}

int Console::getCoords(const char *str, int *coords, uint32_t offset)
{
    int coordsFound = 0;
    //coords = {0};
    uint32_t cursorIdx = (uint32_t)offset;
    for(; cursorIdx < strlen(str); cursorIdx++){
        if(str[cursorIdx] ==','){
            memset(_scratch.bytes, 0 , sizeof(_scratch));
            memcpy(_scratch.bytes, str + offset, cursorIdx - offset);
            //Serial.print("Coord found: "); Serial.println((const char *)_scratch);
            coords[coordsFound] = atoi(_scratch.text);
            coordsFound++;
            offset = cursorIdx + 1;
        }
    }
    if(offset < cursorIdx){
        memset(_scratch.bytes, 0 , sizeof(_scratch));
        memcpy(_scratch.bytes, str + offset, cursorIdx - offset);
        coords[coordsFound] = atoi(_scratch.text);
        coordsFound++;
    }
    // Serial.print("found "); Serial.print(coordsFound); Serial.print(" coords");
    // for(int idx = 0; idx < coordsFound; idx++){
    //     Serial.print(coords[idx]); Serial.print("   ");
    // }
    // Serial.println();
    return coordsFound;
}

///Processing key can be different from different keyboard sources. PS2 and Serial monitor behave differently.
// Serial monitor provides input as char arrays, with commands havings codes to set mode, potentially more codes to set modes, and then command
// PS2 maps directly each key to a byte code
inline void Console::processKey(uint8_t keyCode)
{
    if(keyCode == 13) return; //ignore carriage return, new line advances to beginning of line
    _cmdBuf[_cmdBufIdx++] = keyCode;

    

    if(keyCode == 10 && _commandMode)
    {
        _commandReady = true;
        _saveCommand();
        _initCurrentCommand();
    }
    else{
        if( keyCode == 255) return;
        _currentCommand[_currentCommandIdx++] = keyCode;
        Serial.print("Read 0x"); Serial.println(keyCode, HEX);
        
        if(keyCode == 0x1B){ //todo, handle escape key
        Serial.println("Setting mode to ESC");
            _currentInputMode = Escape;
            return;            
        } 
        if(_currentInputMode == Escape){
            //function keys
            if(keyCode == 0x4F ){
                _currentInputMode = Function;
                return;
            }
            if(keyCode == 0x32 ){ //insert, should be followed by 7E
                //do whatever insert does
                Serial.println("Pressed insert");                    
                _currentInputMode = Command;
            
            }
            if(keyCode == 0x33 ){ //delete, should be followed by 7E           
                Serial.println("Pressed delete");
                _currentInputMode = Command;
            }
            if(keyCode == 0x7E ){ 
                _currentInputMode = Text;
            }

            if(keyCode == 0x5B ){
                 Serial.println("Setting mode to Extended Command mode");
                _currentInputMode = CommandExtended;
                
            }
            return;
        }

        if(_currentInputMode == Function){
            Serial.print("Processing function 0x"); Serial.println(keyCode, HEX);
            //return;
            switch (keyCode)
            {
                case 0x50: //F1
                case 0x51: //F2
                {
                    textColor--; 
                    return;                         
                }
                    
                case 0x53: //F4
                {
                    textColor++; 
                    return; 
                }                       
                // ...
            }
        }        
        if(_currentInputMode == CommandExtended){   
            Serial.print("Processing Extended Command mode key: "); Serial.println(keyCode);
            switch (keyCode)
            {
                case 0x33: //delete, should have been preceeded by 0x33
                    Serial.println("Pressed delete");
                    return;
                case 0x41: //up arrow
                    Serial.println("Pressed up arrow");
                    EraseCursor();
                    MoveCursorUp();
                    return;
                case 0x42: //down arrow
                    Serial.println("Pressed down arrow");
                    EraseCursor();
                    MoveCursorDown();
                    return;         
                case 0x43: //right arrow                    
                    Serial.println("Pressed right arrow");
                    EraseCursor();
                    MoveCursorRight();
                    return;
                case 0x44: //left arrow
                    Serial.println("Pressed left arrow");
                    EraseCursor();
                    MoveCursorLeft();
                    return;
                case 0x46: //end
                    break;
                case 0x48: //home
                    break;
                case 0x7E:
                    _currentInputMode = Text;
                    break;
                default:
                    break;
            }
            return;
        }

        if(keyCode == 0x8){     
            if((_echoPrompt && (echoY != _cursorY || _cursorX > (_promptLength * graphics.settings.charWidth)) ) || (!_echoPrompt) ){        
                        
                EraseCursor();
                //_printChar(0, _cursorX, _cursorY); // get rid of cursor
                ReverseCursor();
                graphics.fillRectangle(_cursorX,_cursorY, graphics.settings.charWidth, graphics.settings.charHeight, Color::BLACK);                
            }
            return;
        }
        
        if(keyCode == 0x12){ //ctrl + r
            stop();
            Serial.println("Closing console");
            return;
        } 
        if(_currentInputMode == Text){
            write(keyCode);
            Serial.print("Received key"); Serial.println(keyCode);
        }       
    
    }
}

bool Console::AdvanceCursor(bool nextLine)
{    
    _lastIdx = max(GetDataPos(), _lastIdx);
    //see if we can move over one pixel to the right
    if (_cursorX + graphics.settings.charWidth < graphics.settings.charWidth * charsPerLine && !nextLine)
    {
        //Serial.println ("**\tAdvancing char");
        _cursorX += graphics.settings.charWidth;
        return false;
    }
    //otherwise advance to next available line 
    if(!nextLine && _consoleRunning){
        //Serial.println ("**\tAdvancing line");
        Serial.print("Advancing to new line, injecting NL into data cache at address 0x"); Serial.println(GetDataPos(), HEX);
        // programmer.WriteByte( 1 << 19 | (GetDataPos() + 1) ,10,1);      
        // programmer.ReadByte(0); //turn off 19th bit   
                 
    }
    
    _cursorX = 0;
    // if(nextLine && _consoleRunning)
    // {
    //     if(_echoPrompt) _printEcho();  
    // }
    
    //if we need to scroll down
    if(_cursorY + 2* graphics.settings.charHeight  >= graphics.settings.screenHeight  && !_commandMode){
        _scrollOffset++;
        Serial.println ("**\tScrolling down");
        _drawTextFromRam();
    } else{ //otherwise move down one
        _cursorY += graphics.settings.charHeight;
        Serial.println("Moving cursor down.");
    }
    
    
    return true;
    
}

bool Console::ReverseCursor()
{
    if(_cursorX == 0 && _cursorY == 0)
        return false;
    
    if(GetDataPos() == LastIdx()) _lastIdx--;

    if(_cursorX == 0){
        _cursorY -= graphics.settings.charHeight;
        _cursorX = ((graphics.settings.screenWidth / graphics.settings.charWidth) -1 ) * graphics.settings.charWidth;
        if(_cursorX % 6 != 0) _cursorX -= _cursorX % 6; // adjust to 6 pixel wide char grid
    } else //if(!_echoPrompt || _cursorX > _promptLength * graphics.settings.charWidth){
        _cursorX -= (graphics.settings.charWidth);
    //}
    return true;
}


bool Console::MoveCursorUp()
{
    if(_commandMode){
        //TODO: replace contents with previous command in cache
        if(--_commandViewIdx > _history.length()) _commandViewIdx = _history.index();
        Serial.print("Replace with previous at idx "); Serial.println(_commandViewIdx);
        memset(_cmdBuf, 0, sizeof(_cmdBuf));
        const char* buf;
        buf = _history.read(_commandViewIdx);
        _cmdBufIdx =  strlen(buf);
        memcpy(_cmdBuf,buf, _cmdBufIdx);
        memcpy(_currentCommand, buf, _cmdBufIdx);
        
        
        _cursorX = _echoPrompt ? _promptLength * graphics.settings.charWidth : 0;
        graphics.clear(_cursorX, _cursorY, graphics.settings.screenWidth - _cursorX, graphics.settings.charHeight);
        write(buf);
        return true;
    }
    if(_cursorY <= 0) 
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

    if(_cursorY > 0) _cursorY -= graphics.settings.charHeight;
    _cursorState = true;
    DrawCursor();
    return true;
}

bool Console::MoveCursorDown()
{
    if(_commandMode){
        //TODO: replace contents with next command in cache        
        if(++_commandViewIdx >_history.length()) {
            _commandViewIdx = 0;
            Serial.print("History Length: "); Serial.println(_history.length());
            Serial.print("Replace with next at idx "); Serial.println(_commandViewIdx);
        }
        
        const char* buf;
        buf = _history.read(_commandViewIdx);
        memcpy(_currentCommand, buf, strlen(buf));
        _cursorX = _echoPrompt ? _promptLength * graphics.settings.charWidth : 0;
        graphics.clear(_cursorX, _cursorY, graphics.settings.screenWidth - _cursorX, graphics.settings.charHeight);
        write(buf);
        return true;
    }
    if(_cursorY >= _windowHeight- graphics.settings.charHeight - 2 ) return false; //with 2px padding
    if(_cursorState){
        _cursorState = false;
        DrawCursor();
    }

     //if we need to scroll down
    if(_cursorY + 2* graphics.settings.charHeight  >= graphics.settings.screenHeight - STATUS_BAR_HEIGHT  && !_commandMode){
        _scrollOffset++;
        _drawTextFromRam();
    } else //otherwise move down one
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
    if(!_cursorVisible) return;
    //if not visible, hide, otherwise if visible show
    memset(_scratch.bytes, _cursorState ? Color::WHITE : Color::BLACK, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, _scratch.bytes, graphics.settings.charWidth);
    
}

void Console::EraseCursor()
{
    if(!_cursorVisible) return;
    memset(_scratch.bytes, 0, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, _scratch.bytes, graphics.settings.charWidth);
    
}

void Console::printDiskInfo()
{
    console.SetEchoMode(false);
   
    print("Card type:         ");

    switch (card.type()) {
        case SD_CARD_TYPE_SD1:
        println("SD1");
        break;
        case SD_CARD_TYPE_SD2:
        println("SD2");
        break;
        case SD_CARD_TYPE_SDHC:
        println("SDHC");
        break;
        default:
        println("Unknown");
    }

    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (volume.fatType() < 12 ) {
        println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
        return;
    }

    print("Clusters:          ");
    println(volume.clusterCount());
    print("Blocks x Cluster:  ");
    print(volume.blocksPerCluster());
    write(10);
    print("Total Blocks:      ");
    println(volume.blocksPerCluster() * volume.clusterCount());
    println();
    // print the type and size of the first FAT-type volume
    uint32_t volumesize;
    memset(_scratch.bytes,0,sizeof(_scratch));
    sprintf(_scratch.text, "Volume type is:    FAT%d", volume.fatType());
    graphics.drawText(_cursorX, _cursorY,(const char*) _scratch.text, textColor, backgroundColor);
    write(10);
    
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
    if(volumesize < 2048){
        sprintf(_scratch.text, "Volume size        %lu KB", volumesize);
    }else if(volumesize < 1024*2048){
        sprintf(_scratch.text, "Volume size        %lu MB", volumesize/1024);
    }else{
        sprintf(_scratch.text, "Volume size        %lu GB", volumesize/(1024*1024));
    }
    graphics.drawText(_cursorX, _cursorY,(const char*) _scratch.text, textColor, backgroundColor);
    write(10);
    
    console.SetEchoMode(true);
}

int Console::_saveCommand()
{
    //auto commandString = String(command);
    int cmdLength = strlen(_currentCommand);
    Serial.print("Received command line: "); Serial.println(_currentCommand);
    char cmd[64];
    char args[240];
    
    
    _commandViewIdx = _history.index();
    _commandViewIdx = _history.addEntry(_currentCommand);
    
    memset(cmd, 0 , sizeof(cmd));
    memset(args, 0 , sizeof(args));
    
    for (int idx = 0; idx < cmdLength;idx++){
        if(_currentCommand[idx] == ' ' || _currentCommand[idx] == '\0'){
            cmdLength = idx;
            break;
        }        
    }
    memcpy(cmd,_currentCommand, cmdLength);
    memcpy(args,_currentCommand + cmdLength + 1, strlen(_currentCommand) - cmdLength);
    // Serial.print("Processing command "); Serial.print(cmd); 
    // if(strlen(args) > 0){
    //     Serial.print(" with args "); Serial.print(args);
    // }
    // Serial.println();

   
    return 0;
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

size_t Console::println(unsigned long num, int base)
{
    String text = String(num);
    
    write(text.c_str());
    write(10,true);
    return text.length();
}

size_t Console::println(double value, int precision)
{
    char buf[24];
    sprintf(buf,"%.*lf",precision, value);
    return println(buf);
}

size_t Console::println(void)
{
    return write(13);
}

size_t Console::print(char c)
{
    return write(c + 48);
}

size_t Console::print(unsigned char c, int)
{
    return print((char)c);
}

size_t Console::print(int num, int base)
{
    char buf[12];
    sprintf(buf,"%d", num);
    return write(buf);
}

size_t Console::print(unsigned int i, int)
{
    char buf[12];
    sprintf(buf,"%i", i);
    return write(buf);
}

size_t Console::print(unsigned long num, int base)
{
    char buf[10];
    sprintf(buf,"%lu", num);
    return write(buf);
}

size_t Console::print(unsigned long long num, int base)
{
    char buf[20];
    sprintf(buf,"%llu", num);
    return write(buf);
}

size_t Console::print(double d, int)
{
    char buf[32];
    sprintf(buf,"%lf", d);
    return write(buf);
}

size_t Console::println(unsigned char c, int base)
{
    
    return write(c + 48) + write(13);
}
