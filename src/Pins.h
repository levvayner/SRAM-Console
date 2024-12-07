#ifndef _PINS_H
#define _PINS_H

#pragma region Pins
#ifdef ARDUINO_SAM_DUE
//address pins
#define PIN_ADDR0	51
#define PIN_ADDR1	50
#define PIN_ADDR2	49
#define PIN_ADDR3	48
#define PIN_ADDR4	47
#define PIN_ADDR5	46
#define PIN_ADDR6	45
#define PIN_ADDR7	44
#define PIN_ADDR8	25
#define PIN_ADDR9	26
#define PIN_ADDR10	27
#define PIN_ADDR11	28
#define PIN_ADDR12	A8
#define PIN_ADDR13	A9
#define PIN_ADDR14	A10
#define PIN_ADDR15	A11


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
#define PIN_CE		3	//ACTIVE HIGH	- pull down to use chip.
#define PIN_WE		4	//ACTIVE HIGH	- pull down for 100ns - 1000ns *AFTER* setting address and data. OE should be HIGH. CE should be HIGH.
#define PIN_OE		2	//ACTIVE HIGH	- pull down *AFTER* setting address and data. WE should be HIGH. CE should be HIGH.

#define PS2_DATA 18
#define PS2_CLK  19

//#define SCREEN_OUTPUT 19 //ACTIVE LOW, when low, cannot write to ram!

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


//#define PIN_LED		13




#endif