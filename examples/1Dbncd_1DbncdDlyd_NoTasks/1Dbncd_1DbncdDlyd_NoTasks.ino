#include <Arduino.h>
#include <mpbToSwitch.h>

//1Dbncd_1DbncdDlyd_NoTasks.ino

const uint8_t ddmpbLoadPin{GPIO_NUM_19};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t ddmpbSwitchPin{GPIO_NUM_26};

DbncdDlydMPBttn blueBttn (dmpbSwitchPin, true, true, 20, 1000);
DbncdMPBttn redBttn (ddmpbSwitchPin);

const unsigned int toggleCount {5};

unsigned int redCount {0};
bool redFlips{false};
unsigned int blueAloneCount {0};
bool blueFlips{false};

void setup() {
  pinMode(ddmpbLoadPin, OUTPUT);
  pinMode(dmpbLoadPin, OUTPUT);

  blueBttn.begin();
  redBttn.begin();
}

void loop() {
  if (blueBttn.getIsOn()){
    digitalWrite(dmpbLoadPin, HIGH);
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
    digitalWrite(dmpbLoadPin, LOW);
    if(blueFlips){
      blueFlips = false;
    }      
  }

  if (redBttn.getIsOn()){
    digitalWrite(ddmpbLoadPin, HIGH);
    if (!redFlips){
      redFlips = true;
      redCount++;
    }
  }
  else {
    digitalWrite(ddmpbLoadPin, LOW);
    if(redFlips){
      redFlips = false;  
      if (redCount == toggleCount){
        redBttn.pause();
      }
    }      
  }
}  
