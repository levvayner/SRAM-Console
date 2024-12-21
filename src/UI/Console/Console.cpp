#include "Console.hpp"
#include "../Editor/Editor.hpp"
#include "ConsoleCommands.h"
extern Editor editor;


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
        char cmd[256];
        uint32_t startAddr = (_scrollOffset + _cursorY/graphics.settings.charHeight) * charsPerLine;
        uint16_t cmdLength = min(((pos & 0x3FFFF) - startAddr), (sizeof(cmd) - 1));
        if(!useFrameBuffer)
            programmer.WriteByte( pos,data);

        if(_commandMode){ //execute command
            if(_echoPrompt){
                // memset(_scratch,0, sizeof(_scratch));
                // sprintf(_scratch.text, _prompt, _path.c_str());
                startAddr += _promptLength;//strlen((const char*)_scratch);
                cmdLength -= _promptLength;//strlen((const char*)_scratch);
            }
            //Serial.print("reading command from ");  Serial.print( startAddr, HEX); Serial.print(" with length "); Serial.println(cmdLength);
            memset(cmd, 0, sizeof(cmd));
            programmer.ReadString((1 << 19 )| startAddr, (uint8_t*)cmd, cmdLength);     

            //ProcessCommand(cmd);
           
           

            AdvanceCursor(true);
            _consoleLine = getCurrentLineNumber();
            _drawEcho();
        } else
            AdvanceCursor(true);
        
        
        
        
        return 1;
    }
    if(data == 13) return 0; 

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

void consoleProcessKey(uint8_t keyCode){
    console.processKey(keyCode);
}

void Console::run(bool blocking )
{
    _consoleRunning = blocking;
    if(blocking){
        commands.clearCommands(CONTEXT_CONSOLE);
        registerConsoleCommands();
    }
    keyboard.onKeyDown = consoleProcessKey;
    
    
    _cursorX = 0;
    _cursorY = 0;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
    charsPerLine = graphics.settings.screenWidth / graphics.settings.charWidth;
    clear();
    if(blocking){
        
        if(!_initialized)
            _initSD();
        _commandMode = blocking;
        clearData(); 
        _drawEcho();   
    }
  
}

void Console::stop()
{
    _consoleRunning = false;
    _commandMode = false;
    commands.clearCommands(CONTEXT_CONSOLE);
    keyboard.onKeyDown = nullptr;
}

void Console::_drawTextFromRam()
{
    uint32_t pos = _scrollOffset * (charsPerLine);
    uint32_t endPos = min(_lastIdx, ((_windowHeight / graphics.settings.charHeight) * charsPerLine));

    if(endPos < pos) return; // nothing to print
    
    //check how many bytes we can fit, updat end pos accordingly.
    char buf[charsPerLine + 1];
    byte colorBuf[charsPerLine];
    uint8_t lineBuf[graphics.settings.charHeight * graphics.settings.screenWidth];

    _consoleRunning = false;            
    clear();        
    char textBuf[128];

    for(uint32_t line = 0; line < (ceil(( endPos - pos + 1) / charsPerLine) + 1); line++){      
        bool lineHasText = false;
        uint16_t lineLength =  min(endPos - pos, charsPerLine );
        memset(buf, 0, charsPerLine + 1);
        memset(lineBuf,backgroundColor, graphics.settings.charHeight * graphics.settings.screenWidth );
        //for(uint16_t idx = 0; idx < charsPerLine; idx++){
        programmer.ReadBytes((pos + (line * (charsPerLine ))) | (1 << 19), (uint8_t*)buf, lineLength);
        programmer.ReadBytes((pos + (line * (charsPerLine ))) | (3 << 18) , (uint8_t*)colorBuf, lineLength);
        
            
        for(uint16_t idx = 0; idx < lineLength; idx++){
            if((buf[idx] >= 32 && buf[idx] < 127)) lineHasText = true;
            //if( isascii(buf[idx]) && buf[idx] != 0) lineHasText = true;
            else buf[idx] = ' ';
        }
        if(lineHasText){
            
            //sprintf(textBuf, "Writing %i chars from idx %d to %d on line %lu", strlen(buf), line * charsPerLine, (line * charsPerLine) + lineLength - 1, line);
            //Serial.println(textBuf);
            Serial.println(buf);
            graphics.drawTextToBuffer(buf, colorBuf, lineBuf, graphics.settings.screenWidth);
            graphics.drawBuffer(0, line * graphics.settings.charHeight, graphics.settings.screenWidth, graphics.settings.charHeight, lineBuf);
        }
    }
    Serial.println("-----------------------");
    programmer.ReadByte(0x0); 
    //graphics.render();
    _consoleRunning = true;
}

void Console::_drawEcho()
{
    char buf[256];
    if(_consoleRunning && _commandMode && _echoPrompt){
        echoY = _cursorY;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%s |>", _path.c_str());
        _promptLength = strlen(buf);
        write((const char*)buf);
    }
}

