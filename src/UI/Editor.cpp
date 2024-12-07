#include "Editor.hpp"


void Editor::run()
{
    SetWindowHeight(graphics.settings.screenHeight - STATUS_BAR_HEIGHT);
    Console::run(false);
    _isEditorRunning = true;
    
    Serial.println("Enter text to render. Ctrl+R to quit");
    //DRAW BOTTOM SECTION
    DrawStatusBar();
    ConsoleKeyPress PS2Key;
    while(_isEditorRunning){
        const ConsoleKeyAction SerialAction = checkPort(Serial);
        //const ConsoleKeyAction PS2Action = checkPort(keyboardPs2);
        PS2Key = ps2Controller.getKey();
        if(SerialAction == ConsoleKeyAction::Cursor)        
            _drawCursorPosition();

        if(PS2Key.action == Cursor){
            switch (PS2Key.keyCode)
            {
            case PS2_KEY_ENTER: /* Cursor to beginning of next line or start */
            case PS2_KEY_KP_ENTER:
                EraseCursor();
                // editor.AdvanceCursor(true);
                AdvanceCursor(true);
                _drawCursorPosition();
                break;
            case PS2_KEY_SPACE:

                AdvanceCursor(false);
                _drawCursorPosition();
                break;
            case PS2_KEY_PGDN: /* Cursor to top row current column */
                //_rows = MAX_ROW - 1;
                break;
            case PS2_KEY_PGUP: /* Cursor to bottom row current column */
                //_rows = 0;
                break;
            case PS2_KEY_L_ARROW: 
                EraseCursor();
                MoveCursorLeft();
                _drawCursorPosition();
                break;
            case PS2_KEY_R_ARROW: 
                EraseCursor();
                MoveCursorRight();
                _drawCursorPosition();
                break;
            case PS2_KEY_UP_ARROW:
                EraseCursor();
                MoveCursorUp();
                _drawCursorPosition();
                break;
            case PS2_KEY_DN_ARROW:
                EraseCursor();
                MoveCursorDown();
                _drawCursorPosition();
                break;          
            
            case PS2_KEY_BS: /* Move cursor back write space move cursor back */
                graphics.fillRect(_cursorX,_cursorY, graphics.settings.charWidth, graphics.settings.charHeight + 1, Color::BLACK);
                ReverseCursor();
                _drawCursorPosition();
                break;
            case PS2_KEY_HOME: /* Cursor to top left */
                EraseCursor();
                SetPosition(0,_cursorY);
                _drawCursorPosition();
                break;
            case PS2_KEY_END: /* Cursor to max position */
            {
                const uint8_t maxChars = floor(graphics.settings.screenWidth / graphics.settings.charWidth);
                SetPosition(maxChars * graphics.settings.charWidth , _cursorY);
                _drawCursorPosition();
                break;
            }       

            case PS2_KEY_F2:
                textColor--;  
                _drawColor();
                break;
            case PS2_KEY_F3:
                textColor++;  
                _drawColor();
                break;         
            
            case PS2_KEY_F4:
                _isEditorRunning = false;
                break;
            default:
                break;
            }
        }
        
        if(SerialAction == ConsoleKeyAction::ColorChange)
            _drawColor();
       
        if(SerialAction == ConsoleKeyAction::Exit)
            _isEditorRunning = false; 

        if(PS2Key.action == ASCII)
            write(PS2Key.keyCode);

        if(_cursorVisible){
            if(millis() - _lastCursorChange > 1200){
                _lastCursorChange = millis();
                _cursorState = !_cursorState;
                DrawCursor();
            }
        }
    }
    clear();
}


bool Editor::AdvanceCursor(bool nextLine)
{
    bool newLine = Console::AdvanceCursor(nextLine);
    

    
    _drawCursorPosition();
}

