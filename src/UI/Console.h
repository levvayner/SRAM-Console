#ifndef _CONSOLE_H
#define _CONSOLE_H
#include "Arduino.h"
#include "Color.h"
#include "SRAM/SRAM.h"
#include "SRAM/VRAM.h"
#include "Programming/ProgramRom.h"
#include "KeyboardController.h"
#include "IO/ps2KeyboardController.h"

/*
Console has two primary responsibilities
   1. Render the editor/console window
   2. Store entered data into a data buffer in the RAM.

   A0-A15 (currently, soon A0-A18) are used for x and y coordinates.
   A19 high, or upper half the RAM, will be reserved for off the screen stuff, leaving 512kB for each.

*/

#define STATUS_BAR_HEIGHT 10

extern SRAM programmer;
extern VRAM graphics;
extern ProgramRom programRom;
extern KeyboardController keyboardUsb;
extern ps2KeyboardController ps2Controller;

//ascii chars with offset of 32, starting with space
const byte CHARS[96][5] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, // space
    {0x0, 0x0, 0xBF, 0X0 , 0X0}, // !
    {0X0,0XE,0X0,0XE,0X0}, // "
    {0X28, 0XFE, 0X28, 0XFE, 0X28}, // #
    {0X4C, 0X92, 0XFF, 0X92, 0X64}, // $
    {0XC3,0X30,0XC,0X3,0X60}, // %
    {0X66, 0X99, 0X91, 0XA1, 0X52}, // &
    {0X0, 0X0, 0XE, 0X0, 0X0}, // '
    {0X0,0X3C,0X42,0X81,0X9}, // (
    {0X0,0X81,0X42,0X3C,0X0}, // )
    {0X2A,0X1C,0X7F,0X1C,0X2A}, // *
    {0X10, 0X10, 0X7C, 0X10, 0X10}, // +
    {0X0, 0XA0, 0X60, 0X0, 0X0}, // ,    
    {0X0, 0X8, 0X8, 0X8, 0X8}, // -
    {0X0, 0XC0, 0XC0, 0X0, 0X0}, // .
    {0XC0, 0X30, 0XC, 0X3, 0X0}, // /
    { 0X7E, 0XE1, 0X9D, 0X83, 0X7E }, // 0
    { 0x84, 0x82, 0xFF, 0X80, 0X80 }, // 1
    { 0X62, 0X91, 0X91, 0X99, 0X86 }, // 2
    { 0X42, 0X81, 0X89, 0X89, 0X76 }, // 3
    { 0X0F, 0X08, 0X08, 0X08, 0XFF }, // 4
    { 0X8F, 0X89, 0X89, 0X89, 0xF9 }, // 5
    { 0X7E, 0X91, 0X89, 0X89, 0X72 }, // 6
    { 0X3, 0X1, 0X1, 0X1, 0XFF }, // 7
    { 0X7E, 0X89, 0X89, 0X89, 0X7E }, // 8
    { 0X6, 0X9, 0X9, 0X9, 0XFE }, // 9

    {0X0, 0X0, 0X6C, 0X6C, 0X0}, // :
    {0X0, 0X0, 0XA6, 0X66, 0X0}, // ;
    {0X08, 0X14, 0X22, 0X41, 0X0}, // <
    {0X0, 0X14, 0X14, 0X14, 0X14}, // =
    {0X41, 0X22, 0X14, 0X8, 0X0}, // > 
    {0X2, 0X1, 0XB1, 0X9, 0X6}, // ?
    {0X1E, 0X25, 0X59, 0X51, 0X4E}, // @
    //UPPER CASE

    {0XE0, 0X3C, 0X23, 0X3C, 0XE0}, // A
    {0XFF, 0X99, 0X99, 0X99, 0X66}, // B
    {0X7E, 0X81, 0X81, 0X81, 0X42}, // C
    {0XFF, 0X81, 0X81, 0X81, 0X7E}, // D
    {0XFF, 0X89, 0X89, 0X89, 0X81}, // E
    {0XFF, 0X9, 0X9, 0X9, 0X1}, // F
    {0X7E, 0X81, 0X91, 0X91, 0X62}, // G
    {0XFF, 0X8, 0X8, 0X8, 0XFF}, // H
    {0X0, 0X81, 0XFF, 0X81, 0X0}, // I
    {0X40, 0X80, 0X80, 0X80, 0X7F}, // J
    {0XFF, 0X18, 0X18, 0X14, 0XE3}, // K
    {0XFF, 0X80, 0X80, 0X80, 0XC0}, // L
    {0XFF, 0X2, 0XC, 0X2, 0XFF}, // M
    {0XFF, 0X6, 0XC, 0X17, 0XFF}, // N
    {0X7E, 0X81, 0X81, 0X81, 0X7E}, // O
    {0XFF, 0X11, 0X11, 0X11, 0XE}, // P
    {0X3E, 0X41, 0X41, 0X41, 0XBE}, // Q
    {0XFF, 0X19, 0X29, 0X49, 0X86}, // R
    {0X4E, 0X89, 0X89, 0X89, 0X72}, // S
    {0X1, 0X1, 0XFF, 0X1, 0X1}, // T
    {0X7F, 0X80, 0X80, 0X80, 0X7F}, // U
    {0X7, 0X78, 0XC0, 0X78 ,0X7}, // V
    {0XF, 0XF0, 0X8, 0XF0, 0XF}, // W
    {0XE3, 0X14, 0X18, 0X14, 0XE3}, // X
    {0X3, 0XC, 0XF0, 0XC, 0X3}, // Y
    {0XC1, 0X81, 0X91, 0X8D, 0X83}, // Z

    //SYMOBOLS
    {0X0, 0XFF, 0X81, 0X81, 0X0}, // [
    {0X1, 0X6, 0X18, 0X60, 0X80}, // "\"
    {0X0, 0X81, 0X81, 0XFF, 0X0}, // ]
    {0X4, 0X2, 0X1, 0X2, 0X4}, // ^
    {0X80, 0X80, 0X80, 0X80, 0X80}, // _
    {0X0, 0X6, 0XC, 0X0, 0X0}, // `
    {0x60, 0x94, 0x94 ,0xF8, 0x00}, // a
    {0XFE, 0X90, 0X90, 0X90, 0X60}, // b
    {0X78, 0X84, 0X84, 0X84, 0X48}, // c
    {0X0, 0X60, 0X90, 0X90, 0XFE}, // d
    {0X78, 0XA4, 0XA4, 0X98, 0X0}, // e
    {0X20, 0XF8, 0X24, 0X8, 0X0}, // f
    {0X18, 0XA4, 0XA4, 0XD8, 0X0}, // g
    {0XFC, 0X10, 0X10, 0XE0 ,0X0}, // h
    {0X0, 0X0, 0XF4, 0X0, 0X0}, // i
    {0X40, 0X80, 0X80, 0X74, 0X0}, // j
    {0XFC, 0X20, 0XD8, 0X0 ,0X0}, // k
    {0X0,  0X0, 0XFE, 0X0, 0X0}, // l
    {0XFC, 0X4, 0XF8, 0X4, 0XF8}, // m
    {0XFC, 0X8, 0X4, 0X4, 0XF8}, // n
    {0X78, 0X84, 0X84, 0X78, 0X0}, // o
    {0XFC, 0X24, 0X24, 0X18, 0X0}, // p
    {0X0, 0X1C, 0X22, 0X22, 0XFC}, // q
    {0XFC, 0X8, 0X4, 0X4, 0X0}, // r
    {0X0, 0X4C, 0X92, 0X92, 0X64}, // s
    {0X10, 0X7E, 0X90, 0X80, 0X40}, // t
    {0X7C, 0X80, 0X80, 0X7C, 0X0}, // u
    {0XC, 0X30, 0XC0, 0X30, 0XC}, // v
    {0X78, 0X80, 0X78, 0X80, 0X78}, // w
    {0X84, 0X48, 0X30, 0X48, 0X84}, // x
    {0X86,0X68, 0X30, 0X8, 0X6}, // y
    {0XC4, 0XA4, 0X94, 0X8C, 0X0}, // z
    //MORE SYMBOLS
    {0X18,0X7E,0X81,0X81,0X0}, // {
    {0X0,0X0, 0X7E, 0X0, 0X0}, // |
    {0X0, 0X81, 0X81, 0X7E, 0X18}, // }
    {0X10, 0X8, 0X18, 0X10, 0X8}, // ~
};



