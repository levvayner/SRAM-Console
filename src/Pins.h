#ifndef _PINS_H
#define _PINS_H

#pragma region Pins
#ifdef ARDUINO_SAM_DUE
//address pins
#define PIN_ADDR0	19
#define PIN_ADDR1	18
#define PIN_ADDR2	17
#define PIN_ADDR3	16
#define PIN_ADDR4	23
#define PIN_ADDR5	24
#define PIN_ADDR6	A0
#define PIN_ADDR7	SDA1
#define PIN_ADDR8	25
#define PIN_ADDR9	26
#define PIN_ADDR10	27
#define PIN_ADDR11	28
#define PIN_ADDR12	14
#define PIN_ADDR13	15
#define PIN_ADDR14	29
#define PIN_ADDR15	11


//data pins
#define PIN_DATA0	33
#define PIN_DATA1	34
#define PIN_DATA2	35
#define PIN_DATA3	36
#define PIN_DATA4	37
#define PIN_DATA5	38
#define PIN_DATA6	39
#define PIN_DATA7	40
//control pins
#define PIN_CE		52	//ACTIVE HIGH	- pull down to use chip.
#define PIN_WE		44	//ACTIVE HIGH	- pull down for 100ns - 1000ns *AFTER* setting address and data. OE should be HIGH. CE should be HIGH.
#define PIN_OE		42	//ACTIVE HIGH	- pull down *AFTER* setting address and data. WE should be HIGH. CE should be HIGH.


#elif defined(__AVR_MEGA__)
//address pins
#define PIN_ADDR0	23
#define PIN_ADDR1	23
#define PIN_ADDR2	24
#define PIN_ADDR3	25
#define PIN_ADDR4	26
#define PIN_ADDR5	27
#define PIN_ADDR6	28
#define PIN_ADDR7	29
#define PIN_ADDR8	37
#define PIN_ADDR9	36
#define PIN_ADDR10	35
#define PIN_ADDR11	34
#define PIN_ADDR12	33
#define PIN_ADDR13	32
#define PIN_ADDR14	31
#define PIN_ADDR15	30


//data pins
#define PIN_DATA0	49
#define PIN_DATA1	48
#define PIN_DATA2	47
#define PIN_DATA3	46
#define PIN_DATA4	45
#define PIN_DATA5	44
#define PIN_DATA6	43
#define PIN_DATA7	42

//control pins
#define PIN_CE		50	//ACTIVE HIGH	- pull down to use chip.
#define PIN_WE		53	//ACTIVE HIGH	- pull down for 100ns - 1000ns *AFTER* setting address and data. OE should be HIGH. CE should be HIGH.
#define PIN_OE		52	//ACTIVE HIGH	- pull down *AFTER* setting address and data. WE should be HIGH. CE should be HIGH.

#endif


#define PIN_LED		13




#endif