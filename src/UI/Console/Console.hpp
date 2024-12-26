#ifndef _CONSOLE_H
#define _CONSOLE_H
#include "SD.h"
#include "Arduino.h"

#include "Color.h"
#include "DueHardwareVGA.h"
#include "Programming/ProgramRom.h"
#include "KeyboardController.h"
#include "CommandHistory.h"
#include "DueFlashStorage.h"

#include <DueTimer.h>

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

enum KeyInputMode{
    Text = 0,
    Escape = 1,
    Command = 2,
    CommandExtended = 3,
    Function = 4,
};

#define STATUS_BAR_HEIGHT 10

extern SRAM programmer;
extern VRAM graphics;
extern ProgramRom programRom;
extern KeyboardController keyboardUsb;
extern ps2KeyboardController ps2Controller;

union ScratchArea{
    byte bytes[256];
    char text[256];
};

void consoleDrawCursor();
class Console : Print{

    public:
    void run(bool blocking = true);
    void loop();
    virtual void stop();
    inline String path(){ return _path;}
    inline void setPath(const char* path){ _path = path;}
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
    inline size_t print(const String &str){ return write(str.c_str());}
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

    virtual inline void clear(){ 
        programmer.Erase(0, (graphics.settings.screenHeight + 1) << graphics.settings.horizontalBits); 
        
    }
    virtual inline void clearData(){ programmer.Erase(1<<19, graphics.settings.screenBufferHeight * (graphics.settings.screenWidth/graphics.settings.charWidth));}

    int getCoords(const char* str, int * coords, uint32_t offset = 0);

    virtual void processKey(uint8_t keyCode);

    virtual inline void SetPosition(int x = 0, int y = 0){ _cursorX = x; _cursorY = y;}
    virtual inline void SetScrollPosition(int offset = 0){ _scrollOffset = offset; }
    virtual inline byte GetColor(){return textColor; }
    virtual inline byte GetBackgroundColor(){return backgroundColor; }
    virtual inline void SetColor(byte color){ textColor = color;}
    virtual inline void SetColor(Color color){ textColor = color.ToByte();}
    virtual inline void SetBackgroundColor(byte color){  backgroundColor = color;}
    virtual inline void SetBackgroundColor(Color color){ backgroundColor = color.ToByte();}
    //virtual void processUSBKey(); //
    //virtual ConsoleKeyType processPS2Key(uint8_t ps2KeyCode); //
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

    virtual inline void SetEchoMode(bool commandModeEnabled){
        //_commandMode = commandModeEnabled;
        _echoPrompt = commandModeEnabled;
    }

    virtual inline bool GetCommandMode(){ return _commandMode;}
    virtual inline void SetCommandMode(bool commandModeEnabled){
        _commandMode = commandModeEnabled;
    }
    virtual inline Point GetPosition(){
        return Point(_cursorX, _cursorY);
    }
    //int RunProgram(const char* programName);

    /// @brief Calculates the array offset for current position
    /// number of chars across the screen * number of rows above current + ( x position / char width)
    /// @return Returns the offset in the data array for the current character 
    virtual inline uint16_t GetDataPos(){ return ((_cursorY / graphics.settings.charHeight) + _scrollOffset) * (charsPerLine) + (_cursorX / graphics.settings.charWidth);}
    
    void DrawCursor();
    void EraseCursor();
    inline void ToggleCursor(){ _cursorState = !_cursorState; }

    virtual void HideCursor(){
        if(!_cursorVisible) return;
        //stop timer
        _cursorTimer->detachInterrupt();
        _cursorTimer->stop();
        _cursorTimer = nullptr;
        _cursorVisible = false;
    }
    virtual void ShowCursor(){ 
        if(_cursorVisible) return;
        _cursorTimer = new DueTimer(Timer.getAvailable());
        _cursorTimer->attachInterrupt(consoleDrawCursor);
        _cursorTimer->start(500000);
        _cursorVisible = true;
    }
    
    void printDiskInfo();
    
    


    protected:
    byte textColor = Color::GREEN;
    byte backgroundColor = 0;
    uint16_t _cursorX = 0;
    uint16_t _cursorY = 0;
    uint32_t charsPerLine = 72;//  floor(graphics.settings.screenWidth / graphics.settings.charWidth);

    bool _cursorState = false;
    bool _cursorVisible = false;
    unsigned long _lastCursorChange = 0;   
    uint16_t _scrollOffset = 0;

    protected:
    inline void SetWindowHeight(int height){ _windowHeight = height;}
    inline int GetWindowHeight(){ return _windowHeight;}
    
    
    inline uint32_t LastIdx(){
        return _lastIdx;
    }
    inline void SetLastIdx(uint32_t idx){ _lastIdx = idx;}

    int getCurrentLineNumber(){
        return _scrollOffset + _cursorY / graphics.settings.charHeight;
    }
    
    private:
    void _drawTextFromRam();
    void _printEcho();
    //void _processKey(char keyVal);
    void _initSD();
    void _listFiles(const char * path, bool recursive = true, int indent = 0, uint8_t flags = LS_DATE | LS_SIZE);

    void _initCurrentCommand();
    int _saveCommand();


    private:    
    
    bool _commandReady = false; // from keyboard input interrupt
    char _cmdBuf[256];
    uint16_t _cmdBufIdx = 0;
    bool _needEcho = true;

    bool _consoleRunning = false; 
    bool _commandMode = false;   
    ScratchArea _scratch;
    uint16_t _lastIdx = 0;
    
    uint16_t _windowHeight = 240;
    //const char* _prompt = "%s |>";
    String _path = "/";
    uint16_t _promptLength = 5;
    bool _echoPrompt = true;
    int16_t echoY = 0;
    //keeping history of commands
    CommandHistory _history;
    uint8_t _commandViewIdx = 0;

    bool _initialized = false;
    //curent input
    char _currentCommand[256];
    uint8_t _currentCommandIdx = 0;
    KeyInputMode _currentInputMode;

    DueTimer *_cursorTimer;
    


    Sd2Card card;
    SdVolume volume;
    SdFile root;
};

#endif