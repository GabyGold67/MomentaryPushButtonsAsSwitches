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

    bool updIsOn();
    bool updIsPressed();
    bool updValidPressPend();

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
public:
    DbncdDlydMPBttn();
    DbncdDlydMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    unsigned long int getStrtDelay();
    bool init(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    boolean setStrtDelay(const unsigned long int &newStrtDelay);

    bool updValidPressPend();

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class LtchMPBttn: public DbncdDlydMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _releasePending{false};
    bool _unlatchPending{false};
    bool unlatch();
public:
    LtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    const bool getUnlatchPend() const;
    bool setUnlatchPend();
    bool updUnlatchPend();

    bool updIsOn();
    bool updValidPressPend();

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class TmLtchMPBttn: public LtchMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _tmRstbl {true};
    unsigned long int _srvcTime {};
    unsigned long int _srvcTimerStrt{0};
public:
    TmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &actTime, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    const unsigned long int getSvcTime() const;
    bool setSvcTime(const unsigned long int &newActTime);
    bool setTmerRstbl(const bool &newIsRstbl);

    bool updIsOn();
    bool updUnlatchPend();

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
public:
    HntdTmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &actTime, const unsigned int &wrnngPrctg = 0, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0);
    const bool getPilotOn() const;
    const bool getWrnngOn() const;
    bool setSvcTime(const unsigned long int &newActTime);
    bool setKeepPilot(const bool &newKeepPilot);
    bool updPilotOn();
    bool updWrnngOn();

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

//==========================================================>>

class XtrnUnltchMPBttn: public LtchMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _unltchPulledUp{};
    bool _unltchTypeNO{};
    DbncdDlydMPBttn* _unLtchBttn {nullptr};
public:
    XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  DbncdDlydMPBttn* unLtchBttn,
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay);
    XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay);

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
    bool updIsOn();
    bool unlatch();
    bool updUnlatchPend();
};

//==========================================================>>

class VdblMPBttn: public DbncdDlydMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);
protected:
    bool _isEnabled{true};
    bool _isOnDisabled{false};
    bool _isVoided{false};
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
    virtual bool updIsVoided() = 0;
};

//==========================================================>>

class TmVdblMPBttn: public VdblMPBttn{
    static void mpbPollCallback(TimerHandle_t mpbTmrCbArg);    
protected:
    unsigned long int _voidTime;
    unsigned long int _voidTmrStrt{0};
public:
    TmVdblMPBttn(const uint8_t &mpbttnPin, unsigned long int voidTime, const bool &pulledUp = true, const bool &typeNO = true, const unsigned long int &dbncTimeOrigSett = 0, const unsigned long int &strtDelay = 0, const bool &isOnDisabled = false);
    virtual ~TmVdblMPBttn();
    void clrStatus();
    const unsigned long int getVoidTime() const;
    bool setIsEnabled(const bool &newEnabledValue);
    bool setIsVoided(const bool &newVoidValue);
    bool setVoidTime(const unsigned long int &newVoidTime);
    bool updIsOn();
    bool updIsPressed();
    virtual bool updIsVoided();

    bool begin(const unsigned long int &pollDelayMs = _StdPollDelay);
};

#endif
