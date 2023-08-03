#include <Arduino.h>
#include <mpbAdaptor.h>

//1Dbncd1Dlyd.ino


// put function declarations here:
//int myFunction(int, int);

const uint8_t redLed{GPIO_NUM_19};
const uint8_t blueLed{GPIO_NUM_21};
const uint8_t blueSwitch{GPIO_NUM_25};
const uint8_t redSwitch{GPIO_NUM_26};

LtchMPBttn blueBttn (blueSwitch, true, true, 20, 50);
DbncdMPBttn redBttn (redSwitch);

void setup() {
  // put your setup code here, to run once:
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  blueBttn.begin();
  redBttn.begin();

}

void loop() {
  if (blueBttn.getIsOn()){
    digitalWrite(blueLed, HIGH);
  }
  else{
    digitalWrite(blueLed, LOW);
  }

  if (redBttn.getIsOn()){
    digitalWrite(redLed, HIGH);
  }
  else {
    digitalWrite(redLed, LOW);
  }
      
}
  

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }