#include <Arduino.h>
#include <mpbToSwitch.h>

//1Dbncd_1LtchMPBttn_NoTasks.ino

const uint8_t dbncdLoad{GPIO_NUM_19};
const uint8_t ltchdLed{GPIO_NUM_21};
const uint8_t ltchdSwitchPin{GPIO_NUM_25};
const uint8_t dbncdSwitchPin{GPIO_NUM_26};

LtchMPBttn ltchdBttn (ltchdSwitchPin, true, true, 20, 50);
DbncdMPBttn dbncdBttn (dbncdSwitchPin);

void setup() {
  // put your setup code here, to run once:
  pinMode(dbncdLoad, OUTPUT);
  pinMode(ltchdLed, OUTPUT);

  ltchdBttn.begin();
  dbncdBttn.begin();
}

void loop() {
  if (ltchdBttn.getIsOn()){
    digitalWrite(ltchdLed, HIGH);
  }
  else{
    digitalWrite(ltchdLed, LOW);
  }

  if (dbncdBttn.getIsOn()){
    digitalWrite(dbncdLoad, HIGH);
  }
  else {
    digitalWrite(dbncdLoad, LOW);
  }      
}
