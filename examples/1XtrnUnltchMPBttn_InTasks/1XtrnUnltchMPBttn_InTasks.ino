#include <Arduino.h>
#include <mpbToSwitch.h>

/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1XtrnUnltchMPBttn_InTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, August 17, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

  Example file to demonstrate XtrnUnltchMPBttn class, required hardware and connections:
  _ 1 push button between GND and xumpSwitchPin
  _ 1 push button between GND and releaseSwitch
  _ 1 led with it's corresponding resistor between GND and loadPin

  Pressing the push button connected to xumpSwitch will turn the led on and keep it lit after it is released until the push button connected to releaseSwitch is pushed
*/

// put Types definitions here:
struct bttnAsArg{   //xTaskCreate accepts only one pointer to a parameter, to use a composed data type a structure is built
  XtrnUnltchMPBttn* bttnArg;  //The switch created using the library
  uint8_t outLoadPinArg;      //The pin where the load (light, valve, lock) will be activated
};

// put function declarations here:
static void updOutPin(void* argp);

// put Global declarations here: 
const uint8_t xumpSwitchPin{GPIO_NUM_25};
const uint8_t loadPin{GPIO_NUM_21};
const uint8_t releaseSwitch{GPIO_NUM_26};

XtrnUnltchMPBttn xumpBttn (xumpSwitchPin, releaseSwitch, 4000, 25, true, true, 20, 50);

bttnAsArg xumpBttnArg {&xumpBttn, loadPin};

void setup() {
  // put your setup code here, to run once:
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;
  
  pinMode(loadPin, OUTPUT);
  TaskHandle_t xumpBttnHndl;
  xumpBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updOutPin,  //function to be called
          "UpdateOutputPin",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &xumpBttnArg,  //Pointer to the parameters for the function to work with, change to &blueBttnArg
          1,      //Priority level given to the task
          &xumpBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(xumpBttnHndl);

}

void loop() {
  //Now unneeded as all runs as independent tasks! Delete the loop() task
  vTaskDelete(nullptr);
}
  

// put function definitions here:
static void updOutPin(void* argp){
  bttnAsArg *myMPB = (bttnAsArg*)argp;

  for (;;){
    if (myMPB->bttnArg->getIsOn()){
      digitalWrite(myMPB->outLoadPinArg, HIGH);
    }
    else{
      digitalWrite(myMPB->outLoadPinArg, LOW);
    }
  }
}
