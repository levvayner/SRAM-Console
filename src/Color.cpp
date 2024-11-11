#include "Color.h"

Color::Color(uint8_t red, uint8_t green, uint8_t blue)
{
    _red = red & 0x7;
    _green = green & 0x7;
    _blue = blue & 0x3;
}

Color::Color(uint8_t color)
{
    _red = (color & 0x7);
    _green = (color >> 3) & 0x7;
    _blue = (color >> 6) & 0x3;
}
