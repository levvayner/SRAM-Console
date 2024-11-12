
#include "UI.h"


UI::UI()
{
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
	Serial.println(F("SRAM TOOL   -   v 0.0.1"));
	Serial.println(F("--------------------------------"));
	Serial.println(F("Press r to read"));
	Serial.println(F("Press w to write"));
	Serial.println(F("Press p to print data"));
	Serial.println(F("Press s to store data"));
	Serial.println(F("Press e to erase RAM"));
    Serial.println(F("Press b for color blocks"));
    Serial.println(F("Press v for vertical lines"));
    Serial.println(F("Press l for horizontal lines"));
	Serial.println(F("Press i to enter interactive terminal"));
	Serial.println(F("--------------------------------"));

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
	for (uint16_t address = addrS - baseOffset; address < addrE; address+=256) {
        //Serial.print("Starting frame at address "); Serial.println(address,HEX);
		for (uint16_t base = address; base <= address + 256 && base <= addrE; base += frameSize) {
			byte data;
            char buf[80];       
            sprintf(buf, "%06x: ", base);     
            byte frameOffset = base == address ? baseOffset : 0;
            bool isLastFrame = base + 16 > addrE;            
            for (int offset = 0; offset <= 15 ; offset += 1) {
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
#define BUFFER_SIZE 256
void UI::EraseRAM()
{

    byte rowBytes[BUFFER_SIZE];
    memset(rowBytes, ERASE_BYTE,BUFFER_SIZE);
    unsigned long startTime = millis();
	Serial.print(F("Erasing RAM.."));
    
    for(uint16_t line = 0; line < SCREEN_HEIGHT + 10;line++){
        
        programmer.WriteBytes(line<< 8, rowBytes,BUFFER_SIZE);
    }

	// for (int i = 0; i < SRAM_SIZE; i+= BUFFER_SIZE) {
	// 	programmer.WriteBytes(i, rowBytes, BUFFER_SIZE);
	// 	if (i / marker >= nextPercent)
	// 	{
	// 		Serial.print(i / marker); Serial.print(F("0%.. "));
	// 		nextPercent++;
	// 	}
		
	// }
	Serial.print(F(" : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
}

void UI::ProcessInput() {
	auto resp = Serial.readString();
	if (resp[0]== 'r' || resp[0] == 'R') {
		//read
		Serial.println("Enter address to read");
		delay(50);
		while (!Serial.available()) {
			blinkLED();
		}
		String addrS = Serial.readString();
		uint16_t addr = addrS.toInt();

		byte data = programmer.ReadByte(addr);
		Serial.print("Read: "); Serial.print(data); Serial.print(" from address 0x"); Serial.print(addr, HEX);
		Serial.println();
		//needPrintMenu = true;
	}
	else if (resp[0] =='w' || resp[0] =='W') {
		//write

		Serial.print("Enter address to write");
		while (!Serial.available());
		String addrS = Serial.readString();
		uint16_t addr = addrS.toInt();
        Serial.print(": "); Serial.println(addr,HEX);

		Serial.print("Enter data to store in decimal form");
		
		while (!Serial.available());
		String dataS = Serial.readString();
		byte data = dataS.toInt();
        Serial.print(": "); Serial.println(data, DEC);
        bool sucess = programmer.WriteByte(addr, data);
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
		EraseRAM();

		Serial.println();
		//needPrintMenu = true;
	}
	else if (resp[0] =='s' || resp[0] =='S') {
		
        programRom.StoreProgramData();

		Serial.println("Done!");
		//needPrintMenu = true;
	} 
    else if (resp[0] == 'l' || resp[0] == 'L') {
		byte rowBytes[SCREEN_WIDTH];
        byte color = 0;
        unsigned long startTime = millis();
        for(uint16_t line = 0; line < SCREEN_HEIGHT;line++){
            color = (line & 0x03) | (line *2 & 0x03) << 3 | (line%6 << 2);
            //Serial.print("Drawing line on Y = "); Serial.print(line); Serial.print(" with color: "); Serial.println(color,BIN);
            //memset(rowBytes,0,5);
            memset(rowBytes, color, SCREEN_WIDTH);
            programmer.WriteBytes(line<< 8, rowBytes,SCREEN_WIDTH);
            // for(byte x = 5; x < SCREEN_WIDTH - 10; x++){
                
            //     programmer.WriteByte(line<< 8 | (x & 0xFF), color,1,false);
            //     //delay(1);
            //     //Serial.print(line << 8 | x,HEX); Serial.print(" ");                
            // }
            //Serial.println();
            //delay(10);
        }
        Serial.print(F("Draw lne : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
		//needPrintMenu = true;
	}
    else if (resp[0] == 'v' || resp[0] == 'V') {
		byte colBytes[max(SCREEN_HEIGHT, SCREEN_WIDTH)];
        byte color = 1;
        unsigned long startTime = millis();
        for(uint16_t line = 0; line < SCREEN_WIDTH;line++){   
            for(int div = 0; div < 8; div++){
                //Serial.print("Drawing line on X = "); Serial.print(line); Serial.print(" with color: "); Serial.println(color,BIN);
                memset(colBytes, color, SCREEN_WIDTH);
                for(int y = (SCREEN_HEIGHT / 8) * div; y < (SCREEN_HEIGHT / 8) * (div + 1); y++){
                    programmer.WriteByte((y << 8) + line, colBytes[y]);
                }
                color+=2;
            }         
            color++;
           
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
        for(uint16_t line = 0; line < SCREEN_HEIGHT;line++){
            programmer.WriteBytes(line << 8, colors + line, SCREEN_WIDTH - line); //write from 0 to end of colors            
            programmer.WriteBytes((line << 8) + (SCREEN_WIDTH - line - 1), colors, line );
            
           //Serial.print("Drawing line on Y = "); Serial.println(line);
          
        }
        Serial.print(F("Draw diagonal line : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
		//needPrintMenu = true;
	}
    
    else if (resp[0] == 'b' || resp[0] == 'B') { //colors
        int blockWidth = floor(SCREEN_WIDTH / 16) + 1; //rather push off screen a bit
        int blockHeight = floor((SCREEN_HEIGHT - 20) / 16);
        //Serial.print("Setting up blocks with width "); Serial.print(blockWidth); Serial.print(" and height "); Serial.println(blockHeight);
        int color = 0xFF;
        byte colors[blockWidth];
        unsigned long  startTime = millis();
        for(int x = 1; x < SCREEN_WIDTH; x+= blockWidth){
            for(int y=1;y < SCREEN_HEIGHT - 20; y+= blockHeight){ 
                
                if(color < 0x0) break;
                
                memset(colors,color, blockWidth);
                for(int pxlY = y; pxlY < y + blockHeight; pxlY++){
                    programmer.WriteBytes((pxlY << 8 ) + x, colors, blockWidth);
                }
                // console.SetPosition(x,y);
                // //hundreds
                // if(color >= 200) console.write('2',Color::WHITE,false);
                // else if (color >= 100) console.write('1',Color::WHITE,false);
                // //tens
                // if(((color%100 - (color % 10)))/10 > 0)
                //     console.write(((color%100 - (color % 10)))/10 + 48,Color::WHITE,false); 
                // //ones               
                // console.write((color % 10) + 48,Color::WHITE,false);
                color--;
                if(color == 0) break;
            }
        }
        Serial.print(F("Blocks : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");

        // startTime = millis();
        // color = 255;
        // for(int x = 0; x < blockWidth * 16; x+= blockWidth){
        //     for(int y=0;y < blockHeight * 16; y+= blockHeight){ 
        //         console.SetPosition(x,y);
        //         sprintf(buf,"%i", color);                
        //         console.write(buf, strlen(buf), Color::WHITE,false);
        //          color--;
        //     }
        // }
        // Serial.print(F("Block labels : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
        console.SetPosition(3, SCREEN_HEIGHT - 9, false);
        console.write("8 ", 2,Color::RED, true);
        console.write("b", 1,Color::GREEN, true);
        console.write("i", 1,Color::GOLD, true);
        console.write("t", 1,Color::BLUE, true);

        console.SetPosition(70, SCREEN_HEIGHT - 9, false);
        console.write("256 Available Colors", 20,Color::WHITE, true);
           
        
    }
    else if (resp[0] == 'g' || resp[0] == 'G') { //graphics
        int numOfObjects = 100;
        graphics.clear();

        Serial.println("Testing drawing circles");
        
        unsigned long startTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.drawCircle(random(5,SCREEN_WIDTH - 10), random(5, SCREEN_HEIGHT - 10), random(1,140),random(0,255));
        }
        Serial.print("Drawing circles took "); Serial.print(millis() - startTime); Serial.println(" ms");

        graphics.clear();

        Serial.println("Testing filling circles");
        startTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.fillCircle(random(5,SCREEN_WIDTH - 10), random(5, SCREEN_HEIGHT - 10), random(5,70),random(0,255));
        }
        Serial.print("Filling circles took "); Serial.print(millis() - startTime); Serial.println(" ms");

        graphics.clear();

        Serial.println("Testing drawing rectangles");
        startTime = millis();
        for(int idx = 0; idx < numOfObjects; idx++){
            graphics.drawRect(random(5,SCREEN_WIDTH - 10), random(5, SCREEN_HEIGHT - 10), random(1,140),random(0,70),random(0,255));
        }
        Serial.print("Drawing rectangles took "); Serial.print(millis() - startTime); Serial.println(" ms");
    }
    else if(resp[0] == 'i' || resp[0] == 'I'){
        console.run();
        needPrintMenu = true;
    }
    else needPrintMenu = true;
}

