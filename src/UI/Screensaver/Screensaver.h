#ifndef SCREEN_SAVER_H
#define SCREEN_SAVER_H
#include "sw/GPU.h"

extern GPU gpu;
//extern VRAM graphics;
#define MAX_COLLISSIONS 10
#define MIN_BLOCK_SIZE 30
#define SIZE_RANGE 20

/// @brief Screen saver draws pipes.
/// Pipes are 4 x 4 pixels, so screen is divided in to 432/4 x 240/4 108 x 60

class ScreenSaver{
public:
    void start();
    void step();
    void stop();
    void restart();

protected:
    inline uint32_t x(){
        return _currentPosition%_xTiles;
    }
    inline uint32_t y(){
        return _currentPosition / _xTiles;
    }


private:    
    uint8_t * _frameBuffer ;
    
    uint32_t _currentPosition = 0;
    uint8_t _currentDirection = 0; // 0 -up, 1 - right, 2 - down, 3 - left
    uint8_t _color;
    uint8_t tileWidth = 25;
    uint8_t tileHeight = 25;
    uint16_t _xTiles, _yTiles;

    
    //Graphics2D* blockObject = nullptr;

    unsigned long _lastStepTime = 0;
    unsigned long stepDuration = 50;
    uint16_t _collisionConter = 0;
    bool _running = false;
};
#endif