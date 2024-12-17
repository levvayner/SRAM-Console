#include "Console.hpp"


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
                // sprintf((char*)_scratch, _prompt, _path.c_str());
                startAddr += _promptLength;//strlen((const char*)_scratch);
                cmdLength -= _promptLength;//strlen((const char*)_scratch);
            }
            //Serial.print("reading command from ");  Serial.print( startAddr, HEX); Serial.print(" with length "); Serial.println(cmdLength);
            memset(cmd, 0, sizeof(cmd));
            programmer.ReadString((1 << 19 )| startAddr, (uint8_t*)cmd, cmdLength);     

            ProcessCommand(cmd);
           
           

            AdvanceCursor(true);
            _consoleLine = getCurrentLineNumber();
            _drawEcho();
        } else
            AdvanceCursor(true);
        
        
        
        
        return 1;
    }
    if(data == 13) return 0; 

    graphics.drawText(_cursorX,_cursorY, data, color, backgroundColor, clearBackround, useFrameBuffer);
    
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
    _consoleRunning = blocking;
    
    
    _cursorX = 0;
    _cursorY = 0;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
    charsPerLine = graphics.settings.screenWidth / graphics.settings.charWidth;
    //DRAW BOTTOM SECTION
    clear();
    if(!_initialized)
        _initSD();
    _commandMode = blocking;
    clearData(); 
    _drawEcho();   
   
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
    }
    programmer.ReadByte(0x0); 
    //graphics.render();
    _consoleRunning = true;
}

void Console::_drawEcho()
{
    char buf[256];
    if(_consoleRunning && _commandMode && _echoPrompt){
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%s |>", _path.c_str());
        _promptLength = strlen(buf);
        write((const char*)buf);
    }
}

void Console::_processKey(char keyVal)
{
    Serial.print("Processing usb key: "); Serial.println(keyVal);
    write(keyVal);
}

void Console::_initSD()
{
    print("\nInitializing SD card...");
    // we'll use the initialization code from the utility libraries
    // since we're just testing if the card is working!
    if (!card.init(SPI_HALF_SPEED, 10)) {
        println("initialization failed. Things to check:");
        println("* is a card inserted?");
        println("* is your wiring correct?");
        println("* did you change the chipSelect pin to match your shield or module?");
        while (1);
    } else {
        println("Wiring is correct and a card is present.");
    }
    // print the type of card
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
    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card)) {
        Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
        while (1);
    }
    print("Clusters:          ");
    println(volume.clusterCount());
    print("Blocks x Cluster:  ");
    println(volume.blocksPerCluster());
    print("     Total Blocks:      ");
    println(volume.blocksPerCluster() * volume.clusterCount());
    println();
    // print the type and size of the first FAT-type volume
    uint32_t volumesize;
    print("Volume type is:    FAT  ");
    println(volume.fatType(), DEC);
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
    print("Volume size (Kb):  ");
    println(volumesize);
    print("Volume size (Mb):  ");
    volumesize /= 1024;
    println(volumesize);
    print("Volume size (Gb):  ");
    println((float)volumesize / 1024.0);    
    // list all files in the card with date and size
    root.ls(LS_R | LS_DATE | LS_SIZE);
    root.close();
    _initialized = true;
}

void Console::_listFiles(const char * path, bool recursive, int indent,  uint8_t  flags)
{
    File rootFile = SD.open(path);
    File entry;
    _commandMode = false;
    while(true){
        entry = rootFile.openNextFile();
        if(!entry) break;
        print(entry.name());
        for(int idx = 0; idx < 20 - strlen(entry.name()); idx++)
            print(" ");
        if(entry.isDirectory())
            print("<dir>");
        else 
            print(entry.size());
        entry = entry.openNextFile();
        write(10,true);
        if(entry.isDirectory()){
            _listFiles(entry.name(), recursive, indent + 1, flags);
        }
    }
    _commandMode = true;
    entry.close();
    rootFile.close();

    
}

