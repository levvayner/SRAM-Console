#include "Editor.hpp"
extern Editor editor;
//extern Console console;
void editorProcessKey(uint8_t data){
    //Serial.print("Received key 0x"); Serial.println(data, HEX);
    editor.processKey(data);
    
}

void Editor::run()
{
    SetCommandMode(false);
    SetWindowHeight(graphics.settings.screenHeight - STATUS_BAR_HEIGHT);
    // if(!IsConsoleRunning())
    //     Console::run(false);
    _isEditorRunning = true;
    if(_fileName == nullptr || strlen(_fileName) == 0){
        _isNewFile = true;
        _fileName = (char *) malloc(14);
        memset(_fileName, 0, 14);
        sprintf(_fileName, "new_%5lu.txt", millis());
        Serial.print("Setting new file to: "); Serial.println(_fileName);
    }
    Serial.println("Enter text to render. Ctrl+R to quit");
    //DRAW BOTTOM SECTION
    DrawStatusBar();
    keyboard.onKeyDown = editorProcessKey;
    charsPerLine = graphics.settings.screenWidth / graphics.settings.charWidth;
    //clear();    
   
    // _cursorX = 0;
    // _cursorY = 0;

    mouse.begin();
    this->ShowCursor();
    //ConsoleKeyPress PS2Key;
    //while(_isEditorRunning){
        //TODO: implement PS2 and Serial key actions

        // const ConsoleKeyType SerialAction = checkPort(Serial);
        // //const ConsoleKeyAction PS2Action = checkPort(keyboardPs2);
        // PS2Key = ps2Controller.getKey();
        // if(SerialAction == Cursor)        
        //     _drawCursorPosition();

        // if(PS2Key.action == Cursor){
        //     switch (PS2Key.keyCode)
        //     {
        //     case PS2_KEY_ENTER: /* Cursor to beginning of next line or start */           
        //         EraseCursor();
        //         // editor.AdvanceCursor(true);
        //         AdvanceCursor(true);
        //         _drawCursorPosition();
        //         break;
        //     case PS2_KEY_SPACE:

        //         AdvanceCursor(false);
        //         _drawCursorPosition();
        //         break;
        //     case PS2_KEY_PGDN: /* Cursor to top row current column */
        //         //_rows = MAX_ROW - 1;
        //         break;
        //     case PS2_KEY_PGUP: /* Cursor to bottom row current column */
        //         //_rows = 0;
        //         break;
        //     case PS2_KEY_L_ARROW: 
        //         EraseCursor();
        //         MoveCursorLeft();
        //         _drawCursorPosition();
        //         break;
        //     case PS2_KEY_R_ARROW: 
        //         EraseCursor();
        //         MoveCursorRight();
        //         _drawCursorPosition();
        //         break;
        //     case PS2_KEY_UP_ARROW:
        //         EraseCursor();
        //         MoveCursorUp();
        //         //_drawCursorPosition();
        //         break;
        //     case PS2_KEY_DN_ARROW:
        //         EraseCursor();
        //         MoveCursorDown();
        //         //_drawCursorPosition();
        //         break;          
            
        //     case PS2_KEY_BS: /* Move cursor back write space move cursor back */
        //         graphics.fillRectangle(_cursorX,_cursorY, graphics.settings.charWidth, graphics.settings.charHeight + 1, Color::BLACK);
        //         ReverseCursor();
        //         _drawCursorPosition();
        //         break;
        //     case PS2_KEY_HOME: /* Cursor to top left */
        //         EraseCursor();
        //         SetPosition(0,_cursorY);
        //         _drawCursorPosition();
        //         break;
        //     case PS2_KEY_END: /* Cursor to max position */
        //     {
        //         const uint8_t maxChars = floor(graphics.settings.screenWidth / graphics.settings.charWidth);
        //         SetPosition(maxChars * graphics.settings.charWidth , _cursorY);
        //         _drawCursorPosition();
        //         break;
        //     }       

        //     case PS2_KEY_F2:
        //         textColor--;  
        //         _drawColor();
        //         break;
        //     case PS2_KEY_F3:
        //         textColor++;  
        //         _drawColor();
        //         break;         
            
        //     case PS2_KEY_F4:
        //         _isEditorRunning = false;
        //         break;
        //     default:
        //         break;
        //     }
        // }
        
        // if(SerialAction == ColorChange)
        //     _drawColor();
       
        // if(SerialAction == Exit)
        //     _isEditorRunning = false; 

        // if(PS2Key.type == ASCII)
        //     write(PS2Key.keyCode);

        // if(_cursorVisible){
        //     if(millis() - _lastCursorChange > 1200){
        //         _lastCursorChange = millis();
        //         _cursorState = !_cursorState;
        //         DrawCursor();
        //     }
        // }
    //}
    
}

