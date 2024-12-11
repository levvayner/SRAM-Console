#include <Arduino.h>
#include "SRAM/SRAM.h"
#include "SRAM/VRAM.h"
#include "UI/Editor.hpp"
#include "PS2KeyAdvanced.h"
#include "UI/UI.h"
#include "IO/ps2KeyboardController.h"



SRAM programmer = SRAM();
VRAM graphics = VRAM();
ProgramRom programRom = ProgramRom();
Console console;
Editor editor;
USBHost usb;
KeyboardController keyboardUsb(usb);
PS2KeyAdvanced keyboardPs2;
ps2KeyboardController ps2Controller;
UI ui = UI();

// This function intercepts key press
void keyPressed() {
    if(editor.IsConsoleRunning())
        editor.processUSBKey();
  Serial.print("Pressed:  ");
}

// This function intercepts key release
void keyReleased() {
  Serial.print("Released: ");
}


void setup() {
    Serial.begin(115200);
    programmer.begin();
    pinMode(PIN_LED, OUTPUT);
    
    ps2Controller.begin();

	digitalWrite(PIN_LED, LOW);
	Serial.println("");
	Serial.println("Starting SRAM tool");
    graphics.begin();
    editor.clear();    

}


void loop() {    
    ui.PrintMenu();
	ui.ProcessInput();
}
