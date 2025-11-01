#include <Arduino.h>
#include "UI/Console/Console.hpp"
#include "UI/Editor/Editor.hpp"

#include "UI/UI.h"
#include <SPI.h>
#include "SD.h"

#define USE_USB_MOUSE 0 // usb.Task slows response time considerably

SRAM programmer;
ProgramRom programRom;
Console console;
Editor editor;
//USBHost usb;
//KeyboardController keyboardUsb(usb);

ps2KeyboardController ps2Controller;
//extern UI ui;
uint8_t steps = 0;

void setup() {
    Serial.begin(115200);
    gpu.begin();    
    pinMode(PIN_LED, OUTPUT);
    
    keyboard.begin(Serial,50);
    //ps2Controller.begin();
    mouse.begin();
    
	digitalWrite(PIN_LED, LOW);
    if(Serial.availableForWrite()){
        Serial.println("");
        Serial.println("Starting SRAM tool");
    }
    //graphics.begin();
    editor.clear();    
    if(!SD.begin(10)){        
        if(Serial.availableForWrite()){
            Serial.println("Failed to start SD");
        }
    }
    ui.begin();
    gpu.PrintRam(Serial);
}

static bool isEditorRunningLast = false, isConsoleRunningLast = false;

void loop() {   
    #if defined(USE_USB_MOUSE) && USE_USB_MOUSE > 0
    if(steps == 0) 
        usb.Task(); 
    steps++; 
    #endif
    ui.PrintMenu();
	ui.ProcessInput();
    if(console.IsConsoleRunning())
        console.loop();

    
    if(editor.IsEditorRunning())
        editor.loop();
    if(isEditorRunningLast && !editor.IsEditorRunning()){
        editor.save();
        //ui.begin();
        //console.run(false);
    }
    isEditorRunningLast = editor.IsEditorRunning();
    mouse.update(); 
}
