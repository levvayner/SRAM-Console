#ifndef _CONSOLE_H
#define _CONSOLE_H
#include "SD.h"
#include "Arduino.h"

#include "Color.h"
#include "Chars.h"
#include "SRAM/SRAM.h"
#include "SRAM/VRAM.h"
#include "Keys.h"
#include "Programming/ProgramRom.h"
#include "KeyboardController.h"
#include "IO/ps2KeyboardController.h"
#include "Code/Execution.h"
#include "CommandHistory.h"
#include "Bitmap.h"
#include "DueFlashStorage.h"


/*
Console has two primary responsibilities
   1. Render the console/console window
   2. Store entered data into a data buffer in the RAM.

   A0-A15 (currently, soon A0-A18) are used for x and y coordinates.
   A19 high, or upper half the RAM, will be reserved for off the screen stuff, leaving 512kB for each.

*/

enum DrawShape{
    shapeLine = 0,
    shapeTriangle = 1,
    shapeRectangle = 2,
    shapeOval = 3,
    shapeCircle = 4,
    shapeUnknown = -1,
};

#define STATUS_BAR_HEIGHT 10

extern SRAM programmer;
extern VRAM graphics;
extern ProgramRom programRom;
extern KeyboardController keyboardUsb;
extern ps2KeyboardController ps2Controller;
class Console : Print{

