#ifndef _MPBTOSWITCH_H_
#define _MPBTOSWITCH_H_

#include <Arduino.h>

#define _HwMinDbncTime 20   //Documented minimum wait time for a MPB signal to stabilize
#define _StdPollDelay 10
#define _MinSrvcTime 100

class DbncdMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    uint8_t _mpbttnPin{};
    bool _pulledUp{};
    bool _typeNO{};
    unsigned long int _dbncTimeOrigSett{};

    volatile bool _isPressed{false};
    volatile bool _validPressPend{false};
    volatile bool _isOn{false};
    unsigned long int _dbncTimeTempSett{0};
    unsigned long int _dbncTimerStrt{0};
    const unsigned long int _stdMinDbncTime {_HwMinDbncTime};
    TimerHandle_t _mpbPollTmrHndl {nullptr};
    char _mpbPollTmrName [17] {'\0'};
    volatile bool _outputsChange {false};
    TaskHandle_t _taskToNotifyHndl {nullptr};

    const bool getIsPressed() const;
    bool updIsOn();
    bool updIsPressed();
    bool updValidPressPend();

public:    
    DbncdMPBttn();
    DbncdMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0);
    ~DbncdMPBttn();
    void clrStatus();
    const unsigned long int getCurDbncTime() const;
    const bool getIsOn () const;
    const bool getOutputsChange() const;
    const TaskHandle_t getTaskToNotify() const;
    bool init(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0);
    bool resetDbncTime();
    bool setDbncTime(const unsigned long int &newDbncTime);
    bool setOutputsChange(bool newOutputsChange);
    bool setTaskToNotify(TaskHandle_t newHandle);
    
    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
    bool pause();
    bool resume();
    bool end();    
};

//==========================================================>>

class DbncdDlydMPBttn: public DbncdMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:    
    unsigned long int _strtDelay {0};

    bool updValidPressPend();
public:
    DbncdDlydMPBttn();
    DbncdDlydMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    unsigned long int getStrtDelay();
    bool init(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    boolean setStrtDelay(const unsigned long int &newStrtDelay);

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class LtchMPBttn: public DbncdDlydMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _releasePending{false};
    bool _unlatchPending{false};
    bool unlatch();
    bool updIsOn();
    bool updUnlatchPend();
    bool updValidPressPend();
public:
    LtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    const bool getUnlatchPend() const;
    bool setUnlatchPend();

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class TmLtchMPBttn: public LtchMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _tmRstbl {true};
    unsigned long int _srvcTime {};
    unsigned long int _srvcTimerStrt{0};
    bool updIsOn();
    bool updUnlatchPend();
public:
    TmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &svcTime, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    const unsigned long int getSvcTime() const;
    bool setSvcTime(const unsigned long int &newSvcTime);
    bool setTmerRstbl(const bool &newIsRstbl);

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class HntdTmLtchMPBttn: public TmLtchMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _wrnngOn {false};
    bool _keepPilot{false};
    bool _pilotOn{false};
    unsigned int _wrnngPrctg {0};
    unsigned long int _wrnngMs{0};
    bool updPilotOn();
    bool updWrnngOn();
public:
    HntdTmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &svcTime, const unsigned int &wrnngPrctg = 0, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    const bool getPilotOn() const;
    const bool getWrnngOn() const;
    bool setSvcTime(const unsigned long int &newSvcTime);
    bool setKeepPilot(const bool &newKeepPilot);

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class XtrnUnltchMPBttn: public LtchMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _unltchPulledUp{};
    bool _unltchTypeNO{};
    DbncdDlydMPBttn* _unLtchBttn {nullptr};
    bool updIsOn();
    bool unlatch();
    bool updUnlatchPend();
public:
    XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  DbncdDlydMPBttn* unLtchBttn,
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay);
    XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay);

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class VdblMPBttn: public DbncdDlydMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _isEnabled{true};
    bool _isOnDisabled{false};
    bool _isVoided{false};
    virtual bool updIsVoided() = 0;
public:
    VdblMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0, const bool &isOnDisabled = false);
    virtual ~VdblMPBttn();
    void clrStatus();
    bool enable();
    bool disable();
    const bool getIsEnabled() const;
    const bool getIsOnDisabled() const;
    const bool getIsVoided() const;
    bool setIsEnabled(const bool &newEnabledValue);
    bool setIsOnDisabled(const bool &newIsOnDisabled);
    bool setIsVoided(const bool &newVoidValue);
};

//==========================================================>>

class TmVdblMPBttn: public VdblMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);    
protected:
    unsigned long int _voidTime;
    unsigned long int _voidTmrStrt{0};
    bool updIsOn();
    bool updIsPressed();
    virtual bool updIsVoided();
public:
    TmVdblMPBttn(const uint8_t &mpbttnPin, unsigned long int voidTime, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0, const bool &isOnDisabled = false);
    virtual ~TmVdblMPBttn();
    void clrStatus();
    const unsigned long int getVoidTime() const;
    bool setIsEnabled(const bool &newEnabledValue);
    bool setIsVoided(const bool &newVoidValue);
    bool setVoidTime(const unsigned long int &newVoidTime);

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

#endif
