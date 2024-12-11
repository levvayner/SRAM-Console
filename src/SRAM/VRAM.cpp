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
void VRAM::drawText(int x, int y, const char *text, byte color, byte backgroundColor, bool clearBackground, bool useFrameBuffer, BusyType busyType)
{
    uint16_t charOffsetY = 0, charOffsetX = 0;   
    //for each character
    for(size_t idx = 0; idx < strlen(text);idx++)    
    {
        uint16_t bufferSize = settings.charWidth * (settings.charHeight + 1);
        byte letterBuffer[bufferSize]; 
        memset(letterBuffer, backgroundColor, bufferSize);
        
        //for each column of character
        for(uint8_t charX = 0;charX < settings.charWidth;charX ++){
            byte column = charX < settings.charWidth - 1 ? CHARS[(uint8_t)(text[idx] - 32)][charX] : 0;


            for(int charY = 0; charY < settings.charHeight; charY++){
                byte isSet = column & (1 << charY);
                if(!isSet && !clearBackground) continue;
                letterBuffer[charY*settings.charWidth + charX] =  isSet ?  color : backgroundColor ;
            }
            
        }
        
        while(Busy(busyType));
        for(int line = 0; line < settings.charHeight; line++){            
            WriteBytes(
                ((y + charOffsetY + line) << settings.horizontalBits) + x + charOffsetX,
                letterBuffer + (line * settings.charWidth), 
                settings.charWidth,
                btVolatile
            );
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
void VRAM::drawText(int x, int y, const char *text, Color color, Color backgrounColor, bool clearBackground, bool useFrameBuffer, BusyType busyType )
{
    drawText(x, y, text, color.ToByte(), backgrounColor.ToByte(), clearBackground, useFrameBuffer, busyType);
}
void VRAM::drawText(int x, int y, char value, byte color,  byte backgroundColor, bool clearBackground, bool useFrameBuffer, BusyType busyType)
{
    char buf[2];
    sprintf(buf,"%c",value);
    drawText(x, y, buf, color, backgroundColor, clearBackground, useFrameBuffer, busyType);    
}
void VRAM::drawText(int x, int y, char text, Color color,  Color backgrounColor, bool clearBackground, bool useFrameBuffer, BusyType busyType)
{
    drawText(x, y, text, color.ToByte(), backgrounColor.ToByte(), clearBackground, useFrameBuffer, busyType);
}

void VRAM::drawTextToBuffer(const char* text, byte *buffer, uint16_t stride, byte color)
{
    uint16_t charOffsetY = 0, charOffsetX = 0;   
    //for each character
    for(size_t idx = 0; idx < strlen(text);idx++)    
    {

        for(uint8_t charX = 0;charX < settings.charWidth;charX ++){
            byte column = charX < settings.charWidth - 1 ? CHARS[(uint8_t)(text[idx] - 32)][charX] : 0;


            for(int charY = 0; charY < settings.charHeight; charY++){
               
                if(column & (1 << charY)) 
                    buffer[((charY + charOffsetY ) * stride) + charX + charOffsetX] =  color;
            }
            
        }

        //see if we can move over one char to the right
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

void VRAM::drawTextToBuffer(const char *text, const byte *colors, byte *buffer, uint16_t stride)
{
    uint16_t charOffsetY = 0, charOffsetX = 0;   
    //for each character
    for(size_t idx = 0; idx < strlen(text);idx++)    
    {

        for(uint8_t charX = 0;charX < settings.charWidth;charX ++){
            byte column = charX < settings.charWidth - 1 ? CHARS[(uint8_t)(text[idx] - 32)][charX] : 0;


            for(int charY = 0; charY < settings.charHeight; charY++){
               uint32_t offset = ((charY + charOffsetY ) * stride) + charX + charOffsetX;
                if(column & (1 << charY)) 
                    buffer[offset] = colors[idx];
            }
            
        }

        //see if we can move over one char to the right
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

void VRAM::drawBuffer(int x, int y, int width, int height, const byte *buffer)
{
    for(int line = 0; line < height; line++){            
        WriteBytes(
            ((y + line) << settings.horizontalBits) + x,
            (uint8_t*)buffer + (line * width), 
            width,
            btAny
        );
    }    
}

bool VRAM::drawPixel(int x, int y, byte color, BusyType busyType)
{
    if(x < 0 || x > settings.screenWidth) return false;
    if(y < 0 || y > settings.screenHeight) return false;
    return WriteByte((y << settings.horizontalBits) + x, color, busyType);
}

bool VRAM::drawPixel(int x, int y, Color color, BusyType busyType)
{
    return drawPixel(x,y, color.ToByte());
}

bool VRAM::drawLine(int x1, int y1, int x2, int y2, byte color, BusyType busyType)
{
    // find slope, increment from x1 to x2, changing y by slope
    if(x2 == x1) { //vertical line
        for(int y = y1; (y1 < y2) ?  y < y2 : y > y2; (y1 < y2) ? y++ : y--)
            drawPixel(x1,y,color);

        return true;
    }
    //horizontal
    if(y2 == y1){
        FillBytes((y1 << settings.horizontalBits) + min(x1,x2), color, abs(x2-x1), busyType);
        
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

bool VRAM::drawLine(Point start, Point end, byte color, BusyType busyType)
{
    return drawLine(start.x, start.y, end.x, end.y, color, busyType);
}

bool VRAM::drawLine(int x1, int y1, int x2, int y2, Color color, BusyType busyType)
{
    return drawLine(x1, y1, x2, y2, color.ToByte(), busyType);
}

bool VRAM::drawLine(Point start, Point end, Color color, BusyType busyType)
{
    return drawLine(start.x, start.y, end.x, end.y, color.ToByte(), busyType);
}

bool VRAM::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color)
{   
    _drawTriangle(x1, y1, x2, y2, x3, y3, color, false);
    return true;
    
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

   
    FillBytes((y1 << settings.horizontalBits) + x1, color, clipWidth);
    
    FillBytes(((y1 + clipHeight) << settings.horizontalBits) + x1, color, clipWidth);

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
    
    for(int y=y1; y < y1 + height; y++){
        FillBytes((y << settings.horizontalBits) + x1, color, width);
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

bool VRAM::drawCircle(int centerX, int centerY, int radius, byte color)
{
    int x = 0, y = -radius, p = -radius;
    while(x < -y){
        if(p > 0){
            y += 1;
            p += 2*(x+y) + 1;
        } else{
            p += 2*x + 1;
        }
        
        drawPixel(centerX + x, centerY + y, color);
        drawPixel(centerX - x, centerY + y, color);
        drawPixel(centerX + x, centerY - y, color);
        drawPixel(centerX - x, centerY - y, color);
        drawPixel(centerX + y, centerY + x, color);
        drawPixel(centerX + y, centerY - x, color);
        drawPixel(centerX - y, centerY + x, color);
        drawPixel(centerX - y, centerY - x, color);

        x += 1;
    }
    return true;
    //return drawArc(x,y,0,360,radius, color);
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
    int x = 0, y = -radius, p = -radius;
    int startX = 0, width = 0;
    while(x < -y){
        if(p > 0){
            //end of line reached
            y += 1;
            p += 2*(x+y) + 1;            
        } else{
            p += 2*x + 1;
        }
        
        if((centerY + y) >= 0 && (centerY + y) < settings.screenHeight){
            startX = centerX - x;
            width = abs(x) << 1;
            if(startX < 0){
                width += startX;
                startX = 0;
            }
            FillBytes(((centerY + y) << settings.horizontalBits) + startX, color, width); //top
        }
        if((centerY + x) >= 0 && (centerY + x) < settings.screenHeight){
            startX = centerX + y;
            width = abs(y) << 1;
            if(startX < 0){
                width += startX;
                startX = 0;
            }
            FillBytes(((centerY + x) << settings.horizontalBits) + startX, color, width);
        }
        if((centerY - x) >= 0 && (centerY - x) < settings.screenHeight){
            startX = centerX + y;
            width = abs(y) << 1;
            if(startX < 0){
                width += startX;
                startX = 0;
            }
            FillBytes(((centerY - x) << settings.horizontalBits) + startX, color, width);
        }
        if((centerY - y) >= 0 && (centerY - y) < settings.screenHeight){            
            startX = centerX - x;
            width = abs(x) << 1;
            if(startX < 0){
                width += startX;
                startX = 0;
            }
            FillBytes(((centerY - y) << settings.horizontalBits) + startX, color, width); //bottom
        }
    
        x += 1;
    }

    return true;
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

void VRAM::drawOval(int centerX, int centerY, int width, int height, byte color)
{
    int hh = height * height;
    int ww = width * width;
    int hhww = hh * ww;
    int x0 = width;
    int dx = 0;

    //if(centerX - width > 0)
        drawPixel(centerX - width, centerY, color);
    //if(centerX + width < settings.screenWidth)
        drawPixel(centerX + width, centerY, color);

    // now do both halves at the same time, away from the diameter
    for (int y = 1; y <= height; y++)
    {
        int x1 = x0 - (dx - 1);  // try slopes of dx - 1 or more
        for ( ; x1 > 0; x1--){        
             drawLine(centerX - x1, centerY - y, centerX - x0, centerY - y, color);
             drawLine(centerX + x0, centerY + y, centerX + x1, centerY + y, color);
             drawLine(centerX + x0, centerY - y, centerX + x1, centerY - y, color);
             drawLine(centerX - x1, centerY + y, centerX - x0, centerY + y, color);
            if (x1*x1*hh + y*y*ww <= hhww)
                break;
        }
        //if(centerX -x1 > 0 && centerY -y > 0)
            //     drawLine(centerX - x1, centerY - y, centerX - x0, centerY - y, color);
        
            // //if(centerX + x1 < settings.screenWidth && centerY + y < settings.screenHeight)
            //     //drawPixel(centerX + x1, centerY + y, color, btVolatile);
            //     drawLine(centerX + x1, centerY + y, centerX + x0, centerY + y, color);
                
            // //if(centerX + x1 < settings.screenWidth && centerY -y > 0)
            //     //drawPixel(centerX + x1, centerY - y, color, btVolatile);
            //     drawLine(centerX + x1, centerY - y, centerX + x0, centerY - y, color);

            // //if(centerX -x1 > 0 && centerY + y < settings.screenHeight)
            //     //drawPixel(centerX - x1, centerY + y, color, btVolatile);    
            //     drawLine(centerX - x1, centerY + y, centerX - x0, centerY + y, color);
        dx = x0 - x1;  // current approximation of the slope
        x0 = x1;
    }
}

void VRAM::fillOval(int centerX, int centerY, int width, int height, byte color)
{
    int hh = height * height;
    int ww = width * width;
    int hhww = hh * ww;
    int x0 = width;
    int dx = 0;

    // do the horizontal diameter
    int diameter = width << 1;
    int leftEdge = centerX - width;
    //uint16_t rightEdge = centerX + width;

    if(leftEdge < 0){
        diameter -= (centerX - width) * -1;
        leftEdge = 0;
    }

    diameter = min(diameter, settings.screenWidth - leftEdge);

    // sprintf(buf,"Filling oval with center (%d,%d), width %d, height: %d, left edge of %d and diameter %d",
    //     centerX, centerY, width, height, leftEdge, diameter
    // );
    // Serial.println(buf);
    
    FillBytes((centerY << settings.horizontalBits) + leftEdge, color, diameter);
    // now do both halves at the same time, away from the diameter
    
    for (int y = 1; y <= height; y++)
    {
        int x1 = x0 - (dx - 1);  // try slopes of dx - 1 or more
        for ( ; x1 > 0; x1--){            
            if (x1*x1*hh + y*y*ww <= hhww)
                break;
        }
        leftEdge = centerX - x1;
        diameter =  x1 << 1;
        if(leftEdge < 0) {
            leftEdge = 0;
            diameter -= (centerX - x1) * -1;
        }
        
        if(centerY - y > 0){
            drawLine(centerX - x1, centerY - y, centerX + x1, centerY - y, color);
            FillBytes(((centerY - y) << settings.horizontalBits) + leftEdge, color, diameter);
            // sprintf(buf,"Filling top half from (%d,%d) to (%d,%d)",
            //     leftEdge, centerY - y, leftEdge + diameter, centerY - y
            // );
            // Serial.println(buf);
        }
        if(centerY + y > 0){
            FillBytes(((centerY + y) << settings.horizontalBits) + leftEdge, color, diameter);
            // sprintf(buf,"Filling bottom half from (%d,%d) to (%d,%d)",
            //     leftEdge, centerY + y, leftEdge + diameter, centerY + y
            // );
            // Serial.println(buf);
        }
            
        dx = x0 - x1;  // current approximation of the slope
        x0 = x1;
    }
}

bool VRAM::_drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color, bool fill)
{

    // 4 possible configurations of a triangle
    // 1. All points on same Y
    // 2. Two points on top
    // 3. One point on top, bottom two are same y
    // 4. One point on top, another point below, third on bottom


    uint16_t yMin = y1;
    if(y2<yMin) yMin = y2;
    if(y3<yMin) yMin = y3;

    //how many virtacies on top y
    if(y1 == yMin && y2 == yMin && y3 == yMin){
        //all 3 verticies on same y, draw a line from min to max
        
        uint16_t minX = min(min(x1,x2),x3);
        uint16_t maxX = max(max(x1,x2),x3);
        drawLine(minX, yMin, maxX, yMin, color);
    }
    else if((y1 == yMin && y2 == yMin)){
       
        _drawTriangleTop2(x1, x2, y1, x3, y3, color, fill);

    }
    else if((y1 == yMin && y3 == yMin) ){
        // two verticies, y1 and y3
        
        _drawTriangleTop2(x1, x3, y1, x2, y2, color, fill);
    }
    else if((y2 == yMin && y3 == yMin)){
        
        // two verticies, y2 and y3
        _drawTriangleTop2(x2, x3, y2, x1, y1, color, fill);
    } else{
        //one verticie at the top, diverge toward next point, then converge towards last point
        if(y1 < y2 && y1 < y3){    
          
            if(y2 == y3){
                _drawTriangleTop1EqualBottoms(x1, y1, x2, x3, y2, color, fill);
            }else {
                bool isY2Middle = y2 < y3;
                _drawTriangleTop1DifferentBottoms(
                    x1,y1,  
                    isY2Middle ? x2 : x3,
                    isY2Middle ? y2 : y3,  
                    isY2Middle ? x3 : x2,
                    isY2Middle ? y3 : y2,  
                    color,
                    fill
                );
            }

        } 
        else if(y2 < y1 && y2 < y3){            
            
            if(y1 == y3){
                _drawTriangleTop1EqualBottoms(x2, y2, x1, x3, y1, color, fill);
            }else {
                bool isY1Middle = y1 < y3;
                _drawTriangleTop1DifferentBottoms(
                    x2,y2,  
                    isY1Middle ? x1 : x3,
                    isY1Middle ? y1 : y3,  
                    isY1Middle ? x3 : x1,
                    isY1Middle ? y3 : y1,  
                    color,
                    fill
                );
            }
            
        }
        else if(y3 < y1 && y3 < y2){
            
            if(y1 == y2){
                _drawTriangleTop1EqualBottoms(x3, y3, x1, x2, y1, color, fill);
            }else {
                bool isY1Middle = y1 < y2;
                _drawTriangleTop1DifferentBottoms(
                    x3,y3,  
                    isY1Middle ? x1 : x2,
                    isY1Middle ? y1 : y2,  
                    isY1Middle ? x2 : x1,
                    isY1Middle ? y2 : y1,  
                    color,
                    fill
                );
            }
        }
    }
    return true;
}

void VRAM::_drawTriangleTop2(int x1, int x2, int topY, int bottomX, int bottomY, byte color, bool fill)
{
    //char buf[128];
    FillBytes((topY << settings.horizontalBits) + min(x1, x2),color,max(x1,x2) - min(x1,x2));
    TrinagleLegDrawObject leg1 = TrinagleLegDrawObject(x1, topY, bottomX, bottomY);
    TrinagleLegDrawObject leg2 = TrinagleLegDrawObject(x2, topY, bottomX, bottomY);
    
    _drawTrinagleScanLines(leg1, leg2, topY, bottomY, color, fill);    
    
}

void VRAM::_drawTriangleTop1EqualBottoms(int topX, int topY, int x1, int x2, int bottomY, byte color, bool fill)
{
    TrinagleLegDrawObject leg1 = TrinagleLegDrawObject(topX, topY, x1, bottomY);
    TrinagleLegDrawObject leg2 = TrinagleLegDrawObject(topX, topY, x2, bottomY);

    _drawTrinagleScanLines(leg1, leg2, topY, bottomY, color, fill);
    FillBytes((bottomY << settings.horizontalBits) + min(x1, x2),color,max(x1,x2) - min(x1,x2));    

}
void VRAM::_drawTriangleTop1DifferentBottoms(int topX, int topY, int middleX, int middleY, int bottomX, int bottomY, byte color, bool fill){

    TrinagleLegDrawObject leg1 = TrinagleLegDrawObject(topX, topY, middleX, middleY);
    TrinagleLegDrawObject leg2 = TrinagleLegDrawObject(topX, topY, bottomX, bottomY);
    TrinagleLegDrawObject leg3 = TrinagleLegDrawObject(middleX, middleY, bottomX, bottomY);

    _drawTrinagleScanLines(leg1, leg2,topY, middleY, color, fill);
    _drawTrinagleScanLines(leg3, leg2,middleY, bottomY, color, fill);

    drawPixel(bottomX, bottomY, color);
}

void VRAM::_drawTrinagleScanLines(TrinagleLegDrawObject & leg1, TrinagleLegDrawObject & leg2, int topY, int bottomY, byte color, bool fill)
{
    int minX    = 0;
    int minX1   = 0;
    int minX2   = 0;
    int maxX    = 0;
    int maxX1   = 0;
    int maxX2   = 0;

    for(int yScan = max(topY,0); yScan <= min(bottomY, settings.screenHeight); yScan++){
        //memset(scanLine,0,sizeof(scanLine));
        
        leg1.prevX = leg1.rowX;
        leg2.prevX = leg2.rowX;
        leg1.rowX = leg1.isVertical ? leg1.rowX : ((float)(yScan - leg1.yIntercept )) / leg1.slope; 
        leg2.rowX = leg2.isVertical ? leg2.rowX : ((float)(yScan - leg2.yIntercept )) / leg2.slope;
        
        minX = min(leg1.rowX,leg2.rowX);
        minX1 = min(leg1.prevX,leg1.rowX);
        minX2 = min(leg2.prevX,leg2.rowX);
        maxX = max(leg1.rowX,leg2.rowX);
        maxX1 = max(leg1.prevX,leg1.rowX);
        maxX2 = max(leg2.prevX,leg2.rowX);
        if(minX < 0) minX = 0; 
        if(minX > settings.screenWidth) return; //entire trinagle is outside of the screen
        if(maxX < 0) return; //entire triangle is outside of the screen
        if(maxX > settings.screenWidth) maxX = settings.screenWidth;

        if(fill)
            FillBytes((yScan << settings.horizontalBits) + minX, color, maxX - minX);        
        else 
        {
            if(minX1 == maxX1){
                drawPixel(leg1.rowX, yScan, color);   
            } else
                FillBytes((yScan << settings.horizontalBits) + minX1, color, maxX1 - minX1);

            if(minX2 == maxX2){
                drawPixel(leg2.rowX, yScan, color);
            } else
                FillBytes((yScan << settings.horizontalBits) + minX2, color, maxX2 - minX2);
                
        }
    }
}
