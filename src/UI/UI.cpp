
#include "UI.h"
#define UI_SOURCE "UI"
UI ui;

char cmdBuf[256];
uint16_t cmdBufIdx = 0;
bool commandReady = false;
void uiProcessKey(uint8_t data){
    //Serial.print("Received key 0x"); Serial.println(data, HEX);
    if(data == 13) return; //ignore carriage return, new line advances to beginning of line
    if(data == 10){
        commandReady = true;
    }
    
    cmdBuf[cmdBufIdx++] = data;    
}

void uiProcessClick(MouseClickArgs args){
    Serial.print("Clicked at"); Serial.print(args.location.x); Serial.print(","); Serial.print(args.location.y);
    Serial.print(" button "); 
    Serial.print( args.button == 1 << 0 ? "1" : args.button == 1 << 1 ? "2" : args.button == 1 << 2 ? "3" : "unknown");
    Serial.println();
}

void readMemory(commandRequest request){
    
    //String addrS = _getResponse(port);
    String addrS = request.args;
    uint32_t addr = addrS.toInt();

    byte data = programmer.ReadByte(addr);
    Serial.print("Read: "); Serial.print(data); Serial.print(" from address 0x"); Serial.print(addr, HEX);
    Serial.println();
}

void writeMemory(commandRequest request){
    Serial.print("Enter address to write");
    while (!Serial.available());
    String addrS = Serial.readString();
    //String addrS =  _getResponse(port);
    uint32_t addr = addrS.toInt();
    Serial.print(": "); Serial.println(addr,HEX);

    Serial.print("Enter data to store in decimal form");
    
    while (!Serial.available());
    String dataS = Serial.readString();
    //String dataS =  _getResponse(port);
    byte data = dataS.toInt();
    Serial.print(": "); Serial.println(data, DEC);
    programmer.WriteByte(addr, data);
}
#ifdef DOUBLE_BUFFER
void swapBanks(commandRequest request){
    unsigned long startTime = millis();
    Serial.print("Swapping banks ..");
    graphics.setReady();
    while(graphics.isWaiting() && millis() - startTime < 400) delay(1);
    if(graphics.isWaiting()){
        Serial.println("Failed to swap banks. Timeout");
        Serial.print("Forcing...");
        graphics.setReady(true);
        delay(1);
        graphics.setReady();
        startTime = millis();
        while(graphics.isWaiting() && millis() - startTime < 400) delay(1);
        if(graphics.isWaiting()){
            Serial.println(" Failed to swap banks. Timeout");
            return;
        } 
        Serial.println(" Done");
    }
    Serial.print("Selected bank: "); Serial.println(digitalRead(PIN_BANK_SELECT) ? "2" : "1");
    Serial.print(". Done in "); Serial.print((millis() - startTime));Serial.println(" ms.");
}
#endif

void printMemory(commandRequest request){
    ui.DumpRAM();
    #ifdef DOUBLE_BUFFER
    //graphics.setReady();
    #endif
}
void clearScreen(commandRequest request){
    gpu.ClearScreen();
    //ui.ClearScreen();
    // #ifdef DOUBLE_BUFFER
    // graphics.setReady();
    // #endif

    Serial.println();
}

void serverDownload(commandRequest request){
    programRom.StoreProgramData();
    Serial.println("Done!");
}

