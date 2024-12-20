#ifndef _V_RAM_H
#define _V_RAM_H
#include "SRAM.h"
#include "Color.h"
#include "UI/Chars.h"


struct Point{
    uint16_t x;
    uint16_t y;
    Point(int x, int y){ this->x = x; this->y = y;}
};

struct TriangleLeg{
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    float slope = 0;
    float yIntercept = 0;
    bool isVertical = false;
    bool isHorizontal = false;
    TriangleLeg(){

    }

    TriangleLeg(int x1, int y1, int x2, int y2){
        this->x1 = x1;
        this->x2 = x2;
        this->y1 = y1;
        this->y2 = y2;
        isVertical = x1 == x2;
        isHorizontal = y1 == y2;
        
        slope = isHorizontal ? 0 : isVertical ? 0 : ((float)(max(y1, y2) - min(y1, y2)) / (float)(y1 > y2? x1 - x2 : x2 - x1));
        yIntercept = -1*(slope * x1 - y1);
    }
};

struct TrinagleLegDrawObject : public TriangleLeg{
    int rowX = 0, prevX  = 0;
    TrinagleLegDrawObject(int x1, int y1, int x2, int y2): TriangleLeg(x1, y1, x2, y2){  
        rowX = x1;
        prevX = x2;
    }
    
};

struct Rectangle{
    public:
    int16_t x1;
    int16_t x2;
    int16_t y1;
    int16_t y2;

    int16_t width(){ return x2 > x1 ? x2 - x1 : x1 - x2; } // abs(x2 - x1);}
    int16_t height(){ return y2 > y1 ? y2 - y1: y1 - y2; } // abs(y2 - y1);}
    int32_t size() { return width() * height();}

    Rectangle(Point p1, Point p2){
        x1 = p1.x;
        x2 = p2.x;
        y1 = p1.y;
        y2 = p2.y;
    }
    Rectangle( int16_t x1, int16_t y1, int16_t x2, int16_t y2){
        this->x1 = x1;
        this->x2 = x2;
        this->y1 = y1;
        this->y2 = y2;
    }
};


struct VRAMSettings{
    uint16_t screenWidth = 432;
    uint16_t screenHeight = 240;
    uint16_t charWidth = 6;
    uint16_t charHeight = 9;
    uint16_t screenBufferHeight = 3000;
    uint16_t horizontalBits = 10;
    VRAMSettings(uint16_t width, uint16_t height, uint8_t charWidth = 6, uint8_t charHeight = 9, uint8_t horizontalBits = 10){
        screenWidth = width;
        screenHeight = height;
        this->charWidth = charWidth;
        this->charHeight = charHeight;
        this->horizontalBits = horizontalBits;
    }
};





class VRAM : public SRAM{
    
    public:
        VRAMSettings settings = VRAMSettings(432, 240);

        VRAM();
        ~VRAM();

        void begin();
        void end();

        

        virtual void drawText(int x, int y, const char * text, byte color = 0xFF, byte backgroundColor = 0x0, bool clearBackground = true, bool useFrameBuffer = false, BusyType busyType = btAny);
        virtual void drawText(int x, int y, const char * text, Color color = Color::WHITE, Color backgroundColor = Color::BLACK, bool clearBackground = true, bool useFrameBuffer = false, BusyType busyType = btAny);

        virtual void drawText(int x, int y, char text, byte color = 0xFF, byte backgroundColor = 0x0, bool clearBackground = true, bool useFrameBuffer = false, BusyType busyType = btAny);
        virtual void drawText(int x, int y, char text, Color color = Color::WHITE, Color backgroundColor = Color::BLACK, bool clearBackground = true, bool useFrameBuffer = false, BusyType busyType = btAny);

        virtual void drawTextToBuffer(const char * text, byte* buffer,  uint16_t stride, byte color);
        virtual void drawTextToBuffer(const char * text, const byte * colors, byte* buffer,  uint16_t stride);

        virtual void drawBuffer(int x, int y, int width, int height, const byte* buffer);

        virtual bool drawPixel(int x, int y, byte color = 0xFF, BusyType busyType = btAny);
        virtual bool drawPixel(int x, int y, Color color, BusyType busyType = btAny);

        virtual uint8_t readPixel(int x, int y, BusyType busyType = btAny);

        virtual bool drawLine(int x1, int y1, int x2, int y2, byte color = 0xFF, BusyType busyType = btAny);
        virtual bool drawLine (Point start, Point end, byte color, BusyType busyType = btAny);
        virtual bool drawLine(int x1, int y1, int x2, int y2, Color color = Color::WHITE, BusyType busyType = btAny);
        virtual bool drawLine (Point start, Point end, Color color = Color::WHITE, BusyType busyType = btAny);

