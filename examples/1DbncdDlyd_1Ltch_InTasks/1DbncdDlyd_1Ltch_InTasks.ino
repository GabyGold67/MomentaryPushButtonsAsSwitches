#include <Arduino.h>
#include <mpbToSwitch.h>

//1DbncdDlyd_1Ltch_InTasks.ino

// put Types definitions here:
struct bttnAsArg{
  DbncdMPBttn* bttnArg;
  uint8_t outLoadPinArg;
};

// put function declarations here:
static void updLEDStruc(void* argp);

// put Global declarations here: 
const uint8_t blueSwitch{GPIO_NUM_25};
const uint8_t blueLed{GPIO_NUM_21};
const uint8_t redSwitch{GPIO_NUM_26};
const uint8_t redLed{GPIO_NUM_19};

LtchMPBttn blueBttn (blueSwitch);
DbncdDlydMPBttn redBttn (redSwitch, true, true, 0, 250);

bttnAsArg blueBttnArg {&blueBttn, blueLed};
bttnAsArg redBttnArg {&redBttn, redLed};

void setup() {
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;
  
  pinMode(blueLed, OUTPUT);
  TaskHandle_t blueBttnHndl = nullptr;
  blueBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
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
  TaskHandle_t redBttnHndl = nullptr;

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
