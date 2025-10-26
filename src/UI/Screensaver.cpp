#include "Screensaver.h"



void ScreenSaver::start()
{
    _xTiles = (graphics.settings.screenWidth / tileWidth);
    _yTiles = (graphics.settings.screenHeight / tileHeight);
    uint32_t gridSize = _xTiles * _yTiles;

    // if (blockObject != nullptr) {
    //     gpu.Set2DObjects(nullptr); // detach GPU from old list
    //     delete blockObject;
    //     blockObject = nullptr;
    // }
    blockObject = new Graphics2D();
    
    _frameBuffer = new uint8_t[gridSize];
    _currentPosition =  ((_yTiles / 2) * _xTiles  - (_xTiles / 2)); //center?
    Serial.print("Set up screen tiles "); Serial.print(_xTiles); Serial.print("x"); Serial.println(_yTiles);
    Serial.print("Using bank #"); Serial.println(gpu.activeBank() ? 1 : 0);
    //uint8_t gridColor = rand()%255;
    //graphics.fillRectangle(0,0,graphics.settings.screenWidth, graphics.settings.screenHeight, gridColor);
    //blockTexture = new Texture2D(tileWidth, tileHeight);
    blockTexture->Fill(Color::DARK_GREEN);
    
    for(int idx=0;idx < _xTiles;idx++){
        for(int line = 0; line < _yTiles; line++){
            blockObject->shapeList->push_back(GraphicsObject2D(Rectangle2D(
                Point2D(idx * tileWidth, line * tileHeight),
                Point2D(idx * tileWidth + tileWidth - 1, line * tileHeight + tileHeight - 1),
                FillStyle::Fill
            ), *blockTexture));
            //gpu.Add2DObject(blockObjects[idx + line * _xTiles]);
            gpu.saveRamStates();
            gpu.PrintRAMstates();        
            //graphics.drawRectangle(idx * tileWidth, line * tileHeight, tileWidth, tileHeight, gridColor);            
        }
    }
    Serial.print("Created "); Serial.print(blockObject->shapeList->size()); Serial.println(" block objects for screen grid");
    Serial.print("Size of block object shape list: "); Serial.println(sizeof(blockObject->shapeList) * sizeof(blockObject->shapeList[0]));
    gpu.Set2DObjects(&blockObject->shapeList);
    Serial.print("Graphics2D shape list: "); Serial.println(sizeof(gpu.Get2DObjects()));
    Serial.print("Added "); Serial.print(gpu.Get2DObjects()->size()); Serial.println(" 2D objects to GPU");
    gpu.Render();
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
    //while(graphics.isWaiting());
    if(!_running) return;
    bool forceTurn = false;
    bool collissionOccured = false;
    if(millis() - _lastStepTime < stepDuration) return;
    if(_collisionConter >= MAX_COLLISSIONS) {
        stop();
        //gpu.Clear2DObjects();
        start();        
        return;
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
    auto cell = gpu.Get2DObjectAt(x() * tileWidth, y() * tileHeight);
    if(cell != nullptr){
        //Serial.print("Drawing at tile "); Serial.print(x() * tileWidth); Serial.print(", "); Serial.println(y() * tileHeight);
        cell->texture->Fill(_color);
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

void ScreenSaver::stop()
{
    delete _frameBuffer;
    _running = false;
    gpu.ClearScreen();    
    gpu.saveRamStates();
    gpu.PrintRAMstates();
}
