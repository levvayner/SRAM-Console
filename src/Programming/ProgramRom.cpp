#include "ProgramRom.h"
#include "UI/UI.h"
extern SRAM programmer;
extern UI ui;
#define TIMEOUT_WAIT_FOR_HOST 1000 * 60


ProgramRom::ProgramRom()
{
}


ProgramRom::~ProgramRom()
{
}



void ProgramRom::StoreProgramData() {

	//slave mode to programming server
    bool automateResult = RunAutomatedProgramming();
    digitalWrite(LED_BUILTIN, automateResult);		
	
	
}
//TODO: frame reserves the full program size (just in case) need to improve that!
bool ProgramRom::RunAutomatedProgramming() {
	Serial.println("Standby mode.. waiting for connection from server.");
	unsigned long startTime = millis();
	bool startComms = false;
	String lineIn;
	int lineCount = 0;
	
	while (millis() - startTime < TIMEOUT_WAIT_FOR_HOST && !startComms) {
		if (TIMEOUT_WAIT_FOR_HOST / 10 % millis() - startTime == 0)
		{
			Serial.print(".");
		}

		if (Serial.available()) {
			lineIn = Serial.readString();
			if (lineIn.startsWith("START_PROGRAM")) {
				Serial.println("READY");
				startComms = true;
			}
		}
	}
	//Serial.flush(); 
	
	if (!startComms) {
		Serial.println("Failed to establish connection with programming server.");
		//first line after START_PROGRAM is line count
	}

	while (!Serial.available());
	delay(10);

	//int readLines = 0; //getLines = false;
	startTime = millis();
	uint8_t counter = 0;
	//while (startTime + 100 > millis() || counter == 0) {
		if (Serial.available()) {
			lineIn = Serial.readString();
			lineCount = lineIn.toInt();
			//lineCount = lineCount << 8 | (byte)Serial.read();
			startTime = millis();
			counter++;
		}
	//}
	
	Serial.print("Established byte count: "); Serial.println(lineCount,BIN);

    while (!Serial.available());
    delay(10);
	int frameSize = 32;
	//frame size
	startTime = millis();	
    while (Serial.available() && startTime + 100 > millis()) {
        frameSize = Serial.readString().toInt();
        Serial.print("Read frame size from serial: "); Serial.println(frameSize);
        startTime = millis();
		
	}

	Serial.print("Established frame size: "); Serial.println(frameSize);

	counter = 0;
	uint8_t bytesRead = 0;
    byte  frameBuffer[frameSize];
	bool sucess = false;
	int frameCount = ceil(lineCount / frameSize);

	Serial.print("Writing "); Serial.print(frameCount); Serial.println(" frames");
	
    delay(10);
    char buf[1024 + 60];
    //do one frame at a time. report sucess if it worked, otherwise report failure and retry.
    //frameBuffer = (byte*)malloc(frameSize);
    // if(frameBuffer == NULL){
    //     Serial.println("Failed to allocate memory for frame");
    //     return false;
    // }
    for (int i = 0; i < frameCount; i++) {
        startTime = millis();
        int frameBytesRead = 0; 
        int actualSize = ((i < frameCount - 1 )? frameSize : lineCount % frameSize);       

        memset(frameBuffer,0,frameSize);
        memset(buf,0,1024);

        //we will receive a frame and respond
        while(!Serial.available() && startTime + 500 > millis());
        while( startTime + 5000 > millis() && frameBytesRead < actualSize)
        {
            if(Serial.available()){
                bytesRead = Serial.readBytes(frameBuffer + frameBytesRead,Serial.available());
                frameBytesRead += bytesRead;
            }
        }
        // sprintf(buf,"Frame %i of %i (%i bytes): ", i + 1, frameCount, frameBytesRead);
        // //Serial.print("Read "); Serial.print(frameBytesRead); Serial.println(" bytes");
        // for(int idx=0;idx < frameBytesRead;idx++){
        //     if(strlen(buf) > 1024) break; //make sure we don't overflow
        //     sprintf(buf,"%s %02X", buf, frameBuffer[idx]);        
        // }
        if(frameBytesRead == 0)	{            
            Serial.println("Failed to read from serial. NO DATA");
            return false;
        }    
        programmer.WriteBytes(i*frameSize, frameBuffer, frameBytesRead);			
        //Serial.println(buf);
	}

    //free(frameBuffer);
	//for (int i = 0; i < frameCount; i++) {
	//	//wait to get line
	//	startTime = millis();
	//	for (int j = 0; j < frameSize; ) {
	//		//while (millis() - startTime < TIMEOUT_WAIT_FOR_HOST) {
	//			if (Serial.available()) {
	//				b = Serial.read();
	//				//fr[j] = b;
	//				
	//				programmer.WriteByte(addr + j, b);
	//				//Serial.print("Writing 0x"); Serial.print(b, HEX); Serial.print(" at address 0x"); Serial.println(addr + j, HEX);
	//				j++;
	//				//Serial.println(".");
	//				
	//			}
	//			if (i * frameSize + j >= lineCount - 1)
	//				continue;
	//		//}
	//	}

	//	//write the frame to memory
	//	/*for (int f = 0; f < frameSize; f++) {
	//		programmer.WriteByte(addr + f, fr[f]);
	//		Serial.print("Writing 0x"); Serial.print(fr[f], HEX); Serial.print(" at address 0x"); Serial.println(addr + f, HEX);
	//	}*/
	//	//increment base address
	//	addr = addr + frameSize;
	//	//reset timeout
	//	startTime = millis();
	//			//get next data byte
	//			//lineIn = Serial.readString();
	//			//Serial.print("Processing input: "); Serial.print(lineIn);
	//			//uint16_t length = lineIn.length();
	//			//uint16_t addr = lineIn.substring(0, 4).toInt();
	//			//uint8_t data = lineIn.substring(4,7).toInt();
	//			//switch (counter) {
	//			//case 0:

	//			//	addr = (b-'0') << 8; counter++;
	//			//	//Serial.print("Writing to programmer: at adr: 0x"); Serial.print(addr, HEX); Serial.print(" data:0x"); Serial.println(data, HEX);

	//			//	break;
	//			//case 1:
	//			//	addr |= (b-'0'); counter++;
	//			//	break;
	//			//case 2:
	//			//	data = b; counter++;
	//			//	break;
	//			//default:
	//			//	counter = 0;
	//			//}				
	//			//if (counter > 2)
	//			//	//addr = Serial.read() << 8 | Serial.read();
	//			//	//data = Serial.read();
	//			//{
	//			//	Serial.print("Writing to programmer: at adr: 0x"); Serial.print(addr, HEX); Serial.print(" data:0x"); Serial.println(data, HEX);
	//			//	programmer.WriteByte(addr, data);
	//			//	counter = 0;
	//			//	//Serial.println("<< Received Byte");
	//			//	sucess = true;
	//			//	startTime = millis();
	//			//}
	//			
	//		
	//	
	//}
	return sucess;
}