bool Editor::open(const char *filename)
{
    char buf[1024];
    if(_fileName != nullptr)
        delete _fileName;

    Serial.print("Edit: opening "); Serial.println(filename);
    clear();
    
    _fileName = (char*) malloc(strlen(filename) + 1);
    memcpy(_fileName, filename, strlen(filename));
    memset(_fileName + strlen(filename), 0,  1);
    if(!SD.exists(_fileName)) _isNewFile = true;
    int idx = 0;
    File f = SD.open(_fileName, FILE_READ);
    Serial.print("Loading "); Serial.print(f.size()); Serial.print(" bytes into editor");
    if(!f){
        Serial.print("Failed to open file. Quitting"); 
        return false;
    }
    if(!_isNewFile) f.seek(0);
    while (true)
    {
        if(!f.available()) break;
        int bytesRead = f.readBytes(buf,sizeof(buf));
        if(bytesRead == 0 ) continue;
        
        programmer.WriteBytes(1<<19 | idx, (uint8_t*)buf,bytesRead);
        idx += bytesRead;
    }
    f.close();
    Serial.println(" done");
    HideCursor();
    SetEchoMode(false);
    SetPosition();
    //data loaded into memory
    //write out visible text to screen
    int charsDrawn = 0, charsMax = charsPerLine * this->GetWindowHeight() /graphics.settings.charHeight;
    while(charsDrawn < charsMax){
        
        int charsToDraw = min((uint32_t)(charsMax - charsDrawn), charsPerLine);
        memset(buf,0, charsToDraw + 1);
        int bytesRead = graphics.ReadBytes(1<<19 | charsDrawn, (uint8_t*) buf,charsToDraw);
        if(strchr(buf,10) != NULL){
            bytesRead = strchr(buf,10) - buf + 1;
            buf[bytesRead] = 0;
            //memset(buf + bytesRead, 0, sizeof(buf) - bytesRead);
        }
        //if(bytesRead == 0) break;
        Serial.print("Drawing "); Serial.print(bytesRead); Serial.print(" chars on line "); Serial.println((_cursorY / graphics.settings.charHeight)+ 1);
        graphics.drawText(_cursorX, _cursorY, buf,textColor);
        _cursorY += graphics.settings.charHeight;
        charsDrawn += bytesRead;
    }
    SetLastIdx( idx);
    return true;
}

bool Editor::save()
{
    if(_fileName == nullptr){
        return false;
    }
    
    File f = SD.open(_fileName,  FILE_WRITE);
    uint32_t idx = 0;
    uint32_t endIdx = LastIdx();
    Serial.print("Saving "); Serial.print(_fileName); Serial.print(": "); Serial.print(endIdx); Serial.println(" bytes");
    char buf[256];
    f.seek(0);
    Serial.print("Seeking to 0 : "); Serial.println( f.position());
    while(idx < endIdx){
         Serial.print("Start Idx: ");  Serial.print(idx); Serial.print(" End idx: "); Serial.println(endIdx);
        uint16_t bytesToRead = min(sizeof(buf), endIdx - idx);
        Serial.print("Reading "); Serial.print(bytesToRead); Serial.print(" bytes from "); Serial.println(idx);
        uint32_t bytesRead = programmer.ReadBytes(1<<19 | idx, (uint8_t*)buf,sizeof(buf));
        Serial.print("Read "); Serial.print(bytesRead); Serial.println(" bytes from RAM");
        uint32_t bytesWritten = f.write(buf,bytesRead);
        Serial.print("Wrote "); Serial.print(bytesWritten); Serial.println(" bytes to SD");
        if(bytesRead == 0 || bytesWritten == 0) break;
        idx += bytesWritten;
    }
    f.close();
    return true;
}

