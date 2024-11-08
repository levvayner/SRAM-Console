#include <Arduino.h>
#include "SRAM/SRAM.h"
#include "UI/Console.h"
#include "UI/UI.h"


SRAM programmer = SRAM();
ProgramRom programRom = ProgramRom();
Console console;
UI ui = UI();

void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_OE, OUTPUT);
    #ifdef PIN_CE
	pinMode(PIN_CE, OUTPUT);
    #endif
	pinMode(PIN_WE, OUTPUT);
	//start with chip in neither read nor write.
	programmer.ModeOff();

	digitalWrite(PIN_LED, LOW);
	Serial.println("");
	Serial.println("Starting SRAM tool");
}


void loop() {
    ui.PrintMenu();
	delay(5);
	if (Serial.available()) {
		ui.ProcessInput();
	}
}