int Console::getCoords(const char *str, int *coords, int offset)
{
    int coordsFound = 0;
    //coords = {0};
    int cursorIdx = offset;
    for(; cursorIdx < strlen(str); cursorIdx++){
        if(str[cursorIdx] ==','){
            memset(_scratch, 0 , sizeof(_scratch));
            memcpy(_scratch, str + offset, cursorIdx - offset);
            //Serial.print("Coord found: "); Serial.println((const char *)_scratch);
            coords[coordsFound] = atoi((const char *)_scratch);
            coordsFound++;
            offset = cursorIdx + 1;
        }
    }
    if(offset < cursorIdx){
        memset(_scratch, 0 , sizeof(_scratch));
        memcpy(_scratch, str + offset, cursorIdx - offset);
        coords[coordsFound] = atoi((const char *)_scratch);
        coordsFound++;
    }
    // Serial.print("found "); Serial.print(coordsFound); Serial.print(" coords");
    // for(int idx = 0; idx < coordsFound; idx++){
    //     Serial.print(coords[idx]); Serial.print("   ");
    // }
    // Serial.println();
    return coordsFound;
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
    } else if(!_echoPrompt || _cursorX < _promptLength * graphics.settings.charWidth){
        _cursorX -= (graphics.settings.charWidth);
    }
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

int Console::RunProgram(const char *programName)
{
    byte buf[256];
    //byte bufVerify[256];
    Serial.print("Running ["); Serial.print(programName); Serial.println("]");
    if(!SD.exists((_path + (_path.length()  == 1 ? "" : "/") + programName).c_str()))
    {
        Serial.println("File not found!");
        return -1;
    }
    File program = SD.open((_path + (_path.length()  == 1 ? "" : "/") + programName).c_str());
    uint32_t  memAddr = 0x0;
    int bytesRead = 0;//, bytesWritten = 0;
    while(true){
        //uint8_t data = 0;
        bytesRead = program.read(buf,sizeof(buf));
        if(!program.available()) break;
        if(bytesRead == 0) break;
        if(!dueFlashStorage.write(memAddr,buf,bytesRead)){
            Serial.println("Error occured writing data");
            return -1;
        }
        memAddr+= bytesRead;
        
    }
    Serial.print("Done loading binary at 0x"); Serial.println(IFLASH1_ADDR, HEX);
    
    
    
    startApp(IFLASH1_ADDR);
    Serial.print("Returned control");
    return 0;
}

void Console::DrawCursor()
{
    //ifnot visible, hide, otherwise if visible show, else hide
    
    memset(_scratch, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, _scratch, graphics.settings.charWidth);
    //graphics.drawLine(_cursorX, _cursorY + graphics.settings.charHeight, _cursorX + 6, _cursorY + graphics.settings.charHeight, (_cursorVisible && _cursorState) ? Color::WHITE : Color::BLACK);
}

void Console::EraseCursor()
{
    memset(_scratch, 0, graphics.settings.charWidth);
    graphics.WriteBytes(((_cursorY + graphics.settings.charHeight) << graphics.settings.horizontalBits) + _cursorX, _scratch, graphics.settings.charWidth);
    
}

