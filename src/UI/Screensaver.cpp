#include "Screensaver.h"
void ScreenSaver::start()
{
    
    graphics.clear();
    _xTiles = graphics.settings.screenWidth / tileWidth;
    _yTiles = graphics.settings.screenHeight / tileHeight;
    uint32_t gridSize = _xTiles * _yTiles;
    _frameBuffer = new uint8_t[gridSize];
    _currentPosition =  ((_yTiles / 2) * _xTiles  - (_xTiles / 2)); //center?
    Serial.print("Set up screen tiles "); Serial.print(_xTiles); Serial.print("x"); Serial.println(_yTiles);
    Serial.print("Using bank #"); Serial.println(digitalRead(PIN_BANK_SELECT));
    uint8_t gridColor = rand()%255;
    for(int idx=0;idx < _xTiles;idx++){
        for(int line = 0; line < _yTiles; line++){
            graphics.drawRectangle(idx * tileWidth, line * tileHeight, tileWidth, tileHeight, gridColor);            
        }
    }
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    while(graphics.isWaiting());
    graphics.clear();
    for(int idx=0;idx < _xTiles;idx++){
        for(int line = 0; line < _yTiles; line++){
            graphics.drawRectangle(idx * tileWidth, line * tileHeight, tileWidth, tileHeight, gridColor);            
        }
    }
    graphics.setReady();
    while(graphics.isWaiting());
    #endif
    srand(millis());
    _currentDirection = (uint8_t)((rand()%4));
    _color = (uint8_t)((rand()%256));
    _collisionConter = 0;
    _running = true;
    memset(_frameBuffer,0, gridSize);
    Serial.print("Setting current direction to "); Serial.println(_currentDirection);
    
}

void ScreenSaver::step()
{
    srand(millis());
    if(!_running) return;
    bool forceTurn = false;
    bool collissionOccured = false;
    if(millis() - _lastStepTime < stepDuration) return;
    if(_collisionConter >= MAX_COLLISSIONS) return start();
    Serial.print("Stepping screen saver. Direction: ");
    Serial.println(_currentDirection == 0 ? "up" : _currentDirection == 1 ? "right" : _currentDirection == 2 ? "down" : _currentDirection == 3 ? "right" :  "UNKNOWN");
    if(_currentDirection < 0 || _currentDirection > 3)
        Serial.println(_currentDirection);
    // char buf[256];
    //determine the next spot based on location and direction
    switch (_currentDirection)
    {
    case 0:
        /* up */
        if(y() == 0){
            //turn
            forceTurn = true;
        }
        else {
            if(_frameBuffer[_currentPosition - _xTiles] != 0){
                //collision
               
                collissionOccured = true;
            }
            _currentPosition -= _xTiles;            
        }
        break;
    case 1:
        /* right */
        if(x() >= _xTiles - 1){
            //turn
            //forceTurn = true;
        }
        else {
            if(_frameBuffer[_currentPosition + 1] != 0){
                //collision
                collissionOccured = true;
            }

            _currentPosition +=1;
        }
        
        break;
    case 2:
        /* down */
        if(y() >= _yTiles - 1){
            //turn
            forceTurn = true;
        }
        else {
            if(_frameBuffer[_currentPosition + _xTiles] != 0){
                //collision
                collissionOccured = true;
                //forceTurn = true;
                //return start();
            }
            _currentPosition +=  _xTiles;
        }
        break;
    case 3:
        /* left */
        if(x() <= 0){
            //turn
            forceTurn = true;
        }
        else {
            if(_frameBuffer[_currentPosition - 1] != 0){
                //collision
                collissionOccured = true;
            }
            _currentPosition -= 1;
        }
        break;
    
    default:
        _collisionConter++;
        return;
        break;
    }
    
    graphics.fillRectangle( x() * tileWidth, y() * tileHeight, tileWidth, tileHeight, _color, btVertical );
    graphics.setReady();
    while(graphics.isWaiting());
    graphics.fillRectangle( x() * tileWidth, y() * tileHeight, tileWidth, tileHeight, _color, btVertical );
    _frameBuffer[_currentPosition] = _color;

    if(collissionOccured == true){
        _collisionConter++;
    }
    if(forceTurn || collissionOccured){ //change color after collision
         _color = (uint8_t)((rand()) + millis())%255;
    }
    //determine next direction
     if( forceTurn || rand() % 8 < 1){
        //time to turn

        uint8_t nextDirection  = (uint8_t)(rand()+ micros())%2;
        if(_currentDirection % 2 == 0)
            _currentDirection = nextDirection * 2 + 1;
        else
            _currentDirection = nextDirection * 2;
        // if(nextDirection + _currentDirection % 2 != 0){
        //     nextDirection+=3;
        // }
        //it has to turn
        //_currentDirection = ((rand() % 4) & 0x3);
       
     }
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    while(graphics.isWaiting());
    #endif
   
    _lastStepTime = millis();
}

void ScreenSaver::stop()
{
    _running = false;
    graphics.clear();
}
