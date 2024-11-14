#include "SRAM.h"
#include "UI/UI.h"
extern UI ui;
// #if defined(ARDUINO_SAM_DUE)
// #define PORTA POIA
// #endif

SRAM::SRAM()
{
    
    //address is A port, pins 10-17 and D port 0-7
    //set address pins to gpio    
    // PIOA->PIO_PER |= 0xFF << 10;
    // PIOD->PIO_PER |=  0xFF;  

    //set data pins to gpio
//    PIOC->PIO_PER |= 0xFF << 1;     // bits 1 to 8

    //set address as output
    // PIOA->PIO_OER |= 0xFF << 10;    // bits 10 - 17
    
    
    /*
    USART3->US_CR = US_CR_RXDIS;
    USART3->US_CR = US_CR_TXDIS;
    */
    PIOC->PIO_PER |= 0xFF << 12;    // lowest 8 bits
    PIOC->PIO_OER |= 0xFF << 12;
    
    PIOD->PIO_PER |= 0xF;    // upper 8 bits
    PIOD->PIO_OER |= 0xF;
    PIOB->PIO_PER |= 0xF << 17;    // upper 8 bits
    PIOB->PIO_OER |= 0xF << 17;

    PMC->PMC_PCER0 |= PMC_PCER0_PID14;
    PMC->PMC_PCER0 = PMC_PCER0_PID13;
    PMC->PMC_PCER0 = PMC_PCER0_PID12;

	pinMode(PIN_ADDR8, OUTPUT);
	pinMode(PIN_ADDR9, OUTPUT);
	pinMode(PIN_ADDR10, OUTPUT);
	pinMode(PIN_ADDR11, OUTPUT);
	pinMode(PIN_ADDR12, OUTPUT);
	pinMode(PIN_ADDR13, OUTPUT);
	pinMode(PIN_ADDR14, OUTPUT);
    pinMode(PIN_ADDR15, OUTPUT);

    pinMode(SCREEN_OUTPUT, INPUT);
}


SRAM::~SRAM()
{
}


#pragma region SRAM Chip Methods

void SRAM::DeviceOff() {
	digitalWrite(PIN_WE, LOW);
	digitalWrite(PIN_OE, LOW);
    #ifdef PIN_CE
	digitalWrite(PIN_CE, LOW);
    #endif
    
    //set pins as input
    PIOC->PIO_ODR |= 0xFF << 1;
    
    ramState = dsOff;	
}

void SRAM::DeviceOutput() {
	if (ramState == dsOutput) //if alredy in output mode, skip
	{
		//Serial.println("Already in output mode, skip setting mode");
		return;
	}
    //while(!digitalRead(SCREEN_OUTPUT)); //wait until blanking period
	digitalWrite(PIN_OE, LOW);
	digitalWrite(PIN_WE, LOW); //do in procedure

    //set pins as input
    PIOC->PIO_ODR |= (0xFF << 1);
    
	if (ramState == dsOff) {
        #ifdef PIN_CE
		digitalWrite(PIN_CE, HIGH); 
        #endif
	} //make sure chip is enabled
	
	
	ramState = dsOutput; //update state

}
void SRAM::DeviceWrite() {
	if (ramState == dsWrite) { //if alredy in write mode, skip
		Serial.println("Already in write mode, skip setting mode");
		return;
	}
	// while(!digitalRead(SCREEN_OUTPUT)); //wait until blanking period
	digitalWrite(PIN_WE, LOW);
	digitalWrite(PIN_OE, LOW); //do in procedure

    //set pins as output
    PIOC->PIO_OER |= 0xFF << 1;
    
	if (ramState == dsOff) {
        #ifdef PIN_CE
		digitalWrite(PIN_CE, HIGH); 		
        #endif
	} //make sure chip is enabled

	ramState = dsWrite; //update state
}
//MAX ADDR is 2048 with 11 address lines, 32k with 15
void SRAM::SetAddress(uint32_t addr) {
	//each bit of address is going to addr pin 0 - 10. 
    #if defined(__AVR_MEGA__)
        PORTA = addr & 0xFF;
        PORTC = addr >> 8;
	#elif defined(ARDUINO_SAM_DUE)
        //lowest 8 bits of address are on port a, pins 10-17, upper are port d pins 0-7
    
    // PIOC->PIO_SODR = (addr & 0xFF) << 12; 
    // PIOC->PIO_CODR = ((addr ^ 1) & 0xFF) << 12;

    // PIOD->PIO_SODR = ((addr >> 8) & 0xFF); 
    // PIOD->PIO_CODR = (((addr >> 8) ^ 1) & 0xFF) ;
    //Serial.print("Setting lower 8 bits in reg a using mask "); Serial.print(0xFF << 10,BIN); Serial.print(" and value "); Serial.println((addr & 0xFF) << 10, BIN);
    //lower address bits
    REG_PIOC_CODR = 0xFF << 12;
    REG_PIOC_SODR = (addr & 0xFF) << 12;

    //upper address bits
    //first four  are register D pins 0-3, upper 4 are port B 17 - 20
    REG_PIOB_CODR = 0xF << 17;
    REG_PIOB_SODR = ((addr >> 12) & 0xF)  << 17;

    REG_PIOD_CODR = 0xF;
    REG_PIOD_SODR = ((addr >> 8) & 0xF);

    /*
    //Serial.print("Setting upper 8 bits in reg a using mask "); Serial.print(0xFF,BIN); Serial.print(" and value "); Serial.println(((addr >> 8) & 0xFF), BIN);
    REG_PIOD_CODR = 0xFF;
    REG_PIOD_SODR = ((addr >> 8) & 0xFF);
    */
   
    // digitalWrite(PIN_ADDR8, addr & 0x1 << 8);
    // digitalWrite(PIN_ADDR9, addr & 0x1 << 9);
    // digitalWrite(PIN_ADDR10, addr & 0x1 << 10);
    // digitalWrite(PIN_ADDR11, addr & 0x1 << 11);
    // digitalWrite(PIN_ADDR12, addr & 0x1 << 12);
    // digitalWrite(PIN_ADDR13, addr & 0x1 << 13);
    // digitalWrite(PIN_ADDR14, addr & 0x1 << 14);
    // digitalWrite(PIN_ADDR15, addr & 0x1 << 15);
        //tAA = 80ns	
    #endif

}
void SRAM::SetDataLines(uint8_t data) {
    #ifdef USE_PORT_IO
        PORTL = data;
    #else
        REG_PIOC_CODR = 0xFF << 1;
        REG_PIOC_SODR = (data & 0xFF) << 1;
    #endif
	
}

