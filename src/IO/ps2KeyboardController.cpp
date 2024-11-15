#include "ps2KeyboardController.h"
#include "UI/Editor.hpp"
// extern Editor editor;
void ps2KeyboardController::begin()
{
    // Configure the keyboard library
    keyboardPs2.begin(PS2_DATA, PS2_CLK);
}

ConsoleKeyPress ps2KeyboardController::getKey()
{
    ConsoleKeyPress kp(None, _c);
    if (keyboardPs2.available())
    {
        // read the next key
        if ((_c = keyboardPs2.read()))
        {
            
            // check for some of the special keys
            Serial.print("Processing PS2 key 0x");
            Serial.println(_c, HEX);

            
            
            kp.isShiftPressed = (_c & 0x4000);
            kp.isCtrlPressed = (_c & 0x2000);
            kp.isCaps = (_c & 0x1000);
            kp.isAltPressed = (_c & 0x800) |  (_c & 0x400);
            kp.isFunctionKey = (_c & 0x100);

            if(!(_c & 0x8000)) return kp; //if not key up, wait
            _mode = 2;
            _c &= 0xFF;
            if(kp.isFunctionKey){
                kp.action = Cursor;
                kp.keyCode = _c;
                return kp;  
            } 
            
            
                _mode = 0;
        

            /* Check for strings or single character to display */
            /* Function or similar key */
            if (_c != PS2_KEY_EUROPE2 && (_c < PS2_KEY_KP0 || _c >= PS2_KEY_F1))
            { // Non printable sort which ones we can print
            Serial.println("Nonprint chars");
                for (size_t idx = 0; idx < sizeof(codes); idx++)
#if defined(PS2_REQUIRES_PROGMEM)
                    if (c == pgm_read_byte(codes + idx))
#else
                    if (_c == codes[idx])
#endif
                    { /* String outputs */
                        _mode = 1;
#if defined(PS2_REQUIRES_PROGMEM)
                        _c = pgm_read_byte(sizes + idx);
#else
                        //_c = sizes[idx];
#endif
#if defined(PS2_REQUIRES_PROGMEM)
                        strcpy_P(buffer, (char *)pgm_read_word(&(keys[idx])));
                        lcd.print(buffer);

#else
                        {
                            // function keys, del
                            // if(editor.IsConsoleRunning()){
                            switch (_c)
                            {
                            case PS2_KEY_F2:
                            case PS2_KEY_F3:
                            Serial.print("Changing color .. ");
                                kp.action = ColorChange;
                                kp.keyCode = _c;
                                return kp;  
                            case PS2_KEY_F4:
                                kp.action = Exit;
                                kp.keyCode = _c;
                                return kp;  
                            default:
                                // Serial.print("Received unmapped ps2 key: 0x"); Serial.println(_c, HEX);
                                break;
                            }
                            //}
                        }
                        // Serial.print(keys[ idx ]);
                        // lcd.print(  );
#endif
                        // check_cursor( );
                        break;
                    }
                /* if not found a string ignore key cant do anything */
            }
            else
            { /* Supported key */
                if (_c <= 127 || _c > 0)
                {
                    // check_cursor( );
                    
                    kp.action = ASCII;
                    kp.keyCode = _c;
                    return kp;  
                    //  if(editor.IsConsoleRunning())
                    //      editor.write(_c);
                    // lcd.write( c );
                    // check_cursor( );
                }
            }
        }
    }
    return kp;
}
