#include <Arduino.h>
#include "UI/Console/Console.hpp"
#include "UI/Editor/Editor.hpp"

#include "UI/UI.h"
#include <SPI.h>
#include "SD.h"


SRAM programmer;
ProgramRom programRom;
Console console;
Editor editor;
USBHost usb;
//KeyboardController keyboardUsb(usb);

ps2KeyboardController ps2Controller;
//extern UI ui;


void setup() {
    Serial.begin(115200);
    graphics.begin();
    pinMode(PIN_LED, OUTPUT);
    
    keyboard.begin(Serial,50);   
    //ps2Controller.begin();

	digitalWrite(PIN_LED, LOW);
	Serial.println("");
	Serial.println("Starting SRAM tool");
    graphics.begin();
    editor.clear();    
    if(!SD.begin(10)){        
        Serial.println("Failed to start SD");
    }
    ui.begin();
}


void loop() {    
    ui.PrintMenu();
	ui.ProcessInput();
    if(console.IsConsoleRunning())
        console.loop();

    if(editor.IsEditorRunning())
        editor.loop();
}
