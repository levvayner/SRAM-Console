#include "SRAM.h"
#include "UI/UI.h"

#define NOP __asm__ __volatile__ ("nop\n\t")
#define isascii(c)  ((c & ~0x7F) == 0)
extern UI ui;


SRAM::SRAM()
{
    
    // USART2->US_CR = US_CR_TXDIS;//Disables USART2 TX
    // USART2->US_CR = US_CR_RXDIS;//Disables USART2 RX

   

    #if defined(SCREEN_OUTPUT)
    pinMode(SCREEN_OUTPUT, INPUT);
    #endif
}


SRAM::~SRAM()
{
}

void SRAM::begin()
{
     PIOD->PIO_PER = 0x7FF;         // address bits 0 - 10 //enable
    PIOD->PIO_OER = 0x7FF;         //set as output
    
    PIOC->PIO_PER = 0x1FF << 1;     // address bits 11-19
    PIOC->PIO_OER = 0x1FF << 1;

    PIOC->PIO_PER = 0xFF << 12;    //enable data pins
   
    
    //enable and set as input, diable pullup D16, SCREEN_VISIBLE (active LOW)
    PIOA->PIO_PER = PIO_PA13;
    PIOA->PIO_ODR = PIO_PA13;
    PIOA->PIO_PUDR = PIO_PA13;

    //pinMode(16,INPUT);
    //enable and set as input, diable pullup D8 , VERTICAL_VISIBLE (active LOW) C22
    PIOC->PIO_PER = PIO_PC22;
    PIOC->PIO_ODR = PIO_PC22;
    PIOC->PIO_PUDR = PIO_PC22;

    //enable and set as input, diable pullup D9 , HORIZONTAL_VISIBLE (active HIGH) C21
    PIOC->PIO_PER = PIO_PC21;
    PIOC->PIO_ODR = PIO_PC21;
    PIOC->PIO_PUDR = PIO_PC21;

    //enable and set as output D4 WE
    PIOA->PIO_PER = PIO_PA29;
    PIOA->PIO_OER = PIO_PA29;
    //PIOA->PIO_CODR = PIO_PA29;

    //enable and set as output D2 OE
    PIOB->PIO_PER = PIO_PB25;
    PIOB->PIO_OER = PIO_PB25;
    //PIOB->PIO_CODR = PIO_PB25;

    //enable and set as output D3 CE
    PIOC->PIO_PER = PIO_PC28;
    PIOC->PIO_OER = PIO_PC28; 
    //PIOC->PIO_CODR = PIO_PC28;

	DeviceOff();
}

#pragma region SRAM Chip Methods

void SRAM::DeviceOff() {

    PIOA->PIO_CODR = PIO_PA29;
    PIOB->PIO_CODR = PIO_PB25;
    PIOC->PIO_CODR = PIO_PC28;    
    
    //set pins as input
    PIOC->PIO_ODR = 0xFF << 12;
    
    ramState = dsOff;	
}

void SRAM::DeviceOutput() {
	if (ramState == dsOutput) //if alredy in output mode, skip
	{
        return;
	}

    //set pins as input
    PIOC->PIO_ODR = (0xFF << 12);
    
	if (ramState == dsOff) {
        #ifdef PIN_CE
        PIOC->PIO_SODR = PIO_PC28;
		//digitalWrite(PIN_CE, HIGH); 
        #endif
	} //make sure chip is enabled
	
	
	ramState = dsOutput; //update state

}
void SRAM::DeviceWrite() {
	if (ramState == dsWrite) { //if alredy in write mode, skip
		Serial.println("Already in write mode, skip setting mode");
		return;
	}

    //set pins as output
    PIOC->PIO_OER = 0xFF << 12;
    
	if (ramState == dsOff) {
        #ifdef PIN_CE
        PIOC->PIO_SODR = PIO_PC28;        
		//digitalWrite(PIN_CE, HIGH); 		
        #endif
	} //make sure chip is enabled

	ramState = dsWrite; //update state
}


#pragma endregion

#pragma region Value Read/Write

//construct byte from data bits
uint8_t SRAM::ReadByte(uint32_t addr) {
	DeviceOutput();
	SetAddress(addr);
    PIOB->PIO_SODR = PIO_PB25;
	//digitalWrite(PIN_OE, HIGH);
    //tOE = 35ns, @84Mhz 1 tick is 1.2e-8s or 12ns. 3 clocks will pass at least
    #ifdef USE_PORT_IO
        uint8_t readValue = PINL;
    #else        
        //PIOC->PIO_ODR |= (0xFF << 1);
        uint8_t readValue =  (PIOC->PIO_PDSR >> 12) & 0xFF;	  
    #endif
    DeviceOff();
	return readValue;
}

