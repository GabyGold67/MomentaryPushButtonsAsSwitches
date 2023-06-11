#ifndef _MMNTRYPSHBTTN_H_
#define _MMNTRYPSHBTTN_H_

#include "Arduino.h"

class DbncdMPBttn{
protected:
    uint8_t _mpbttnPin{};
    bool _isPressed{};
    bool _wasPressed{false};
    bool _typeNO{};
    bool _pulledUp{};
    unsigned long int _dbncTimeSet{};
    unsigned long int _dbncTimerStrt{};
    const unsigned long int _stdMinDbncTime {20};

public:    
    DbncdMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTime = 0);
    bool getPressed();
    bool updateStatus();
    bool resetDbncTime();
    bool setDbncTime(const unsigned long int &newDbncTime);
    void updIsPressed();

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
    bool updStatus();

};

#endif
