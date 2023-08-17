#include <Arduino.h>
#include <mpbToSwitch.h>

//1DbncdDlyd_1TmLtchMPBttn_InTasks.ino

// put Types definitions here:
struct bttnAsArg{
  DbncdMPBttn* bttnArg;
  uint8_t outLoadPinArg;
};

struct bttnNWarnAsArg{
  TmLtchMPBttn* bttnArg;
  uint8_t outLoadPinArg;
  uint8_t outWarnPinArg;
};
// put function declarations here:
static void updLEDStruc(void* argp);
static void updLEDnWrnStruc(void* argp);

// put Global declarations here: 
const uint8_t xumpbSwitchPin{GPIO_NUM_25};
const uint8_t blueLed{GPIO_NUM_21};
const uint8_t redSwitch{GPIO_NUM_26};
const uint8_t redLed{GPIO_NUM_19};
const uint8_t wnngLed{GPIO_NUM_15};

TmLtchMPBttn blueBttn (xumpbSwitchPin, 4000, 25, true, true, 20, 50);
DbncdMPBttn redBttn (redSwitch);

bttnNWarnAsArg blueBttnArg {&blueBttn, blueLed, wnngLed};
bttnAsArg redBttnArg {&redBttn, redLed};

void setup() {
  // put your setup code here, to run once:
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;

  pinMode(blueLed, OUTPUT);
  blueBttn.setWnngPinOut(wnngLed);
  TaskHandle_t blueBttnHndl;
  blueBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDnWrnStruc,  //function to be called
          "UpdBlueLed",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &blueBttnArg,  //Pointer to the parameters for the function to work with, change to &blueBttnArg
          1,      //Priority level given to the task
          &blueBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(blueBttnHndl);

  pinMode(redLed, OUTPUT);
  redBttn.begin();
  TaskHandle_t redBttnHndl;

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
          "UpdRedLed",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &redBttnArg,  //Pointer to the parameters for the function to work with
          1,      //Priority level given to the task
          &redBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(redBttnHndl);
}

void loop() {
  //Now unneeded as all runs as independent tasks! Delete the loop() task
  vTaskDelete(nullptr);
}


// put function definitions here:
static void updLEDStruc(void* argp){
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

static void updLEDnWrnStruc(void* argp){
  bttnNWarnAsArg *myMPB = (bttnNWarnAsArg*)argp;

  for (;;){
    if (myMPB->bttnArg->getIsOn()){
      //Turn on the Load
      digitalWrite(myMPB->outLoadPinArg, HIGH);
    }
    else{
        //Turn off the Load
        digitalWrite(myMPB->outLoadPinArg, LOW);
    }

    if (myMPB->bttnArg->getWrnngOn()){
      //turn WrnngPin on
      digitalWrite(myMPB->bttnArg->getWrnngPin(), HIGH);
    }
    else{
      //turn WrnngPin off
      digitalWrite(myMPB->bttnArg->getWrnngPin(), LOW);
    }

  }

}
