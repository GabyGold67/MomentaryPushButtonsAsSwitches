#ifndef _MMNTRYPSHBTTN_H_
#define _MMNTRYPSHBTTN_H_

#include "Arduino.h"

class DbncdMPBttn{
protected:
    uint8_t _mpbttnPin{};
    bool _isPressed{false};
    bool _wasPressed{false};
    bool _typeNO{};
    bool _pulledUp{};
    bool _validPress{false};
    unsigned long int _dbncTimeOrigSett{};
    unsigned long int _dbncTimeTempSett{};
    unsigned long int _dbncTimerStrt{};
    const unsigned long int _stdMinDbncTime {20};

public:    
    DbncdMPBttn(uint8_t mpbttnPin, bool pulledUp = true, bool typeNO = true, unsigned long int dbncTime = 0);
    unsigned long int getCurDbncTime();
    bool getIsPressed();
    bool resetDbncTime();
    bool setDbncTime(const unsigned long int &newDbncTime);
    void updIsPressed();
    bool updValidPress();

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

//Addd subclass for toggle bttn
//Add subclass for voidable bttn
//Add subclass for security button with display
#endif