void drawLines(commandRequest request){
    byte color = 0;
    unsigned long startTime = millis();
    graphics.clear();
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    // while(graphics.isWaiting()){
    //     delay(10);
    // };
    // for(uint16_t line = 0; line < graphics.settings.screenHeight;line++){
    //     color = (line & 0x03) | (line >> 3 & 0x03) << 3 | (line%12 << 2);        
    //     programmer.FillBytes((line << graphics.settings.horizontalBits) + 0, color, graphics.settings.screenWidth - 200);            
        
    // }
    #endif
    Serial.print(F("Draw lines : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");

    //needPrintMenu = true;
}
void drawDiagonalLines(commandRequest request){
    //row of colors in array, for each line, start farther down the list by one. wrap back to beggining of the list when done
    uint8_t bufSize = 256;
    uint8_t colors[bufSize];
    for(int idx = 0; idx < 256; idx++){
        colors[idx] = idx;
    }
    
    unsigned long startTime = millis();
    for(uint16_t line = 0; line < graphics.settings.screenHeight;line++){
        for(uint16_t x = 0; x < graphics.settings.screenWidth; x++){
            graphics.drawPixel(line,x, colors[line - x % bufSize]);
        }
        // programmer.WriteBytes(line << graphics.settings.horizontalBits, colors + line, graphics.settings.screenWidth - line); //write from 0 to end of colors            
        // programmer.WriteBytes((line << graphics.settings.horizontalBits) + (graphics.settings.screenWidth - line - 1), colors, line );
       
        //Serial.print("Drawing line on Y = "); Serial.println(line);
        
    }
    
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    // for(uint16_t line = 0; line < graphics.settings.screenHeight;line++){
    //     programmer.WriteBytes(line << graphics.settings.horizontalBits, colors + line, graphics.settings.screenWidth - line); //write from 0 to end of colors            
    //     programmer.WriteBytes((line << graphics.settings.horizontalBits) + (graphics.settings.screenWidth - line - 1), colors, line );
       
    //     //Serial.print("Drawing line on Y = "); Serial.println(line);
        
    // }
    #endif
    Serial.print(F("Draw diagonal line : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
    //needPrintMenu = true;
}
void drawVerticalLines(commandRequest request){
    // byte colBytes[graphics.settings.screenWidth];
    // byte color = 1;
    unsigned long startTime = millis();
    

    for(uint16_t line = 0; line < graphics.settings.screenHeight;line++){    
        for(uint16_t x = 0; x < graphics.settings.screenWidth; x++)
        {
            graphics.drawPixel(x,line, line / (line*5) + 1);        
        }        
    }
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    #endif
    Serial.print(F("Draw vertical lines : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
    //needPrintMenu = true;
}

void showScreenSaver(commandRequest request){
    //screenSaver
    ScreenSaver saver;
    gpu.SetRenderMode(RenderMode::rmBuffered);
    gpu.GetTextBuffer()->Clear();
    console.HideCursor();
    //char c = '\0';
    saver.start();
    //keyboard.SetMode(false);
    while(true){
        saver.step();
        // #ifdef DOUBLE_BUFFER
        // graphics.setReady();
        // #endif
        char key = keyboard.getKey();
            
        if(key == 'q' || key == 'Q'){
            saver.stop();              
            break;
        }
        
    }
    //keyboard.SetMode(true); 
    ui.PrintMenu(true);     
    
}
void setGraphicsRenderMode(commandRequest request){
    
    //String addrS = _getResponse(port);
    if(strlen(request.args) <= 0){
        console.println("Usage: graphics [MODE]    where MODE is 1 - 4 for safe to volatile rendering");
        #ifdef DOUBLE_BUFFER
        graphics.setReady();
        #endif
        return;
    }
    int mode = atoi(request.args);
    if(mode > 0 && mode <= 4){        
        graphics.SetRenderMode((BusyType)mode);
    }
    else {
        Serial.print("Unkown graphics mode: "); Serial.println(mode);
    }    
}

void drawBlocks(commandRequest request){
    int blockWidth = floor(graphics.settings.screenWidth / 16); //rather push off screen a bit
    int blockHeight = ceil((graphics.settings.screenHeight - 12) / 16);

    Serial.print("Setting up blocks with width "); Serial.print(blockWidth); Serial.print(" and height "); Serial.println(blockHeight);
    //graphics.clear(0,0,graphics.settings.screenWidth, graphics.settings.screenHeight);
    uint8_t color = 0xFF;
    //byte colors[blockWidth];
    char label[4];
    Rectangle2D* rect = nullptr;
    ui.blockObject = new Graphics2D();
    //auto textBuffer = gpu.GetTextBuffer();
    //if(textBuffer)
    gpu.SetRenderMode(rmText);
    gpu.GetTextBuffer()->Clear();
    //gpu.ClearScreen();
    
    unsigned long  startTime = millis();
    //byte block[blockWidth * blockHeight ];
    for(int x = 1; x < graphics.settings.screenWidth; x+= blockWidth){
        for(int y=1;y < blockHeight * 16; y+= blockHeight){ 
            rect = new Rectangle2D(
                x,
                y,
                x + blockWidth,
                y + blockHeight,
                FillStyle::Fill
            );
            
            // create a local GraphicsObject2D (will take ownership of rect pointer)
            GraphicsObject2D localObj(rect, color);
            ui.blockObject->shapeList->push_back(std::move(localObj));
            // gpu.saveRamStates();
            // gpu.PrintRAMstates();      
            
            //gpu.Add2DObject(localObj);
            
            memset(label,0,4);
            sprintf(label, "%i", color);
            console.SetPosition(x + graphics.settings.charWidth,y + graphics.settings.charHeight);
            console.write(label,strlen(label), color ^ 0xFF, color,true);
            //gpu.GetTextBuffer()->AddString(row,line,label,color ^ 0xFF);           
            color--;
        }               
    }
    #ifdef DOUBLE_BUFFER
    //graphics.setReady();

    // for(int x = 1; x < graphics.settings.screenWidth; x+= blockWidth){
    //     for(int y=1;y < blockHeight * 16; y+= blockHeight){ 
    //         memset(block, color, blockWidth * blockHeight);
    //         memset(label,0,4);
    //         sprintf(label, "%i", color);
    //         graphics.drawTextToBuffer(label, block, blockWidth, color ^ 0xFF);
    //         graphics.drawBuffer(x, y, blockWidth, blockHeight, block);            
    //         //graphics.fillRectangle(x,y, blockWidth, blockHeight,color);                
    //         //graphics.drawText(x + 2, y + 2, label,color ^ 0xFF, color, false);
    //         color--;
    //     }               
    // }
    #endif

    gpu.Set2DObjects(ui.blockObject->shapeList);

    //graphics.render();
    Serial.print(F("Blocks : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
    
    console.SetPosition(3, graphics.settings.screenHeight -graphics.settings.charHeight);
    console.write("8 ", 2,Color::RED, true);
    console.write("b", 1,Color::GREEN, true);
    console.write("i", 1,Color::GOLD, true);
    console.write("t", 1,Color::BLUE, true);

    console.SetPosition(70, graphics.settings.screenHeight - 9);
    console.write("256 Available Colors", 20,Color::WHITE, true);
    gpu.Render();
    #ifdef DEBUG_GPU
    gpu.saveRamStates();
    gpu.PrintRAMstates();
    #endif
}

void waitUntilDone(unsigned long timeoutMs = 400) {
    unsigned long t0 = millis();
    while (graphics.isWaiting() && millis() - t0 < timeoutMs) {
        delay(1);
    }
    // optional: detect timeout
    if (graphics.isWaiting()) { Serial.println("WARN: GPU wait timeout"); }
}

void _renderTestObjects(unsigned long * time){
    *(time) = millis() - *time;
    Serial.print(". Rendering "); Serial.print(*time); Serial.print(" ms");
    unsigned long drawTime = millis();

    gpu.Set2DObjects(ui.blockObject->shapeList);  // MOVE into GPU
    gpu.Render();
    waitUntilDone();                               // bounded wait
    drawTime = millis() - drawTime;
    Serial.print(". Drawing "); Serial.print(drawTime); Serial.println(" ms");
    gpu.ClearScreen();
    *(time) += drawTime;
}

void graphicsTest(commandRequest request){
    const int numOfObjects = 100;
    char buf[128];

    if (!ui.blockObject) {
        ui.blockObject = new Graphics2D();
    }
    if (!ui.blockObject->shapeList) {                 // if your Graphics2D uses a raw pointer
        ui.blockObject->shapeList = new ShapeList<GraphicsObject2D>();
    }

    // start clean
    gpu.ClearScreen();               // clears VRAM + GPU objects safely
    ui.blockObject->shapeList->clear(); // just in case

    // 1) LINES (GPU object path)
    unsigned long dlStartTime = millis();
    Serial.print("Testing drawing lines .. ");

    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Line2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10)
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
    }
    _renderTestObjects(&dlStartTime);

    // 2) TRIANGLES (GPU object path)
    unsigned long dtStartTime = millis();
    Serial.print("Testing drawing triangles .. ");

    ui.blockObject->shapeList->clear();
    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Triangle2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10)
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
    }
    _renderTestObjects(&dtStartTime);

    // 3) RECTANGLES (GPU object path)  **you were missing Set2DObjects here**
    unsigned long drStartTime = millis();
    Serial.print("Testing drawing rectangles .. ");

    ui.blockObject->shapeList->clear();
    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Rectangle2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenHeight - 10)
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
    }
   _renderTestObjects(&drStartTime);

    // 4) CIRCLES (direct VRAM path)
    Serial.print("Testing drawing circles .. ");
    unsigned long dcStartTime = millis();

    ui.blockObject->shapeList->clear();

    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Circle2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(1, 140)
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
    }
    
    _renderTestObjects(&dcStartTime);

    // 5) OVALS (direct VRAM path)
    Serial.print("Testing drawing ovals .. ");
    unsigned long doStartTime = millis();
    ui.blockObject->shapeList->clear();

    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Oval2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(1, 140),
                random(1, 140)
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
        
    }
    _renderTestObjects(&doStartTime);

    // 6) Fill triangles (direct VRAM)
    Serial.print("Testing filling triangles .. ");
    unsigned long ftStartTime = millis();
    ui.blockObject->shapeList->clear();

    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Triangle2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                Fill
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
       
    }
   _renderTestObjects(&ftStartTime);


    // 7) Fill rectangles (direct VRAM)
    Serial.print("Testing filling rectangles .. ");
    unsigned long frStartTime = millis();
    ui.blockObject->shapeList->clear();

    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Rectangle2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                Fill
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
       
    }
    
    _renderTestObjects(&frStartTime);

    // 8) Fill circles (direct VRAM)
    Serial.print("Testing filling circles .. ");
    unsigned long fcStartTime = millis();
    graphics.clear();

    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Circle2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, 70),
                Fill
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));
        
    }
    _renderTestObjects(&fcStartTime);

    // 9) Fill ovals (direct VRAM)
    Serial.print("Testing filling ovals .. ");
    unsigned long foStartTime = millis();
    graphics.clear();

    for (int idx = 0; idx < numOfObjects; ++idx) {
        GraphicsObject2D obj(
            new Oval2D(
                random(5, graphics.settings.screenWidth  - 10),
                random(5, graphics.settings.screenHeight - 10),
                random(5, 70),
                random(5, 70),
                Fill
            ),
            random(0,255)
        );
        ui.blockObject->shapeList->push_back(std::move(obj));       
    }
    _renderTestObjects(&foStartTime);
   
    // summary
    gpu.ClearScreen();
    
    gpu.SetRenderMode(rmText);
    gpu.PrintRam(Serial);

    console.SetEchoMode(false);
    console.SetPosition(0,0);
    sprintf(buf,"Drawing %i lines:      % 5lu ms", numOfObjects, dlStartTime); console.println(buf);
    sprintf(buf,"Drawing %i triangles:  % 5lu ms", numOfObjects, dtStartTime); console.println(buf);
    sprintf(buf,"Drawing %i rectangles: % 5lu ms", numOfObjects, drStartTime); console.println(buf);
    sprintf(buf,"Drawing %i circles:    % 5lu ms", numOfObjects, dcStartTime); console.println(buf);
    sprintf(buf,"Drawing %i ovals:      % 5lu ms", numOfObjects, doStartTime); console.println(buf);
    sprintf(buf,"Filling %i triangles:  % 5lu ms", numOfObjects, ftStartTime); console.println(buf);
    sprintf(buf,"Filling %i rectangles: % 5lu ms", numOfObjects, frStartTime); console.println(buf);
    sprintf(buf,"Filling %i circles:    % 5lu ms", numOfObjects, fcStartTime); console.println(buf);
    sprintf(buf,"Filling %i ovals:      % 5lu ms", numOfObjects, foStartTime); console.println(buf);
    unsigned long totalTime = dlStartTime + dtStartTime + drStartTime + dcStartTime + doStartTime + ftStartTime + frStartTime + fcStartTime + foStartTime;
    sprintf(buf, "--------------------------------"); console.println(buf);
    sprintf(buf,"Total rendering time:   %05lu ms", totalTime); console.println(buf);

    if (totalTime > 4000)      sprintf(buf,"\n--------------------------------\n Congradulations\n\n    You are farming a potato!");
    else if (totalTime > 2000) sprintf(buf,"\n--------------------------------\n Congradulations\n\n    You are working with a video card!");
    else                      sprintf(buf,"\n--------------------------------\n Congradulations\n\n    You are blazing fast!");

    console.println(buf);
    console.SetEchoMode(true);
    gpu.Render();
}


