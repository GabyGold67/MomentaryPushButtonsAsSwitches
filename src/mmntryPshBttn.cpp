#include "mmntryPshBttn.h"


DbncdMPBttn::DbncdMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTime)
: _mpbttnPin{mpbttnPin}, _pulledUp{pulledUp}, _typeNO{typeNO}
{
    if(dbncTime < _stdMinDbncTime)
        _dbncTimeOrigSett = _stdMinDbncTime;
    else
        _dbncTimeOrigSett = dbncTime;
    _dbncTimeTempSett = _dbncTimeOrigSett;

    pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT);
}

unsigned long int DbncdMPBttn::getCurDbncTime(){

    return _dbncTimeTempSett;
}

bool DbncdMPBttn::getIsPressed(){
    updIsPressed();
    return _isPressed;
}

bool DbncdMPBttn::resetDbncTime(){

    return setDbncTime(_dbncTimeOrigSett);
}

bool DbncdMPBttn::setDbncTime(const unsigned long int &newDbncTime){
    bool result {false};

    if (newDbncTime >= _stdMinDbncTime){
        _dbncTimeTempSett = newDbncTime;
        result = true;
    }

    return result;
}

void DbncdMPBttn::updIsPressed(){
    /*To be pressed the conditions are:
    1) For NO == true
        a)  _pulledUp == false
            digitalRead == HIGH
        b)  _pulledUp == true
            digitalRead == FALSE
    2) for NO == false
        a)  _pulledUp == false
            digitalRead == LOW
        b)  _pulledUp == true
            digitalRead == HIGH
    */
    bool result {false};
    bool tmpPinLvl {digitalRead(_mpbttnPin)};
    
    if (_typeNO == true){
        if (_pulledUp == false){
            if (tmpPinLvl == HIGH)
                result = true;
        }
        else{
            if (tmpPinLvl == LOW)
                result = true;
        }
    }
    else{
        //For NC MPBs
        if (_pulledUp == false){
            if (tmpPinLvl == LOW)
                result = true;
        }
        else{
            if (tmpPinLvl == HIGH)
                result = true;
        }
    }    
    _isPressed = result;

    return;
}

bool DbncdMPBttn::updValidPress(){
    bool result {false};

    updIsPressed();

    if(_isPressed){
        if(_wasPressed == false){
            //Started to be pressed
            _wasPressed = true;
            _dbncTimerStrt = millis();
        }
        else{
            if ((millis() - _dbncTimerStrt) >= _dbncTimeTempSett){
                result = true;
            }
        }
    }
    else{
        _wasPressed = false;
    }
    _validPress = result;

    return result;
}

AutoRptCntlMPBttn::AutoRptCntlMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTime, unsigned long int rptRate, bool autoRptOn)
:DbncdMPBttn(mpbttnPin, pulledUp, typeNO, dbncTime), _autoRptOn {autoRptOn}
{
    if (rptRate <_minRptRate)
        _rptRate = _minRptRate;
    else
        _rptRate = rptRate;    
}

bool AutoRptCntlMPBttn::setAutoRptRate(unsigned long int newRptRate){
    bool result {false};

    if (newRptRate >= _minRptRate){
        _rptRate = newRptRate;
        result = true;
    }

    return result;
}

bool AutoRptCntlMPBttn::getSrvcPend(){
    return _servicePend;
}

bool AutoRptCntlMPBttn::notifySrvd(){
    bool result {false};

    if (_servicePend){
        _servicePend = false;
        if(_autoRptOn){
            _rearmed = true;
            _dbncTimerStrt = millis();
            _wasPressed = true;
        }
        result = true;
    }

    return result;
}

bool AutoRptCntlMPBttn::updValidPress()
{
    bool result {false};
    
    updIsPressed();

    int pinState {digitalRead(_mpbttnPin)};
    bool nowPushed {_isPressed};    //Button is pushed == true
    if (!_servicePend){
        //There's no Service pending of being treated
        if (nowPushed){
            if (_autoRptOn || (!_autoRptOn && _rearmed) ){
                if(_wasPressed){
                    //It was already being pushed, timer is already running
                    if ((millis() - _dbncTimerStrt) >= _dbncTimeOrigSett){
                        _servicePend = true;
                        _rearmed = false;
                    }
                }
                else{
                    //It wasn't already pushed, timer must be started and status changed
                    _dbncTimerStrt = millis();
                    _wasPressed = true;
                }
            }
        }
        else{
            _wasPressed = false;
            if (!_autoRptOn)
                _rearmed = true;
        }
        result = true;
    }
    else{
        //Could not be treated as a Service is pending of treatment        
    }

    return result;

}
