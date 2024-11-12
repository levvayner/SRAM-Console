#include <Arduino.h>
#include "SRAM/SRAM.h"
#include "SRAM/VRAM.h"
#include "UI/Console.h"
#include "UI/UI.h"



SRAM programmer = SRAM();
VRAM graphics = VRAM();
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
	programmer.DeviceOff();

	digitalWrite(PIN_LED, LOW);
	Serial.println("");
	Serial.println("Starting SRAM tool");
    console.clear();
    console.write("Welcome");
}


void loop() {
    
    ui.PrintMenu();
	delay(5);
	if (Serial.available()) {
		ui.ProcessInput();
	}
}
