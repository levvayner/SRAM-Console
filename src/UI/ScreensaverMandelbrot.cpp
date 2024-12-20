#include "ScreensaverMandelbrot.h"

void ScreenSaverMandelbrot::start()
{
    graphics.clear();
    _running = true;
    /* The window in the plane. */
       
    xmin = 0.20085;
    xmax = 0.37100;
    ymin = 0.014640;
    ymax = 0.014810;
    yres = 320;//(xres*(ymax-ymin))/(xmax-xmin);
}
// <xmin> <xmax> <ymin> <ymax> <maxiter> <xres>
void ScreenSaverMandelbrot::step()
{
    if(!_running) return;
    if(millis() - _lastStepTime < stepDuration) return;

    double dx=(xmax-xmin)/xres;
  double dy=(ymax-ymin)/yres;

  double x, y; /* Coordinates of the current point in the complex plane. */
  double u, v; /* Coordinates of the iterated point. */
  int i,j; /* Pixel counters */
  int k; /* Iteration counter */
  for (j = 0; j < yres; j++) {
    y = ymax - j * dy;
    for(i = 0; i < xres; i++) {
      double u = 0.0;
      double v= 0.0;
      double u2 = u * u;
      double v2 = v*v;
      x = xmin + i * dx;
      /* iterate the point */
      for (k = 1; k < maxiter && (u2 + v2 < 4.0); k++) {
            v = 2 * u * v + y;
            u = u2 - v2 + x;
            u2 = u * u;
            v2 = v * v;
      };
      /* compute  pixel color and write it to file */
      if (k >= maxiter) {
        /* interior */
        graphics.drawPixel(i,j,0);
      }
      else {
        /* exterior */
        graphics.drawPixel(i,j,k);
        
      };
    }
  }

    // auto f = 300/240;
    // auto s = 1/f;
    // for (int x = 0; x < 420; x+=s) {
    //     for (int y = 0; y < 230; y+=s) { 
    //         auto color =  getColor(x,y);
    //         Serial.print("Drawing pixel at ("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.print(")");
    //         Serial.print(" using color "); Serial.println(color);  
    //         graphics.drawPixel(x,y,color); //depending on the number of iterations, color a pixel.    
    //         //plot(,s*f,(x-A+h)*f,(y-B+h)*f); 
    //     }
    // }  
    _lastStepTime = millis();
}

void ScreenSaverMandelbrot::stop()
{
    _running = false;
}