        virtual bool drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color = 0xFF);
        virtual bool drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color)
        {
            return drawTriangle(x1, y1, x2, y2, x3, y3, color.ToByte());
        }

        virtual inline bool fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color = 0xFF){
            return _drawTriangle(x1, y1, x2, y2, x3, y3, color, true);
        }
        virtual inline bool fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color = Color::WHITE){
            return fillTriangle(x1, y1, x2, y2, x3, y3, color.ToByte());
        }

        virtual bool drawRectangle(int x1, int y1, int width, int height, byte color = 0xFF, BusyType busyType = btAny);
        virtual bool drawRectangle(Point topLeft, Point bottomRight, byte color = 0xFF, BusyType busyType = btAny);
        virtual bool drawRectangle(int x1, int y1, int width, int height, Color color = Color::WHITE, BusyType busyType = btAny);
        virtual bool drawRectangle(Point topLeft, Point bottomRight, Color color = Color::WHITE, BusyType busyType = btAny);

        virtual bool fillRectangle(int x1, int y1, int width, int height, byte color = 0xFF, BusyType busyType = btAny);
        virtual bool fillRectangle(int x1, int y1, int width, int height,  Color color =Color::WHITE, BusyType busyType = btAny);
        virtual bool fillRectangle(Point topLeft, Point bottomRight, byte color = 0xFF, BusyType busyType = btAny);
        virtual bool fillRectangle(Point topLeft, Point bottomRight, Color color =Color::WHITE, BusyType busyType = btAny);

        virtual bool drawCircle(int centerX, int centerY, int radius, byte color = 0xFF, BusyType busyType = btAny);
        virtual inline bool drawCircle(int centerX, int centerY, int radius, Color color = Color::WHITE, BusyType busyType = btAny){
            return drawCircle(centerX, centerY, radius, color.ToByte(), busyType);
        }

        virtual bool drawArc(int x, int y, int startAngle, int endAngle, int radius, byte color = 0xFF);
        virtual inline bool drawArc(int x, int y, int startAngle, int endAngle, int radius, Color color = Color::WHITE){
            return drawArc(x, y, startAngle, endAngle, radius, color.ToByte());
        }

        virtual bool fillCircle(int x, int y, int radius, byte color = 0xFF);
        virtual inline bool fillCircle(int x, int y, int radius, Color color = Color::WHITE) { 
            return fillCircle(x, y, radius, color.ToByte());
        }

        virtual void drawOval(int centerX, int centerY, int width, int height, byte color = Color::WHITE);
        inline virtual void drawOval(int centerX, int centerY, int width, int height, Color color = Color::WHITE){
            drawOval(centerX, centerY, width, height, color.ToByte());
        }

        virtual void fillOval(int centerX, int centerY, int width, int height, byte color = Color::WHITE);
        

        virtual inline bool clear(int x1 = 0, int y1 = 0, int width = 0, int height = 0){
            if(width == 0) width = settings.screenWidth - x1 + 2;
            if(height == 0) height = settings.screenHeight - y1 + 2;
            return fillRectangle(x1, y1, width, height, Color::BLACK);
        }

        /// @brief renders frame buffer to screen (writes to ram)
        virtual void render();

    static inline uint8_t averageColors(uint8_t a, uint8_t b){
        return (((a>> 5) + (b >> 5)) >> 1) << 5 | 
        ((((a >> 3 )& 0x7) + ((b >> 3)  & 0x7)) >> 1) << 3 | 
        ((((a & 0x3) + (b & 0x3)) >> 1) & 0x3);
    }


    static inline  uint8_t diffColors(uint8_t a, uint8_t b) {
        return ((a>> 5) + (b >> 5) / 2) << 5 | 
        (((a >> 3 )& 0x7) + ((b >> 3)  & 0x7)/ 2) << 3 | 
        ((a & 0x3 + b&0x3)/2) & 0x3;
        // uint8_t s = a + b;
        // uint8_t m = (s - ((a ^ b) & 0x01010101)) & 0x01010101;
	    // return s - m;
    }

    static inline  uint8_t mulitplyColors(uint8_t a, uint8_t b) {
        uint8_t s = a + b;
        uint8_t m = (s - ((a ^ b) & 0x01010101)) & 0x01010101;
	    return m * 0xFF >> 8 | (s - m);
    }

    private:
    bool _drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color = 0xFF, bool fill = false);
    void _drawTriangleTop2(int x1, int x2, int topY, int x3, int bottomY, byte color, bool fill = false);
    void _drawTriangleTop1EqualBottoms(int topX, int topY, int x1, int x2, int bottomY, byte color, bool fill = false);
    void _drawTriangleTop1DifferentBottoms(int topX, int topY, int middleX, int middleY, int bottomX, int bottomY, byte color, bool fill = false);

    void _drawTrinagleScanLines(TrinagleLegDrawObject & leg1, TrinagleLegDrawObject & leg2, int topY, int bottomY, byte color, bool fill);
    private:
    uint8_t *_frameBuffer;

};
#endif

extern VRAM graphics;
