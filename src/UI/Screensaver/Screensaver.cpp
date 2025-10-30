#include "Screensaver.h"
#include "../UI.h"
#include <stdlib.h>
#include <utility> 
extern UI ui;

void ScreenSaver::start()
{
    gpu.ClearScreen();
    srand(millis());
    if(_useRandomSize){
        tileWidth = rand() % SIZE_RANGE + MIN_BLOCK_SIZE; //30-50
        tileHeight = rand() % SIZE_RANGE + MIN_BLOCK_SIZE; //30-50
    }
    _xTiles = (graphics.settings.screenWidth / tileWidth);
    _yTiles = (graphics.settings.screenHeight / tileHeight);
    uint32_t gridSize = _xTiles * _yTiles;

    if (!ui.blockObject) ui.blockObject = new Graphics2D();
    else ui.blockObject->shapeList->clear();
    
    _frameBuffer = new uint8_t[gridSize];
    _currentPosition =  ((_yTiles / 2) * _xTiles  - (_xTiles / 2)); //center?
    // Serial.print(F("Set up screen tiles ")); Serial.print(_xTiles); Serial.print("x"); Serial.println(_yTiles);
    // Serial.print("Using bank #"); Serial.println(gpu.activeBank() ? 1 : 0);

    //draw loading screen and switch bank
    gpu.SetRenderMode(rmText);
    //center with 6 char offset to left
    console.SetPosition(graphics.settings.screenWidth / 2 - (graphics.settings.charWidth * 6), graphics.settings.screenHeight / 2);
    console.write("Loading ...");
    gpu.Render();        
    gpu.SetRenderMode(rmBuffered);

    Rectangle2D* rect = nullptr;
   // Texture2D* blockTexture = nullptr;

    for(int line = 0; line < _yTiles; line++){
        for(int idx=0;idx < _xTiles;idx++){
            // allocate shape on heap so ownership can transfer into the GraphicsObject2D
            rect = new Rectangle2D(
                idx * tileWidth,
                line * tileHeight,
                idx * tileWidth + tileWidth - 1,
                line * tileHeight + tileHeight - 1,
                FillStyle::Fill
            );
            //blockTexture = new Texture2D(1,1, Color::BRICK);

            // create a local GraphicsObject2D (will take ownership of rect pointer)
            GraphicsObject2D localObj(rect, graphics.settings.backgroundColor);
            

            //Serial.print("["); Serial.print(_xTiles * line + idx); Serial.print("] ");
            // move the local into the list so the move ctor runs
            ui.blockObject->shapeList->push_back(std::move(localObj));
            //gpu.PrintRam(Serial);    
        }
    }
    //gpu.PrintRam(Serial);      
    gpu.Set2DObjects(ui.blockObject->shapeList);
    //Serial.print("Added "); Serial.print(gpu.Get2DObjects()->size()); Serial.println(" 2D objects to GPU");
    gpu.Render();
    while(graphics.isWaiting());
    graphics.clear();
    
    _currentDirection = (uint8_t)((rand()%4));
    _color = (uint8_t)((rand()%256));
    _collisionConter = 0;
    _running = true;
    memset(_frameBuffer,0, gridSize);
    //Serial.print("Setting current direction to "); Serial.println(_currentDirection);
}

void ScreenSaver::step()
{
    srand(millis());
    //while(graphics.isWaiting());
    if(!_running) return;
    bool forceTurn = false;
    bool collissionOccured = false;
    if(millis() - _lastStepTime < stepDuration) return;
    if(_collisionConter >= MAX_COLLISSIONS) {        
        return restart();
    }
    // Serial.print("Stepping screen saver. Direction: ");
    // Serial.println(_currentDirection == 0 ? "up" : _currentDirection == 1 ? "right" : _currentDirection == 2 ? "down" : _currentDirection == 3 ? "right" :  "UNKNOWN");
    // if(_currentDirection < 0 || _currentDirection > 3)
    //     Serial.println(_currentDirection);
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
    auto *cell = gpu.Get2DObjectAt(x() * tileWidth, y() * tileHeight);
    if(cell != nullptr){
        //Serial.print("Drawing at tile "); Serial.print(x() * tileWidth); Serial.print(", "); Serial.println(y() * tileHeight);
        if(cell->texture == nullptr){
            cell->color = _color;
        }
        else{
            cell->texture->Fill(_color);
        }
        cell->drawnOnMem1 = false;
        cell->drawnOnMem2 = false;
    }else{
        Serial.print("No cell found at "); Serial.print(x() * tileWidth); Serial.print(", "); Serial.println(y() * tileHeight);
        
    }
    //Serial.print("Current position: "); Serial.print(_currentPosition); Serial.print(" ("); Serial.print(x()); Serial.print(", "); Serial.print(y()); Serial.println(")");
    // graphics.fillRectangle( x() * tileWidth, y() * tileHeight, tileWidth, tileHeight, _color, btVertical );
    // graphics.setReady();
    // while(graphics.isWaiting());
    // graphics.fillRectangle( x() * tileWidth, y() * tileHeight, tileWidth, tileHeight, _color, btVertical );
    _frameBuffer[_currentPosition] = _color;

    if(collissionOccured == true){
        _collisionConter++;
    }
    else {
        _collisionConter = 0;
    }
    if((forceTurn || (collissionOccured && _collisionConter == 1) ) ){ //change color after collision
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
    // graphics.setReady();
    // while(graphics.isWaiting());
    #endif
    //Serial.println("Rendering frame");
    gpu.Render();
   
    _lastStepTime = millis();
}
void ScreenSaver::restart(){
    delete[] _frameBuffer;
    _frameBuffer = nullptr;
    _running = false;
    gpu.ClearObjects();
    start();
}

void ScreenSaver::stop()
{
    if (_frameBuffer) { delete[] _frameBuffer; _frameBuffer = nullptr; }
    _running = false;

    // UI container (usually empty list at this point)
    if (ui.blockObject) {
        delete ui.blockObject;      // Graphics2D::~Graphics2D clears & deletes its shapeList
        ui.blockObject = nullptr;
    }

    // Let GPU drop the currently displayed objects it moved-into earlier.
    gpu.ClearObjects();

    gpu.Render();
    waitUntilIdle();
    gpu.ClearScreen();
}
