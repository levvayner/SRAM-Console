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

bool VRAM::drawPixel(int x, int y, byte color)
{
    if(x < 0 || x > settings.screenWidth) return false;
    if(y < 0 || y > settings.screenHeight) return false;
    return WriteByte((y << settings.horizontalBits) + x, color);
}

bool VRAM::drawPixel(int x, int y, Color color)
{
    return drawPixel(x,y, color.ToByte());
}

bool VRAM::drawLine(int x1, int y1, int x2, int y2, byte color)
{
    // find slope, increment from x1 to x2, changing y by slope
    if(x2 == x1) { //vertical line
        for(int y = y1; (y1 < y2) ?  y < y2 : y > y2; (y1 < y2) ? y++ : y--)
            drawPixel(x1,y,color);

        return true;
    }
    //horizontal
    if(y2 == y1){
        FillBytes(y1 << settings.horizontalBits + min(x1,x2), color, abs(x2-x1));
        
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
    while(x < -y){
        if(p > 0){
            //end of line reached
            y += 1;
            p += 2*(x+y) + 1;
        } else{
            p += 2*x + 1;
        }
        drawLine(centerX - x, centerY + y, centerX + x, centerY + y, color); // top
        drawLine(centerX - y, centerY + x, centerX + y, centerY + x, color); // second
        drawLine(centerX - y, centerY - x, centerX + y, centerY - x, color); // third
        drawLine(centerX - x, centerY - y, centerX + x, centerY - y, color); // bottom
    
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


bool VRAM::_drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color, bool fill)
{

    // 4 possible configurations of a triangle
    // 1. All points on same Y
    // 2. Two points on top
    // 3. One point on top, bottom two are same y
    // 4. One point on top, another point below, third on bottom


    //char buf[256];
    uint16_t yMin = y1;
    if(y2<yMin) yMin = y2;
    if(y3<yMin) yMin = y3;

    // sprintf(buf,"P1: (% 3d, % 3d)  P2: (% 3d, % 3d)  P3: (% 3d, % 3d)", x1, y1, x2, y2, x3, y3);
    // drawText(10,20, buf, Color::WHITE);
    

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
}

void VRAM::_drawTriangleTop2(int x1, int x2, int topY, int bottomX, int bottomY, byte color, bool fill)
{
    //char buf[128];
    FillBytes((topY << settings.horizontalBits) + min(x1, x2),color,max(x1,x2) - min(x1,x2));
    //converge towards y3
    int rowX1 = x1, prevX1 = x1; //line from x1 to x3
    int rowX2 = x2, prevX2 = x2; // line from x2 to x3
    bool slope1Vertical = (x1 - bottomX) == 0;
    bool slope2Vertical = (x2 - bottomX) == 0;
    float slopeX1 = ((float)(bottomY - topY) / (float)(bottomX - x1)); //rise over run
    float slopeX2 = ((float)(bottomY - topY) / (float)(bottomX - x2)); //rise over run
    int yInterceptX1 = -1*(slopeX1 * x1 - topY);    // slopeX1 =  (y1 - yInt) / x1, -1 * (slopeX1 * x1 - y1) = yInt
    int yInterceptX2 = -1*(slopeX2 * x2 - topY);    // 

    
    for(int yScan = topY; yScan <= bottomY; yScan++){
        //memset(buf,0,sizeof(buf));
        prevX1 = rowX1;
        prevX2 = rowX2;
        rowX1 = slope1Vertical ? rowX1 : ((float)(yScan - yInterceptX1 )) / slopeX1; 
        rowX2 = slope2Vertical ? rowX2 : ((float)(yScan - yInterceptX2 )) / slopeX2; 
        // sprintf(buf,"(%d,%d)  -  (%d,%d)", rowX1, yScan, rowX2, yScan);
        // Serial.println(buf);
        if(fill)
            FillBytes((yScan << settings.horizontalBits) + min(rowX1,rowX2), color, max(rowX1,rowX2) - min(rowX1,rowX2));        
        else 
        {
            if(slopeX1 < 1.0 && slopeX1 > -1.0)
                FillBytes((yScan << settings.horizontalBits) + min(prevX1,rowX1), color, max(rowX1, prevX1) - min(rowX1,prevX1));
            else
                drawPixel(rowX1, yScan, color);            

            if(slopeX2 < 1.0 && slopeX2 > -1.0)
                FillBytes((yScan << settings.horizontalBits) + min(prevX2,rowX2), color, max(rowX2, prevX2) - min(rowX2,prevX2));
            else
                drawPixel(rowX2, yScan, color);
        }
    }
    
}

void VRAM::_drawTriangleTop1EqualBottoms(int topX, int topY, int x1, int x2, int bottomY, byte color, bool fill)
{
    //char buf[128];
    float rowX1 = topX, prevX1 = topX; //line from x2 to x1
    float rowX2 = topX, prevX2 = topX; // line from x3 to x1
    bool slope1Vertical = (x1 - topX) == 0;
    bool slope2Vertical = (x2 - topX) == 0;
    float slopeX1 = ((float)(bottomY - topY) / (float)(x1 - topX)); //rise over run
    float slopeX2 = ((float)(bottomY - topY) / (float)(x2 - topX)); //rise over run    
    int yInterceptX1 = -1*((slopeX1 * (float)topX) - topY);    // slopeX1 =  (y1 - yInt) / x1, -1 * (slopeX1 * x1 - y1) = yInt
    int yInterceptX2 = -1*((slopeX2 * (float)topX) - topY);    // 
    // sprintf(buf, "P1-P3 = %03.3fx + %d  P2-P3 = %03.3fx + %0d", slopeX1, yInterceptX1, slopeX2, yInterceptX2);
    // drawText(10,30,buf,Color::BROWN);
    
    for(uint16_t yScan = topY; yScan <= bottomY;yScan++){
        prevX1 = rowX1;
        prevX2 = rowX2;
        rowX1 = slope1Vertical ? rowX1 : ((float)(yScan - yInterceptX1 )) / slopeX1; 
        rowX2 = slope2Vertical ? rowX2 : ((float)(yScan - yInterceptX2 )) / slopeX2; 
        if(fill)
            FillBytes((yScan << settings.horizontalBits) + min(rowX1,rowX2), color, max(rowX1,rowX2) - min(rowX1,rowX2));
         
        else {
            if(slopeX1 < 1.0 && slopeX1 > -1.0)
                FillBytes((yScan << settings.horizontalBits) + min(prevX1,rowX1), color, max(rowX1, prevX1) - min(rowX1,prevX1));
            else
                drawPixel(rowX1, yScan, color);            

            if(slopeX2 < 1.0 && slopeX2 > -1.0)
                FillBytes((yScan << settings.horizontalBits) + min(prevX2,rowX2), color, max(rowX2, prevX2) - min(rowX2,prevX2));
            else
                drawPixel(rowX2, yScan, color);
        }
    }
    FillBytes((bottomY << settings.horizontalBits) + min(x1, x2),color,max(x1,x2) - min(x1,x2));    

}
void VRAM::_drawTriangleTop1DifferentBottoms(int topX, int topY, int middleX, int middleY, int bottomX, int bottomY, byte color, bool fill){
    float rowX1 = topX, prevX1 = topX; //line from x2 to x1
    float rowX2 = topX, prevX2 = topX; // line from x3 to x1
    bool slope1Vertical = (middleX - topX) == 0;
    bool slope2Vertical = (bottomX - topX) == 0;
    bool slope3Vertical = (bottomX - middleX) == 0;
    float slopeX1 = ((float)(middleY - topY) / (float)(middleX - topX)); //rise over run
    float slopeX2 = ((float)(bottomY - topY) / (float)(bottomX - topX)); //rise over run    
    float slopeX3 = ((float)(bottomY - middleY)/ (float)(bottomX - middleX));
    int yInterceptX1 = -1*((slopeX1 * (float)topX) - topY);    // slopeX1 =  (y1 - yInt) / x1, -1 * (slopeX1 * x1 - y1) = yInt
    int yInterceptX2 = -1*((slopeX2 * (float)topX) - topY);    // 
    int yInterceptX3 = -1*((slopeX3 * (float)middleX) - middleY);    // 

    //diverge towards middle point
    for(uint16_t yScan = topY; yScan < middleY;yScan++){
        prevX1 = rowX1;
        prevX2 = rowX2;
        rowX1 = slope1Vertical ? rowX1 : ((float)(yScan - yInterceptX1 )) / slopeX1; 
        rowX2 = slope2Vertical ? rowX2 : ((float)(yScan - yInterceptX2 )) / slopeX2; 

        if(fill)
            FillBytes((yScan << settings.horizontalBits) + min(rowX1,rowX2), color, max(rowX1,rowX2) - min(rowX1,rowX2));
        else {
            if(slopeX1 < 1.0 || slopeX1 > 1.0 || slope1Vertical){
                drawPixel(rowX1, yScan, color);
            }
            else
                FillBytes((yScan << settings.horizontalBits) + min(prevX1,rowX1), color, max(rowX1, prevX1) - min(rowX1,prevX1));
                    

            drawPixel(rowX2, yScan, color);

            if(slopeX2 < 1.0 || slopeX2 > 1.0 || slope2Vertical)
                drawPixel(rowX2, yScan, color);
            else
                FillBytes((yScan << settings.horizontalBits) + min(prevX2,rowX2), color, max(rowX2, prevX2) - min(rowX2,prevX2));
        }

        
        
            
    }
    //converge towards bottom point
    // use slope x3 instead of slipe x1
    for(uint16_t yScan = middleY; yScan < bottomY;yScan++){
        prevX1 = rowX1;
        prevX2 = rowX2;
        rowX1 = slope3Vertical ? rowX1 : ((float)(yScan - yInterceptX3 )) / slopeX3; 
        rowX2 = slope2Vertical ? rowX2 : ((float)(yScan - yInterceptX2 )) / slopeX2; 

        if(fill)
            FillBytes((yScan << settings.horizontalBits) + min(rowX1,rowX2), color, max(rowX1,rowX2) - min(rowX1,rowX2));
        else {
            if(slopeX3 < 1.0 || slopeX3 > 1.0 || slope3Vertical)
                drawPixel(rowX1, yScan, color);            
            
            else
                FillBytes((yScan << settings.horizontalBits) + min(prevX1,rowX1), color, max(rowX1, prevX1) - min(rowX1,prevX1));
        
            if(slopeX2 < 1.0 || slopeX2 > 1.0 || slope2Vertical)
                drawPixel(rowX2, yScan, color);
            else
                FillBytes((yScan << settings.horizontalBits) + min(prevX2,rowX2), color, max(rowX2, prevX2) - min(rowX2,prevX2));
        }
    }
    drawPixel(bottomX, bottomY, color);


}


