#include "SRAM.h"
#include "UI/UI.h"
extern UI ui;


SRAM::SRAM()
{
    pinMode(PIN_ADDR0, INPUT);
	pinMode(PIN_ADDR1, INPUT);
	pinMode(PIN_ADDR2, INPUT);
	pinMode(PIN_ADDR3, INPUT);
	pinMode(PIN_ADDR4, INPUT);
	pinMode(PIN_ADDR5, INPUT);
	pinMode(PIN_ADDR6, INPUT);
	pinMode(PIN_ADDR7, INPUT);
	pinMode(PIN_ADDR8, INPUT);
	pinMode(PIN_ADDR9, INPUT);
	pinMode(PIN_ADDR10, INPUT);
	pinMode(PIN_ADDR11, INPUT);
	pinMode(PIN_ADDR12, INPUT);
	pinMode(PIN_ADDR13, INPUT);
	pinMode(PIN_ADDR14, INPUT);
    pinMode(PIN_ADDR15, INPUT);
}


SRAM::~SRAM()
{
}


#pragma region SRAM Chip Methods

void SRAM::ModeOff() {
	digitalWrite(PIN_WE, HIGH);
	digitalWrite(PIN_OE, HIGH);
    #ifdef PIN_CE
	digitalWrite(PIN_CE, HIGH);
    #endif

    pinMode(PIN_DATA0, INPUT);
	pinMode(PIN_DATA1, INPUT);
	pinMode(PIN_DATA2, INPUT);
	pinMode(PIN_DATA3, INPUT);
	pinMode(PIN_DATA4, INPUT);
	pinMode(PIN_DATA5, INPUT);
	pinMode(PIN_DATA6, INPUT);
	pinMode(PIN_DATA7, INPUT);

    pinMode(PIN_ADDR0, INPUT);
	pinMode(PIN_ADDR1, INPUT);
	pinMode(PIN_ADDR2, INPUT);
	pinMode(PIN_ADDR3, INPUT);
	pinMode(PIN_ADDR4, INPUT);
	pinMode(PIN_ADDR5, INPUT);
	pinMode(PIN_ADDR6, INPUT);
	pinMode(PIN_ADDR7, INPUT);
	pinMode(PIN_ADDR8, INPUT);
	pinMode(PIN_ADDR9, INPUT);
	pinMode(PIN_ADDR10, INPUT);
	pinMode(PIN_ADDR11, INPUT);
	pinMode(PIN_ADDR12, INPUT);
	pinMode(PIN_ADDR13, INPUT);
	pinMode(PIN_ADDR14, INPUT);
    pinMode(PIN_ADDR15, INPUT);


	ramState = dsOff;
	//delayMicroseconds(10);
}

