#include <Arduino.h>
#include <mpbToSwitch.h>

//1Dbncd_1DbncdDlyd_NoTasks.ino

const uint8_t redLed{GPIO_NUM_19};
const uint8_t blueLed{GPIO_NUM_21};
const uint8_t blueSwitch{GPIO_NUM_25};
const uint8_t redSwitch{GPIO_NUM_26};

DbncdDlydMPBttn blueBttn (blueSwitch, true, true, 20, 1000);
DbncdMPBttn redBttn (redSwitch);

const unsigned int toggleCount {5};

unsigned int redCount {0};
bool redFlips{false};
unsigned int blueAloneCount {0};
bool blueFlips{false};

void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  blueBttn.begin();
  redBttn.begin();
}

void loop() {
  if (blueBttn.getIsOn()){
    digitalWrite(blueLed, HIGH);
    if (!blueFlips){
      if(redCount == toggleCount){
        blueFlips = true;
        blueAloneCount ++;
        if(blueAloneCount == toggleCount){
          redCount = 0;
          redFlips = false;
          blueAloneCount = 0;
          redBttn.resume();
        }
      }
    }
  }
  else{
    digitalWrite(blueLed, LOW);
    if(blueFlips){
      blueFlips = false;
    }      
  }

  if (redBttn.getIsOn()){
    digitalWrite(redLed, HIGH);
    if (!redFlips){
      redFlips = true;
      redCount++;
    }
  }
  else {
    digitalWrite(redLed, LOW);
    if(redFlips){
      redFlips = false;  
      if (redCount == toggleCount){
        redBttn.pause();
      }
    }      
  }
}  
