#include "ScreensaverMandelbrot.h"

void ScreenSaverMandelbrot::start()
{
    graphics.clear();
    _running = true;
}

void ScreenSaverMandelbrot::step()
{
    if(!_running) return;
    if(millis() - _lastStepTime < stepDuration) return;

    auto f = 300/240;
    auto s = 1/f;
    for (int x = 120; x < 420; x+=s) {
        for (int y = 20; y < 230; y+=s) {   
            graphics.drawPixel(x,y, getColor(x,y)); //depending on the number of iterations, color a pixel.    
            //plot(,s*f,(x-A+h)*f,(y-B+h)*f); 
        }
    }  
    _lastStepTime = millis();
}

void ScreenSaverMandelbrot::stop()
{
    _running = false;
}