// void Console::_processKey(char keyVal)
// {
//     Serial.print("Processing usb key: "); Serial.println(keyVal);
//     write(keyVal);
// }

void Console::_initSD()
{
    if(this->_commandMode)
        print("\nInitializing SD card...");
    // we'll use the initialization code from the utility libraries
    // since we're just testing if the card is working!
    if (!card.init(SPI_HALF_SPEED, 10) && this->_commandMode) {
        println("initialization failed. Things to check:");
        println("* is a card inserted?");
        println("* is your wiring correct?");
        println("* did you change the chipSelect pin to match your shield or module?");
        while (1);
    } else {
        if(this->_commandMode)
            println("Found!");
    }
    // print the type of card
    if(this->_commandMode){
        println();
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
    }
    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card) && _commandMode) {
        Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
        while (1);
    }

    if(_commandMode)
    {
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
    }
    //root.ls(LS_R | LS_DATE | LS_SIZE);
    //root.close();
    _initialized = true;
}

int Console::getCoords(const char *str, int *coords, int offset)
{
    int coordsFound = 0;
    //coords = {0};
    int cursorIdx = offset;
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

inline void Console::processKey(uint8_t keyCode)
{
    //Serial.print("Received key"); Serial.println(keyCode);
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
        //Serial.print("Advancing to new line, injecting NL into data cache at address 0x"); Serial.println(GetDataPos(), HEX);
        programmer.WriteByte( 1 << 19 | (GetDataPos() + 1) ,10,1);      
        programmer.ReadByte(0); //turn off 19th bit             
    }

    _cursorX = 0;
    //if we need to scroll down
    if(_cursorY + 2* graphics.settings.charHeight  >= graphics.settings.screenHeight - STATUS_BAR_HEIGHT  && !_commandMode){
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
    } else //if(!_echoPrompt || _cursorX > _promptLength * graphics.settings.charWidth){
        _cursorX -= (graphics.settings.charWidth);
    //}
    return true;
}


bool Console::MoveCursorUp()
{
    if(_commandMode && getCurrentLineNumber() == _consoleLine){
        //TODO: replace contents with previous command in cache
        if(--_commandViewIdx > _history.length()) _commandViewIdx = _history.index();
        Serial.print("Replace with previous at idx "); Serial.println(_commandViewIdx);
        const char* buf;
        buf = _history.read(_commandViewIdx);
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
    if(_commandMode && getCurrentLineNumber() == _consoleLine){
        //TODO: replace contents with next command in cache        
        if(++_commandViewIdx >_history.length()) {
            _commandViewIdx = 0;
            Serial.print("History Length: "); Serial.println(_history.length());
            Serial.print("Replace with next at idx "); Serial.println(_commandViewIdx);
        }
        
        const char* buf;
        buf = _history.read(_commandViewIdx);
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


void Console::DrawCursor()
{
    //ifnot visible, hide, otherwise if visible show, else hide
    
    memset(_scratch.bytes, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, _scratch.bytes, graphics.settings.charWidth);
    //graphics.drawLine(_cursorX, _cursorY + graphics.settings.charHeight, _cursorX + 6, _cursorY + graphics.settings.charHeight, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK);
}

void Console::EraseCursor()
{
    memset(_scratch.bytes, 0, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, _scratch.bytes, graphics.settings.charWidth);
    
}

// int Console::ProcessCommand(const char *command)
// {
//     //auto commandString = String(command);
//     int cmdLength = strlen(command);
//     Serial.print("Received command line: "); Serial.println(command);
//     char cmd[64];
//     char args[240];
//     bool drawCommand = false;
//     DrawShape shape = shapeUnknown;
    
    
//     _commandViewIdx = _history.index();
//     _commandViewIdx = _history.addEntry(command);
    
//     memset(cmd, 0 , sizeof(cmd));
//     memset(args, 0 , sizeof(args));
    
//     for (int idx = 0; idx < cmdLength;idx++){
//         if(command[idx] == ' ' || command[idx] == '\0'){
//             cmdLength = idx;
//             break;
//         }        
//     }
//     memcpy(cmd,command, cmdLength);
//     memcpy(args,command + cmdLength + 1, strlen(command) - cmdLength);
//     Serial.print("Processing command "); Serial.print(cmd); 
//     if(strlen(args) > 0){
//         Serial.print(" with args "); Serial.print(args);
//     }
//     Serial.println();

   
//     return 0;
// }

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
    println(buf);
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
    write(buf);
}

size_t Console::print(unsigned long num, int base)
{
     char buf[16];
    sprintf(buf,"%lu", num);
    write(buf);
}

size_t Console::println(unsigned char c, int base)
{
    
    return write(c + 48) + write(13);
}
