#ifndef _V_RAM_H
#define _V_RAM_H
#include "SRAM.h"
#include "Color.h"
#include "UI/Chars.h"


struct Point{
    int x;
    int y;
    Point(int x, int y){ this->x = x; this->y = y;}
};

struct Rectangle{
    public:
    int x1;
    int x2;
    int y1;
    int y2;

    int width(){ return abs(x2 - x1);}
    int height(){ return abs(y2 - y1);}

    Rectangle(Point p1, Point p2){
        x1 = p1.x;
        x2 = p2.x;
        y1 = p1.y;
        y2 = p2.y;
    }
    Rectangle( int x1, int y1, int x2, int y2){
        this->x1 = x1;
        this->x2 = x2;
        this->y1 = y1;
        this->y2 = y2;
    }
};


struct VRAMSettings{
    int screenWidth = 200;
    int screenHeight = 120;
    int charWidth = 6;
    int charHeight = 9;
    int screenBufferHeight = 3000;
};


class VRAM : SRAM{
    
    public:
        VRAMSettings settings;

        void drawText(int x, int y, const char * text, byte color = 0xFF, bool clearBackground = true);
        void drawText(int x, int y, const char * text, Color color = Color::WHITE, bool clearBackground = true);

        void drawText(int x, int y, char value, byte color = 0xFF, bool clearBackground = true);
        void drawText(int x, int y, char value, Color color = Color::WHITE, bool clearBackground = true);

        bool drawPixel(int x, int y, byte color = 0xFF);
        bool drawPixel(int x, int y, Color color);

        bool drawLine(int x1, int y1, int x2, int y2, byte color = 0xFF);
        bool drawLine (Point start, Point end, byte color);
        bool drawLine(int x1, int y1, int x2, int y2, Color color = Color::WHITE);
        bool drawLine (Point start, Point end, Color color = Color::WHITE);

        bool drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color = 0xFF);
        bool drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color = Color::WHITE);

        bool drawRect(int x1, int y1, int width, int height, byte color = 0xFF);
        bool drawRect (Point topLeft, Point bottomRight, byte color = 0xFF);
        bool drawRect(int x1, int y1, int width, int height, Color color = Color::WHITE);
        bool drawRect (Point topLeft, Point bottomRight, Color color = Color::WHITE);

        bool fillRect(int x1, int y1, int width, int height, byte color = 0xFF);
        bool fillRect(int x1, int y1, int width, int height,  Color color =Color::WHITE);
        bool fillRect (Point topLeft, Point bottomRight, byte color = 0xFF);
        bool fillRect (Point topLeft, Point bottomRight, Color color =Color::WHITE);

        bool drawCircle(int x, int y, int radius, byte color = 0xFF);
        bool drawCircle(int x, int y, int radius, Color color = Color::WHITE);

        bool drawArc(int x, int y, int startAngle, int endAngle, int radius, byte color = 0xFF);
        bool drawArc(int x, int y, int startAngle, int endAngle, int radius, Color color = Color::WHITE);

        bool fillCircle(int x, int y, int radius, byte color = 0xFF);
        bool fillCircle(int x, int y, int radius, Color color = Color::WHITE);

        

        bool clear(int x1 = 0, int y1 = 0, int width = 800, int height = 600){
            return fillRect(x1, y1, width, height, Color::BLACK);
        }

    

};
#endif