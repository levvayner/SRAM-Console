
#include "UI.h"


UI::UI()
{
    console.SetColor(Color::GREEN);
    console.SetBackgroundColor(Color::BLACK);
}


UI::~UI()
{
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

void UI::PrintMenu() {
	if (!needPrintMenu) return; 
    Serial.println(F("VGA TOOL   -   v 0.0.1"));
	Serial.println(F("--------------------------------"));
	Serial.println(F("Press r to read"));
	Serial.println(F("Press w to write"));
	Serial.println(F("Press p to print data"));
	Serial.println(F("Press s to store data"));
	Serial.println(F("Press e to erase RAM"));
    Serial.println(F("Press b for color blocks"));
    Serial.println(F("Press v for vertical lines"));
    Serial.println(F("Press l for horizontal lines"));
    Serial.println(F("Press g for graphics test"));
	Serial.println(F("Press i to enter interactive terminal"));
	Serial.println(F("--------------------------------"));

    console.SetPosition();
    console.println("VGA TOOL   -   v 0.0.1");
	console.println("---------------------------------");
	console.println("SRAM: ");
    console.println(" r - read   w - write   p - print");
    console.println(" s - store  e - erase ");
    console.println("Graphics: ");
    console.println(" b- blocks  v/l - lines  g -test");
    console.println("Aps: i - editor");
	console.println("--------------------------------");

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
    
    for(uint16_t line = 0; line < graphics.settings.screenHeight + 1;line++){
        programmer.FillBytes(line << graphics.settings.horizontalBits, ERASE_BYTE,graphics.settings.screenWidth + 2);
    }
    Serial.print(F(" : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
	
}

void UI::ProcessInput() {
    _processInput(Serial);
    _processInput(keyboardPs2);
}


template <typename TPort>
String UI::_getResponse(TPort port){
    String resp;
    unsigned long lastCharTime = millis();
    while(port.available() && millis() - lastCharTime < 1000)
    {
        char c = port.read();
        lastCharTime = millis();
        //Serial.print("Read "); Serial.println(c);
        resp += c;
    }
    return resp;
}

template <typename TPort>
inline void UI::_processInput(TPort port)
{
    unsigned long startTime = millis();
    while(millis() - startTime < 200 && ! port.available());
    
    String resp = _getResponse(port);
    
    if(resp.length() <= 0) return;
    //Serial.print("Received response: "); Serial.println(resp);
    //auto resp = port.readString();
	if (resp[0]== 'r' || resp[0] == 'R') {
		//read
		Serial.println("Enter address to read");
		delay(50);
		while (!port.available()) {
			blinkLED();
		}
		//String addrS = _getResponse(port);
        String addrS = Serial.readString();
		uint32_t addr = addrS.toInt();

		byte data = programmer.ReadByte(addr);
		Serial.print("Read: "); Serial.print(data); Serial.print(" from address 0x"); Serial.print(addr, HEX);
		Serial.println();
		//needPrintMenu = true;
	}
	else if (resp[0] =='w' || resp[0] =='W') {
		//write

		Serial.print("Enter address to write");
		while (!port.available());
        String addrS = Serial.readString();
		//String addrS =  _getResponse(port);
		uint32_t addr = addrS.toInt();
        Serial.print(": "); Serial.println(addr,HEX);

		Serial.print("Enter data to store in decimal form");
		
		while (!port.available());
        String dataS = Serial.readString();
		//String dataS =  _getResponse(port);
		byte data = dataS.toInt();
        Serial.print(": "); Serial.println(data, DEC);
        programmer.WriteByte(addr, data);
#ifdef DEBUG

		if (sucess) {
			Serial.print("Sucess writing data: 0x"); Serial.print(data, HEX); Serial.print(" to address "); Serial.println(addr, BIN);
		}
		else {
			Serial.print("Error writing data: 0x"); Serial.print(data, HEX); Serial.print(" to address "); Serial.println(addr, BIN);
		}

#endif

	}
	else if (resp[0] =='p' || resp[0] =='P') {
		DumpRAM();

		Serial.println();
		//needPrintMenu = true;
	}
	else if (resp[0] =='e' || resp[0] =='E') {
		ClearScreen();

		Serial.println();
		//needPrintMenu = true;
	}
	else if (resp[0] =='s' || resp[0] =='S') {
		
        programRom.StoreProgramData();

		Serial.println("Done!");
		//needPrintMenu = true;
	} 
    else if (resp[0] == 'l' || resp[0] == 'L') {
        byte color = 0;
        unsigned long startTime = millis();
        for(uint16_t line = 0; line < graphics.settings.screenHeight;line++){
            color = (line & 0x03) | (line >> 3 & 0x03) << 3 | (line%12 << 2);
           
            programmer.FillBytes(line << graphics.settings.horizontalBits, color, graphics.settings.screenWidth);
            // for(byte x = 5; x < graphics.settings.screenWidth - 10; x++){
                
            //     programmer.WriteByte(line<< graphics.settings.horizontalBits | (x & 0xFF), color,1,false);
            //     //delay(1);
            //     //Serial.print(line << graphics.settings.horizontalBits | x,HEX); Serial.print(" ");                
            // }
            //Serial.println();
            //delay(10);
        }
        Serial.print(F("Draw lines : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
		//needPrintMenu = true;
	}
    else if (resp[0] == 'v' || resp[0] == 'V') {
		byte colBytes[graphics.settings.screenWidth];
        byte color = 1;
        unsigned long startTime = millis();
        for(uint16_t x = 0; x < graphics.settings.screenWidth; x++)
            colBytes[x] = color++;

        for(uint16_t line = 0; line < graphics.settings.screenHeight;line++){              
            programmer.WriteBytes((line << graphics.settings.horizontalBits), colBytes, graphics.settings.screenWidth);    
        }
        Serial.print(F("Draw vertical lines : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
		//needPrintMenu = true;
	}
    else if (resp[0] == 'd' || resp[0] == 'D') {
        //row of colors in array, for each line, start farther down the list by one. wrap back to beggining of the list when done
        byte colors[256];
        for(int idx = 0; idx < 256; idx++){
            colors[idx] = idx;
        }
		
        unsigned long startTime = millis();
        for(uint16_t line = 0; line < graphics.settings.screenHeight;line++){
            programmer.WriteBytes(line << graphics.settings.horizontalBits, colors + line, graphics.settings.screenWidth - line); //write from 0 to end of colors            
            programmer.WriteBytes((line << graphics.settings.horizontalBits) + (graphics.settings.screenWidth - line - 1), colors, line );
            
           //Serial.print("Drawing line on Y = "); Serial.println(line);
          
        }
        Serial.print(F("Draw diagonal line : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
		//needPrintMenu = true;
	}
    
    else if (resp[0] == 'b' || resp[0] == 'B') { //bloks
        int blockWidth = ceil(graphics.settings.screenWidth / 16); //rather push off screen a bit
        int blockHeight = ceil((graphics.settings.screenHeight - 14) / 16);
        //Serial.print("Setting up blocks with width "); Serial.print(blockWidth); Serial.print(" and height "); Serial.println(blockHeight);
        graphics.clear(0,260,graphics.settings.screenWidth, graphics.settings.screenHeight - 260);
        byte color = 0xFF;
        //byte colors[blockWidth];
        char label[4];
        
        unsigned long  startTime = millis();
        byte block[blockWidth * blockHeight ];
        for(int x = 1; x < graphics.settings.screenWidth; x+= blockWidth){
            for(int y=1;y < blockHeight * 16; y+= blockHeight){ 
                memset(block, color, blockWidth * blockHeight);
                memset(label,0,4);
                sprintf(label, "%i", color);
                graphics.drawTextToBuffer(label, block, blockWidth, color ^ 0xFF);
                graphics.drawBuffer(x, y, blockWidth, blockHeight, block);

                //graphics.fillRect(x,y, blockWidth, blockHeight,color);                
                //graphics.drawText(x + 2, y + 2, label,color ^ 0xFF, color, false);
                color--;
            }               
        }
        //graphics.render();
        Serial.print(F("Blocks : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
        
        console.SetPosition(3, graphics.settings.screenHeight - 9, false);
        console.write("8 ", 2,Color::RED, true);
        console.write("b", 1,Color::GREEN, true);
        console.write("i", 1,Color::GOLD, true);
        console.write("t", 1,Color::BLUE, true);

        console.SetPosition(70, graphics.settings.screenHeight - 9, false);
        console.write("256 Available Colors", 20,Color::WHITE, true);
           
        
    }
    else if (resp[0] == 'c' || resp[0] == 'C') { //graphics
        graphics.drawCircle(90, 160, 30, Color::BRICK);
        graphics.drawCircle(190, 60, 90,Color::BLUE);

        graphics.drawCircle(40, 20, 30, Color::BLUE);

        graphics.fillCircle(210, 150, 50, Color::BLUE);
        graphics.drawCircle(30, 150, 150, Color::BLUE);
        graphics.fillCircle(90, 130, 50,  Color::GREEN);
    }
    else if (resp[0] == 'o' || resp[0] == 'O') { //graphics
        graphics.drawOval(90, 160, 30, 50, Color::BRICK);
        graphics.drawOval(190, 120, 30, 100, Color::BLUE);

        graphics.fillOval(40, 20, 30, 20, Color::BLUE);

        graphics.fillOval(210, 150, 50, 25, Color::BLUE);
        graphics.fillOval(30, 150, 150, 55, Color::BLUE);
        graphics.fillOval(90, 130, 150, 55, Color::GREEN);
    }
    
    else if (resp[0] == 't' || resp[0] == 'T') { //graphics


        graphics.drawTriangle(5, 20, 230, 20, 130, 175, Color::BLUE);
        graphics.drawTriangle(230, 60, 140, 70, 190, 70, Color::GREEN);
        graphics.drawTriangle(80,120, 110, 155, 240, 190, Color::LIGHT_BLUE);
        graphics.drawTriangle(5,120,15,120,190,135, Color::GOLD);
        graphics.drawTriangle(128, 211, 390, 41, 121, 98, Color::GRAY);
        graphics.drawTriangle(120, 211, 120, 41, 190, 98, Color::GRAY);
    }
    
    else if (resp[0] == 'g' || resp[0] == 'G') { //graphics
        int numOfObjects = 100;
        graphics.clear();
        char buf[128];

        unsigned long dtStartTime = millis();
        Serial.print("Testing drawing triangles .. ");
        dtStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.drawTriangle(
                random(5,graphics.settings.screenWidth - 10), 
                random(5, graphics.settings.screenHeight - 10), 
                random(5,graphics.settings.screenWidth - 10), 
                random(5, graphics.settings.screenHeight - 10), 
                random(5,graphics.settings.screenWidth - 10), 
                random(5, graphics.settings.screenHeight - 10), 
                random(0,255)
            );
        }
        dtStartTime = millis() - dtStartTime;
        Serial.print(". "); Serial.print(dtStartTime); Serial.println(" ms");

        graphics.clear();

        unsigned long drStartTime = millis();
        Serial.print("Testing drawing rectangles .. ");
        drStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.drawRect(random(5,graphics.settings.screenWidth - 10), random(5, graphics.settings.screenHeight - 10), random(1,140),random(0,70),random(0,255));
        }
        drStartTime = millis() - drStartTime;
        Serial.print(". "); Serial.print(drStartTime); Serial.println(" ms");

        graphics.clear();

        Serial.print("Testing drawing circles .. ");
        
        unsigned long dcStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.drawCircle(random(5,graphics.settings.screenWidth - 10), random(5, graphics.settings.screenHeight - 10), random(1,140),random(0,255));
        }
        dcStartTime = millis() - dcStartTime;
        Serial.print(".  "); Serial.print(dcStartTime); Serial.println(" ms");

        graphics.clear();

        Serial.print("Testing drawing ovals .. ");
        
        unsigned long doStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.drawOval(random(5,graphics.settings.screenWidth - 10), random(5, graphics.settings.screenHeight - 10), random(1,140),random(1,140), random(0,255));
        }
        doStartTime = millis() - doStartTime;
        Serial.print(".  "); Serial.print(doStartTime); Serial.println(" ms");

        graphics.clear();

        unsigned long ftStartTime;
        Serial.print("Testing filling triangles .. ");
        ftStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.fillTriangle(
                random(5,graphics.settings.screenWidth - 10), 
                random(5, graphics.settings.screenHeight - 10), 
                random(5,graphics.settings.screenWidth - 10), 
                random(5, graphics.settings.screenHeight - 10), 
                random(5,graphics.settings.screenWidth - 10), 
                random(5, graphics.settings.screenHeight - 10), 
                random(0,255)
            );
        }
        ftStartTime = millis() - ftStartTime;
        Serial.print(". "); Serial.print(ftStartTime); Serial.println(" ms");

        graphics.clear();

        unsigned long frStartTime = millis();
        Serial.print("Testing filling rectangles .. ");
        frStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.fillRect(random(5,graphics.settings.screenWidth - 10), random(5, graphics.settings.screenHeight - 10),  random(1,140),random(5,70),random(0,255));
        }
        frStartTime = millis() - frStartTime;
        Serial.print(". "); Serial.print(frStartTime); Serial.println(" ms");


        graphics.clear();

        unsigned long fcStartTime = millis();
        Serial.print("Testing filling circles .. ");
        fcStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.fillCircle(random(5,graphics.settings.screenWidth - 10), random(5, graphics.settings.screenHeight - 10), random(5,70),random(0,255));
        }
        fcStartTime = millis() - fcStartTime;
        Serial.print(". "); Serial.print(fcStartTime); Serial.println(" ms");

        graphics.clear();

        unsigned long foStartTime = millis();
        Serial.print("Testing filling ovals .. ");
        foStartTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.fillOval(random(5,graphics.settings.screenWidth - 10), random(5, graphics.settings.screenHeight - 10), random(5,70), random(5,70), random(0,255));
        }
        foStartTime = millis() - foStartTime;
        Serial.print(". "); Serial.print(foStartTime); Serial.println(" ms");

        graphics.clear();

        console.SetPosition(0,0);
        sprintf(buf,"Drawing %i triangles:  % 5lu ms", numOfObjects, dtStartTime);
        console.println(buf);
        sprintf(buf,"Drawing %i rectangles: % 5lu ms", numOfObjects, drStartTime);
        console.println(buf);
        sprintf(buf,"Drawing %i circles:    % 5lu ms", numOfObjects, dcStartTime);
        console.println(buf);
        sprintf(buf,"Drawing %i ovals:      % 5lu ms", numOfObjects, doStartTime);
        console.println(buf);
        sprintf(buf,"Filling %i triangles:  % 5lu ms", numOfObjects, ftStartTime);
        console.println(buf);
        sprintf(buf,"Filling %i rectangles: % 5lu ms", numOfObjects, frStartTime);
        console.println(buf);
        sprintf(buf,"Filling %i circles:    % 5lu ms", numOfObjects, fcStartTime);
        console.println(buf);
        sprintf(buf,"Filling %i ovals:      % 5lu ms", numOfObjects, foStartTime);
        console.println(buf);

        unsigned long totalTime = dtStartTime + drStartTime + dcStartTime + doStartTime + ftStartTime + frStartTime + fcStartTime + foStartTime;
        sprintf(buf, "--------------------------------");
        console.println(buf);
        sprintf(buf,"Total rendering time:   % 5lu ms", totalTime);
        console.println(buf);
        if(totalTime >  4000){
            sprintf(buf,"\n--------------------------------\n Congradulations\n\n    You are farming a potato!");
        }
        else if( totalTime > 2000)
        {
            sprintf(buf,"\n--------------------------------\n Congradulations\n\n    You are working with a video card!");
        }
        else{
            sprintf(buf,"\n--------------------------------\n Congradulations\n\n    You are blazing fast!");
        }

        
        console.println(buf);

        
    }
    else if(resp[0] == 'i' || resp[0] == 'I'){
        editor.run();        
        needPrintMenu = true;
    }
    else if(resp[0] == 'h' || resp[0] == 'H'){
        needPrintMenu = true;
    }
    //else 
}