void Editor::stop()
{
    _isEditorRunning = false;
    keyboard.onKeyDown = nullptr;
    //keyboard.SetMode(false);
    if(_cursorVisible){
        HideCursor();
    }
    mouse.end();    
}

bool Editor::AdvanceCursor(bool nextLine)
{
    bool newLine = Console::AdvanceCursor(nextLine);
    

    _drawLineNo();
    _drawCursorPosition();
    return newLine;
}

bool Editor::ReverseCursor()
{
    if(!Console::ReverseCursor())
        return false;
    
    _drawLineNo();
    _drawCursorPosition();
    return true;
}

// ConsoleKeyAction Editor::processPS2Key(uint8_t ps2KeyCode)
// {
//     auto action = Console::processPS2Key(ps2KeyCode);
//     if(action == ConsoleKeyAction::Cursor)        
//             _drawCursorPosition();
        
//         if(action == ConsoleKeyAction::ColorChange)
//             _drawColor();
        
//         if(action == ConsoleKeyAction::Exit)
//             _isEditorRunning = false; 

// }

/// @brief status bar is one row high with top and bottom border (10px)
void Editor::DrawStatusBar()
{
    char buf[256];
    memset(buf,0,256);
    unsigned long startTime = millis();
    graphics.drawRectangle(0, graphics.settings.screenHeight - STATUS_BAR_HEIGHT, graphics.settings.screenWidth, 9,Color::DARK_GREEN);
    graphics.fillRectangle(2, graphics.settings.screenHeight - 9, 8* (graphics.settings.charWidth), 8,Color::FromRGB(2,3,2));
    sprintf(buf,"Cursor");
    graphics.drawText(2, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::DARK_GREEN, false);
    //_printChars(buf,Color::WHITE, 2, graphics.settings.screenHeight - 9, false);     
    
    _drawCursorPosition();

    graphics.fillRectangle(90, graphics.settings.screenHeight - 9, graphics.settings.screenWidth - 80, 9, Color::FromRGB(2,3,0));

    sprintf(buf,"Line:");
    graphics.drawText(91, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::FromRGB(2,3,0), false);
    
    sprintf(buf,"F2:color     F4:Quit");
    graphics.drawText(150, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::FromRGB(2,3,0), false);
    //_printChars(buf,Color::WHITE, 80, graphics.settings.screenHeight - 9, false);     
    //see where we end up with color text

    _drawColor();
    _drawLineNo();
    _drawFilename();

    sprintf(buf, "Draw Status bar took %lu ms", millis() - startTime);
    Serial.println(buf);
    
}

void Editor::processKey(uint8_t keyCode)
{
    if(keyCode == 10)
    {
        AdvanceCursor(true);
        return;
    }    

    if( keyCode == 255) return;
    Serial.print("Editor: Read 0x"); Serial.println(keyCode, HEX);
    
    if(keyCode == 0x1B){ //todo, handle escape key
    //Serial.println("Setting mode to ESC");
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
        switch (keyCode)
        {
            case 0x50: //F1
            case 0x51: //F2
            {
                textColor++; 
                return;                         
            }
                
            case 0x53: //F4
            {
                //dump data to serial
                uint16_t pos = GetDataPos();
                char nextChar = ' ';
                if(pos <=  0) return;
                Serial.println("Dumping data contents... ");
                for(int idx = 0; idx < GetDataPos();idx++){
                    nextChar = programmer.ReadByte(1 << 19 | idx);
                    if((nextChar >= 32 && nextChar < 127) || nextChar == 10)
                        Serial.print(nextChar);
                }
                programmer.ReadByte(0x0); // unset bit 19 so screen accesses video ram
            }
                return;                        
            // ...
        }
    }        
    if(_currentInputMode == CommandExtended){   
        Serial.print("Processing Extended Command mode key: 0x"); Serial.println(keyCode, HEX);
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
         _currentInputMode = Text; //no 7e from serial
        return;
    }

    if(keyCode == 0x8){     
        EraseCursor();
        ReverseCursor();
        graphics.fillRectangle(_cursorX,_cursorY, graphics.settings.charWidth, graphics.settings.charHeight, Color::BLACK);                
        
        return;
    }
    
    if(keyCode == 0x12){ //ctrl + r
        stop();
        Serial.println("Closing editor");
        return;
    } 
    if(_currentInputMode == Text){
        write(keyCode);
        Serial.print("Received key"); Serial.println(keyCode);
    }       
    
    

}

