#ifndef _fgColor_H_
#define _fgColor_H_
#include "stdint.h"

class Color {

    public:
    Color(uint8_t red, uint8_t green, uint8_t blue);
    Color(uint8_t color);
    Color(Color *source);
        
    inline uint8_t Red(){ return _red;}
    inline uint8_t Green(){ return _green;}
    inline uint8_t Blue(){ return _blue;}

    inline uint8_t ToByte(){ return _red | _green << 3 | _blue << 6;}

       
    static inline Color FromRGB(uint8_t red, uint8_t green, uint8_t blue){
        return Color(red,green,blue);
    }
    const static uint8_t WHITE = 0xFF;
    const static uint8_t GRAY = 0xDF;
    const static uint8_t BLACK = 0x0;
    const static uint8_t GREEN = 0x1 << 3;
    const static uint8_t BLUE = 0x1 << 6;
    const static uint8_t CYAN = 71;
    const static uint8_t YELLOW  = 63;
    const static uint8_t GOLD = 31;
    const static uint8_t ORANGE = 15;    
    const static uint8_t PEACH = 159;
    const static uint8_t RED = 0x7;
    const static uint8_t TEAL = 248;
    const static uint8_t PURPLE = 198 ;
    const static uint8_t LIME = 60;
    const static uint8_t BRICK = 151;
    const static uint8_t MAROON = 2;
    const static uint8_t BROWN = 17;
    const static uint8_t NAVY_BLUE = 80;
    const static uint8_t LIGHT_BLUE = 240;
    const static uint8_t LIGHT_GREEN = 30;
    const static uint8_t DARK_GREEN = 16;

    private:
        uint8_t _red;
        uint8_t _green;
        uint8_t _blue;
};
#endif
