#ifndef _SWITCHHIL_H_
#define _SWITCHHIL_H_

#include <Arduino.h>
#include <mpbToSwitch.h>

#define MAX_SWITCHES 25
#define MIN_BLINK_RATE 100

// BaseType_t rc;
// TaskHandle_t sHilTskHndl = nullptr;  //TaskHandle to the updating task that manages the switch outputs by calling each switch own updOutputs() method
// uint8_t _exePrty {1};   //Execution priority of the updating Task

//Global variables needed for this file (declaring them at the end of file produces "was not declared in this scope" errors)
static uint8_t switchesCount = 0; //Counter of all instantiated HILSwitches objects

//Classes definitions
class HILSwitches{  //Virtual class used as superclass of all the switches, one of the solutions to manage several switch subclasses through it
  static void swtchsTaskCallback(void* argp);

protected:
      uint8_t _loadPin{};
public:
  HILSwitches();
  virtual bool updOutputs() = 0;  //Makes it a virtual class and forces all subclasses to  implement this method, used to update outputs states
};

class StrcsTmrSwitch: public HILSwitches{
protected:
    HntdTmLtchMPBttn* _underlMPB;
    uint8_t _loadPin{};
    uint8_t _wrnngPin{};
    uint8_t _pltPin{};

    bool _actvWrnng{false};
    bool _actvPlt {false};
    bool _wrnngBlnks{false};
    unsigned long int _wrnngBlnkRate{250};
    unsigned long int _wrnngLstSwp{0};
    bool _blnkOutputOn{true};
public:
    StrcsTmrSwitch(HntdTmLtchMPBttn &lgcMPB, uint8_t loadPin,uint8_t wnngPin = 0, uint8_t pltPin = 0);
    bool updOutputs();
    bool setActvPilot(bool actvPilot);
    bool setActvWarning(bool actvWarning);
    const bool getActvPilot() const;
    const bool getActvWarning() const;
    HntdTmLtchMPBttn* getUnderlMPB();
    bool setBlnkWrnng(bool newBlnkSett);
    bool setBlnkRate(unsigned long int newBlnkRate);
    bool blinkWrnng();
    bool noBlinkWrnng();    
};

class DbncdDlydSwitch: public HILSwitches{
protected:
  DbncdDlydMPBttn* _underlMPB;
  uint8_t _loadPin{};
public:
  DbncdDlydSwitch(DbncdDlydMPBttn &lgcMPB, uint8_t loadPin);
  bool updOutputs();
  DbncdDlydMPBttn* getUnderlMPB();
};

//Global variables that need class definitions declared in this file (declaring them before the classes produces "error: '(class)' does not name a type" errors)
//Declare an array of pointers to superclass objects, initialize with nullptr
static HILSwitches *actSwtchsArr[MAX_SWITCHES] {};

// Put function definitions here:
//Fuctions implementation
/*bool createSwitchesUpdateTask(){    //To create the Task that keeps outputs updated
    if (!sHilTskHndl){  //The task has not been created yet, create it through a function that all switch classes must use
        //Call the appropriate function that creates the Task!
        //createSwitchesUpdateTask();
          int app_cpu = xPortGetCoreID();

        if (!sHilTskHndl){  //The task has not been created yet, create it through a function that all switch classes must use

            //Set the task to keep the outputs updated and set the function name to the updater function
            rc = xTaskCreatePinnedToCore(
            updSwitchStatus,  //function to be called
            "UpdSwitchOutputs",  //Name of the task
            2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
            &actSwtchsArr,  //Pointer to the parameters for the function to work with
            _exePrty,      //Priority level given to the task
            &sHilTskHndl, //Task handle
            app_cpu //Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
            );
        assert(rc == pdPASS);
        assert(sHilTskHndl);
        }

    }

  return true;
}*/

//Create the task callback function that keeps the physical switch state updated: Load and Auxiliary pins
// static void udpSwitchStatus(void* argp){
//   HILSwitches *mySwtch = (HILSwitches*)argp;  //Parameter casting: the parameter passed by the task is casted to its real type

//   for (;;){
//       mySwtch->updOutputs();
//   }
// }

//Alternative testing task callback function
static void swtchsTaskCallback(void* argp){
  HILSwitches *mySwtch = (HILSwitches*)argp;  //Parameter casting: the parameter passed by the task is casted to its real type

  for (;;){
    for(int i{0}; i < switchesCount; i++){
      // actSwtchsArr[i]->updOutputs();
    }
  }
}

#endif