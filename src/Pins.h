#ifndef _PINS_H
#define _PINS_H

#pragma region Pins

//address pins
#define PIN_ADDR0	38
#define PIN_ADDR1	36
#define PIN_ADDR2	34
#define PIN_ADDR3	32
#define PIN_ADDR4	30
#define PIN_ADDR5	28
#define PIN_ADDR6	26
#define PIN_ADDR7	24
#define PIN_ADDR8	22
#define PIN_ADDR9	23
#define PIN_ADDR10	25
#define PIN_ADDR11	27
#define PIN_ADDR12	29
#define PIN_ADDR13	31
#define PIN_ADDR14	33
#define PIN_ADDR15	35


//data pins
#define PIN_DATA0	53
#define PIN_DATA1	51
#define PIN_DATA2	49
#define PIN_DATA3	47
#define PIN_DATA4	45
#define PIN_DATA5	43
#define PIN_DATA6	41
#define PIN_DATA7	39


//control pins
#define PIN_CE		52	//ACTIVE LOW	- pull down to use chip.
#define PIN_WE		46	//ACTIVE LOW	- pull down for 100ns - 1000ns *AFTER* setting address and data. OE should be HIGH. CE should be HIGH.
#define PIN_OE		42	//ACTIVE LOW	- pull down *AFTER* setting address and data. WE should be HIGH. CE should be HIGH.
#define PIN_LED		13
#pragma endregion



#endif