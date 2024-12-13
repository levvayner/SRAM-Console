#ifndef _PINS_H
#define _PINS_H

#pragma region Pins
#define PIN_CE		3	//ACTIVE HIGH	- pull down to use chip.
#define PIN_WE		4	//ACTIVE HIGH	- pull down for 100ns - 1000ns *AFTER* setting address and data. OE should be HIGH. CE should be HIGH.
#define PIN_OE		2

#define PS2_DATA 18
#define PS2_CLK  19
#define PIN_RESOLUTION 5 //SET Resolution

#endif