void SRAM::ModeOutput() {
	if (ramState == dsOutput) //if alredy in output mode, skip
	{
		//Serial.println("Already in output mode, skip setting mode");
		return;
	}

	digitalWrite(PIN_OE, HIGH);
	digitalWrite(PIN_WE, HIGH); //do in procedure

	pinMode(PIN_DATA0, INPUT);
	pinMode(PIN_DATA1, INPUT);
	pinMode(PIN_DATA2, INPUT);
	pinMode(PIN_DATA3, INPUT);
	pinMode(PIN_DATA4, INPUT);
	pinMode(PIN_DATA5, INPUT);
	pinMode(PIN_DATA6, INPUT);
	pinMode(PIN_DATA7, INPUT);

	if (ramState == dsOff) {
        #ifdef PIN_CE
		digitalWrite(PIN_CE, LOW); 
        #endif
	} //make sure chip is enabled
	
    pinMode(PIN_ADDR0, OUTPUT);
	pinMode(PIN_ADDR1, OUTPUT);
	pinMode(PIN_ADDR2, OUTPUT);
	pinMode(PIN_ADDR3, OUTPUT);
	pinMode(PIN_ADDR4, OUTPUT);
	pinMode(PIN_ADDR5, OUTPUT);
	pinMode(PIN_ADDR6, OUTPUT);
	pinMode(PIN_ADDR7, OUTPUT);
	pinMode(PIN_ADDR8, OUTPUT);
	pinMode(PIN_ADDR9, OUTPUT);
	pinMode(PIN_ADDR10, OUTPUT);
	pinMode(PIN_ADDR11, OUTPUT);
	pinMode(PIN_ADDR12, OUTPUT);
	pinMode(PIN_ADDR13, OUTPUT);
	pinMode(PIN_ADDR14, OUTPUT);
    pinMode(PIN_ADDR15, OUTPUT);
	
	ramState = dsOutput; //update state

}
void SRAM::ModeWrite() {
	if (ramState == dsWrite) { //if alredy in write mode, skip
		//Serial.println("Already in write mode, skip setting mode");
		return;
	}
	
	digitalWrite(PIN_WE, HIGH);
	digitalWrite(PIN_OE, HIGH); //do in procedure
	
	pinMode(PIN_DATA0, OUTPUT);
	pinMode(PIN_DATA1, OUTPUT);
	pinMode(PIN_DATA2, OUTPUT);
	pinMode(PIN_DATA3, OUTPUT);
	pinMode(PIN_DATA4, OUTPUT);
	pinMode(PIN_DATA5, OUTPUT);
	pinMode(PIN_DATA6, OUTPUT);
	pinMode(PIN_DATA7, OUTPUT);

    pinMode(PIN_ADDR0, OUTPUT);
	pinMode(PIN_ADDR1, OUTPUT);
	pinMode(PIN_ADDR2, OUTPUT);
	pinMode(PIN_ADDR3, OUTPUT);
	pinMode(PIN_ADDR4, OUTPUT);
	pinMode(PIN_ADDR5, OUTPUT);
	pinMode(PIN_ADDR6, OUTPUT);
	pinMode(PIN_ADDR7, OUTPUT);
	pinMode(PIN_ADDR8, OUTPUT);
	pinMode(PIN_ADDR9, OUTPUT);
	pinMode(PIN_ADDR10, OUTPUT);
	pinMode(PIN_ADDR11, OUTPUT);
	pinMode(PIN_ADDR12, OUTPUT);
	pinMode(PIN_ADDR13, OUTPUT);
	pinMode(PIN_ADDR14, OUTPUT);
    pinMode(PIN_ADDR15, OUTPUT);

	if (ramState == dsOff) {
        #ifdef PIN_CE
		digitalWrite(PIN_CE, LOW); 		
        #endif
	} //make sure chip is enabled

	ramState = dsWrite; //update state
}
//MAX ADDR is 2048 with 11 address lines, 32k with 15
void SRAM::SetAddress(uint16_t addr) {
	//each bit of address is going to addr pin 0 - 10. 
	
	digitalWrite(PIN_ADDR0, addr & 0x1 << 0);
	digitalWrite(PIN_ADDR1, addr & 0x1 << 1);
	digitalWrite(PIN_ADDR2, addr & 0x1 << 2);
	digitalWrite(PIN_ADDR3, addr & 0x1 << 3);
	digitalWrite(PIN_ADDR4, addr & 0x1 << 4);
	digitalWrite(PIN_ADDR5, addr & 0x1 << 5);
	digitalWrite(PIN_ADDR6, addr & 0x1 << 6);
	digitalWrite(PIN_ADDR7, addr & 0x1 << 7);
	digitalWrite(PIN_ADDR8, addr & 0x1 << 8);
	digitalWrite(PIN_ADDR9, addr & 0x1 << 9);
	digitalWrite(PIN_ADDR10, addr & 0x1 << 10);
	digitalWrite(PIN_ADDR11, addr & 0x1 << 11);
	digitalWrite(PIN_ADDR12, addr & 0x1 << 12);
	digitalWrite(PIN_ADDR13, addr & 0x1 << 13);
	digitalWrite(PIN_ADDR14, addr & 0x1 << 14);
    digitalWrite(PIN_ADDR15, addr & 0x1 << 15);
    //tAA = 80ns	
    //delayMicroseconds(1);

}
void SRAM::SetDataLines(uint8_t data) {
	digitalWrite(PIN_DATA0, data & 0x1 << 0);
    digitalWrite(PIN_DATA1, data & 0x1 << 1);
    digitalWrite(PIN_DATA2, data & 0x1 << 2);
    digitalWrite(PIN_DATA3, data & 0x1 << 3);
    digitalWrite(PIN_DATA4, data & 0x1 << 4);
    digitalWrite(PIN_DATA5, data & 0x1 << 5);
    digitalWrite(PIN_DATA6, data & 0x1 << 6);
    digitalWrite(PIN_DATA7, data & 0x1 << 7);		
	
}

