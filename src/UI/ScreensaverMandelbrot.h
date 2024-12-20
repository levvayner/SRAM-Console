
#ifndef SCREEN_SAVER_MANDELBROT_H
#define SCREEN_SAVER_MANDELBROT_H
#include "SRAM/SRAM.h"
#include "SRAM/VRAM.h"
extern VRAM graphics;
class ScreenSaverMandelbrot{
    public:
    
    void start();
    void step();
    void stop();

protected:
    uint8_t inline getColor(int x, int y){
   
        int a=0; int b=0;
        for (int i = 0; i<250; ++i) {
            // Complex z = z^2 + c
            int t = a*a - b*b;
            b = 2*a*b;
            a = t;
            a = a + x;
            b = b + y;
            int m = a*a + b*b;
            if (m > 10)  return i;
        }
        return 250;
                
                
    }

    inline uint16_t x(){
        return _x;
    }
    inline uint16_t y(){
        return _y;
    }
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    const int xres = 432;
    int yres;
    const uint16_t maxiter = 5000;

    private:

    

    int _x = 0, _y = 0;
    unsigned long _lastStepTime = 0;
    unsigned long stepDuration = 50;
    
    bool _running = false;

};
#endif