bool Editor::MoveCursorDown()
{
    if(_cursorY >= graphics.settings.screenHeight - graphics.settings.charHeight - STATUS_BAR_HEIGHT - 2 ) return false; //with 2px padding    
    Console::MoveCursorDown();
    _drawLineNo();
    _drawCursorPosition();
    return true;
}

bool Editor::MoveCursorUp()
{
    Console::MoveCursorUp();
    _drawLineNo();
    return false;
}

void Editor::_drawCursorPosition(){
    //if(!_consoleRunning) return;
    if(_cursorX > graphics.settings.screenWidth || _cursorY > graphics.settings.screenHeight ) return;
    char label[24];
    uint8_t block[8 * graphics.settings.charWidth * graphics.settings.charHeight];

    sprintf(label,"%03d, %03d%c", _cursorX, _cursorY,'\0');        
    memset(block, Color::LIGHT_GREEN, 8 * graphics.settings.charWidth * graphics.settings.charHeight);
  
    graphics.drawTextToBuffer(label, block, 8 * graphics.settings.charWidth , Color::NAVY_BLUE);
    graphics.drawBuffer((6* (graphics.settings.charWidth) + 4), graphics.settings.screenHeight - 9 , 8 * graphics.settings.charWidth , graphics.settings.charHeight, block);
                     
    memset(block, Color::LIGHT_GREEN, 8 * graphics.settings.charWidth * graphics.settings.charHeight);                
    sprintf(label,"%d",GetDataPos());    
    graphics.drawTextToBuffer(label, block, 8 * graphics.settings.charWidth , Color::NAVY_BLUE); 
    graphics.drawBuffer(300, graphics.settings.screenHeight - 9 , 8 * graphics.settings.charWidth , graphics.settings.charHeight, block);


}

void Editor::_drawLineNo()
{
    char buf[6];
    graphics.fillRectangle(150, graphics.settings.screenHeight - 9, 32, 8, Color::FromRGB(1,1,0));
    sprintf(buf,"%i", (_scrollOffset + _cursorY / 9) + 1);
    graphics.drawText(152, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::FromRGB(1,1,0), false);
}

void Editor::_drawColor()
{
    char buf[4];
    graphics.fillRectangle(200, graphics.settings.screenHeight - 9, 32, 8, Color::FromRGB(1,1,0));
    sprintf(buf,"%i", textColor);
    graphics.drawText(202, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::FromRGB(1,1,0), false);
    //_printChars(buf,Color::WHITE, 131, graphics.settings.screenHeight - 9, false);  
    graphics.fillRectangle(226, graphics.settings.screenHeight - 8, 5, 6, textColor);
}

void Editor::_drawFilename()
{
    graphics.fillRectangle(300, graphics.settings.screenHeight - 9, (strlen(_fileName) * graphics.settings.charWidth) + 4, 8, Color::FromRGB(1,1,0));
    graphics.drawText(302, graphics.settings.screenHeight - 9, _fileName, Color::WHITE, Color::FromRGB(1,1,0), false);
    //_printChars(buf,Color::WHITE, 131, graphics.settings.screenHeight - 9, false);  
    // graphics.fillRectangle(226, graphics.settings.screenHeight - 8, 5, 6, textColor);
}