#pragma endregion

#pragma region Value Read/Write

//construct byte from data bits
uint8_t SRAM::ReadByte(uint16_t addr) {
	ModeOutput();
	SetAddress(addr);
	digitalWrite(PIN_OE, LOW);
    //tOE = 35ns
    delayMicroseconds(1);
	//delay(1);
	uint8_t readValue = 
        (digitalRead(PIN_DATA0) << 0) | (digitalRead(PIN_DATA1) << 1) | (digitalRead(PIN_DATA2) << 2) 
            | (digitalRead(PIN_DATA3) << 3) | (digitalRead(PIN_DATA4) << 4) | (digitalRead(PIN_DATA5) << 5) 
            | (digitalRead(PIN_DATA6) << 6) | (digitalRead(PIN_DATA7) << 7);	    	
	ModeOff();
	//Serial.print(" Read: 0x"); Serial.println(readValue,HEX);
	return readValue;
}

void SRAM::WriteFirstByte(uint8_t data, uint16_t offsetAddress) {
	counter = offsetAddress;
	WriteByte(counter, data);
}
void SRAM::WriteNextByte(uint8_t data) {
	counter++;
	WriteByte(counter, data);
}

uint8_t SRAM::WriteBytes(uint16_t addr, uint8_t *data, uint16_t length)
{
    auto retryCount = 0, idx=0;
    _retries = 0;
	bool done = false;
	//while (_retries <= retryCount && !done) {
		ModeWrite();
        while(idx < length){
            SetAddress(addr);
            SetDataLines(data[idx]);
            //toggle WE low for 100ns - 1000ns
            //delayMicroseconds(1); //give 1us for setup time
            digitalWrite(PIN_WE, LOW);
            delayMicroseconds(1);		
            digitalWrite(PIN_WE, HIGH);
            idx++;
            addr++;
            
        } ;

        ModeOff();
   // }
   return idx;
}

