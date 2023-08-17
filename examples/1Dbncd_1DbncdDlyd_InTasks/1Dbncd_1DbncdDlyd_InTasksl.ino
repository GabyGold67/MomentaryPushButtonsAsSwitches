#include <Arduino.h>
#include <mpbToSwitch.h>

/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1Dbncd_1DbncdDlyd_InTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, August 17, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

  Example file to demonstrate DbncdMPBttn and DbncdDlydMPBttn classes, required hardware and connections:
  _ 1 push button between GND and dmpbSwitchPin
  _ 1 push button between GND and ddmpbSwitchPin
  _ 1 led with it's corresponding resistor between GND and dmpbLoadPin
  _ 1 led with it's corresponding resistor between GND and loadPin

  Pressing the push button connected to dmpbSwitchPin will turn the led immediately on and keep it lit while it's being pressed
  Pressing the push button connected to ddmpbSwitchPin will turn the led on after a 500 miliseconds  delay and keep it lit while it's being pressed
*/

// put Types definitions here:
struct bttnAsArg{
  DbncdMPBttn* bttnArg;
  uint8_t outLoadPinArg;
};

// put function declarations here:
static void updOutPin(void* argp);

// put Global declarations here: 
const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t ddmpbSwitchPin{GPIO_NUM_26};
const uint8_t ddmpbLoadPin{GPIO_NUM_19};

DbncdMPBttn dmpbBttn (dmpbSwitchPin);
DbncdDlydMPBttn ddmpbBttn (ddmpbSwitchPin, true, true, 0, 500);

bttnAsArg dmpbBttnArg {&dmpbBttn, dmpbLoadPin};
bttnAsArg ddmpbBttnArg {&ddmpbBttn, ddmpbLoadPin};

void setup() {
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;
  
  pinMode(dmpbLoadPin, OUTPUT);
  TaskHandle_t dmpbBttnHndl = nullptr;
  dmpbBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updOutPin,  //function to be called
          "Update dmpbLoadPin",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &dmpbBttnArg,  //Pointer to the parameters for the function to work with, change to &blueBttnArg
          1,      //Priority level given to the task
          &dmpbBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(dmpbBttnHndl);
  
  pinMode(ddmpbLoadPin, OUTPUT);
  ddmpbBttn.begin();
  TaskHandle_t ddmpbBttnHndl = nullptr;

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updOutPin,  //function to be called
          "Update ddmpbLoadPin",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &ddmpbBttnArg,  //Pointer to the parameters for the function to work with
          1,      //Priority level given to the task
          &ddmpbBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(ddmpbBttnHndl);
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
