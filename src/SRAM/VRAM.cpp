#include "VRAM.h"
#include "math.h"
VRAM::VRAM()
{
    

}
VRAM::~VRAM()
{
    free(_frameBuffer);
}
void VRAM::begin(){
    // _frameBuffer = (uint8_t *)malloc((settings.screenWidth- 20) * (settings.screenHeight - 20));
    // if(_frameBuffer == NULL){
    //     Serial.print("Failed to init frame buffer!");
    // }
    // else {
    //     memset(_frameBuffer, ERASE_BYTE, (settings.screenWidth - 20) * (settings.screenHeight - 20));
    //     Serial.print("Initialized frame buffer with "); Serial.print(settings.screenWidth * settings.screenHeight); Serial.println(" bytes");
    // }
}
void VRAM::drawText(int x, int y, const char *text, byte color, bool clearBackground, bool useFrameBuffer)
{
    uint16_t charOffsetY = 0, charOffsetX = 0;   
    //for each character
    for(size_t idx = 0; idx < strlen(text);idx++)    
    {
        
        //for each column of character
        for(uint8_t charX = 0;charX < settings.charWidth - 1;charX ++){
        //TODO: optimize to write line to buffer, then flish out the buffer to ram
            byte column = charX < settings.charWidth ? CHARS[(uint8_t)(text[idx] - 32)][charX] : 0;


            for(int charY = 0; charY < settings.charHeight; charY++){
                byte isSet = (column & (1 << charY)) > 0;
                if(!isSet && !clearBackground) continue;
                if(useFrameBuffer)
                    _frameBuffer[((y + charOffsetY + charY) * settings.screenWidth) + x + charOffsetX + charX] =  isSet ? color : 0;
                else
                    WriteByte(((y + charOffsetY + charY) << settings.horizontalBits) + x + charOffsetX + charX, isSet ? color : 0);
            }
            if(clearBackground)
                if(useFrameBuffer)
                     _frameBuffer[((y + charOffsetY + settings.charHeight) * settings.screenWidth) + x + charOffsetX + charX] =  0;
                else
                    WriteByte(((y + charOffsetY + settings.charHeight) << settings.horizontalBits) + x + charOffsetX + charX, 0);
            
        }

        //see if we can move over one pixel to the right
        if (charOffsetX + settings.charWidth < settings.screenWidth)
        {
            charOffsetX += settings.charWidth;            
        } else{
            //otherwise advance to next available line or beginning
            charOffsetX = 0;
            if(charOffsetY + settings.screenHeight * 2 < settings.screenHeight ){
                charOffsetY += settings.charHeight;
            } else {
                charOffsetY = 0;
            }
        }
    }
}
void VRAM::drawText(int x, int y, const char *text, Color color, bool clearBackground, bool useFrameBuffer )
{
    drawText(x, y, text, color.ToByte(), clearBackground, useFrameBuffer);
}
void VRAM::drawText(int x, int y, char value, byte color, bool clearBackground, bool useFrameBuffer)
{
    char buf[2];
    sprintf(buf,"%c",value);
    drawText(x, y, buf, color, clearBackground, useFrameBuffer);    
}
void VRAM::drawText(int x, int y, char value, Color color, bool clearBackground, bool useFrameBuffer)
{
    drawText(x, y, value, color.ToByte(), clearBackground, useFrameBuffer);
}
bool VRAM::drawPixel(int x, int y, byte color)
{
    if(x < 0 || x > settings.screenWidth) return false;
    if(y < 0 || y > settings.charHeight) return false;
    return WriteByte((y << settings.horizontalBits) + x, color);
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
        WriteBytes((y1 << settings.horizontalBits) + (x1 > x2 ? x2 : x1),buf,length);
        return true;
    }
    double slope = (double)(y2 - y1) / double(x2 - x1);

    //sprintf(buf,"Drawing line from (%i,%i) to (%i, %i).\nSlope: %lf\n", x1, y1, x2, y2, slope);
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
    return true;
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

bool VRAM::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color)
{
    return drawLine(x1,y1,x2,y2,color) && drawLine(x2,y2,x3,y3,color) && drawLine(x3, y3, x1, y1, color);
}

bool VRAM::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color)
{
    return drawTriangle(x1, y1, x2, y2, x3, y3, color.ToByte());
}

bool VRAM::drawRect(int x1, int y1, int width, int height, byte color)
{
    //TODO: clip to screen
    //draw buffered top and bottom
    if(x1 > settings.screenWidth) return false;
    if(y1 > settings.screenHeight) return false;
    int clipWidth = width;
    int clipHeight = height;
    if(settings.screenWidth - (width + x1) < width) clipWidth = settings.screenWidth - x1;
    if(settings.screenHeight - (y1 + height) < height) clipHeight = settings.screenHeight - y1; 

    byte buf[clipWidth];
    memset(buf,color, clipWidth);
    WriteBytes((y1 << settings.horizontalBits) + x1, buf, clipWidth);
    
    WriteBytes(((y1 + clipHeight) << settings.horizontalBits) + x1, buf, clipWidth);

    //draw pixeled left and right
    for(byte y=y1; y < y1 + clipHeight; y++){
        WriteByte((y << settings.horizontalBits) + x1, color);        
        WriteByte((y << settings.horizontalBits) + x1 + clipWidth, color);
    }

    return true;
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
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    if(width > settings.screenWidth - x1) width = settings.screenWidth - x1;
    if(height > settings.screenHeight - y1) height = settings.screenHeight - y1;
    byte buf[width];
    memset(buf,color, width);

    for(int y=y1; y < y1 + height; y++){
        WriteBytes((y << settings.horizontalBits) + x1, buf, width);
    }  
    return true;  
}

bool VRAM::fillRect(int x1, int y1, int width, int height, Color color)
{
    return fillRect(x1,y1,width, height, color.ToByte());
}

bool VRAM::fillRect(Point topLeft, Point bottomRight, byte color)
{
    return fillRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, color);
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
        if(xVal <= 0 || yVal <= 0 || xVal >= settings.screenWidth || yVal >= settings.screenHeight)
            continue; // do not render off-screen content
        WriteByte((int) yVal << settings.horizontalBits | (uint16_t)xVal, color);
    }
    return false;
}

bool VRAM::fillCircle(int centerX, int centerY, int radius, byte color)
{
    Rectangle boundRect = Rectangle(centerX - radius,centerY - radius, centerX + radius, centerY + radius);
    if(boundRect.x1 < 0) boundRect.x1 = 0;
    if(boundRect.y1 < 0) boundRect.y1 = 0;
    if(boundRect.width() > settings.screenWidth - boundRect.x1) boundRect.x2 = settings.screenWidth;
    if(boundRect.height() > settings.screenHeight - boundRect.y1) boundRect.y2 = settings.screenHeight ;
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
                data[(pxlX - boundRect.x1)] = ReadByte((pxlY << settings.horizontalBits) + pxlX);
            }
        }
        WriteBytes((pxlY << settings.horizontalBits) + boundRect.x1, data, boundRect.width());
    }

 

    return false;
}

void VRAM::render()
{
    unsigned long startTime = millis();
    Serial.print("Rendering frame .. ");
    for(uint8_t line = 0; line < settings.screenHeight; line++){
        WriteBytes(line << settings.horizontalBits, _frameBuffer + (line * settings.screenWidth), settings.screenWidth);
    }
    Serial.print(" completed in "); Serial.print(millis() - startTime); Serial.println(" ms.");
}
