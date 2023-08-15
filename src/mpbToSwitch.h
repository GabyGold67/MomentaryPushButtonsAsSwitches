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
    bool updValidPressPend();
    bool begin(unsigned long int pollDelayMs = 5);
};

class VdblMPBttn: public DbncdDlydMPBttn{
    //static void mpbPollCallback(TimerHandle_t mpbTmrCb);
protected:
    bool _isEnabled{true};
    bool _isOnDisabled{false};
    bool _isVoided{false};
public:
    VdblMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0, bool isOnDisabled = false);
    bool getIsVoided();
    bool setIsVoided(bool voidValue);
    bool getIsEnabled();
    bool setIsEnabled(bool enabledValue);
    bool enable();
    bool disable();

    virtual bool updIsVoided() = 0;
};

class TmVdblMPBttn: public VdblMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);
protected:
    unsigned long int _voidTime;
    unsigned long int _voidTmrStrt{0};

public:
    TmVdblMPBttn(uint8_t mpbttnPin, unsigned long int voidTime, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0, bool isOnDisabled = false);
    bool begin(unsigned long int pollDelayMs = 5);
    bool updIsVoided();
    bool updIsOn();
};

class LtchMPBttn: public DbncdDlydMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCb);

protected:
    bool _releasePending{false};
    bool _unlatchPending{false};

public:
    LtchMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0);
    bool begin(unsigned long int pollDelayMs = 5);
    bool setUnlatchPend();
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
    bool _unltchPulledUp{};
    bool _unltchTypeNO{};
    DbncdDlydMPBttn _unLtchBttn;

public:
    XtrnUnltchMPBttn(uint8_t mpbttnPin, uint8_t unltchPin, 
        bool pulledUp = true, bool typeNO = true, unsigned long int dbncTimeOrigSett = 0, unsigned long int strtDelay = 0,
        bool upulledUp = true, bool utypeNO = true, unsigned long int udbncTimeOrigSett = 0, unsigned long int ustrtDelay = 0);
    bool begin(unsigned long int pollDelayMs = 5);
    bool updUnlatchPend();

};

#endif
