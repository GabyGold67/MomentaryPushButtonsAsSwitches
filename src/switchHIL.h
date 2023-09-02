#ifndef _SWITCHHIL_H_
#define _SWITCHHIL_H_

#include <Arduino.h>
#include <mpbToSwitch.h>

#define MAX_SWITCHES 25
#define MIN_BLINK_RATE 100

//Declare function prototypes
static void updSwitchStatus(void* argp);
//static void createSwitchTask();

// BaseType_t rc;
// TaskHandle_t sHilTskHndl = nullptr;  //TaskHandle to the updating task that manages the switch outputs by calling each switch own updOutputs() method
// uint8_t _exePrty {1};   //Execution priority of the updating Task

//Global counter of all instantiated HILSwitches objects
static uint8_t switchesCount = 0;

class HILSwitches{  //Virtual class used as superclass of all the switches, one of the solutions to manage several switch subclasses through it
  //static uint8_t switchesCount;
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

//Create the task callback function that keeps the physical switch state updated: Load and Auxiliary pins
// static void updSwitchStatus(void* argp){
//   HILSwitches *mySwtch = (HILSwitches*)argp;  //Parameter casting: the parameter passed by the task is casted to its real type

//   for (;;){
//     mySwtch->updOutputs();
    
//     digitalWrite(19, HIGH);
//     vTaskDelay(250/portTICK_PERIOD_MS);
//     digitalWrite(19, LOW);
//     vTaskDelay(250/portTICK_PERIOD_MS);
//   }
// }



#endif