class Console : Print{

    public:

    void run();
    size_t write(uint8_t data, Color color, bool clearBackround = false);
    size_t write(uint8_t data);
    size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }
    virtual size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *buffer, size_t size) {
      return write((const uint8_t *)buffer, size);
    }

    //virtual size_t write(const uint8_t *buffer, size_t size, Color color);
    size_t write(const char *buffer, size_t size, Color color, bool clearBackground = true);
    
    size_t print(const __FlashStringHelper *);
    size_t print(const String &);
    size_t print(const char[]);    
    size_t print(char);
    size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long, int = DEC);
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

    inline void clear(){ programmer.EraseRam(0, graphics.settings.screenHeight << 8);}
    inline void clearData(){ programmer.EraseRam(1<<19, graphics.settings.screenHeight * graphics.settings.screenWidth);}

    inline void SetPosition(int x = 0, int y = 0, bool drawPosition = true){ _cursorX = x; _cursorY = y; if(drawPosition && _consoleRunning) {_drawCursorPosition();}}
    void processUSBKey(); //
    inline bool IsConsoleRunning(){ return _consoleRunning;}

    protected: 
    void AdvanceCursor( bool nextLine = false);
    
    bool ReverseCursor ();
    void DrawStatusBar();

    bool MoveCursorUp();
    bool MoveCursorDown();
    bool MoveCursorRight();
    bool MoveCursorLeft();
    
    /// @brief Calculates the array offset for current position
    /// number of chars across the screen * number of rows above current + ( x position / char width)
    /// @return Returns the offset in the data array for the current character 
    inline uint16_t GetDataPos(){ return ((_cursorY - 1 >= 0) ? _cursorY / graphics.settings.charHeight : 0) * (graphics.settings.screenWidth / graphics.settings.charWidth) + (_cursorX / graphics.settings.charWidth);}
    

    

    private:

    byte _cursorX = 0;
    byte _cursorY = 0;
    bool _cursorState = false;
    bool _cursorVisible = true;
    unsigned long _lastCursorChange = 0;


    byte _charHeight = 9;

    byte _fgColor = 240;
    bool _consoleRunning = false;
    
    
    void _printChar(uint8_t chr, byte charX, byte charY, bool clearBackground = true);
    void _printChars(const char *data, Color color, byte charX, byte charY, bool clearBackground = true);
    void _drawCursorPosition();
    void _drawColor();
    void _drawCursor();

    void _processKey(char keyVal);
    template< typename T>
    void _checkPort(T & port);
};

#endif