#pragma endregion

#pragma region Value Read/Write

//construct byte from data bits
uint8_t SRAM::ReadByte(uint32_t addr) {
	DeviceOutput();
	SetAddress(addr);
	digitalWrite(PIN_OE, HIGH);
    //tOE = 35ns, @84Mhz 1 tick is 1.2e-8s or 12ns. 3 clocks will pass at least
    #ifdef USE_PORT_IO
        uint8_t readValue = PINL;
    #else        
        PIOC->PIO_ODR = (0xFF << 1);
        uint8_t readValue =  (PIOC->PIO_PDSR >> 1) & 0xFF;	  
    #endif
    DeviceOff();
	return readValue;
}


uint16_t SRAM::WriteBytes(uint32_t addr, uint8_t *data, uint16_t length)
{
    DeviceWrite();
    uint16_t idx = 0;
    
    while(idx < length){
        // if(!digitalRead(SCREEN_OUTPUT)) //wait until blanking period)
        // {
        //     DeviceOff();
        //     while(!digitalRead(SCREEN_OUTPUT));
        // }
        
        SetAddress(addr);
        SetDataLines(data[idx]);
        //toggle WE low for 100ns - 1000ns
        //give 1us for setup time
        digitalWrite(PIN_WE, HIGH);
        delayMicroseconds(1);		
        digitalWrite(PIN_WE, LOW);
        idx++;
        addr++;       
    } ;
    DeviceOff();	
    
   
   return idx;
}

void SRAM::EraseRam(int startAddress, int length)
{
    byte rowBytes[BUFFER_SIZE];
    memset(rowBytes, ERASE_BYTE,BUFFER_SIZE);
    
    for(int pos = startAddress; pos < length / BUFFER_SIZE;pos ++){        
        WriteBytes(pos<< 8, rowBytes, min(BUFFER_SIZE, length - pos));
    }
}


bool SRAM::WriteByte(uint32_t addr, uint8_t data, uint8_t retryCount, bool showDebugData) {
	_retries = 0;
	bool done = false;
    DeviceWrite();
	//while (_retries <= retryCount && !done) {		        
        SetAddress(addr);
		SetDataLines(data);
		//toggle WE low for 100ns - 1000ns
		digitalWrite(PIN_WE, HIGH);
        //tCW 70ns
		delayMicroseconds(1);		
		digitalWrite(PIN_WE, LOW);
		


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
		// ReadByte(addr > 0 ? addr - 1 : addr + 1); //force data change
		// delayMicroseconds(1);
        DeviceOutput();
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
			Serial.print("0x"); Serial.print(addr, HEX); Serial.print(F(" - Failed #")); Serial.print(_retries); Serial.print(F(". Expected: ")); Serial.print(data, BIN); Serial.print(F(" but found: ")); Serial.println(readByte, BIN);
			
			if (RETRY_COUNT == _retries) {				
				Serial.println("*****************************************");
				Serial.println("Critical Error. Failed to write to chip!");
				Serial.println("*****************************************");
				return false;
			}
		}
        
#endif
	//}
    DeviceOff();
	return done;
}
#ifdef DUAL_CHIP
bool SRAM::WriteShort(uint32_t addr, uint16_t data,bool showDebugData) {
	_retries = 0;
	uint16_t data_org = data;
	bool done = false;
	while (_retries < RETRY_COUNT && !done) {

		DeviceWrite();
		SetAddress(addr);
		SetDataLines(data);
		SetDataLines(data >> 8);
		//toggle WE low for 100ns - 1000ns
		delay(1); //give 1ms for setup time
		digitalWrite(PIN_WE, LOW);
		delayMicroseconds(100);
		digitalWrite(PIN_WE, HIGH);
        delayMicroseconds(100);
		DeviceOff();
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