size_t SRAM::ReadBytes(uint32_t addr, uint8_t *buffer, uint32_t length)
{
    DeviceOutput();
    uint32_t endAddr = addr + length;
    uint32_t curAddr = addr;
    
    PIOB->PIO_SODR = PIO_PB25;
    //digitalWrite(PIN_OE, HIGH);
    //tOE = 35ns, @84Mhz 1 tick is 1.2e-8s or 12ns. 3 clocks will pass at least
    while(curAddr < endAddr){
        SetAddress(curAddr);
        //NOP; //tAA ~70 ns
        
        #ifdef USE_PORT_IO
            uint8_t readValue = PINL;
        #else        
            
            //PIOC->PIO_ODR |= (0xFF << 1);
            buffer[curAddr - addr] = (PIOC->PIO_PDSR >> 12) & 0xFF;            
            //Serial.print("Read byte 0x"); Serial.print(val); Serial.print(" at index "); Serial.print(curAddr - addr); Serial.print(" from address 0x");	  Serial.println(curAddr, HEX);
           
        #endif
        curAddr++;
    }
    DeviceOff();
	return curAddr - addr;
}

uint16_t SRAM::WriteBytes(uint32_t addr, uint8_t *data, uint32_t length, BusyType busyType)
{
    //write buffer to screen
    #if defined(DEBUG_SRAM)
    unsigned long startTime, runTime;
    #endif
    while(Busy(busyType)); //wait for screen to not be drawing
    #if defined(DEBUG_SRAM)
    startTime = micros();
    #endif
    DeviceWrite();
    uint16_t idx = 0;
    SetAddress(addr);
    while(idx < length){
        
        SetRow(addr);
        SetDataLines(data[idx]);
        PIOA->PIO_SODR = PIO_PA29;
        NOP;
        idx++;
        addr++;   
        PIOA->PIO_CODR = PIO_PA29;
        
        // digitalWrite(PIN_WE, HIGH);
        // digitalWrite(PIN_WE, LOW);
            
    } ;
    DeviceOff();	
    #if defined(DEBUG_SRAM)
    runTime = micros() - startTime;
        Serial.print("Wrote "); Serial.print(length); Serial.print(" bytes to screen in "); Serial.print(runTime); Serial.println(" microseconds");
    #endif
    return idx;
}

uint16_t SRAM::FillBytes(uint32_t startAddr, uint8_t data, uint32_t length, BusyType busyType)
{
    //write buffer to screen
    #if defined(DEBUG_SRAM)
    unsigned long startTime, runTime;
    #endif
    while(Busy(busyType)); //wait for screen to not be drawing
    #if defined(DEBUG_SRAM)
    startTime = micros();
    #endif
    DeviceWrite();
    uint16_t idx = 0;
    uint32_t addr = startAddr;
    SetDataLines(data);
    SetAddress(addr);   
    while(idx < length){
        SetRow(addr);     
        PIOA->PIO_SODR = PIO_PA29; // D4 WE
        NOP;
        NOP;
        NOP;
        PIOA->PIO_CODR = PIO_PA29; // D4 WE
        idx++;
        addr++; 
              
    } ;
    DeviceOff();	
    #if defined(DEBUG_SRAM)
    runTime = micros() - startTime;
        Serial.print("Filled "); Serial.print(length); Serial.print(" bytes to screen in "); Serial.print(runTime); Serial.println(" microseconds");
    #endif
    return idx;
}

void SRAM::Erase(uint32_t startAddress, uint32_t length)
{
    //Serial.print(F("Erasing RAM from 0x"));Serial.print(startAddress, HEX); Serial.print(F(" to 0x")); Serial.print(startAddress + length,HEX);
    // unsigned long startTime = millis();
    uint32_t pos = 0;
    uint32_t idx = 0;
    uint32_t minLegth = 0;

    SetDataLines(ERASE_BYTE);
    DeviceWrite();
    for(pos = startAddress; pos < startAddress + length ;){  
        idx = 0;
        minLegth =  min(BUFFER_SIZE, length - pos);
    
        while(idx <  minLegth){
            SetAddress(pos + idx);           
            PIOA->PIO_SODR = PIO_PA29; // D4 WE
            NOP;
            NOP;
            PIOA->PIO_CODR = PIO_PA29; // D4 WE
            idx++;                       
        };       
        pos += minLegth;
    }
    DeviceOff();	
    SetAddress(0);
    //Serial.print(F(" : Done in ")); Serial.print((millis() - startTime));Serial.println(" ms.");
}


bool SRAM::WriteByte(uint32_t addr, uint8_t data, uint8_t retryCount, BusyType busyType) {
	_retries = 0;
	bool done = false;
    SetAddress(addr);
    while(Busy(busyType)); //wait for screen to not be drawing
    DeviceWrite();
	//while (_retries <= retryCount && !done) {		        
        
		SetDataLines(data);
		//toggle WE low for 100ns - 1000ns
		PIOA->PIO_SODR = PIO_PA29; // D4 WE
        NOP;
        PIOA->PIO_CODR = PIO_PA29; // D4 WE
		


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
		delayMicroseconds(1);
		digitalWrite(PIN_WE, HIGH);
        delayMicroseconds(1);
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