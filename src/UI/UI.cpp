
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
	Serial.println(F("Press l to slow draw lines"));
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
    uint16_t addrS = addrSS.toInt();
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
    uint16_t addrE = addrES.toInt();

    Serial.print(": ");
    Serial.print(addrE);
    Serial.print(" / 0x");
    Serial.println(addrE,HEX);

    if(addrS < 0 || addrS > (uint16_t)SRAM_SIZE){
        Serial.print("Start address "); Serial.print(addrS); Serial.println(" is invalid. Setting to 0");
        addrS = 0;
    }
    if(addrE < 0 || addrE < addrS || addrE > SRAM_SIZE){
        Serial.print("End address "); Serial.print(addrE); Serial.println(" is invalid. Setting to "); Serial.println(SRAM_SIZE,DEC);
        addrE = SRAM_SIZE;
    }

	Serial.println("Dumping RAM");
    

    
    byte baseOffset = addrS%frameSize;
    //Serial.print("Base Offset: "); Serial.println(baseOffset);
	for (long address = addrS - baseOffset; address < addrE; address+=256) {
        //Serial.print("Starting frame at address "); Serial.println(address,HEX);
		for (int base = address; base <= address + 256 && base <= addrE; base += frameSize) {
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
}
#define BUFFER_SIZE 256
void UI::EraseRAM()
{

    size_t memSize = SCREEN_WIDTH * SCREEN_HEIGHT;
    byte ERASE_BYTE = 0x0;
    byte rowBytes[BUFFER_SIZE];
    memset(rowBytes, ERASE_BYTE,BUFFER_SIZE);
    unsigned long startTime = millis();
	Serial.print(F("Erasing RAM.."));
    
    for(uint16_t line = 0; line < SCREEN_HEIGHT;line++){
        
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
	Serial.print(F(" : Done in ")); Serial.print((millis() - startTime)/1000);Serial.println(" seconds.");
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

#ifdef DEBUG

		if (programmer.WriteByte(addr, data)) {
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
        byte color = 0;
        unsigned long startTime = millis();
        for(uint16_t line = 0; line < SCREEN_WIDTH;line++){
            color = (line & 0x03) | (line *2 & 0x03) << 3 | (line%6 << 2);
            //Serial.print("Drawing line on X = "); Serial.print(line); Serial.print(" with color: "); Serial.println(color,BIN);
            memset(colBytes, color, SCREEN_WIDTH);
            for(int y = 0; y < SCREEN_HEIGHT; y++){
                programmer.WriteByte((y << 8) + line, colBytes[y]);
            }
        }
        Serial.print(F("Draw lne : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
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
        char buf[256];
        int blockWidth = SCREEN_WIDTH / 16;
        int blockHeight = SCREEN_HEIGHT / 16;
        //Serial.print("Setting up blocks with width "); Serial.print(blockWidth); Serial.print(" and height "); Serial.println(blockHeight);
        int color = 0xFF;
        byte colors[blockWidth];
            
            for(int x = 0; x < SCREEN_WIDTH; x+= blockWidth){
                for(int y=0;y < SCREEN_HEIGHT; y+= blockHeight){ 
            
                if(color < 0x0) break;
                //draw pixels
                //sprintf(buf, "Drawing block at (%i,%i) with color %i", x,y, color);
                Serial.println(buf);
                memset(colors,color, blockWidth);
                for(int pxlY = y; pxlY < y + blockHeight; pxlY++){
                    programmer.WriteBytes((pxlY << 8 ) + x, colors, blockWidth);
                }
                color--;
            }
        }
           
        
    }
    else if(resp[0] == 'i' || resp[0] == 'I'){
        console.run();
        needPrintMenu = true;
    }
    else needPrintMenu = true;
}