bool Editor::ReverseCursor()
{
    if(!Console::ReverseCursor())
        return false;
    
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
    graphics.drawRect(0, graphics.settings.screenHeight - STATUS_BAR_HEIGHT, graphics.settings.screenWidth, 9,Color::DARK_GREEN);
    graphics.fillRect(2, graphics.settings.screenHeight - 9, 6* (graphics.settings.charWidth), 8,Color::FromRGB(2,3,2));
    sprintf(buf,"Cursor");
    graphics.drawText(2, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::DARK_GREEN, false);
    //_printChars(buf,Color::WHITE, 2, graphics.settings.screenHeight - 9, false);     
    
    _drawCursorPosition();

    graphics.fillRect(79, graphics.settings.screenHeight - 9, graphics.settings.screenWidth - 80, 8, Color::FromRGB(2,3,0));
    
    sprintf(buf,"F2:color     F4:Quit");
    graphics.drawText(80, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::FromRGB(2,3,0), false);
    //_printChars(buf,Color::WHITE, 80, graphics.settings.screenHeight - 9, false);     
    //see where we end up with color text

    _drawColor();

    sprintf(buf, "Draw Status bar took %lu ms", millis() - startTime);
    Serial.println(buf);
    
}


bool Editor::MoveCursorDown()
{
    if(_cursorY >= graphics.settings.screenHeight - graphics.settings.charHeight - STATUS_BAR_HEIGHT - 2 ) return false; //with 2px padding    
    Console::MoveCursorDown();
    _drawCursorPosition();
    return true;
}



void Editor::_drawCursorPosition(){
    //if(!_consoleRunning) return;
    if(_cursorX > graphics.settings.screenWidth || _cursorY > graphics.settings.screenHeight ) return;
    char label[9];
    uint8_t block[8 * graphics.settings.charWidth * graphics.settings.charHeight];

    sprintf(label,"%03d, %03d%c", _cursorX, _cursorY,'\0');        
    memset(block, Color::LIGHT_GREEN, 8 * graphics.settings.charWidth * graphics.settings.charHeight);
  
    graphics.drawTextToBuffer(label, block, 8 * graphics.settings.charWidth , Color::NAVY_BLUE);
    graphics.drawBuffer((6* (graphics.settings.charWidth) + 4), graphics.settings.screenHeight - 9 , 8 * graphics.settings.charWidth , graphics.settings.charHeight, block);

    //graphics.fillRect(x,y, blockWidth, blockHeight,color);                
    //graphics.drawText(x + 2, y + 2, label,color ^ 0xFF, color, false);
            
                     
    
    
    // //cursor x
    // memset(buf,0,9);
    // graphics.fillRect(6* (graphics.settings.charWidth) + 4, graphics.settings.screenHeight - 9, 18, 8,Color::LIME);    
    // sprintf(buf,"%i",_cursorX); 
    // graphics.drawText((6* (graphics.settings.charWidth) + 5), graphics.settings.screenHeight - 9, buf, Color::NAVY_BLUE, Color::LIME, false);
    // //_printChars(buf,Color::YELLOW, (6* (graphics.settings.charWidth) + 5), graphics.settings.screenHeight - 9, false);      
    
    
    // //cursor y
    // memset(buf,0,9);
    // graphics.fillRect(10* (graphics.settings.charWidth), graphics.settings.screenHeight - 9, 18, 8,Color::LIME);    
    // sprintf(buf,"%i",_cursorY);    
    // graphics.drawText((10 * (graphics.settings.charWidth) + 1), graphics.settings.screenHeight - 9, buf, Color::NAVY_BLUE, Color::LIME, false);
    // //_printChars(buf,Color::YELLOW, (9 * (graphics.settings.charWidth)) + 7, graphics.settings.screenHeight - 9, false);   

    // sprintf(buf,"%i, %i", _cursorX, _cursorY);
    //Serial.print("Set cursor to "); Serial.println(buf);

}

void Editor::_drawColor()
{
    char buf[4];
    graphics.fillRect(130, graphics.settings.screenHeight - 9, 24, 8, Color::FromRGB(1,1,0));
     sprintf(buf,"%i", textColor);
     graphics.drawText(131, graphics.settings.screenHeight - 9, buf, Color::WHITE, Color::FromRGB(1,1,0), false);
    //_printChars(buf,Color::WHITE, 131, graphics.settings.screenHeight - 9, false);  
    graphics.fillRect(150, graphics.settings.screenHeight - 8, 3, 6, textColor);
}