    public:
    void run(bool blocking = true);
    virtual size_t write(uint8_t data, byte color, byte backgroundColor, bool clearBackround = false, bool useFrameBuffer = false);
    virtual size_t write(uint8_t data, Color color, Color backgroundColor, bool clearBackround = false, bool useFrameBuffer = false);
    virtual size_t write(uint8_t data, bool useFrameBuffer);
    virtual inline  size_t write(uint8_t data){ return write(data,textColor, backgroundColor, true, false);}
    virtual size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }
    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual size_t write(const char *buffer, size_t size) {
      return write((const uint8_t *)buffer, size);
    }

    //virtual size_t write(const uint8_t *buffer, size_t size, Color color);
    size_t write(const char *buffer, size_t size, Color color, Color backgroundColor, bool clearBackground = true, bool useFrameBuffer = false);
    
    size_t print(const __FlashStringHelper *);
    size_t print(const String &);
    size_t print(const char[]);    
    size_t print(char);
    size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long num, int base = DEC);
    size_t print(double, int = 2);
    size_t print(const Printable&);

    size_t println(const __FlashStringHelper *);
    size_t println(const String &s);
    size_t println(const char[]);
    size_t println(char);
    size_t println(unsigned char, int = DEC);
    size_t println(int, int = DEC);
    size_t println(unsigned int, int = DEC);
    size_t println(long, int = DEC);
    size_t println(unsigned long, int = DEC);
    size_t println(double, int = 2);
    size_t println(const Printable&);
    size_t println(void);

    virtual inline void clear(){ programmer.Erase(0, (_windowHeight + 1) << graphics.settings.horizontalBits);}
    virtual inline void clearData(){ programmer.Erase(1<<19, graphics.settings.screenBufferHeight * (graphics.settings.screenWidth/graphics.settings.charWidth));}

    virtual inline void SetPosition(int x = 0, int y = 0, bool drawPosition = true){ _cursorX = x; _cursorY = y; }
    virtual inline void SetColor(byte color){ textColor = color;}
    virtual inline void SetColor(Color color){ textColor = color.ToByte();}
    virtual inline void SetBackgroundColor(byte color){  backgroundColor = color;}
    virtual inline void SetBackgroundColor(Color color){ backgroundColor = color.ToByte();}
    virtual void processUSBKey(); //
    virtual ConsoleKeyAction processPS2Key(uint8_t ps2KeyCode); //
    inline bool IsConsoleRunning(){ return _consoleRunning;}

    /// @brief Advances cursor in console.
    /// @param nextLine True if cursor should advance to next line
    /// @return True if cursor advanced to a new line
    virtual bool AdvanceCursor( bool nextLine = false);    
    virtual bool ReverseCursor ();
    virtual bool MoveCursorUp();
    virtual bool MoveCursorDown();
    virtual bool MoveCursorRight();
    virtual bool MoveCursorLeft();

    virtual inline void SetCommandMode(bool commandModeEnabled){
        _commandMode = commandModeEnabled;
    }

    virtual inline Point GetPosition(){
        return Point(_cursorX, _cursorY);
    }
    int RunProgram(const char* programName);

    protected: 
    
    
    /// @brief Calculates the array offset for current position
    /// number of chars across the screen * number of rows above current + ( x position / char width)
    /// @return Returns the offset in the data array for the current character 
    virtual inline uint16_t GetDataPos(){ return ((_cursorY / graphics.settings.charHeight) + _scrollOffset) * ((graphics.settings.screenWidth / graphics.settings.charWidth))+ (_cursorX / graphics.settings.charWidth);}
    
    


    protected:
    byte textColor = 240;
    byte backgroundColor = Color::BLACK;
    uint16_t _cursorX = 0;
    uint16_t _cursorY = 0;
    uint32_t charsPerLine = 72;//  floor(graphics.settings.screenWidth / graphics.settings.charWidth);

    bool _cursorState = false;
    bool _cursorVisible = true;
    unsigned long _lastCursorChange = 0;   
    uint16_t _scrollOffset = 0;

    protected:
    inline void SetWindowHeight(int height){ _windowHeight = height;}
    void DrawCursor();
    void EraseCursor();

    int getCurrentLineNumber(){
        return _scrollOffset + _cursorY / graphics.settings.charHeight;
    }
    

    int ProcessCommand(const char* command);

    template< typename T>
    ConsoleKeyAction checkPort(T &port)
    {
        byte chr=0;
        bool writable = true;

        if(port.available()){
            chr = port.read();
            if( chr == 255) return None;
            
            //Serial.print("Read 0x"); Serial.println(chr, HEX);
            if(chr == 0x1B){
                writable = false;
                //escape key. If not directly followed by other chars, treat as escape
                auto startTime = millis();
                while(port.available() < 2 && millis() - startTime < 500);
                if(!port.available())                
                    return None;
                
                char nextChar = port.read();
                //Serial.print("Read 0x"); Serial.println(nextChar, HEX);
                //function keys
                if(nextChar == 0x4F ){
                    if(!port.available())  return None;
                    nextChar = port.read();
                    switch (nextChar)
                    {
                        case 0x50: //F1
                        case 0x51: //F2
                        {
                            textColor++; 
                            return ConsoleKeyAction::ColorChange;                         
                        }
                            
                        case 0x53: //F4
                        {
                            //dump data to serial
                            uint16_t pos = GetDataPos();
                            if(pos <=  0) return None;
                            Serial.println("Dumping data contents... ");
                            for(int idx = 0; idx < GetDataPos();idx++){
                                nextChar = programmer.ReadByte(1 << 19 | idx);
                                if((nextChar >= 32 && nextChar < 127) || nextChar == 10)
                                    Serial.print(nextChar);
                            }
                            programmer.ReadByte(0x0); // unset bit 19 so screen accesses video ram
                        }
                            return None;                        
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
                            return ConsoleKeyAction::Cursor ;
                        case 0x41: //up arrow
                            //Serial.println("Pressed up arrow");
                            EraseCursor();
                            MoveCursorUp();
                            return ConsoleKeyAction::Cursor;
                        case 0x42: //down arrow
                            //Serial.println("Pressed down arrow");
                            EraseCursor();
                            MoveCursorDown();
                            return ConsoleKeyAction::Cursor;         
                        case 0x43: //right arrow                    
                            //Serial.println("Pressed right arrow");
                            EraseCursor();
                            MoveCursorRight();
                            return ConsoleKeyAction::Cursor;
                        case 0x44: //left arrow
                            //Serial.println("Pressed left arrow");
                            EraseCursor();
                            MoveCursorLeft();
                            return ConsoleKeyAction::Cursor;
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
                        if(!this->IsConsoleRunning() || !_echoPrompt || _cursorX < _promptLength * graphics.settings.charWidth){
                            EraseCursor();
                            //_printChar(0, _cursorX, _cursorY); // get rid of cursor
                            ReverseCursor();
                            return ConsoleKeyAction::Cursor;
                        }
                        //_printChar(0, _cursorX, _cursorY);
                    }
                    //Serial.println();
                    return None;
                }
            }
            
            if(chr == 0x8){                
                EraseCursor();
                //_printChar(0, _cursorX, _cursorY); // get rid of cursor
                ReverseCursor();
                graphics.fillRectangle(_cursorX,_cursorY, graphics.settings.charWidth, graphics.settings.charHeight, Color::BLACK);
                //_printChar(0, _cursorX, _cursorY); // get rid of last char
                return ConsoleKeyAction::Cursor;
            }
            
            if(chr == 18){ //ctrl + r
                _consoleRunning = false;
                return ConsoleKeyAction::Exit;
            }
                
                
            if(writable) {                
                write(chr);
                return ConsoleKeyAction::ASCII;
            }        
            
        }
        return None;
    }

    private:
    void _drawTextFromRam();
    void _drawEcho();
    void _processKey(char keyVal);
    void _initSD();
    void _listFiles(const char * path, bool recursive = true, int indent = 0, uint8_t flags = LS_DATE | LS_SIZE);

    int getCoords(const char* str, int * coords, int offset = 0);


    private:    
    
    bool _consoleRunning = false; 
    bool _commandMode = false;   
    byte _scratch[256];
    uint16_t _lastIdx = 0;
    
    uint16_t _windowHeight = 240;
    //const char* _prompt = "%s |>";
    String _path = "/";
    uint16_t _promptLength = 5;
    bool _echoPrompt = true;
    uint16_t _consoleLine = 0;
    CommandHistory _history;
    uint8_t _commandViewIdx = 0;
    bool _initialized = false;

    Sd2Card card;
    SdVolume volume;
    SdFile root;
    
    DueFlashStorage dueFlashStorage;
};

#endif