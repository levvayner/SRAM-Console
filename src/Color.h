#ifndef textColor_H_
#define textColor_H_
#include "stdint.h"

class Color {

    public:
    Color(uint8_t red, uint8_t green, uint8_t blue);
    Color(uint8_t color);
    Color(Color *source);

    
        
    inline uint8_t Red(){ return _red;}
    inline uint8_t Green(){ return _green;}
    inline uint8_t Blue(){ return _blue;}

    inline void FromByte(uint8_t color){
        _red = (color & 0x7);
        _green = (color >> 3) & 0x7;
        _blue = (color >> 6) & 0x3;
    }

    inline uint8_t ToByte(){ return (_red << 5) | ((_green & 0x7) << 2) | (_blue & 0x3);}

       
    static inline Color FromRGB(uint8_t red, uint8_t green, uint8_t blue){
        return Color(red,green,blue);
    }
    const static uint8_t WHITE = 0xFF;
    const static uint8_t GRAY = 210;
    const static uint8_t BLACK = 0x0;
    const static uint8_t GREEN = 0x7 << 3;
    const static uint8_t BLUE = 0x3 << 6 ;
    const static uint8_t YELLOW  = 252;
    const static uint8_t GOLD = 216;
    const static uint8_t ORANGE = 237;    
    const static uint8_t PEACH = 242;
    const static uint8_t RED = 224;
    const static uint8_t TEAL = 87;
    const static uint8_t PURPLE = 96 ;
    const static uint8_t LIME = 125;
    const static uint8_t BRICK = 165;
    const static uint8_t MAROON = 129;
    const static uint8_t BROWN = 140;
    const static uint8_t NAVY_BLUE = 1;
    const static uint8_t LIGHT_BLUE = 119;
    const static uint8_t LIGHT_GREEN = 93;
    const static uint8_t DARK_GREEN = 9;

    private:
        uint8_t _red;
        uint8_t _green;
        uint8_t _blue;
};
#endif
