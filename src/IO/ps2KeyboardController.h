#ifndef _PS2_KEYBOARD_H_
#define _PS2_KEYBOARD_H_

#include "Arduino.h"
#include "Pins.h"
#include "PS2KeyAdvanced.h"
#include "UI/Keys.h"

/* Columns in display */
#define MAX_COL 16
/* Rows in display */
#define MAX_ROW  2


extern PS2KeyAdvanced keyboardPs2;
/* current cursor position */

#if defined(PS2_REQUIRES_PROGMEM)
const uint8_t codes[] PROGMEM = { PS2_KEY_SPACE, PS2_KEY_TAB, PS2_KEY_ESC, PS2_KEY_DELETE,
                                   PS2_KEY_F1, PS2_KEY_F2, PS2_KEY_F3, PS2_KEY_F4,
                                   PS2_KEY_F5, PS2_KEY_F6, PS2_KEY_F7, PS2_KEY_F8,
                                   PS2_KEY_F9, PS2_KEY_F10, PS2_KEY_F11, PS2_KEY_F12 };
const char spacestr[] PROGMEM  = " ";
const char tabstr[] PROGMEM  = "[Tab]";
const char escstr[] PROGMEM  = "[ESC]";
const char delstr[] PROGMEM  = "[Del]";
const char f1str[]  PROGMEM  = "[F1]";
const char f2str[]  PROGMEM  = "[F2]";
const char f3str[]  PROGMEM  = "[F3]";
const char f4str[]  PROGMEM  = "[F4]";
const char f5str[]  PROGMEM  = "[F5]";
const char f6str[]  PROGMEM  = "[F6]";
const char f7str[]  PROGMEM  = "[F7]";
const char f8str[]  PROGMEM  = "[F8]";
const char f9str[]  PROGMEM  = "[F9]";
const char f10str[] PROGMEM  = "[F10]";
const char f11str[] PROGMEM  = "[F11]";
const char f12str[] PROGMEM  = "[F12]";

// Due to AVR Harvard architecture array of string pointers to actual strings
const char *const keys[] PROGMEM =  {
                                spacestr, tabstr, escstr, delstr, f1str, f2str,
                                f3str, f4str, f5str, f6str, f7str, f8str,
                                f9str, f10str, f11str, f12str };
const int8_t sizes[] PROGMEM = { 1, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5 };
char buffer[ 8 ];

#else

const uint8_t codes[] = { PS2_KEY_SPACE, PS2_KEY_TAB, PS2_KEY_ESC,
                          PS2_KEY_DELETE, PS2_KEY_F1, PS2_KEY_F2, PS2_KEY_F3,
                          PS2_KEY_F4, PS2_KEY_F5, PS2_KEY_F6, PS2_KEY_F7,
                          PS2_KEY_F8, PS2_KEY_F9, PS2_KEY_F10, PS2_KEY_F11,
                          PS2_KEY_F12 };
const char *const keys[]  =  { " ", "    ", "[ESC]", "[Del]", "[F1]", "[F2]", "[F3]",
                               "[F4]", "[F5]", "[F6]", "[F7]", "[F8]",
                               "[F9]", "[F10]", "[F11]", "[F12]" };
const int8_t sizes[]  = { 1, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5 };
#endif

// initialize the keyboard library with the numbers of the interface pins


class ps2KeyboardController{
public:
    void begin();
    ConsoleKeyPress getKey();

private:

/* mode = 0 echo character
   mode = 1 print string
   mode = 2 cursor movement NO other echo
   mode = 4 ignore key no echo */
    byte _mode = 0;
    byte _idx = 0;
    uint16_t _c;

    int8_t _cols = 0;
    int8_t _rows = 0;

};

#endif