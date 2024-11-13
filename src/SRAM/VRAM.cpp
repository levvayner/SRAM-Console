#include "VRAM.h"
#include "math.h"
bool VRAM::drawPixel(int x, int y, byte color)
{
    return WriteByte((y << 8) + x, color);
}

bool VRAM::drawPixel(int x, int y, Color color)
{
    return drawPixel(x,y, color.ToByte());
}

bool VRAM::drawLine(int x1, int y1, int x2, int y2, byte color)
{
    // find slope, increment from x1 to x2, changing y by slope
    if(x2 == x1) {
        for(int y = y1; (y1 < y2) ?  y < y2 : y > y2; (y1 < y2) ? y++ : y--)
            drawPixel(x1,y,color);

        return true;
    }
    if(y2 == y1){
        byte length = abs(x2 - x1);
        uint8_t buf[length];
        memset(buf,color,length);
        WriteBytes((y1 << 8) + (x1 > x2 ? x2 : x1),buf,length);
        return true;
    }
    double slope = (double)(y2 - y1) / double(x2 - x1) ;
    char buf[256];

    //sprintf(buf,"Drawing line from (%i,%i) to (%i, %i).\nSlope: %lf\n", x1, y1, x2, y2, slope);
    Serial.println(buf);
    if(slope > 1 || slope < -1){
        double step = 1/slope;
        double xCoord = x1;
        //more x for each y, move 1 x at a time
        for(double yCoord = y1; (y1 < y2) ? yCoord < y2 : yCoord > y2;  (y1 < y2) ? yCoord+= 1 : yCoord-= 1){     
            (x1 < x2) ? xCoord+= step : xCoord-= step ;
            drawPixel((int)xCoord, (int)yCoord, color);
        }
    

    } else{
        
        //more y for each x, move one y at a time
        for(double xCoord = x1; (x1 < x2) ? xCoord < x2 : xCoord > x2;  (x1 < x2) ? xCoord+= 1 : xCoord-= 1){
            double yCoord = abs(xCoord - x1) * slope + y1;
            drawPixel((int)xCoord, (int)yCoord, color);
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
    WriteBytes((y1 << 8) + x1, buf, width);
    WriteBytes(((y1 + height) << 8) + x1, buf, width);

    //draw pixeled left and right
    for(int y=y1; y < y1 + height; y++){
        WriteByte((y << 8) + x1, color);
        WriteByte((y << 8) + x1 + width, color);
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
        WriteBytes((y << 8) + x1, buf, width);
    }    
}

bool VRAM::fillRect(Point topLeft, Point bottomRight, Color color)
{
    return fillRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, color.ToByte());
}

bool VRAM::drawCircle(int x, int y, int radius, byte color)
{
    
    return drawArc(x,y,0,360,radius, color);
}

bool VRAM::drawArc(int x, int y, int startAngle, int endAngle, int radius, byte color)
{
    int i;
    double radian;

    for (i = startAngle; i <= endAngle; i++) {
        radian = i * 3.14159 / 180;
        auto yVal = (y + radius * sin(radian));
        auto xVal = (x + (radius * cos(radian)));
        if(xVal <= 0 || yVal <= 0 || xVal >= SCREEN_WIDTH || yVal >= SCREEN_HEIGHT)
            continue; // do not render off-screen content
        WriteByte((int) yVal << 8 | (byte)xVal, color);
    }
    return false;
}

bool VRAM::fillCircle(int centerX, int centerY, int radius, byte color)
{
    Rectangle boundRect = Rectangle(centerX - radius,centerY - radius, centerX + radius, centerY + radius);
    byte data[boundRect.width()];

    //memset(data, 0, boundRect.height() * boundRect.width()); //TODO: consider using global background color or not filling
    for(int pxlY = boundRect.y1; pxlY < boundRect.y2; pxlY++){
        for(int pxlX = boundRect.x1; pxlX < boundRect.x2; pxlX ++){
            //test if point is in circle
            double distance = sqrt(pow(abs(pxlX - centerX),2) + pow(abs(pxlY - centerY), 2));
            if(distance < radius){
                data[(pxlX - boundRect.x1)] = color;
            }
            else {
                data[(pxlX - boundRect.x1)] = ReadByte((pxlY << 8) + pxlX);
            }
        }
        WriteBytes((pxlY << 8) + boundRect.x1, data, boundRect.width());
    }

 

    return false;
}