void runConsole(commandRequest request){
    console.run();
    graphics.clear();
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    #endif
    ui.PrintMenu();
    
}

void runEditor(commandRequest request){
    editor.run();       
    console.clear(); 
    ui.PrintMenu();
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    #endif
}

void showHelp(commandRequest request){
    ui.PrintMenu(true);
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    #endif
}

void writeProgram(commandRequest request){
    ui.setProgrammingMode(true);
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    #endif
}

void startProgram(commandRequest request){
    startApp(FLASH1, Serial);
    #ifdef DOUBLE_BUFFER
    graphics.setReady();
    #endif
}

void reboot(commandRequest request){
    RSTC->RSTC_CR = 0xA5000005; // Reset processor and internal peripherals
}

UI::UI()
{
    
}


UI::~UI()
{
    delete blockObject;
    blockObject = nullptr;
}

void UI::begin()
{
    
    commands.registerCommand(UI_SOURCE,"read", "",readMemory, "read a byte from memory");
    commands.registerCommand(UI_SOURCE,"write", "",writeMemory, "write a byte to memory");
    //commands.registerCommand(UI_SOURCE,"erase", "",writeMemory, "e");
    commands.registerCommand(UI_SOURCE,"clear", "",clearScreen, "clear screen");
    commands.registerCommand(UI_SOURCE,"print", "", printMemory, "print a block of memory");
    commands.registerCommand(UI_SOURCE,"server", "",serverDownload, "connect to server to download data");
    commands.registerCommand(UI_SOURCE,"line", "",drawLines, "draw horizontal lines");
    commands.registerCommand(UI_SOURCE,"diag", "",drawDiagonalLines, "draw diagonal lines");
    commands.registerCommand(UI_SOURCE,"vert", "",drawVerticalLines, "draw vertical lines");
    commands.registerCommand(UI_SOURCE,"saver","", showScreenSaver, "show sceen saver");
    commands.registerCommand(UI_SOURCE,"blocks","", drawBlocks, "draw color blocks");
    commands.registerCommand(UI_SOURCE,"test","", graphicsTest, "execute graphics test");
    commands.registerCommand(UI_SOURCE,"console","", runConsole, "run console app");
    commands.registerCommand(UI_SOURCE,"edit","", runEditor, "run editor app");
    commands.registerCommand(UI_SOURCE,"graphics","", setGraphicsRenderMode, "set graphics render mode");
    commands.registerCommand(UI_SOURCE,"help","", showHelp, "show help menu");
    commands.registerCommand(UI_SOURCE,"reboot","", reboot, "reboot MCU");
    commands.registerCommand(UI_SOURCE,"program", "", writeProgram, "program device");
    commands.registerCommand(UI_SOURCE,"start", "", startProgram, "start a program");
    #ifdef DOUBLE_BUFFER
    commands.registerCommand(UI_SOURCE,"swap","", swapBanks, "swap memory banks");
    #endif
    keyboard.onKeyDown = uiProcessKey;
    mouse.onClick = uiProcessClick;
    console.begin();
    console.SetColor(graphics.settings.foregroundColor);
    console.SetBackgroundColor(graphics.settings.backgroundColor);
    
    console.SetBackgroundColor(graphics.settings.backgroundColor);
    console.SetColor(graphics.settings.foregroundColor);
    console.clear();
    gpu.SetRenderMode(RenderMode::rmText);

    
}

