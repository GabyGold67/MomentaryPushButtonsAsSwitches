#ifndef _MPBADAPTOR_H_
#define _MPBADAPTOR_H_

#include "Arduino.h"

#define _HWMinDbncTime 20   //Documented minimum wait time for a MPB signal to stabilize

class DbncdMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);
protected:
    uint8_t _mpbttnPin{};
    bool _pulledUp{};
    bool _typeNO{};
    unsigned long int _dbncTimeOrigSett{};

    volatile bool _isPressed{false};
    volatile bool _wasPressed{false};
    volatile bool _isOn{false};
    unsigned long int _dbncTimeTempSett{};
    unsigned long int _dbncTimerStrt{};
    const unsigned long int _stdMinDbncTime {_HWMinDbncTime};

    TimerHandle_t mpbPollTmrHndl {nullptr};
    char _mpbPollTmrName [17] {'\0'};

    bool getIsPressed();

public:    
    DbncdMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0);
    unsigned long int getCurDbncTime();
    bool getIsOn ();
    bool resetDbncTime();
    bool setDbncTime(const unsigned long int &newDbncTime);
    bool updIsPressed();
    bool updIsOn();

    bool begin(unsigned long int pollDelayMs = 5);
    bool pause();
    bool resume();
    bool end();

};

class DbncdDlydMPBttn: public DbncdMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);
private:    
    unsigned long int _strtDelay;
public:
    DbncdDlydMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0);
    unsigned long int getStrtDelay();
    boolean setStrtDelay(unsigned long int newStrtDelay);
    bool updIsOn();
    bool updIsPressed();
    bool begin(unsigned long int pollDelayMs = 5);
};

class SnglSrvcMPBttn: public DbncdMPBttn{
protected:
    bool _released{true};
    bool _servicePend{false};

public:
    SnglSrvcMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTime = 0);
    bool getSrvcPend(); 
    bool notifySrvd ();
    bool updIsOn();

};

/*
class AutoRptCntlMPBttn: public DbncdMPBttn{
private:
    const unsigned long int _minRptRate = 250;
    bool _autoRptOn;
    bool _released{true};
    bool _rearmed {false};
    bool _servicePend{false};
    unsigned long int _rptRate;

public:
    AutoRptCntlMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTime = 0, unsigned long int rptRate = 0, bool autoRptOn = true);
    bool setAutoRptRate(unsigned long int newRate);
    bool getSrvcPend(); 
    bool notifySrvd ();
    bool updValidPress();

};
*/

#endif
