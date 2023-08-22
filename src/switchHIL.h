#ifndef _SWITCHHIL_H_
#define _SWITCHHIL_H_

#include <Arduino.h>
#include <mpbToSwitch.h>

class StrcsTmrSwitch{
protected:    
    uint8_t _wnngPin {0};
    uint8_t _pltPin;
    TmLtchMPBttn *_lgcMPB;
    bool _keepPilot {};
    bool _pltBlnks{false};
    unsigned long int _blnkRate{250};
public:
    StrcsTmrSwitch(TmLtchMPBttn *lgcMPB, uint8_t wnngPin = 0, bool keepPilot = false);
    bool setKeepPilot(bool keepPilot);
    bool getKeepPilot();

};

#endif