void UI::blinkLED() {
	//has been long enough since last toggle
	if (millis() - lastToggle > toggleDuration)
	{
		ledState = !ledState; //togle state
		digitalWrite(PIN_LED, ledState); //update LED
		lastToggle = millis(); //update time
	}
}

void UI::PrintMenu(bool force) {
	if (!needPrintMenu && !force) return; 
    Serial.println(F("VGA TOOL   -   v 0.2.0"));
	Serial.println(F("--------------------------------"));
    auto registeredCommands = commands.getCommands();
    for(int idx = 0; idx < commands.commandCount(); idx++){
        Serial.print(idx + 1);
        Serial.print(". ");
        Serial.print(registeredCommands[idx].name);
        if(strlen(registeredCommands[idx].desc) > 0)
        {
            Serial.print(" - ");
            Serial.print(registeredCommands[idx].desc);
        }
         Serial.println();
    }
	//Serial.println(F("Press i to enter interactive terminal"));
	Serial.println(F("--------------------------------"));
    Serial.print(F("Sreen resoltion: ")); Serial.print(graphics.settings.screenWidth);
        Serial.print(F("x"));Serial.println(graphics.settings.screenHeight);

    gpu.SetRenderMode(rmText);
    gpu.ClearScreen();
    console.SetPosition();
    console.println("VGA TOOL   -   v 0.2.0");
	console.println("---------------------------------");
    

    for(int idx = 0; idx < commands.commandCount(); idx++){
        console.print(idx + 1);
        console.print(". ");
        console.print(registeredCommands[idx].name);
        if(strlen(registeredCommands[idx].desc) > 0)
        {
            console.print(" - ");
            console.print(registeredCommands[idx].desc);
        }
        console.println();
    }
	console.println("--------------------------------");
    console.print("Screen resoltion: "); console.print(graphics.settings.screenWidth);
        console.print("x");console.println(graphics.settings.screenHeight);
    
    //#ifdef DOUBLE_BUFFER
    gpu.Render();
    //#endif
    gpu.PrintRam(Serial);  
    //gpu.PrintRam(console);

   needPrintMenu = false;
}

