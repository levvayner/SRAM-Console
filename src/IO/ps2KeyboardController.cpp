#include "ps2KeyboardController.h"
#include "UI/Console.h"
extern Console console;
void ps2KeyboardController::begin()
{
    // Configure the keyboard library
    keyboardPs2.begin( PS2_DATA, PS2_CLK );        
}

void ps2KeyboardController::loop()
{
    if( keyboardPs2.available() )
    {
        // read the next key
        if( (_c = keyboardPs2.read()) )
        {
        // check for some of the special keys
        _mode = 2;
        _c &= 0xFF;
        switch( _c )                     /* Cursor movements */
            {
            case PS2_KEY_ENTER:           /* Cursor to beginning of next line or start */
            case PS2_KEY_KP_ENTER:
                    _cols = 0;
                    _rows++;
                    if( _rows >= MAX_ROW )
                        _rows = 0;
                    break;
            case PS2_KEY_PGDN:            /* Cursor to top row current column */
                    _rows = MAX_ROW - 1;
                    break;
            case PS2_KEY_PGUP:            /* Cursor to bottom row current column */
                    _rows = 0;
                    break;
            case PS2_KEY_L_ARROW:         /* Cursor left or end of previous line */
                    _cols--;
                    if( _cols < 0 )
                        {
                        _cols = MAX_COL - 1;
                        _rows--;
                        if( _rows < 0 )
                        _rows = MAX_ROW - 1;
                        }
                    break;
            case PS2_KEY_R_ARROW:         /* Cursor right or start of next line */
                    _cols++;
                    if( _cols >= MAX_COL )
                        {
                        _cols = 0;
                        _rows++;
                        if( _rows >= MAX_COL )
                        _rows = 0;
                        }
                    break;
            case PS2_KEY_UP_ARROW:    /* Cursor up one line no wrap */
                    _rows--;
                    if( _rows < 0 )
                        _rows = 0;
                    break;
            case PS2_KEY_DN_ARROW:    /* Cursor down one line no wrap */
                    _rows++;
                    if( _rows >= MAX_ROW )
                        _rows = MAX_ROW - 1;
                    break;
            case PS2_KEY_BS:      /* Move cursor back write space move cursor back */
                    _cols--;
                    if( _cols < 0 )
                        {
                        _cols = MAX_COL - 1;
                        _rows--;
                        if( _rows < 0 )
                            _rows = MAX_ROW - 1;
                        }
                    // lcd.setCursor( _cols, _rows );
                    // lcd.write( ' ' );
                    break;
            case PS2_KEY_HOME:        /* Cursor to top left */
                    _cols = 0;
                    _rows = 0;
                    break;
            case PS2_KEY_END:         /* Cursor to max position */
                    _cols = MAX_COL - 1;
                    _rows = MAX_ROW - 1;
                    break;
            default:  /* Not cursor movement */
                    _mode = 0;
            }
            /* if was cursor movement do last movement */
        //   if( mode == 2 )
        //     lcd.setCursor( _cols, _rows );
           
            /* Check for strings or single character to display */
            /* Function or similar key */
            if( _c != PS2_KEY_EUROPE2 && ( _c < PS2_KEY_KP0 || _c >= PS2_KEY_F1 ) )
                {  // Non printable sort which ones we can print
                for( int idx = 0; idx < sizeof( codes ); idx++ )
        #if defined(PS2_REQUIRES_PROGMEM)
                if( c == pgm_read_byte( codes + idx ) )
        #else
                if( _c == codes[ idx ] )
        #endif
                {  /* String outputs */
                _mode = 1;
        #if defined(PS2_REQUIRES_PROGMEM)
                _c = pgm_read_byte( sizes + idx );
        #else
                _c = sizes[ idx ];
        #endif
                
                /* when cursor reset keep track */
                if( _cols == 0 )
                    _cols = _c;
        #if defined(PS2_REQUIRES_PROGMEM)
                strcpy_P( buffer, (char*)pgm_read_word( &( keys[ idx ] ) ) );
                lcd.print( buffer );
                
        #else
            {
                if(console.IsConsoleRunning())
                    console.write(keys[ idx ]);
            }
            //Serial.print(keys[ idx ]);
                //lcd.print(  );
        #endif
                _cols++;
                //check_cursor( );
                break;
                }
                /* if not found a string ignore key cant do anything */
                }
            else
                {  /* Supported key */
                if( _c <= 127 || _c > 0 )
                {
                //check_cursor( );
                _cols++;
                 if(console.IsConsoleRunning())
                     console.write(_c);               
                //lcd.write( c );
                //check_cursor( );
                }
                }
            }  

        }
}
