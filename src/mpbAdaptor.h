#ifndef _MPBADAPTOR_H_
#define _MPBADAPTOR_H_

#include "Arduino.h"

#define _HWMinDbncTime 20

class DbncdMPBttn{
protected:
    uint8_t _mpbttnPin{};
    bool _pulledUp{};
    bool _typeNO{};
    unsigned long int _dbncTimeOrigSett{};

    bool _isHit{false};
    bool _wasHit{false};
    bool _isPressed{false};
    unsigned long int _dbncTimeTempSett{};
    unsigned long int _dbncTimerStrt{};
    const unsigned long int _stdMinDbncTime {_HWMinDbncTime};

    TimerHandle_t mpbPollTmr {nullptr};

    bool getIsHit();

public:    
    DbncdMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0);
    unsigned long int getCurDbncTime();
    bool getIsPressed ();
    bool resetDbncTime();
    bool setDbncTime(const unsigned long int &newDbncTime);
    void updIsHit();
    bool updIsPressed();

    bool begin(TickType_t pollDelay = 1000);
    bool pause();
    bool resume();
    bool end();

    static void mpbPollCallback(TimerHandle_t mpbTmrCb);

};

class DbncdDlydMPBttn: public DbncdMPBttn{
private:    
    unsigned long int _strtDelay;
public:
    unsigned long int getStrtDelay();
    boolean setStrtDelay(unsigned long int newStrtDelay);
    bool updIsPressed();
};

class SnglSrvcMPBttn: public DbncdMPBttn{
protected:
    bool _released{true};
    bool _servicePend{false};

public:
    SnglSrvcMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTime = 0);
    bool getSrvcPend(); 
    bool notifySrvd ();
    bool updIsPressed();

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