bool SRAM::WriteByte(uint16_t addr, uint8_t data, uint8_t retryCount, bool showDebugData) {
	_retries = 0;
	bool done = false;
    ModeWrite();
	//while (_retries <= retryCount && !done) {		        
        SetAddress(addr);
		SetDataLines(data);
		//toggle WE low for 100ns - 1000ns
		delayMicroseconds(1); //give 1us for setup time
		digitalWrite(PIN_WE, LOW);
        //tCW 70ns
		delayMicroseconds(1);		
		digitalWrite(PIN_WE, HIGH);
		delayMicroseconds(1);
		


#if DEBUG
		
		if (showDebugData) {
			uint8_t step = addr >> 8;
			//uint8_t opDest = addr & 0xFF >> 4; //top 3 bits
			uint8_t opCode = addr & 0x1F; //bottom 5 bits
			Serial.print(F("Address: 0x")); Serial.print(addr < 0x1000 ? "0" : "");Serial.print( addr, HEX);
			Serial.print(F("\tData:[0x")); 
            if(data < 16) {Serial.print("0");} 
            Serial.print(data & 0xFF, HEX);
			/*if(opDest != 0)
			Serial.print(F("]  with destination [")); Serial.print(opDest, DEC);*/
			Serial.print(F(" - "));  Serial.print(data & 0xFF, BIN); Serial.println(F("]"));
		}
		//else
			//Serial.print("Instructed to write 0");


#endif
		done = true;
#if VERIFY
		//verify
		ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
		delayMicroseconds(1);
		uint8_t readByte = ReadByte(addr);
		done = (readByte == data);
		unsigned long startT = micros();
		
		while (!done && micros() - 5 < startT) {
			if (readByte == data)
				Serial.println("WAIT..");
			delayMicroseconds(1);
		}
		_retries++;
		if (!done) {
			Serial.print(F("Failed attempt #")); Serial.print(_retries); Serial.print(F(". Expected: ")); Serial.print(data, BIN); Serial.print(F(" but found: ")); Serial.println(readByte, BIN);
			
			if (RETRY_COUNT == _retries) {				
				Serial.println("*****************************************");
				Serial.println("Critical Error. Failed to write to chip!");
				Serial.println("*****************************************");
				break;
			}
		}
        
#endif
	//}
    ModeOff();
	return done;
}
#ifdef DUAL_CHIP
bool SRAM::WriteShort(uint16_t addr, uint16_t data,bool showDebugData) {
	_retries = 0;
	uint16_t data_org = data;
	bool done = false;
	while (_retries < RETRY_COUNT && !done) {

		ModeWrite();
		SetAddress(addr);
		SetDataLines(data);
		SetDataLines(data >> 8);
		//toggle WE low for 100ns - 1000ns
		delay(1); //give 1ms for setup time
		digitalWrite(PIN_WE, LOW);
		delayMicroseconds(100);
		digitalWrite(PIN_WE, HIGH);
        delayMicroseconds(100);
		ModeOff();
		delay(1);

#if DEBUG
		if (showDebugData) {
			uint8_t step = addr >> 8 & 0xF;
			//uint8_t opDest = addr & 0xFF >> 4; //top 3 bits
			uint8_t opCode = addr & 0xFF; //bottom 8 bits
			Serial.print(F("Address: 0x")); Serial.print(addr, HEX);			
			Serial.print(F("\tData: 0x")); Serial.print(data,HEX); Serial.print(F("["));
			BinToSerial(data >> 8 & 0xFF); Serial.print("  ");
			BinToSerial(data & 0xFF);
			/*if(opDest != 0)
			Serial.print(F("]  with destination [")); Serial.print(opDest, DEC);*/
			Serial.println(F("]"));
		}

#endif
		done = true;
#if VERIFY
		//verify
		//ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
		//uint8_t readByte = ReadByte(addr);

		//verify
		ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
		uint8_t  bOut;
		
		delay(1);
		bOut = ReadByte(addr);

		done = (data_org == bOut);
		_retries++;
		if (!done) {
			delay(50);
			
			if (RETRY_COUNT == _retries) {
				Serial.print(F("Attempts:")); Serial.print(_retries); Serial.print(F(".  Failed to write at address: 0x")); Serial.print(addr, HEX);
				Serial.print(F(". Expected: ")); Serial.print(data_org, BIN); Serial.print(F(" but found: ")); 
				Serial.println(bOut, BIN);
				Serial.println("*****************************************");
				Serial.println("Critical Error. Failed to write to chip!");
				Serial.println("*****************************************");
				return true;//return false;
			}
				
		}
#endif
	}

	return done;
}
#endif
#pragma endregion

//Prints binary representation of number
void SRAM::BinToSerial(uint8_t var) {
	for (int i = sizeof(var) * 8 - 1; i >= 0; i--) {
		Serial.print(((var >> i) & 1) == 1 ? "1" : "0");
	}
	
}