int Console::ProcessCommand(const char *command)
{
    //auto commandString = String(command);
    int cmdLength = strlen(command);
    Serial.print("Received command line: "); Serial.println(command);
    char cmd[64];
    char args[240];
    bool drawCommand = false;
    DrawShape shape = shapeUnknown;
    
    
    _commandViewIdx = _history.index();
    _commandViewIdx = _history.addEntry(command);
    
    memset(cmd, 0 , sizeof(cmd));
    memset(args, 0 , sizeof(args));
    
    for (int idx = 0; idx < cmdLength;idx++){
        if(command[idx] == ' ' || command[idx] == '\0'){
            cmdLength = idx+1;
            break;
        }        
    }
    memcpy(cmd,command, cmdLength);
    memcpy(args,command + cmdLength, strlen(command) - cmdLength);
    Serial.print("Processing command "); Serial.print(cmd); 
    if(strlen(args) > 0){
        Serial.print(" with args "); Serial.print(args);
    }
    Serial.println();

    int coords[6];
    int coordsLength = 0;
    //"executable" is first word
    //TODO: crate table of commands with string representation to match and pointer to function
    if(strncasecmp(cmd,"clear", cmdLength - 1) == 0){
        clear();
        SetPosition(0);
        return 0;
    } 
    else if(strncasecmp(cmd,"cd", cmdLength - 1) == 0){
        if(strncasecmp("../", args, 4) == 0 ||  strncasecmp("..", args, 3) == 0 ){
            //go up one dir
            if(_path.lastIndexOf('/') > 0)                
                _path = _path.substring(0,_path.lastIndexOf('/') - 1);
            else
                _path = "/";
        }
        else if(SD.exists(args)){
            File f = SD.open(args);
            _path = "/";
            _path +=  f.name();
            f.close();
        }
            
        else{
            print("Invalid path provided: "); println(args);
        }

    }
    else if(strncasecmp(cmd,"ls", cmdLength - 1) == 0){
       //check if we are in sd volume
       AdvanceCursor(true);
       if(!_path.startsWith("/")){
        
        println("Cannot LS. Invalid path");
        return 0;
       }
       _listFiles(strlen(args) > 0 ? args : _path.c_str(), true,0);
    }
    else if(strncasecmp(cmd,"cat", cmdLength - 1) == 0){
        //check if we are in sd volume
        _commandMode = false;
        AdvanceCursor(true);
        File f = SD.open(_path + "/" + args);
        if(f){
            uint8_t buffer[72];
            int readBytes = 0;
            while(true){
                if(!f.available())
                    break;
                readBytes = f.readBytes(buffer,sizeof(buffer));
                if(readBytes == 0)
                    continue;
                
                write(buffer,readBytes);
                AdvanceCursor(true);
                
            }
            f.close();
        }
        _commandMode = true;
       
    }
    else if(strncasecmp(cmd,"draw", cmdLength - 1) == 0){
        //check if we are in sd volume
        _commandMode = false;
        AdvanceCursor(true);
        Bitmap img;
        img.drawFile((_path + "/" + args).c_str(), 0, 0);
           // while(Serial.read()); //wait for any key
        
        
        // File f = SD.open(_path + "/" + args);
        // if(f){
        //     uint8_t buffer[graphics.settings.screenWidth];
        //     int readBytes = 0;
        //     uint32_t length = 0;
        //     while(true){
        //         if(!f.available())
        //             break;
        //         readBytes = f.readBytes(buffer,sizeof(buffer));
        //         if(readBytes == 0)
        //             continue;                
        //         uint32_t xCoord = length % graphics.settings.screenWidth;
        //         uint32_t yCoord = (length - xCoord) / graphics.settings.screenHeight;
                
        //         graphics.WriteBytes(
        //             (yCoord << graphics.settings.horizontalBits) + xCoord,
        //             buffer,readBytes
        //         );
        //         length += readBytes;
        //         //write(buffer,sizeof(buffer));
                
                
        //     }
        //     f.close();
        // }
       
        _commandMode = true;

        AdvanceCursor(true);
    }
    else if(strncasecmp(cmd,"line", cmdLength - 1)  == 0){
        
        shape = shapeLine;
        coordsLength = getCoords(command,coords, cmdLength);
        if(coordsLength == 4)
            drawCommand = true;


    }
    else if(strncasecmp(cmd,"triangle", cmdLength - 1)  == 0){
        shape = shapeTriangle;

        coordsLength = getCoords(command,coords, cmdLength);
        if(coordsLength == 6)
             drawCommand = true;

    }
    else if(strncasecmp(cmd,"rectangle", cmdLength - 1)  == 0){
        shape = shapeRectangle;

        coordsLength = getCoords(command,coords, cmdLength);
        if(coordsLength == 4)
            drawCommand = true;

    }
    else if(strncasecmp(cmd,"print", cmdLength - 1) == 0){
        graphics.drawText(_cursorX, _cursorY, command + cmdLength, textColor, backgroundColor);

    }
    else if(strncasecmp(cmd,"run", cmdLength - 1) == 0){
        RunProgram(command + cmdLength);

    } else {
        char buf[348];
        sprintf(buf,"Unknown command: %s", command);
        _echoPrompt = false;       
        AdvanceCursor(true);
        _echoPrompt = true;
        graphics.drawText(_cursorX, _cursorY, buf, Color::RED);
        
    }
    if(drawCommand){
        
        switch (shape)
        {
        case shapeLine:
            Serial.println("Drawing line");
            graphics.drawLine(coords[0], coords[1], coords[2], coords[3],textColor);
            
            break;
        case shapeTriangle:
            Serial.println("Drawing triangle");
            graphics.drawTriangle(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5],textColor);
            
            break;
        
        case shapeRectangle:       
            Serial.println("Drawing rectangle   "); 
            graphics.drawRectangle(coords[0], coords[1], coords[2], coords[3],textColor);
            break;
        
        default:
            Serial.println("Shape not implemented");
            break;
        }
    }

    AdvanceCursor();
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
