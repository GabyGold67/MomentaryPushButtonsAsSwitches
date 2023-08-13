#ifndef _MPBTOSWITCH_H_
#define _MPBTOSWITCH_H_

#include "Arduino.h"

#define _HwMinDbncTime 20   //Documented minimum wait time for a MPB signal to stabilize
class DbncdMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);
protected:
    uint8_t _mpbttnPin{};
    bool _pulledUp{};
    bool _typeNO{};
    unsigned long int _dbncTimeOrigSett{};

    volatile bool _isPressed{false};
    volatile bool _wasPressed{false};
    volatile bool _validPressPend{false};
    volatile bool _isOn{false};
    unsigned long int _dbncTimeTempSett{};
    unsigned long int _dbncTimerStrt{};
    const unsigned long int _stdMinDbncTime {_HwMinDbncTime};

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
    bool updValidPressPend();

    bool begin(unsigned long int pollDelayMs = 5);
    bool pause();
    bool resume();
    bool end();

};

class DbncdDlydMPBttn: public DbncdMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);
protected:    
    unsigned long int _strtDelay {0};
public:
    DbncdDlydMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0);
    unsigned long int getStrtDelay();
    boolean setStrtDelay(unsigned long int newStrtDelay);
    bool updIsOn();
    bool updIsPressed();
    bool updValidPressPend();
    bool begin(unsigned long int pollDelayMs = 5);
};

class LtchMPBttn: public DbncdDlydMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);

protected:
    bool _releasePending{false};
    bool _unlatchPending{false};

public:
    LtchMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0);
    bool begin(unsigned long int pollDelayMs = 5);
    bool updIsOn();
    bool updIsPressed();
    bool updUnlatchPend();
    bool updValidPressPend();

};

class TmLtchMPBttn: public LtchMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);

protected:
    uint8_t _wnngPinOut {0};
    bool _tmRstbl {true};
    bool _wrnngOn {false};
    unsigned int _wrnngPrctg {0};
    unsigned long int _wrnngMs{0};
    unsigned long int _srvcTime {};
    unsigned long int _srvcTimerStrt{0};
    bool _unlatchPending{false};

public:
    TmLtchMPBttn(uint8_t mpbttnPin, unsigned long int actTime, unsigned int wrnngPrctg = 0, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0);
    bool begin(unsigned long int pollDelayMs = 5);
    bool getWrnngOn();
    uint8_t getWrnngPin();
    bool setTmerRstbl(bool isRstbl);
    bool setWnngPinOut(uint8_t wrnngPinOut);
    bool updIsOn();
    bool updIsPressed();
    bool updValidPressPend();
    bool updUnlatchPend();
    bool updWrnngOn();

};

class XtrnUnltchMPBttn: public LtchMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);

protected:
    uint8_t _unltchPinIn {};
    bool _unltchPulledUp{};
    bool _unltchTypeNO{};


public:
    XtrnUnltchMPBttn(uint8_t mpbttnPin, uint8_t unltchPinIn, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0);
    bool begin(unsigned long int pollDelayMs = 5);

};


/*
class SnglSrvcMPBttn: public DbncdDlydMPBttn{
protected:
    bool _released{true};
    bool _servicePend{false};

public:
    SnglSrvcMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0);
    bool getSrvcPend(); 
    bool notifySrvd ();
    bool updIsOn();

};

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
