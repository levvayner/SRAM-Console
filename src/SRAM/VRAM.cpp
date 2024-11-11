#include "VRAM.h"
#include "math.h"
bool VRAM::drawPixel(int x, int y, byte color)
{
    return WriteByte(y << 8 + x, color);
}

bool VRAM::drawPixel(int x, int y, Color color)
{
    return drawPixel(x,y, color.ToByte());
}

bool VRAM::drawLine(int x1, int y1, int x2, int y2, byte color)
{
    // find slope, increment from x1 to x2, changing y by slope
    double slope = (x2 - x1) / (y2 - y1);
    for(double xCoord = x1; x1 != x2; (x1 - x2 > 0) ? xCoord-- : xCoord++){
        for(double yCoord = y1; yCoord != y2; (y1 - y2 > 0 )? yCoord -= slope : yCoord += slope){
            drawPixel(xCoord, yCoord, color);
        }
    }
}

bool VRAM::drawLine(Point start, Point end, byte color)
{
    return drawLine(start.x, start.y, end.x, end.y, color);
}

bool VRAM::drawLine(int x1, int y1, int x2, int y2, Color color)
{
    return drawLine(x1, y1, x2, y2, color.ToByte());
}

bool VRAM::drawLine(Point start, Point end, Color color)
{
    return drawLine(start.x, start.y, end.x, end.y, color.ToByte());
}

bool VRAM::drawRect(int x1, int y1, int width, int height, byte color)
{
    //draw buffered top and bottom
    byte buf[width];
    memset(buf,color, width);
    WriteBytes(y1 << 8 + x1, buf, width);
    WriteBytes((y1 + height) << 8 + x1, buf, width);

    //draw pixeled left and right
    for(int y=y1; y < y1 + height; y++){
        WriteByte(y << 8 + x1, color);
        WriteByte(y << 8 + x1 + width, color);
    }
}

bool VRAM::drawRect(Point topLeft, Point bottomRight, byte color)
{
    return drawRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, color);
}

bool VRAM::drawRect(int x1, int y1, int width, int height, Color color)
{
    return drawRect(x1, y1, width, height, color.ToByte());
}

bool VRAM::drawRect(Point topLeft, Point bottomRight, Color color)
{
    return drawRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, color.ToByte());
}

bool VRAM::fillRect(int x1, int y1, int width, int height, byte color)
{
    byte buf[width];
    memset(buf,color, width);

    for(int y=y1; y < y1 + height; y++){
        WriteBytes(y << 8 + x1, buf, width);
    }    
}

bool VRAM::fillRect(Point topLeft, Point bottomRight, Color color)
{
    return fillRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, color.ToByte());
}

bool VRAM::drawCircle(int x, int y, int radius, byte color)
{
    return false;
}

bool VRAM::drawArc(int x, int y, int startAngle, int endAngle, int radius, byte color)
{
    int i;
    double angle, radian;

    for (i = startAngle; i <= endAngle; i++) {
        radian = i * 3.14159 / 180;
        WriteByte((int)(y + radius * sin(radian)) << 8 | (byte)(x + (radius * cos(radian))), color);
    }
    return false;
}