void UI::DumpRAM() {
    uint8_t frameSize = 16;
    Serial.print("Enter start address to read");
    delay(50);
    while (!Serial.available()) {
        blinkLED();
    }
    String addrSS = Serial.readString();
    unsigned long addrS = addrSS.toInt();
    Serial.print(": ");
    Serial.print(addrS);
    Serial.print(" / 0x");
    Serial.println(addrS,HEX);

    Serial.print("Enter end address to read");
    delay(50);
    while (!Serial.available()) {
        blinkLED();
    }
    String addrES = Serial.readString();
    unsigned long addrE = addrES.toInt();

    Serial.print(": ");
    Serial.print(addrE);
    Serial.print(" / 0x");
    Serial.println(addrE,HEX);

    if(addrS < 0 || addrS > SRAM_SIZE){
        Serial.print("Start address "); Serial.print(addrS); Serial.println(" is invalid. Setting to 0");
        addrS = 0;
    }
    if(addrE < 0 || addrE < addrS || addrE > SRAM_SIZE){
        Serial.print("End address "); Serial.print(addrE); Serial.print(" is invalid. Setting to "); Serial.println(SRAM_SIZE,DEC);
        addrE = SRAM_SIZE;
    }

	Serial.println("Dumping RAM");
    

    
    byte baseOffset = addrS%frameSize;
    //Serial.print("Base Offset: "); Serial.println(baseOffset);
	for (uint32_t address = addrS - baseOffset; address < addrE; address+=256) {
        //Serial.print("Starting frame at address "); Serial.println(address,HEX);
		for (uint32_t base = address; base <= address + 256 && base <= addrE; base += frameSize) {
			byte data;
            char buf[80];       
            sprintf(buf, "%p: ", (void*)base);     
            byte frameOffset = base == address ? baseOffset : 0;
            bool isLastFrame = base + 16 > addrE;            
            for (uint32_t offset = 0; offset <= 15 ; offset += 1) {
                if(frameOffset != 0 && (offset < frameOffset)){
                    sprintf(buf, "%s --",buf);
                }
                else if(isLastFrame && offset > addrE%frameSize){
                    sprintf(buf, "%s --",buf);
                }
                else{
                    data = programmer.ReadByte(base + offset);
                    sprintf(buf, "%s %02x",buf, data);
                }
			}
			Serial.println(buf);
		}
		Serial.println();
	}
    Serial.print("Done dumping ram");
}
//#define BUFFER_SIZE 512
void UI::ClearScreen()
{
    unsigned long startTime = millis();
	Serial.print(F("Clearning screen"));    
    gpu.ClearScreen();
    //graphics.clear();
    Serial.print(F(" : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
    // #ifdef DOUBLE_BUFFER
    // graphics.setReady();
    // #endif
	
}

void UI::ProcessInput() {
    if(commandReady){
        //Serial.print("Receieved command: "); Serial.println(cmdBuf);
        //check if registered command
        auto command = commands.buildCommand(cmdBuf);
        if(command.valid){
            command.onExecute(command);
        }

        memset(cmdBuf,0,sizeof(cmdBuf));
        cmdBufIdx = 0;
        commandReady = false;
    }
    if(_programmingMode){
        if(programRom.RunAutomatedProgramming()){
            Serial.print("Starting app!");
            startApp(FLASH1,Serial);
        }
    }
    // checkingTime = millis();
    // if(checkingTime  - lastUpdated >= updateFrequency){        
    //     keyboard.onTick();
    //     lastUpdated = checkingTime;
    // }
}


